// =================================================================================
// Filename:    Salix/core/Engine.cpp
// Author:      SalixGameStudio
// Description: Implements the main Engine class, which orchestrates all subsystems.
// =================================================================================

// C++ includes
#include <iostream>

// Core includes
#include <Salix/core/InitContext.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/core/Engine.h>
#include <Salix/core/SDLTimer.h>
#include <Salix/core/ChronoTimer.h>

// Reflection
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/reflection/ui/TypeDrawerLive.h>


// Rendering includes
#include <Salix/rendering/DummyCamera.h>
#include <Salix/rendering/sdl/SDLRenderer.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
// Input includes
#include <Salix/input/sdl/SDLInputManager.h>
#include <Salix/input/ImGuiInputManager.h>

// Gui includes
#include <imgui/imgui.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>

// Gui/Imgui includes
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>

// Asset management includes
#include <Salix/assets/AssetManager.h>

// Events includes
#include <Salix/events/IEventPoller.h>
#include <Salix/events/sdl/SDLEventPoller.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/ApplicationEventListener.h>
#include <Editor/events/ThemeReloadEvent.h>
// States includes
#include <Salix/states/IAppState.h>
#include <Salix/states/LaunchState.h>
#include <Salix/states/OptionsMenuState.h>

// ecs includes
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Sprite2D.h>

// 3rd-party includes
#include <Windows.h>
#include <SDL.h>
#include <SDL_ttf.h>



namespace Salix {

    struct Engine::Pimpl {
        bool is_running;
        float time_scale = 1.0f;
        float dpi_scale = 1.0f;
        RendererType renderer_type;
        std::unique_ptr<IRenderer> renderer;
        std::unique_ptr<AssetManager> asset_manager;
        std::unique_ptr<IInputManager> game_input_manager;
        std::unique_ptr<IInputManager> gui_input_manager;
        std::unique_ptr<ITimer> timer;
        std::unique_ptr<IEventPoller> event_poller;
        std::unique_ptr<EventManager> event_manager;
        std::unique_ptr<IThemeManager> theme_manager;
        std::unique_ptr<IFontManager> font_manager;
        std::unique_ptr<ApplicationEventListener> app_event_listener;       
        std::unique_ptr<IAppState> current_state;
        std::unique_ptr<ApplicationConfig>  app_config;
        std::unique_ptr<DummyCamera> dummy_camera;
        std::unique_ptr<IIconManager> icon_manager;
        TimerType timer_type;
        EngineMode engine_mode = EngineMode::None;
        HMODULE game_dll_handle = nullptr;
        HMODULE editor_dll_handle = nullptr;
        GuiType gui_type;
        std::unique_ptr<IGui>gui_system;
        InitContext context;

        using CreateStateFn = IAppState* (*)(AppStateType);
        CreateStateFn game_state_factory = nullptr;
        CreateStateFn editor_state_factory = nullptr;
        

        void setup_theme();
        void setup_fonts();
        void setup_icons();
    };

