#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/window/IWindow.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/events/IEventPoller.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/ImGuiInputEvent.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/IDialog.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <imgui.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <functional>
#include <iostream>
#include <SDL.h>
 
namespace Salix {
    
    struct OpenGLImGui::Pimpl {
        ImGuiContext* context = nullptr;
        IWindow* i_window = nullptr; // Not strictly needed to store, but fine if you use it
        IRenderer* i_renderer = nullptr; // Not strictly needed to store, but fine if you use it
        SDL_Window* sdl_window = nullptr; // Actual native SDL_Window pointer
        SDL_GLContext gl_context = nullptr; // Actual native SDL_Renderer pointer
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

    OpenGLImGui::OpenGLImGui() : pimpl(std::make_unique<Pimpl>()) {}



    OpenGLImGui::~OpenGLImGui() {
        std::cout << "OpenGLImGui::~OpenGLImGui - Destructor called!" << std::endl;
        pimpl->unregister_raw_event_callback_if_registered();
    }



    bool OpenGLImGui::initialize(IWindow* window_interface, IRenderer* renderer_interface,
        IThemeManager* theme_manager_interface, IFontManager* font_manager_interface) {
        // Method start
        if (!window_interface || !renderer_interface) { 
            std::cerr << "OpgenGLImGui::initialize(IWindow*, IRenderer*) incorrect arguments types or null detected." << std::endl;
            return false;
        }
        pimpl->i_theme_manager = theme_manager_interface;
        pimpl->i_font_manager = font_manager_interface;
        pimpl->i_window = window_interface;
        pimpl->i_renderer = renderer_interface;

        pimpl->sdl_window = static_cast<SDL_Window*>(window_interface->get_native_handle());
        pimpl->gl_context = static_cast<SDL_GLContext>(renderer_interface->get_native_handle());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable; // | ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & !ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplSDL2_InitForOpenGL(pimpl->sdl_window, pimpl->gl_context);
        ImGui_ImplOpenGL3_Init("#version 450");

        io.Fonts->AddFontDefault(); // Load ImGui's default font
        io.Fonts->Build();          // Build the font atlas (CPU side)

        ImGui_ImplOpenGL3_UpdateTexture(io.Fonts->TexData);
        pimpl->context = ImGui::GetCurrentContext();
        std::cout << "OpenGLImGui: Initialized successfully." << std::endl;
        return true;
    }




    void OpenGLImGui::shutdown() {
        // Unregister raw event callback first
        std::cout << "OpenGLImGUi::Shutdown - Shutdown called." << std::endl;
        pimpl->unregister_raw_event_callback_if_registered();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        if (ImGui::GetCurrentContext()) { ImGui::DestroyContext(); }
    }




    void OpenGLImGui::new_frame() {
        ImGuiIO& io = ImGui::GetIO(); // Get current IO state

        // Create and dispatch the ImGuiInputEvent
        ImGuiInputEvent event(io.WantCaptureMouse, io.WantCaptureKeyboard);
        if (pimpl->event_manager) { // Ensure event manager is valid
            pimpl->event_manager->dispatch(event);
        } else {
            std::cerr << "SDLImGui Warning: EventManager is null when dispatching ImGuiInputEvent." << std::endl;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }




    void OpenGLImGui::render() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    



    void OpenGLImGui::set_app_config(ApplicationConfig* config) {
        if (!config) { return; }  // cannot accept a nullptr
        pimpl->app_config = config;
    }
    

    ApplicationConfig* OpenGLImGui::get_app_config() {
        return pimpl->app_config;
    }
    
    void OpenGLImGui::Pimpl::unregister_raw_event_callback_if_registered() {
        if (event_poller && raw_event_callback_handle != 0) {
            event_poller->unregister_raw_event_callback(raw_event_callback_handle);
            raw_event_callback_handle = 0; // Reset handle after unregistration
        }
    }
    
    void OpenGLImGui::setup_event_polling(IEventPoller* event_poller_ptr, EventManager* event_manager_ptr) {
        if (!event_poller_ptr || !event_manager_ptr) {
            std::cerr << "OpenGLImGui Error: Null event poller or event manager passed to setup_event_polling." << std::endl;
            return;
        }
        
        pimpl->event_poller = event_poller_ptr;
        pimpl->event_manager = event_manager_ptr;

        pimpl->raw_event_callback_handle = pimpl->event_poller->register_raw_event_callback(
            [&](void* native_event) {
                // Cast the void* back to SDL_Event* and pass to ImGui's backend
                ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(native_event));

                process_raw_input_event(native_event);
            }
        );
        std::cout << "OpenGLImGui: Registered raw SDL event callback with handle " << pimpl->raw_event_callback_handle << std::endl;

    }
    
    
    
