// Salix/gui/imgui/SDLImGui.cpp
#include <Salix/gui/imgui/SDLImGui.h>
#include <imgui/imgui.h>
#include <Salix/window/SDLWindow.h> // Assuming SDLWindow has get_native_handle()
#include <Salix/rendering/SDLRenderer.h> // Assuming SDLRenderer has get_native_handle()
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h> // Or imgui_impl_opengl3.h if using OpenGL
#include <iostream>
#include <SDL.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
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
        IThemeManager* i_theme_manager = nullptr;
        IFontManager* i_font_manager = nullptr;
        IEventPoller* event_poller = nullptr;   // <-- NEW: Store IEventPoller
        EventManager* event_manager = nullptr;   // <-- NEW: Store EventManager
        RawEventCallbackHandle raw_event_callback_handle = 0; // <-- NEW: Store the handle

        // Helper to unregister the callback on shutdown
        void unregister_raw_event_callback_if_registered();
    };

    SDLImGui::SDLImGui() : pimpl(std::make_unique<Pimpl>()) {}

    SDLImGui::~SDLImGui() {
        // Ensure raw event callback is unregistered during shutdown
        pimpl->unregister_raw_event_callback_if_registered();
        // The rest of the shutdown logic is handled in SDLImGui::shutdown()
    }

    bool SDLImGui::initialize(IWindow* window_interface, IRenderer* renderer_interface,
         IThemeManager* theme_manager_interface, IFontManager* font_manager_interface) {
        if (!window_interface || !renderer_interface) { 
            std::cerr << "SDLImGui::initialize(IWindow*, IRenderer*) incorrect arguments types or null detected." << std::endl;
            return false;
        }


        pimpl->i_window = window_interface;
        pimpl->i_renderer = renderer_interface;
        pimpl->i_theme_manager = theme_manager_interface;
        pimpl->i_font_manager = font_manager_interface;


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

        // This explicitly creates the font texture.
        //ImGui_ImplSDLRenderer2_CreateDeviceObjects();


        io.Fonts->AddFontDefault(); // Load ImGui's default font
        io.Fonts->Build();          // Build the font atlas (CPU side)
        ImGui_ImplSDLRenderer2_UpdateTexture(io.Fonts->TexData);
        std::cout << "SDLImGui: ✅ Initialized Dear ImGui backend." << std::endl;
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

    IWindow* SDLImGui::get_window() { return pimpl->i_window; }

    IRenderer* SDLImGui::get_renderer() { return pimpl->i_renderer; }

    IThemeManager* SDLImGui::get_theme_manager() { return pimpl->i_theme_manager; }

    IFontManager* SDLImGui::get_font_manager() { return pimpl->i_font_manager; }





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

    // NEW: Implementation of reinitialize_backend()
    void SDLImGui::reinitialize_backend() {
        if (!ImGui::GetCurrentContext()) {
            std::cerr << "SDLImGui::reinitialize_backend - ImGui context is null. Cannot reinitialize." << std::endl;
            return;
        }
        if (!pimpl->sdl_window || !pimpl->sdl_renderer) {
            std::cerr << "SDLImGui::reinitialize_backend - SDL window or renderer is null. Cannot reinitialize." << std::endl;
            return;
        }

        // --- CRITICAL FIX: Manually manage font texture destruction and creation ---
        // This is necessary because ImGui_ImplSDLRenderer2_CreateDeviceObjects is empty
        // and DestroyDeviceObjects only marks for destruction.

        // 1. Mark font atlas texture for destruction and update it
        // This will call SDL_DestroyTexture
        ImGuiIO& io = ImGui::GetIO();
        if (io.Fonts->TexData != nullptr) {
            io.Fonts->TexData->SetStatus(ImTextureStatus_WantDestroy);
            ImGui_ImplSDLRenderer2_UpdateTexture(io.Fonts->TexData);
        }

        // 2. Re-initialize the backend (this is still needed for other state)
        ImGui_ImplSDLRenderer2_Shutdown(); // Full backend shutdown
        ImGui_ImplSDL2_Shutdown();

        ImGui_ImplSDL2_InitForSDLRenderer(pimpl->sdl_window, pimpl->sdl_renderer);
        ImGui_ImplSDLRenderer2_Init(pimpl->sdl_renderer);

        // 3. Mark font atlas for creation and update it
        // This will call SDL_CreateTexture and SDL_UpdateTexture
        if (io.Fonts->TexData != nullptr) { // Ensure TexData is valid after re-init
            io.Fonts->TexData->SetStatus(ImTextureStatus_WantCreate);
            ImGui_ImplSDLRenderer2_UpdateTexture(io.Fonts->TexData);
        } else {
            std::cerr << "SDLImGui Error: io.Fonts->TexData is null after backend re-initialization. Cannot recreate font texture." << std::endl;
        }
        
        std::cout << "SDLImGui: Backend re-initialized successfully." << std::endl;
    }

} // namespace Salix