// Salix/gui/IGui.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string> // For std::string
#include <imgui/imgui.h> // Only include ImGui's core header if SALIX_GUI_IMGUI is defined
#include <Salix/gui/IDialog.h>
#include <Salix/core/ApplicationConfig.h>

namespace Salix {
    // Forward declarations for engine interfaces that a GUI might need to interact with
    class IWindow;
    class IRenderer;
    class IThemeManager;
    class IFontManager;
    class DialogBox;
    enum ImGuiFileDialogFlags_ : int;
    struct FileDialogResult {
        bool is_ok = false;
        std::string file_path_name;
        std::string file_name;
        std::string folder_path;
    };


    class SALIX_API IGui {
    public:
        virtual ~IGui() = default;

        // Initializes the GUI system.
        // The concrete implementation will cast the 'window' and 'renderer'
        // to their specific types (e.g., SDLWindow*, SDLRenderer*) internally.
        virtual bool initialize(
            IWindow* window, IRenderer* renderer,
            IThemeManager* theme_manager, IFontManager* font_manager) = 0;

        // Shuts down the GUI system and cleans up resources.
        virtual void shutdown() = 0;

        // Starts a new GUI frame.
        // This method should be called once per frame before any UI elements are built.
        // It handles input processing for the GUI.
        virtual void new_frame() = 0;

        // Renders the GUI elements that have been built for the current frame.
        // This method should be called after all UI building calls (e.g., ImGui::Begin/End)
        // have been made for the current frame.
        virtual void render() = 0;

        // Handles updates and rendering for platform-specific GUI windows (e.g., docking viewports).
        // This abstracts away multi-window/multi-viewport management if the GUI library supports it.
        virtual void update_and_render_platform_windows() = 0;

        virtual IWindow* get_window() = 0;

        virtual IRenderer* get_renderer() = 0;

        virtual IThemeManager* get_theme_manager() = 0;

        virtual IFontManager* get_font_manager() = 0;
        // --- Basic GUI Configuration / Persistence ---

        // Controls the visibility of the GUI's mouse cursor.
        virtual void set_mouse_cursor_visible(bool visible) = 0;
        
        // Saves the current GUI layout/configuration to a file.
        virtual void save_layout(const std::string& file_path) = 0;

        // Loads a GUI layout/configuration from a file.
        virtual void load_layout(const std::string& file_path) = 0;

        // To re-initialize the ImGui backend (including font texture)
        virtual void reinitialize_backend() = 0;

        // --- NEW: Abstract File Dialog Control ---
        // Returns true if dialog was opened. Dialog will be displayed in subsequent new_frame/render calls.
        virtual bool open_save_file_dialog(const std::string& key, const std::string& title, const std::string& filters, const std::string& default_path = ".", const std::string& default_filename = "") = 0;
        virtual bool open_load_file_dialog(const std::string& key, const std::string& title, const std::string& filters, const std::string& default_path = ".") = 0;
        virtual bool open_folder_dialog(const std::string& key, const std::string& title, const std::string& default_path = ".") = 0;

        // Returns true if a dialog is currently open.
        virtual bool is_dialog_open(const std::string& key) const = 0;
        // Returns true if a dialog was just closed this frame (either OK or Cancel).
        virtual bool is_dialog_closed_this_frame(const std::string& key) const = 0;
        // Returns the result of a closed dialog. Only valid if is_dialog_closed_this_frame() and result.is_ok is true.
        virtual FileDialogResult get_dialog_result(const std::string& key) const = 0;
        // Call this to display all active dialogs. Should be called once per frame.
        virtual void display_dialogs() = 0;

        // Sets the common file dialog properties
        virtual void set_common_dialog_properties() = 0;

        virtual FileDialogResult populate_dialog_result(const std::string& key) = 0;
        
        virtual DialogBox* create_dialog(std::string& key, std::string& title, DialogType type, bool overwrite) = 0;

        virtual bool register_dialog(std::unique_ptr<DialogBox> dialog) = 0;

        virtual void show_dialog_by_key(std::string& key) = 0;
        // --- Abstract Input Handling for GUI ---
        // This method receives raw input events (e.g., from EventPoller)
        // and processes them for GUI-level interactions (like Escape to close dialogs).
        // Returns true if the input was consumed by the GUI.
        virtual bool process_raw_input_event(void* native_event) = 0; // e.g., SDL_Event*

        virtual void set_app_config(ApplicationConfig* config) = 0;

        virtual ApplicationConfig* get_app_config() = 0;
    };
}