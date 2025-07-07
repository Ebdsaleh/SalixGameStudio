// Salix/gui/imgui/SDLImGui.cpp
#include <Salix/gui/imgui/SDLImGui.h>
#include <imgui/imgui.h>
#include <Salix/window/SDLWindow.h> // Assuming SDLWindow has get_native_handle()
#include <Salix/rendering/SDLRenderer.h> // Assuming SDLRenderer has get_native_handle()
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h> // Or imgui_impl_opengl3.h if using OpenGL
#include <iostream>
#include <SDL.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/events/IEventPoller.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/ImGuiInputEvent.h> // To dispatch the ImGuiInputEvent

namespace Salix {

    struct SDLImGui::Pimpl {
        IWindow* i_window = nullptr; // Not strictly needed to store, but fine if you use it
        IRenderer* i_renderer = nullptr; // Not strictly needed to store, but fine if you use it
        SDL_Window* sdl_window = nullptr; // Actual native SDL_Window pointer
        SDL_Renderer* sdl_renderer = nullptr; // Actual native SDL_Renderer pointer
        ImGuiThemeManager* theme_manager = nullptr;
        IEventPoller* event_poller = nullptr;   // <-- NEW: Store IEventPoller
        EventManager* event_manager = nullptr;   // <-- NEW: Store EventManager
        RawEventCallbackHandle raw_event_callback_handle = 0; // <-- NEW: Store the handle

        bool initialize_with_SDL(SDL_Window* window, SDL_Renderer* renderer);
        
        // Helper to unregister the callback on shutdown
        void unregister_raw_event_callback_if_registered();
    };

    SDLImGui::SDLImGui() : pimpl(std::make_unique<Pimpl>()) {}

    SDLImGui::~SDLImGui() {
        // Ensure raw event callback is unregistered during shutdown
        pimpl->unregister_raw_event_callback_if_registered();
        // The rest of the shutdown logic is handled in SDLImGui::shutdown()
    }

    bool SDLImGui::initialize(IWindow* window_interface, IRenderer* renderer_interface, IThemeManager* theme_manager_interface) {
        if (!window_interface || !renderer_interface) { 
            std::cerr << "SDLImGui::initialize(IWindow*, IRenderer*) incorrect arguments types or null detected." << std::endl;
            return false;
        }
        pimpl->i_window = window_interface;
        pimpl->i_renderer = renderer_interface;
        // try to cast the internal IThemeManager* to the correct type of ImGuiThemeManager
        if (theme_manager_interface) {

        pimpl->theme_manager = dynamic_cast<ImGuiThemeManager*>(theme_manager_interface);
        } else {
            pimpl->theme_manager = nullptr;
        }
        // if cast was unsucessful, report back as false.
        if (!pimpl->theme_manager) {
            std::cerr << "SDLImGui::initialize - Failed when trying to cast theme_manager_interface to <ImGuiThemeManager*>" <<
                std::endl;
                return false;
        }
        // If successful, we continue as normal.
        
        // Cast IWindow* and IRenderer* to concrete SDL types to get raw pointers
        // Assuming your SDLWindow and SDLRenderer implement get_native_handle()
        pimpl->sdl_window = static_cast<SDL_Window*>(window_interface->get_native_handle());
        pimpl->sdl_renderer = static_cast<SDL_Renderer*>(renderer_interface->get_native_handle());
        
        if (!pimpl->sdl_window || !pimpl->sdl_renderer) {
            std::cerr << "SDLImGui Error: Raw SDL_Window or SDL_Renderer pointer is null after casting from interfaces." << std::endl;
            return false;
        }

        // --- IMGUI INITIALIZATION ---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark(); // Default dark style
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Initialize Platform/Renderer backends
        // Note: For SDLRenderer2, you need ImGui_ImplSDLRenderer2_Init, not ImGui_ImplOpenGL3_Init.
        // Your current code already uses SDLRenderer2 backend which is correct.
        ImGui_ImplSDL2_InitForSDLRenderer(pimpl->sdl_window, pimpl->sdl_renderer);
        ImGui_ImplSDLRenderer2_Init(pimpl->sdl_renderer);

        std::cout << "SDLImGui: ✅ Initialized Dear ImGui backend." << std::endl;
        return true;
    }
    
