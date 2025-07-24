// Salix/gui/imgui/SDLImGui.cpp
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/core/ApplicationConfig.h>
#include <imgui/imgui.h>
#include <Salix/window/sdl/SDLWindow.h> 
#include <Salix/rendering/sdl/SDLRenderer.h> 
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
#include <Salix/gui/IDialog.h>
#include <Salix/gui/DialogBox.h>
#include <memory>

namespace Salix {

    struct SDLImGui::Pimpl {
        ImGuiContext* context = nullptr;
        IWindow* i_window = nullptr; // Not strictly needed to store, but fine if you use it
        IRenderer* i_renderer = nullptr; // Not strictly needed to store, but fine if you use it
        SDL_Window* sdl_window = nullptr; // Actual native SDL_Window pointer
        SDL_Renderer* sdl_renderer = nullptr; // Actual native SDL_Renderer pointer
        IThemeManager* i_theme_manager = nullptr;
        IFontManager* i_font_manager = nullptr;
        IEventPoller* event_poller = nullptr;   // <-- Store IEventPoller
        EventManager* event_manager = nullptr;   // <-- Store EventManager
        RawEventCallbackHandle raw_event_callback_handle = 0; // <-- Store the handle
        ApplicationConfig* app_config = nullptr;
        std::unordered_map<std::string, bool> active_dialog_keys; // NEW: Tracks which dialogs are currently open
        std::unordered_map<std::string, std::unique_ptr<DialogBox>> dialog_registry; // store all the registered dialogs
        // Store dialog results for the current frame
        mutable std::unordered_map<std::string, FileDialogResult> dialog_results_this_frame;
        bool theme_reload_requested = false;
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
        pimpl->context = ImGui::GetCurrentContext();
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
        if (io.ConfigFlags& ImGuiConfigFlags_ViewportsEnable) {
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
        std::vector<std::string> keys_to_process; // Use a new name to avoid confusion with existing copy
        for (const auto& pair : pimpl->active_dialog_keys) {
            keys_to_process.push_back(pair.first);
        }

        for (const std::string& key : keys_to_process) {
            // Retrieve the DialogBox instance from the registry
            auto it_registry = pimpl->dialog_registry.find(key);
            if (it_registry == pimpl->dialog_registry.end()) {
                // This should ideally not happen if active_dialog_keys is consistent with dialog_registry
                std::cerr << "SDLImGui::display_dialogs - Active dialog '" << key << "' not found in registry." << std::endl;
                pimpl->active_dialog_keys.erase(key); // Remove from active list if not found
                continue; // Skip to next dialog
            }

            DialogBox* dialog_instance = it_registry->second.get();

            // **CRITICAL FIX 1: Check if the dialog needs to be opened (first frame it's active)**
            // ImGuiFileDialog::Instance()->IsOpened(key.c_str()) checks if IGFD internally considers it open.
            // If it's in our active_dialog_keys but IGFD doesn't know it's open, then open it.
            if (!ImGuiFileDialog::Instance()->IsOpened(key.c_str())) { // Check if IGFD has it open already
                // If it's active in our map but not yet opened by IGFD, then open it now.
                // This happens on the first frame show_dialog_by_key() is called for it.

                // CRITICAL FIX 2: Apply common properties just before opening
                // This ensures the size and position are set for the FIRST frame the dialog appears.
                set_common_dialog_properties(); // 

                dialog_instance->Open(); // This calls ImGuiFileDialog::Instance()->OpenDialog() 
                std::cout << "DEBUG: Dialog '" << key << "' just called Open() for the first time." << std::endl; // Debug
            }

            // Now, call Display() every frame it's active (including the first after Open())
            if (ImGuiFileDialog::Instance()->Display(key.c_str())) { // Display returns true when dialog is closed.
                // Dialog was just closed this frame (either OK or Cancel)
                FileDialogResult result = populate_dialog_result(key);
                pimpl->dialog_results_this_frame[key] = result;

                const FileDialogCallback& callback = dialog_instance->get_callback(); // Get the callback.
                if (callback) { // Check if a callback was set
                    callback(result); // Execute the callback with the result
                }

                ImGuiFileDialog::Instance()->Close(); // Explicitly close (though Display() might have already).
                pimpl->active_dialog_keys.erase(key); // Remove from active list.
                std::cout << "DEBUG: Dialog '" << key << "' was just closed and removed from active list." << std::endl; // Debug
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

        bool event_consumed_by_gui = false;

        if (sdl_event->type == SDL_KEYDOWN) {
            if (sdl_event->key.keysym.sym == SDLK_ESCAPE) {
                if (ImGuiFileDialog::Instance()->IsOpened()) {
                    ImGuiFileDialog::Instance()->Close(); // Explicitly close dialog
                    event_consumed_by_gui = true;
                    std::cout << "Escape key: Dialog closed by GUI system." << std::endl;
                    pimpl->active_dialog_keys.clear();
                    std::cout << "DEBUG: active_dialog_keys cleared due to Escape." << std::endl;
                }
            }
            // No explicit handling for SDLK_RETURN (Enter) needed here,
            // as ImGuiFileDialog typically handles it internally.
        }

        return event_consumed_by_gui;
    } 


    void SDLImGui::set_common_dialog_properties() {
        // 1. Declare dialog_size here, in the main scope, with a default value.
        ImVec2 dialog_size = ImVec2(800, 600);

        // 2. If the config exists, we'll UPDATE the dialog_size variable.
        if (pimpl->app_config) {
            float width = pimpl->app_config->window_config.width * pimpl->app_config->gui_settings.dialog_width_ratio;
            float height = pimpl->app_config->window_config.height * pimpl->app_config->gui_settings.dialog_height_ratio;
            
            // Assign a new value to the existing dialog_size variable
            dialog_size = ImVec2(width, height);
        }

        // 3. Now, dialog_size can be used here because it was declared in the outer scope.
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

    DialogBox* SDLImGui::create_dialog(std::string& key, std::string& title, DialogType type, bool overwrite) {
        // Check if dialog already exists in the registry
        auto it = pimpl->dialog_registry.find(key);
        if (it != pimpl->dialog_registry.end()) {
            std::cerr << "SDLImGui::create_dialog - Dialog with key '" << key <<
            "' already exists. Returning existing dialog." << std::endl;
            return it->second.get(); // Return existing dialog
        }

        // Create the concrete dialog wrapper object
        auto dialog_box = std::make_unique<DialogBox> ( key, title, type, overwrite);

        // Set flags based on overwrite
        if (overwrite) {
            dialog_box->get_config().flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
        }

        // Store the dialog in the registry and get a raw pointer
        DialogBox* raw_dialog_ptr = dialog_box.get();

        // CRITICAL FIX: Use the register_dialog method to add it to the registry.
        // register_dialog takes ownership of the unique_ptr.
        if (!register_dialog(std::move(dialog_box))) { // Call the method
            std::cerr << "SDLImGui::create_dialog - Failed to register dialog '" << key << "' after creation." << std::endl;
            return nullptr; // Return null if registration fails
        }

        std::cout << "SDLImGui: Dialog '" << key << "' created and registered." << std::endl;
        return raw_dialog_ptr;
    }

    
    bool SDLImGui::register_dialog(std::unique_ptr<DialogBox> dialog) {
        if (!dialog) {
            std::cerr << "SDLImGui::register_dialog - Provided dialog is null." << std::endl;
            return false;
        }

        std::string key = dialog->get_key();
        if (pimpl->dialog_registry.count(key)) { // Check if key already exists
            std::cerr << "SDLImGui::register_dialog - Dialog with key '" << key << "' already registered." << std::endl;
            return false;
        }

        // emplace returns a pair<iterator, bool>, where bool is true if insertion happened.
        auto [it, inserted] = pimpl->dialog_registry.emplace(key, std::move(dialog));
        
        if (inserted) {
            std::cout << "SDLImGui: Dialog '" << key << "' registered." << std::endl;
            return true;
        } else {
            // This case should ideally not be hit due to the .count(key) check above,
            // but it's good defensive programming.
            std::cerr << "SDLImGui::register_dialog - Failed to register dialog '" << key << "' (unexpected error)." << std::endl;
            return false;
        }
    }

    void SDLImGui::show_dialog_by_key(std::string& key) {
        if (key.empty()) { return;}  // empty string
        pimpl->active_dialog_keys[key] = true;
        std::cout << "DEBUG: Dialog '" << key << "' marked as active." << std::endl;
    }

    void SDLImGui::set_app_config(ApplicationConfig*  config) {
        pimpl->app_config = config;
    }

    ApplicationConfig* SDLImGui::get_app_config() {
        return pimpl->app_config;
    }

    ImGuiContext* SDLImGui::get_context() { return pimpl->context; }

    void SDLImGui::request_theme_reload() {
        pimpl->theme_reload_requested = true;
    }
    bool SDLImGui::is_theme_reload_requested() {
        return pimpl->theme_reload_requested;
    }
    void SDLImGui::clear_theme_reload_request() {
        pimpl->theme_reload_requested = false;

    }

} // namespace Salix