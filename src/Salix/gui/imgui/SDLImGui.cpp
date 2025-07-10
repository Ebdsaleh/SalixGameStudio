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
#include <ImGuiFileDialog.h>

namespace Salix {

    struct SDLImGui::Pimpl {
        IWindow* i_window = nullptr; // Not strictly needed to store, but fine if you use it
        IRenderer* i_renderer = nullptr; // Not strictly needed to store, but fine if you use it
        SDL_Window* sdl_window = nullptr; // Actual native SDL_Window pointer
        SDL_Renderer* sdl_renderer = nullptr; // Actual native SDL_Renderer pointer
        IThemeManager* i_theme_manager = nullptr;
        IFontManager* i_font_manager = nullptr;
        IEventPoller* event_poller = nullptr;   // <-- Store IEventPoller
        EventManager* event_manager = nullptr;   // <-- Store EventManager
        RawEventCallbackHandle raw_event_callback_handle = 0; // <-- Store the handle

        std::unordered_map<std::string, bool> active_dialog_keys; // NEW: Tracks which dialogs are currently open

        // Store dialog results for the current frame
        mutable std::unordered_map<std::string, FileDialogResult> dialog_results_this_frame;

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

                process_raw_input_event(native_event);
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



    // --- Implement Abstract File Dialog Control ---
    bool SDLImGui::open_save_file_dialog(
        const std::string& key, const std::string& title, const std::string& filters, 
        const std::string& default_path, const std::string& default_filename) {
        set_common_dialog_properties(); 

        IGFD::FileDialogConfig config;
        config.path = default_path;
        config.fileName = default_filename;
        config.countSelectionMax = 1;
        config.userDatas = nullptr;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog(key, title, filters.c_str(), config);
        // Set internal flag based on the key
        pimpl->active_dialog_keys[key] = true; // Mark dialog as active
        return true;
    }



    bool SDLImGui::open_load_file_dialog(
        const std::string& key, const std::string& title, const std::string& filters,
        const std::string& default_path) {
            
        set_common_dialog_properties();
        IGFD::FileDialogConfig config;
        config.path = default_path;
        config.fileName = "";
        config.countSelectionMax = 1;
        config.userDatas = nullptr;
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog(key, title, filters.c_str(), config);
        pimpl->active_dialog_keys[key] = true; // Mark dialog as active
        return true;
    }


    bool SDLImGui::open_folder_dialog(
        const std::string& key, const std::string& title, const std::string& default_path) {
        
        set_common_dialog_properties();
        IGFD::FileDialogConfig config;
        config.path = default_path;
        config.flags = ImGuiFileDialogFlags_Modal;
          
        ImGuiFileDialog::Instance()->OpenDialog(key, title, "", config); // No filters for folder dialog
        pimpl->active_dialog_keys[key] = true; // Mark dialog as active
        return true;
    }


    bool SDLImGui::is_dialog_open(const std::string& key) const {
        return pimpl->active_dialog_keys.count(key) > 0; // Check if key exists in active map    
    }

    
    bool SDLImGui::is_dialog_closed_this_frame(const std::string& key) const {
        // This relies on the `display_dialogs()` method setting results for the current frame.
        return pimpl->dialog_results_this_frame.count(key) > 0; // Check if result was stored this frame    
    }


    void SDLImGui::display_dialogs() {
        // Clear results from previous frame
        pimpl->dialog_results_this_frame.clear();

        // Iterate through all currently active dialogs and display them
        // Create a copy of keys to avoid modifying map during iteration
        std::vector<std::string> keys_to_display;
        for (const auto& pair : pimpl->active_dialog_keys) {
            keys_to_display.push_back(pair.first);
        }

        for (const std::string& key : keys_to_display) {
            // Check if the dialog is still open by ImGuiFileDialog itself
            // ImGuiFileDialog::Instance()->IsOpened() returns true if *any* dialog is open.
            // We need to pass the specific key to Display().
            if (ImGuiFileDialog::Instance()->Display(key)) { // Display returns true when dialog is closed
                // Dialog was just closed this frame (either OK or Cancel)
                pimpl->dialog_results_this_frame[key] = populate_dialog_result(key); // Store result
                ImGuiFileDialog::Instance()->Close(); // Explicitly close (though Display() might have already)
                pimpl->active_dialog_keys.erase(key); // Remove from active list
            }
        }
    }


    FileDialogResult SDLImGui::get_dialog_result(const std::string& key) const {
        auto it = pimpl->dialog_results_this_frame.find(key);
        if (it != pimpl->dialog_results_this_frame.end()) {
            return it->second;
        }
        return FileDialogResult(); // Return empty/default result if key not found
    }


   bool SDLImGui::process_raw_input_event(void* native_event) {
        if (!native_event) {
            return false;
        }

        SDL_Event* sdl_event = static_cast<SDL_Event*>(native_event);
        ImGui_ImplSDL2_ProcessEvent(sdl_event); // Let ImGui (and ImGuiFileDialog) process the event first

        bool event_consumed_by_gui = false;

        if (sdl_event->type == SDL_KEYDOWN) {
            if (sdl_event->key.keysym.sym == SDLK_ESCAPE) {
                if (ImGuiFileDialog::Instance()->IsOpened()) {
                    ImGuiFileDialog::Instance()->Close(); // Explicitly close dialog
                    event_consumed_by_gui = true;
                    std::cout << "Escape key: Dialog closed by GUI system." << std::endl;
                }
            }
            // No explicit handling for SDLK_RETURN (Enter) needed here,
            // as ImGuiFileDialog typically handles it internally.
        }

        return event_consumed_by_gui;
    } 


    void SDLImGui::set_common_dialog_properties() {
        ImVec2 dialog_size = ImVec2(800, 600);
        ImVec2 dialog_pos = ImVec2(
            (ImGui::GetIO().DisplaySize.x - dialog_size.x) * 0.5f,
            (ImGui::GetIO().DisplaySize.y - dialog_size.y) * 0.5f
        );
        ImGui::SetNextWindowSize(dialog_size);
        ImGui::SetNextWindowPos(dialog_pos);
    }

    // NEW: Helper to populate FileDialogResult
    FileDialogResult SDLImGui::populate_dialog_result(const std::string& key) {
        FileDialogResult result;
        if (key.empty()) { return result; }
        result.is_ok = ImGuiFileDialog::Instance()->IsOk();
        if (result.is_ok) {
            result.file_path_name = ImGuiFileDialog::Instance()->GetFilePathName();
            result.file_name = ImGuiFileDialog::Instance()->GetCurrentFileName();
            result.folder_path = ImGuiFileDialog::Instance()->GetCurrentPath();
        }
        return result;
    }

} // namespace Salix