    // This helper method is part of Pimpl implementation
    bool SDLImGui::Pimpl::initialize_with_SDL(SDL_Window* sdl_window_ptr, SDL_Renderer* sdl_renderer_ptr) {
        // This method is now effectively merged into SDLImGui::initialize directly above.
        // You can remove this Pimpl method declaration and definition entirely,
        // as its logic is now contained within the main initialize.
        // Keeping it for now to avoid breaking your current structure, but it's redundant.
        if (!sdl_renderer_ptr || !sdl_window_ptr) {
            std::cerr << "SDLImGui Error: Window or Renderer is null during Pimpl::initialize_with_SDL." << std::endl;
            return false;
        }
        sdl_window = sdl_window_ptr; // Store the raw pointers in Pimpl
        sdl_renderer = sdl_renderer_ptr; // Store the raw pointers in Pimpl

        // IMGUI INITIALIZATION (already done in SDLImGui::initialize)
        // This section should be removed from here if merging.
        // It's duplicated logic if initialize_with_SDL is still called.
        // The ImGui_ImplSDL2_InitForSDLRenderer and ImGui_ImplSDLRenderer2_Init calls
        // should only happen once in SDLImGui::initialize().
        
        std::cout << "SDLImGui: ✅ Pimpl::initialize_with_SDL called." << std::endl; // Debug print
        return true;
    }

    // New setup_event_polling method implementation
    void SDLImGui::setup_event_polling(IEventPoller* event_poller_ptr, EventManager* event_manager_ptr) {
        if (!event_poller_ptr || !event_manager_ptr) {
            std::cerr << "SDLImGui Error: Null event poller or event manager passed to setup_event_polling." << std::endl;
            return;
        }
        pimpl->event_poller = event_poller_ptr;
        pimpl->event_manager = event_manager_ptr;

        // Register the raw SDL event callback with the event poller
        // This lambda will be called by SDLEventPoller::poll_events
        pimpl->raw_event_callback_handle = pimpl->event_poller->register_raw_event_callback(
            [&](void* native_event) {
                // Cast the void* back to SDL_Event* and pass to ImGui's backend
                ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(native_event));
            }
        );
        std::cout << "SDLImGui: Registered raw SDL event callback with handle " << pimpl->raw_event_callback_handle << std::endl;
    }

    // Helper method to unregister callback
    void SDLImGui::Pimpl::unregister_raw_event_callback_if_registered() {
        if (event_poller && raw_event_callback_handle != 0) {
            event_poller->unregister_raw_event_callback(raw_event_callback_handle);
            raw_event_callback_handle = 0; // Reset handle after unregistration
        }
    }


    void SDLImGui::shutdown() {
        // Unregister raw event callback first
        pimpl->unregister_raw_event_callback_if_registered();

        // --- IMGUI SHUTDOWN ---
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        std::cout << "SDLImGui: ✅ Dear ImGui backend shut down." << std::endl;
    }

    void SDLImGui::new_frame() {
        // Dispatch ImGuiInputEvent BEFORE ImGui::NewFrame() if you want input managers
        // to react to "captured state" in the *current* frame.
        // ImGuiIO& io = ImGui::GetIO(); is already setup in Pimpl::initialize_with_SDL
        ImGuiIO& io = ImGui::GetIO(); // Get current IO state

        // Create and dispatch the ImGuiInputEvent
        ImGuiInputEvent event(io.WantCaptureMouse, io.WantCaptureKeyboard);
        if (pimpl->event_manager) { // Ensure event manager is valid
            pimpl->event_manager->dispatch(event);
        } else {
            std::cerr << "SDLImGui Warning: EventManager is null when dispatching ImGuiInputEvent." << std::endl;
        }

        // --- IMGUI NEW FRAME ---
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void SDLImGui::render() {
        // ImGui::Render() prepares the draw data
        ImGui::Render();
        // Render ImGui's draw data using the backend
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), pimpl->sdl_renderer);
    }

    void SDLImGui::update_and_render_platform_windows() {
        // Update and Render additional Platform Windows (for multi-viewport/docking)
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void SDLImGui::set_mouse_cursor_visible(bool visible) {
        ImGuiIO& io = ImGui::GetIO();
        // This flag tells ImGui whether *it* should manage the cursor.
        // If false, it tries to hide/show its internal cursor.
        // If true, it lets the OS/application manage it.
        // For actual OS cursor visibility, you might still need SDL_ShowCursor from your SDLWindow.
        io.ConfigFlags = visible ? (io.ConfigFlags & ~ImGuiConfigFlags_NoMouseCursorChange) : (io.ConfigFlags | ImGuiConfigFlags_NoMouseCursorChange);
        
        // You might also want to call SDL_ShowCursor from your SDLWindow if available
        // pimpl->i_window->set_cursor_visible(visible); // if IWindow has such a method
    }

    void SDLImGui::save_layout(const std::string& file_path) {
        ImGui::SaveIniSettingsToDisk(file_path.c_str());
        std::cout << "SDLImGui: Layout saved to " << file_path << std::endl;
    }

    void SDLImGui::load_layout(const std::string& file_path) {
        ImGui::LoadIniSettingsFromDisk(file_path.c_str());
        std::cout << "SDLImGui: Layout loaded from " << file_path << std::endl;
    }

} // namespace Salix