    IWindow* OpenGLImGui::get_window() {
        return pimpl->i_window;
    }
    
    
    
    IRenderer* OpenGLImGui::get_renderer() {
        return pimpl->i_renderer;
    }



    IThemeManager* OpenGLImGui::get_theme_manager() {
        return pimpl->i_theme_manager;
    }



    IFontManager* OpenGLImGui::get_font_manager() {
        return pimpl->i_font_manager; 
    }



    void OpenGLImGui::update_and_render_platform_windows() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags& ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }



    void OpenGLImGui::set_mouse_cursor_visible(bool visible) {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = visible ? (io.ConfigFlags & ~ImGuiConfigFlags_NoMouseCursorChange) : (io.ConfigFlags | ImGuiConfigFlags_NoMouseCursorChange);
        
    }



    void OpenGLImGui::save_layout(const std::string& file_path) {
        ImGui::SaveIniSettingsToDisk(file_path.c_str());
        std::cout << "OpenGLImGui: Layout saved to " << file_path << std::endl;
    }



    void OpenGLImGui::load_layout(const std::string& file_path) {
        ImGui::LoadIniSettingsFromDisk(file_path.c_str());
        std::cout << "OpenGLImGui: Layout loaded from " << file_path << std::endl;
    }



    void OpenGLImGui::reinitialize_backend() {
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "OpenGLImGui::reinitialize_backend - ImGui context is null." << std::endl;
        return;
    }
    // Check for the correct handles for this class
    if (!pimpl->sdl_window || !pimpl->gl_context) {
        std::cerr << "OpenGLImGui::reinitialize_backend - SDL window or GL context is null." << std::endl;
        return;
    }

    // For the OpenGL backend, we just need to re-initialize the implementation files.
    // This will automatically handle destroying and recreating the font atlas texture.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui_ImplSDL2_InitForOpenGL(pimpl->sdl_window, pimpl->gl_context);
    ImGui_ImplOpenGL3_Init("#version 450");
     
