// Salix/gui/imgui/SDLImGui.h
#pragma once

#include <Salix/gui/IGui.h>
#include <imgui/imgui.h>
#include <memory>
// New includes needed for the setup_event_polling method
#include <Salix/events/IEventPoller.h> 
#include <Salix/events/EventManager.h> // For dispatching ImGuiInputEvent
#include <Salix/events/ImGuiInputEvent.h> // For the event definition
#include <ImGuiFileDialog.h> // Needed for ImGuiFileDialogFlags and IGFD::FileDialogConfig

// Forward declarations for specific SDL types if your Pimpl needs them directly
class IThemeManager;
class IFontManager;
class DialogBox;
enum class DialogType;

struct FileDialogResult {
        bool is_ok = false;
        std::string file_path_name;
        std::string file_name;
        std::string folder_path;
    };

namespace Salix {
    
    class SALIX_API SDLImGui : public IGui {
    public:
        SDLImGui();
        ~SDLImGui() override; // Ensure override is present for virtual destructor
        bool initialize(IWindow* window, IRenderer* renderer,
             IThemeManager* theme_manager, IFontManager* font_manager) override;
        void shutdown() override;
        void new_frame() override;
        void render() override;
        IWindow* get_window() override;
        IRenderer* get_renderer() override;
        IThemeManager* get_theme_manager() override;
        IFontManager* get_font_manager() override;
        void update_and_render_platform_windows() override;
        void set_mouse_cursor_visible(bool visible) override;
        void save_layout(const std::string& file_path) override;
        void load_layout(const std::string& file_path) override;
        void reinitialize_backend() override;
        // Specific method to set up event system dependencies.
        // This is NOT part of the IGui interface.
        void setup_event_polling(IEventPoller* event_poller, EventManager* event_manager);
        
        // --- Implement Abstract File Dialog Control ---
        bool open_save_file_dialog(const std::string& key, const std::string& title, const std::string& filters, const std::string& default_path = ".", const std::string& default_filename = "") override;
        bool open_load_file_dialog(const std::string& key, const std::string& title, const std::string& filters, const std::string& default_path = ".") override;
        bool open_folder_dialog(const std::string& key, const std::string& title, const std::string& default_path = ".") override;

        bool is_dialog_open(const std::string& key) const override;
        bool is_dialog_closed_this_frame(const std::string& key) const override;
        FileDialogResult get_dialog_result(const std::string& key) const override;
        void display_dialogs() override;
        void set_common_dialog_properties() override;
        FileDialogResult populate_dialog_result(const std::string& key) override;

        DialogBox* create_dialog(std::string& key, std::string& title, DialogType type, bool overwrite) override;
        bool register_dialog(std::unique_ptr<DialogBox> dialog) override;

        void show_dialog_by_key(std::string& key) override;
        // --- Implement Abstract Input Handling for GUI ---
        bool process_raw_input_event(void* native_event) override;
        
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix