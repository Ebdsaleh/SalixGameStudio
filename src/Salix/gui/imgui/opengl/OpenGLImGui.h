#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IGui.h>
#include <ImGuiFileDialog.h>
#include <Salix/gui/IDialog.h>
#include <memory>

namespace Salix {


    // Forward declare interfaces
    class IWindow;
    class IRenderer;
    class IThemeManager;
    class IFontManager;
    class IEventPoller;
    class EventManager;
    struct ApplicationConfig;
    struct FileDialogResult;

    class SALIX_API OpenGLImGui : public IGui {
    public:
        OpenGLImGui();
        virtual ~OpenGLImGui() override;

        // IGui interface implementation
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
        void set_app_config(ApplicationConfig* config) override;
        // Specific method to set up event system dependencies.
        void setup_event_polling(IEventPoller* event_poller, EventManager* event_manager);
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}