    Engine::Engine() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->is_running = false;
    }

    Engine::~Engine() {
        std::cout << "Engine is shutting down (destructor)." << std::endl; // Debug
        // Unsubscribe ApplicationEventListener before EventManager is reset
        if (pimpl->event_manager && pimpl->app_event_listener) {
            pimpl->event_manager->unsubscribe(EventCategory::Application, pimpl->app_event_listener.get()); 
        }
        // Reset the listener
        pimpl->app_event_listener.reset(); 
        
        // General cleanup order:
        pimpl->current_state.reset(); // States might use input/renderer, so clean them early
        pimpl->game_input_manager.reset();
        pimpl->gui_input_manager.reset();
        
        pimpl->event_poller.reset(); // Poller before manager
        pimpl->event_manager.reset(); // Event manager last of these core systems

        pimpl->gui_system.reset(); // GUI system might depend on renderer
        pimpl->timer.reset();
        pimpl->asset_manager.reset();
        pimpl->renderer.reset();
        pimpl->app_config.reset();

        if (pimpl->game_dll_handle) {
            FreeLibrary(pimpl->game_dll_handle);
            pimpl->game_dll_handle = nullptr;
        }
        if (pimpl->editor_dll_handle) {
            FreeLibrary(pimpl->editor_dll_handle);
            pimpl->editor_dll_handle = nullptr;
        }
        SDL_Quit(); // SDL shutdown last
    }

    bool Engine::initialize(const ApplicationConfig& config) {

        pimpl->app_config = std::make_unique<ApplicationConfig>(config);
        // Force High-DPI support to be enabled. This must be called BEFORE SDL_Init().
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0"); 

        // --- INITIALIZE SDL---
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
            std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        




        // --- INITIALIZE SDL_TTF ---
        if (TTF_Init() == -1) {
            std::cerr << "Engine::initialize - SDL_ttf could not be initialized! TTF_Error: " << TTF_GetError() << std::endl;
            SDL_Quit(); // Ensure SDL is quit if TTF fails
            return false;
        }
        std::cout << "SDL_ttf initialized successfully." << std::endl;



        // --- INITIALIZE RENDERER
        pimpl->renderer_type = config.renderer_type;
        switch (config.renderer_type) {
            case RendererType::SDL:
                pimpl->renderer = std::make_unique<SDLRenderer>();
                break;

            case RendererType::OpenGL:
                pimpl->renderer = std::make_unique<OpenGLRenderer>();
                // Give the Renderer a dummy camera so it can initialize properly
                pimpl->dummy_camera = std::make_unique<DummyCamera>();
            
                break;

            default:
                std::cerr << "Engine::initialize - Invalid or unsupported renderer type requested!" << std::endl;
                SDL_Quit();
                return false;
        }

        if (!pimpl->renderer->initialize(config.window_config)) {
            std::cerr << "Engine::initialize - Renderer subsystem failed to initialize." << std::endl;
            return false;
        }

        // --- Get and Set DPI Scale ---
            pimpl->dpi_scale = pimpl->renderer->get_window()->get_dpi_scale();
            std::cout << "Engine::initialize - Calculated DPI Scale: " << pimpl->dpi_scale << std::endl;
            pimpl->app_config->gui_settings.global_dpi_scaling = pimpl->dpi_scale;
        if (config.renderer_type == RendererType::OpenGL) {
            OpenGLRenderer* opengl_renderer = dynamic_cast<OpenGLRenderer*>(pimpl->renderer.get());
            opengl_renderer->set_active_camera(pimpl->dummy_camera.get());
        }



        // --- INITIALIZE ASSET MANAGER ---
        pimpl->asset_manager = std::make_unique<AssetManager>();
        pimpl->asset_manager->initialize(pimpl->renderer.get());




        // --- INITIALIZE TIMER ---
        switch (config.timer_type) {
            case TimerType::SDL:
                pimpl->timer = std::make_unique<SDLTimer>();
                break;
            case TimerType::Chrono:
                pimpl->timer = std::make_unique<ChronoTimer>();
                break;
            default:
                std::cerr << "Engine::initialize - Invalid or unsupported timer type requested." << std::endl;
                return false;
        }
        pimpl->timer_type = config.timer_type;
        pimpl->timer->set_target_fps(config.target_fps);



        // --- INITIALIZE  GAME INPUT MANAGER ---
        pimpl->game_input_manager = std::make_unique<SDLInputManager>();


        // --- INITIALIZE  GUI INPUT MANAGER ---
        pimpl->gui_input_manager = std::make_unique<ImGuiInputManager>();


        // --- INITIALIZE EVENT MANAGER ---
        pimpl->event_manager = std::make_unique<EventManager>();


        // --- INITIALIZE EVENT POLLER ---
        pimpl->event_poller = std::make_unique<SDLEventPoller>();



        // -- INITIALIZE APPLICATION EVENT LISTENER ---
        pimpl->app_event_listener = std::make_unique<ApplicationEventListener>();
        if (!pimpl->app_event_listener->initialize(this)) {
            std::cerr << "Engine Error: Application Event Listener failed to initialize." << std::endl;
            return false;
        }

        // ---  SUBSCRIBE THE LISTENER TO THE EVENT MANAGER ---
        pimpl->event_manager->subscribe(EventCategory::Application, pimpl->app_event_listener.get()); 
        



        // --- SETUP DLL HANDLES ---
        

        // --- GAME DLL HANDLE 'game_dll_handle' 'Game.dll' ---
        std::cout << "Engine: Loading Game.dll..." << std::endl;
        pimpl->game_dll_handle = LoadLibraryA("Game.dll");
        if (!pimpl->game_dll_handle) {
            std::cerr << "FATAL ERROR: Could not load Game.dll!" << std::endl;
            return false;
        }

        pimpl->game_state_factory = (Pimpl::CreateStateFn)GetProcAddress(pimpl->game_dll_handle, "create_game_state");
        if (!pimpl->game_state_factory) {
            std::cerr << "FATAL ERROR: Could not find 'create_game_state' function in Game.dll!" << std::endl;
            return false;
        }



        


        // --- ASSIGN GUI TYPE ---
        pimpl->gui_type = config.gui_type;
        switch (pimpl->gui_type) {
            case GuiType::None:
                std::cout << "[ENGINE] GUI system not initialized (GuiType is None)." << std::endl;
                pimpl->gui_system = nullptr;
                break; // Safe to break here, as no GUI system is expected.
            case GuiType::ImGui: {

                if (config.renderer_type == RendererType::SDL) {
                    pimpl->gui_system = std::make_unique<SDLImGui>();
                    

                } else if (config.renderer_type == RendererType::OpenGL) {
                    pimpl->gui_system = std::make_unique<OpenGLImGui>();
                    
                }

                // --- SETUP ALL IMGUI REQUIRED CLASSES ---
                pimpl->theme_manager = std::make_unique<ImGuiThemeManager>();  // This must be intialized first.

                pimpl->font_manager = std::make_unique<ImGuiFontManager>();
                
                pimpl->icon_manager = std::make_unique<ImGuiIconManager>();
                
                pimpl->gui_system->set_app_config(pimpl->app_config.get());
                
                if (!pimpl->gui_system->initialize(
                        pimpl->renderer->get_window(),
                        pimpl->renderer.get(),
                        pimpl->theme_manager.get(),
                        pimpl->font_manager.get(),
                        pimpl->icon_manager.get()
                    )
                    ) {
                    std::cerr << "Engine Error: GUI system initialization failed!" << std::endl;
                    // IMPORTANT: If GUI init fails, the whole engine initialization fails.
                    // We MUST return false from Engine::initialize here.
                    return false;
                } else {
                    std::cout << "[ENGINE] GUI system initialized." << std::endl;
                }

                
                // 1. Initialize the Theme Manager
                pimpl->setup_theme();
                // 2. Load the default theme's data from the file, but DO NOT apply it yet.
                const std::string default_theme_name = "Default Dark Theme";
                const std::string default_theme_file_path = "Assets/Themes/default_theme.yaml";
                if (!pimpl->theme_manager->load_theme_from_file(default_theme_name, default_theme_file_path, false)) { // <-- Set apply_immediately to false
                    std::cerr << "Engine Error: Failed to load default theme. Using built-in defaults." << std::endl;
                    // Fallback logic to register a theme if the file is missing...
                    std::string fallback_theme_name = "FALLBACK THEME";
                    std::string fallback_theme_path = "FALLBACK THEME - NO PATH DEFINED";
                    if (!pimpl->theme_manager->get_theme(default_theme_name)) {
                        auto fallback_theme = std::make_unique<Salix::ImGuiTheme>(fallback_theme_name, fallback_theme_path);
                        pimpl->theme_manager->register_theme(std::move(fallback_theme));
                        break;
                    }
                    break;
                }

                // 3. Set the newly loaded theme as the active one internally.
                pimpl->theme_manager->set_active_theme(default_theme_name);

                // 4. Now, set up all the fonts. This will pre-load the batches.
                pimpl->setup_fonts();

                // 5. Set up all icons for preparation of entering the EditorState.
                pimpl->setup_icons();

                // 6. Finally, apply the active theme. This will now correctly find and
                //    apply the font specified in the theme because all fonts have been pre-loaded.
                pimpl->theme_manager->apply_theme(default_theme_name);


                
                
                
                
                if ( config.renderer_type == RendererType::SDL) {
                    if (auto* sdl_imgui = dynamic_cast<SDLImGui*>(pimpl->gui_system.get())) {
                        sdl_imgui->setup_event_polling(
                            pimpl->event_poller.get(),
                            pimpl->event_manager.get()
                        );

                        std::cout << "[ENGINE] SDLImGui event polling setup complete." << std::endl;
                    } else {

                        std::cerr << "Engine Error: Failed to cast gui_system to SDLImGui* for event polling setup." << std::endl;
                        // This is a critical error if SDLImGui was just created and casting fails.
                        return false;
                    }
                } else if (config.renderer_type == RendererType::OpenGL) {
                    if (auto* opengl_imgui = dynamic_cast<OpenGLImGui*>(pimpl->gui_system.get())) {
                        opengl_imgui->setup_event_polling(
                            pimpl->event_poller.get(),
                            pimpl->event_manager.get()
                        );

                        std::cout << "[ENGINE] OpenGLImGui event polling setup complete." << std::endl;
                    } else {

                        std::cerr << "Engine Error: Failed to cast gui_system to OpenGLImGui* for event polling setup." << std::endl;
                        // This is a critical error if SDLImGui was just created and casting fails.
                        return false;
                    }
                } else {

                    std::cerr << "Engine Error: Failed to cast the appropriate gui_system to the for event polling setup." << std::endl;
                    // This is a critical error if X_ImGui was just created and casting fails.
                    return false;
                }

                break; 
            }
            default: // This case handles unsupported GUI types
                std::cerr << "Engine Error: Unsupported GuiType requested!" << std::endl;
                return false; 
        }



        // ---  INITIALIZE THE EDITOR FACTORY AND SET THE CONTEXT TO THE INITIALIIZE GUI SYSTEM.


        // --- EDITOR DLL HANDLE 'editor_dll_handle' 'SalixEditor.dll' ---
        std::cout << "Engine: Loading SalixEditor.dll..." << std::endl;
        pimpl->editor_dll_handle = LoadLibraryA("SalixEditor.dll");
        if (!pimpl->editor_dll_handle) {
            std::cerr << "FATAL ERROR: Could not load SalixEditor.dll!" << std::endl;
            return false;
        }
        std::cout << "[ENGINE] SalixEditor.dll loaded!" << std::endl;
        pimpl->editor_state_factory = (Pimpl::CreateStateFn)GetProcAddress(pimpl->editor_dll_handle, "create_editor_state");
        if (!pimpl->editor_state_factory) {
            std::cerr << "FATAL ERROR: Could not find 'create_editor_state' in SalixEditor.dll!" << std::endl;
            return false;
        }
        std::cout << "[ENGINE] Attempting to resolve create_editor_state..." << std::endl;


        // --- Synchronize ImGui Context with the Editor DLL ---

        // 1. Define the function pointer type for our new function.
        typedef void(*SetImGuiContextFn)(ImGuiContext*);

        // 2. Get the function's address from the loaded DLL.
        SetImGuiContextFn set_context_func = (SetImGuiContextFn)GetProcAddress(pimpl->editor_dll_handle, "set_imgui_context");

        // 3. Check if we found it and then call it.
        if (set_context_func) {

            // This is the crucial call: Pass the engine's current ImGui context
            // to the function inside the DLL.
            set_context_func(ImGui::GetCurrentContext());
            std::cout << "[ENGINE] ImGui context synchronized with SalixEditor.dll." << std::endl;
        } else {

            // If we can't find the function, it's a fatal error.
            std::cerr << "FATAL ERROR: Could not find 'set_imgui_context' function in SalixEditor.dll!" << std::endl;
            return false;
        }


        

        // --- INITIALIZE ENUMS ---
        EnumRegistry::register_all_enums();
        
        // --- INITIALIZE BYTE MIRROR ---
        ByteMirror::register_all_types();
        
        // --- INITIALIZE TYPE DRAWER ---
        TypeDrawerLive::register_all_type_drawers();
        

        // --- SWITCH INTO THE INITIAL STATE PASSED INTO THIS METHOD ---
        switch_state(config.initial_state);
        if (!pimpl->current_state) { // Check if switch_state failed to create the initial state
            std::cerr << "Engine::initialize - Failed to set initial state to " << static_cast<int>(config.initial_state) << ". Engine cannot start." << std::endl;
            return false;
        }


        // --- START THE ENGINE ---
        pimpl->is_running = true;
        std::cout << "Engine initialized successfully." << std::endl;
        return true;
    }

    void Engine::run() {
        while (pimpl->is_running) {
            pimpl->timer->tick_start();
            float delta_time = pimpl->timer->get_delta_time();

            if (pimpl->gui_system && pimpl->gui_system->is_theme_reload_requested()) {
                // Perform the reload safely BEFORE the new frame begins.
                ITheme* active_theme = pimpl->theme_manager->get_active_theme();
                if (active_theme) {
                    pimpl->theme_manager->apply_theme(active_theme->get_name());
                }
                // Clear the flag so it only runs once.
                pimpl->gui_system->clear_theme_reload_request();
            }

            if (pimpl->gui_system) {
            pimpl->gui_system->new_frame();
            
            }
            
           
            
            // Apply time_scale to delta_time.
            float scaled_delta_time = delta_time * pimpl->time_scale;
            process_input();
            update(scaled_delta_time);
            render();

            pimpl->timer->tick_end();
        }
    }

    void Engine::shutdown() {
        std::cout << "Shutting down engine." << std::endl;
        
        // Shutdown GUI System
        if (pimpl->gui_system) {
            pimpl->gui_system->shutdown();
            pimpl->gui_system.reset();
        }

        // Shutdown ThemeManager
        if (pimpl->theme_manager) {
            pimpl->theme_manager->shutdown();
            pimpl->theme_manager.reset();
        }

        // Shutdown FontManager
        if (pimpl->font_manager) {
            pimpl->font_manager->shutdown();
            pimpl->font_manager.reset();
        }

        pimpl->icon_manager.reset();

        pimpl->app_event_listener.reset();

        pimpl->event_poller.reset();

        pimpl->event_manager.reset();

        pimpl->current_state.reset();
        
        pimpl->game_input_manager.reset();

        pimpl->gui_input_manager.reset();

        pimpl->timer.reset();

        pimpl->asset_manager.reset();

        if (pimpl->editor_dll_handle) {
            FreeLibrary(pimpl->editor_dll_handle);
            
            pimpl->editor_dll_handle = nullptr; // Clear handle after freeing
        }

        pimpl->renderer.reset();
        if (pimpl->game_dll_handle) {
            FreeLibrary(pimpl->game_dll_handle);
            pimpl->game_dll_handle = nullptr; // Clear handle after freeing
        }

        
        TTF_Quit();
        SDL_Quit();
    }

    

    void Engine::process_input() {
        pimpl->event_poller->poll_events([&](IEvent& event) {
            // Events are dispatched to the EventManager for any system that subscribes.
            // This will now send WindowCloseEvent to ApplicationEventListener::on_event
            pimpl->event_manager->dispatch(event); 

            // The currently active input manager processes the event.
            if (get_input_manager()) {
                get_input_manager()->process_event(event);
            }
        });

       
    }

    void Engine::switch_state(AppStateType new_state_type) {
        if (pimpl->current_state) {
            pimpl->current_state->on_exit();
        }

        std::unique_ptr<IAppState> new_state;

        

        switch (new_state_type) {
            case AppStateType::Launch:
                set_mode(EngineMode::Launch);
                pimpl->context = make_context();
                new_state = std::make_unique<LaunchState>();
                break;

            case AppStateType::Editor:
                set_mode(EngineMode::Editor);
                pimpl->context = make_context();
                
                if (pimpl->editor_state_factory) {
                    new_state.reset(pimpl->editor_state_factory(new_state_type));
                } else {
                    std::cerr << "Editor state factory is not available!" << std::endl;
                }
                break;

            case AppStateType::Options:
                set_mode(EngineMode::Options);
                pimpl->context = make_context();
                new_state = std::make_unique<OptionsMenuState>();
                break;

            default:
                if (pimpl->game_state_factory) {
                    set_mode(EngineMode::Game);
                    pimpl->context = make_context();
                    new_state.reset(pimpl->game_state_factory(new_state_type));
                }
                break;
        }

        // pimpl->context = make_context(); 


        pimpl->current_state = std::move(new_state);

        if (pimpl->current_state) {
            pimpl->current_state->on_enter(pimpl->context);
        } else {
            std::cerr << "Engine::switch_state - Failed to create new state!" << std::endl;
        }

    }

    void Engine::update(float delta_time) {
        if (pimpl->is_running) {
        if (pimpl->current_state) {
            pimpl->current_state->update(delta_time);
        }
        // Engine Mode Dependant update:
        if (get_input_manager()) {
            get_input_manager()->update(delta_time);
        }
    }

    }

    void Engine::render() {
        if (!pimpl->renderer) return;


            pimpl->renderer->begin_frame();
            

            if (pimpl->current_state) {
                pimpl->current_state->render(pimpl->renderer.get());
            }

            

            // Render GUI if active
            if (pimpl->gui_system) {
                // 1. Prepare ImGui's draw data
                pimpl->gui_system->render(); 
            }
            
            // 2. Swap the main window's buffer to show the result
            pimpl->renderer->end_frame();

            
        
            // 3. NOW, update and render any extra ImGui windows
            if (pimpl->gui_system) {
                pimpl->gui_system->update_and_render_platform_windows();
            }
        
    }

    IRenderer* Engine::get_renderer() { return pimpl->renderer.get(); }
    AssetManager* Engine::get_asset_manager() { return pimpl->asset_manager.get(); }
    IInputManager* Engine::get_input_manager() {
       if (pimpl->context.engine_mode == EngineMode::Game) {
        return pimpl->game_input_manager.get(); 
       } else {
        return pimpl->gui_input_manager.get();
       }
    }
    EventManager* Engine::get_event_manager() { return pimpl->event_manager.get(); }
    bool Engine::is_running() const { return pimpl->is_running; }
    const bool Engine::is_running(bool keep_running) {
        pimpl->is_running = keep_running;
        return pimpl->is_running;
    }
    void Engine::push_state(IAppState* /*state*/) {}
    void Engine::pop_state() {}
    void Engine::change_state(IAppState* /*state*/) {}
    EngineMode Engine::get_mode() const { return pimpl->engine_mode; }
    void Engine::set_mode(EngineMode mode) { pimpl->engine_mode = mode; }
    float Engine::get_time_scale() const { return pimpl->time_scale; }
    void Engine::set_time_scale(float new_time_scale) { pimpl->time_scale = new_time_scale; }
    // NEW: Provide a fully populated InitContext on demand
        InitContext Engine::make_context() const {
        InitContext ctx;
        ctx.dpi_scale       = pimpl->dpi_scale;
        ctx.app_config      = pimpl->app_config.get();
        ctx.engine          = const_cast<Engine*>(this); // <-- Add this safely
        ctx.renderer_type   = pimpl->renderer_type;
        ctx.timer           = pimpl->timer.get();
        ctx.timer_type      = pimpl->timer_type;
        ctx.asset_manager   = pimpl->asset_manager.get();
        ctx.renderer        = pimpl->renderer.get();
        ctx.window          = ctx.renderer->get_window();
        ctx.engine_mode     = pimpl->engine_mode;
        ctx.gui_type        = pimpl->gui_type;
        ctx.gui = pimpl->gui_system.get();
        ctx.event_manager = pimpl->event_manager.get();

        if (ctx.engine_mode == EngineMode::Game) {
            std::cout << "Engine::make_context - InputManager = 'game_input_manager'." << std::endl;
            ctx.input_manager = pimpl->game_input_manager.get();
        } else {
            std::cout << "Engine::make_context - InputManager = 'gui_input_manager'." << std::endl;
            ctx.input_manager = pimpl->gui_input_manager.get();
            // Only assign the pimpl->theme_manager if we're in a GUI context.
            ctx.theme_manager = pimpl->theme_manager.get();
            ctx.font_manager = pimpl->font_manager.get();
            ctx.icon_manager = pimpl->icon_manager.get();
        }

       
        return ctx;
    }

     void Engine::Pimpl::setup_theme()  {
        // Initialize the Theme Manager (AFTER GUI System is initialized)
        // The Theme Manager needs the IGui* to apply themes.
        if (!theme_manager->initialize(gui_system.get())) {
            std::cerr << "Engine Error: Theme Manager failed to initialize with GUI system!" << std::endl;
            return;
        }         
        std::cout << "[ENGINE] Theme Manager initialized." << std::endl;
                
     }


     void Engine::Pimpl::setup_fonts()  {

        if (!font_manager->initialize(gui_system.get())) {
                std::cerr << "Engine Error: Font Manager failed to initialize with GUI system!" << std::endl;
                return;
            }
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        font_manager->setup_default_fonts();
        io.Fonts->Build();
        font_manager->rebuild_font_atlas_texture();
        font_manager->set_active_font("Roboto-Regular_20px");
        std::cout << "[ENGINE] Font Manager initialized." << std::endl;
    }

    void Engine::Pimpl::setup_icons() {
        if (!icon_manager) {
        std::cerr << "Engine Error: Icon Manager is null, cannot set up icons." << std::endl;
        return;
        }
        icon_manager->initialize(asset_manager.get());
        icon_manager->register_default_icons();

        std::cout << "[ENGINE] Icon Manager initialized and default icons registered." << std::endl;
    }

    

} // namespace Salix