    std::cout << "OpenGLImGui: Backend re-initialized successfully." << std::endl;
}



    bool OpenGLImGui::open_save_file_dialog(const std::string& key, const std::string& title, const std::string& filters,
        const std::string& default_path, const std::string& default_filename) {
        // Method start
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

        return false;
    }



    bool OpenGLImGui::open_load_file_dialog(const std::string& key, const std::string& title,
        const std::string& filters, const std::string& default_path) {
        // Method start    
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



    bool OpenGLImGui::open_folder_dialog(const std::string& key, const std::string& title,
        const std::string& default_path) {
        // Method start
        set_common_dialog_properties();
        IGFD::FileDialogConfig config;
        config.path = default_path;
        config.flags = ImGuiFileDialogFlags_Modal;
          
        ImGuiFileDialog::Instance()->OpenDialog(key, title, "", config); // No filters for folder dialog
        pimpl->active_dialog_keys[key] = true; // Mark dialog as active
        return true;
    }



    bool OpenGLImGui::is_dialog_open(const std::string& key) const {
        return pimpl->active_dialog_keys.count(key) > 0; // Check if key exists in active map    
    }




    bool OpenGLImGui::is_dialog_closed_this_frame(const std::string& key) const {
    // This relies on the `display_dialogs()` method setting results for the current frame.
        return pimpl->dialog_results_this_frame.count(key) > 0; // Check if result was stored this frame    
    }



    FileDialogResult OpenGLImGui::get_dialog_result(const std::string& key) const {
        auto it = pimpl->dialog_results_this_frame.find(key);
        if (it != pimpl->dialog_results_this_frame.end()) {
            return it->second;
        }
        return FileDialogResult(); // Returns an Default/empty FileDialogResult if the key isn't found.
    }



    void OpenGLImGui::display_dialogs() {
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
                std::cerr << "OpenGLImGui::display_dialogs - Active dialog '" << key << "' not found in registry." << std::endl;
                pimpl->active_dialog_keys.erase(key); // Remove from active list if not found
                continue; // Skip to next dialog
            }

            DialogBox* dialog_instance = it_registry->second.get();


            if (!ImGuiFileDialog::Instance()->IsOpened(key.c_str())) { // Check if IGFD has it open already
                
                set_common_dialog_properties(); // 

                dialog_instance->open(); // This calls ImGuiFileDialog::Instance()->OpenDialog() 
                std::cout << "DEBUG: Dialog '" << key << "' just called Open() for the first time." << std::endl;
            }

            // Now, call Display() every frame it's active (including the first after Open())
            if (ImGuiFileDialog::Instance()->Display(key.c_str())) { // Display returns true when dialog is closed.
                // Dialog was just closed this frame (either OK or Cancel)
                FileDialogResult result = populate_dialog_result(key);
                pimpl->dialog_results_this_frame[key] = result;

                FileDialogCallback callback = dialog_instance->get_callback(); // Get the callback.
                // Check if a callback was set
                if (callback) { 
                    callback(result); // Execute the callback with the result
                }

                ImGuiFileDialog::Instance()->Close(); // Explicitly close (though Display() might have already).
                pimpl->active_dialog_keys.erase(key); // Remove from active list.
                std::cout << "DEBUG: Dialog '" << key << "' was just closed and removed from active list." << std::endl; // Debug
            }
        }
    }



    void OpenGLImGui::set_common_dialog_properties() {
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



    FileDialogResult OpenGLImGui::populate_dialog_result(const std::string& key) {
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



    DialogBox* OpenGLImGui::create_dialog(std::string& key, std::string& title, DialogType type, bool overwrite) { 
        // Check if dialog already exists in the registry
        auto it = pimpl->dialog_registry.find(key);
        if (it != pimpl->dialog_registry.end()) {
            std::cerr << "OpenGLImGui::create_dialog - Dialog with key '" << key <<
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
            std::cerr << "OpenGLImGui::create_dialog - Failed to register dialog '" << key << "' after creation." << std::endl;
            return nullptr; // Return null if registration fails
        }

        std::cout << "OpenGLImGui: Dialog '" << key << "' created and registered." << std::endl;
        return raw_dialog_ptr;
    }


    DialogBox* OpenGLImGui::get_dialog(const std::string& key) {
    if (pimpl->dialog_registry.count(key)) { // Assuming your map is called m_dialogs
        return pimpl->dialog_registry.at(key).get();
    }

    return nullptr;
}

    bool OpenGLImGui::register_dialog(std::unique_ptr<DialogBox> dialog) {
        if (!dialog) {
            std::cerr << "SDLImGui::register_dialog - Provided dialog is null." << std::endl;
            return false;
        }

        std::string key = dialog->get_key();

        if (pimpl->dialog_registry.count(key)) { // Check if key already exists
            std::cerr << "OpenGLImGui::register_dialog - Dialog with key '" << key << "' already registered." << std::endl;
            return false;
        }

        // emplace returns a pair<iterator, bool>, where bool is true if insertion happened.
        auto [it, inserted] = pimpl->dialog_registry.emplace(key, std::move(dialog));

        if (inserted) {
            std::cout << "OpenGLImGui: Dialog '" << key << "' registered." << std::endl;
            return true;
        } else {
            std::cerr << "OpenGLImGui::register_dialog - Failed to register dialog '" << key << "' (unexpected error)." << std::endl;
            return false;
        }
    }



    void OpenGLImGui::show_dialog_by_key(std::string& key) {
       if (key.empty()) { return; } 

       pimpl->active_dialog_keys[key] = true;
        std::cout << "DEBUG: Dialog '" << key << "' marked as active." << std::endl;
    }



    // --- Implement Abstract Input Handling for GUI ---
    bool OpenGLImGui::process_raw_input_event(void* native_event) {
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
    
    ImGuiContext* OpenGLImGui::get_context() { return pimpl->context; }

    void OpenGLImGui::request_theme_reload() {
        pimpl->theme_reload_requested = true;
    }
    bool OpenGLImGui::is_theme_reload_requested() {
        return pimpl->theme_reload_requested;
    }
    void OpenGLImGui::clear_theme_reload_request() {
        pimpl->theme_reload_requested = false;

    }

}  // namespace Salix.