#pragma once

#include <Salix/gui/IGui.h>
#include <Salix/gui/DialogBox.h>
#include <memory>
#include <string>
#include <utility>

class MockIGui : public Salix::IGui {
public:
    bool initialize(
        Salix::IWindow* window,
        Salix::IRenderer* renderer,
        Salix::IThemeManager* theme_manager,
        Salix::IFontManager* font_manager,
        Salix::IIconManager* icon_manager) override {
        window_ = window;
        renderer_ = renderer;
        theme_manager_ = theme_manager;
        font_manager_ = font_manager;
        icon_manager_ = icon_manager;
        ++initialize_calls;
        return initialize_result;
    }

    void shutdown() override { ++shutdown_calls; }
    void new_frame() override { ++new_frame_calls; }
    void render() override { ++render_calls; }
    void update_and_render_platform_windows() override { ++platform_window_calls; }

    Salix::IWindow* get_window() override { return window_; }
    Salix::IRenderer* get_renderer() override { return renderer_; }
    Salix::IThemeManager* get_theme_manager() override { return theme_manager_; }
    Salix::IFontManager* get_font_manager() override { return font_manager_; }
    Salix::IIconManager* get_icon_manager() override { return icon_manager_; }

    void set_mouse_cursor_visible(bool visible) override {
        mouse_cursor_visible = visible;
        ++mouse_cursor_calls;
    }

    void save_layout(const std::string& file_path) override {
        last_saved_layout = file_path;
        ++save_layout_calls;
    }

    void load_layout(const std::string& file_path) override {
        last_loaded_layout = file_path;
        ++load_layout_calls;
    }

    void reinitialize_backend() override { ++reinitialize_calls; }

    bool open_save_file_dialog(
        const std::string& key,
        const std::string& title,
        const std::string& filters,
        const std::string& default_path = ".",
        const std::string& default_filename = "") override {
        last_dialog_key = key;
        last_dialog_title = title;
        last_dialog_filters = filters;
        last_dialog_path = default_path;
        last_dialog_filename = default_filename;
        ++open_save_calls;
        return dialog_open_result;
    }

    bool open_load_file_dialog(
        const std::string& key,
        const std::string& title,
        const std::string& filters,
        const std::string& default_path = ".") override {
        last_dialog_key = key;
        last_dialog_title = title;
        last_dialog_filters = filters;
        last_dialog_path = default_path;
        ++open_load_calls;
        return dialog_open_result;
    }

    bool open_folder_dialog(
        const std::string& key,
        const std::string& title,
        const std::string& default_path = ".") override {
        last_dialog_key = key;
        last_dialog_title = title;
        last_dialog_path = default_path;
        ++open_folder_calls;
        return dialog_open_result;
    }

    bool is_dialog_open(const std::string& key) const override {
        return dialog_is_open && key == last_dialog_key;
    }

    bool is_dialog_closed_this_frame(const std::string& key) const override {
        return dialog_closed_this_frame && key == last_dialog_key;
    }

    Salix::FileDialogResult get_dialog_result(const std::string&) const override {
        return dialog_result;
    }

    void display_dialogs() override { ++display_dialog_calls; }
    void set_common_dialog_properties() override { ++common_dialog_property_calls; }

    Salix::FileDialogResult populate_dialog_result(const std::string& key) override {
        last_dialog_key = key;
        ++populate_result_calls;
        return dialog_result;
    }

    Salix::DialogBox* create_dialog(std::string& key, std::string& title, Salix::DialogType type, bool overwrite) override {
        last_dialog_key = key;
        last_dialog_title = title;
        last_dialog_type = type;
        last_dialog_overwrite = overwrite;
        ++create_dialog_calls;
        return nullptr;
    }

    Salix::DialogBox* get_dialog(const std::string&) override { return nullptr; }

    bool register_dialog(std::unique_ptr<Salix::DialogBox> dialog) override {
        ++register_dialog_calls;
        return static_cast<bool>(dialog);
    }

    void show_dialog_by_key(std::string& key) override {
        last_dialog_key = key;
        ++show_dialog_calls;
    }

    bool process_raw_input_event(void* native_event) override {
        last_native_event = native_event;
        ++raw_input_calls;
        return raw_input_result;
    }

    void set_app_config(Salix::ApplicationConfig* config) override { app_config_ = config; }
    Salix::ApplicationConfig* get_app_config() override { return app_config_; }
    ImGuiContext* get_context() override { return context_; }

    void request_theme_reload() override { theme_reload_requested = true; }
    bool is_theme_reload_requested() override { return theme_reload_requested; }
    void clear_theme_reload_request() override { theme_reload_requested = false; }

    bool initialize_result = true;
    bool dialog_open_result = true;
    bool dialog_is_open = false;
    bool dialog_closed_this_frame = false;
    bool raw_input_result = false;
    bool mouse_cursor_visible = true;
    bool theme_reload_requested = false;
    Salix::FileDialogResult dialog_result{};
    Salix::DialogType last_dialog_type = Salix::DialogType::File;
    bool last_dialog_overwrite = false;
    int initialize_calls = 0;
    int shutdown_calls = 0;
    int new_frame_calls = 0;
    int render_calls = 0;
    int platform_window_calls = 0;
    int mouse_cursor_calls = 0;
    int save_layout_calls = 0;
    int load_layout_calls = 0;
    int reinitialize_calls = 0;
    int open_save_calls = 0;
    int open_load_calls = 0;
    int open_folder_calls = 0;
    int display_dialog_calls = 0;
    int common_dialog_property_calls = 0;
    int populate_result_calls = 0;
    int create_dialog_calls = 0;
    int register_dialog_calls = 0;
    int show_dialog_calls = 0;
    int raw_input_calls = 0;
    std::string last_saved_layout;
    std::string last_loaded_layout;
    std::string last_dialog_key;
    std::string last_dialog_title;
    std::string last_dialog_filters;
    std::string last_dialog_path;
    std::string last_dialog_filename;
    void* last_native_event = nullptr;

private:
    Salix::IWindow* window_ = nullptr;
    Salix::IRenderer* renderer_ = nullptr;
    Salix::IThemeManager* theme_manager_ = nullptr;
    Salix::IFontManager* font_manager_ = nullptr;
    Salix::IIconManager* icon_manager_ = nullptr;
    Salix::ApplicationConfig* app_config_ = nullptr;
    ImGuiContext* context_ = nullptr;
};
