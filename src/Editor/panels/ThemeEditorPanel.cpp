// Editor/panels/ThemeEditorPanel.cpp

#include <imgui/imgui.h>
#include <Editor/panels/ThemeEditorPanel.h>
#include <Editor/EditorContext.h>
#include <Salix/core/InitContext.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/imgui/ImguiIconManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h> 
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/management/FileManager.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>


namespace Salix {

    struct ThemeEditorPanel::Pimpl {
        std::string name = "Theme Editor";
        EditorContext* context = nullptr;
        ImGuiIconManager* icon_manager = nullptr;
        bool is_visible = false;        
        bool is_locked = false;
        char theme_name_buffer[256] = {0};
        char theme_path_buffer[512] = {0};
        std::string icon_type_being_edited;
        std::shared_ptr<int> callback_lifetime = std::make_shared<int>(0);
        unsigned icon_request = 0;
        void browse_icon(const std::string& type_name);
        std::string last_icon_directory = ".";
        // Helper functions for rendering UI sections
        void create_dialog_boxes();
        void render_layout();
        void draw_lock_button();
        void save_current_theme(ImGuiThemeData* current_theme_data); 
        void load_selected_theme(ImGuiThemeData* data);
        void render_edit_colors(ImGuiThemeData* data);
        void render_edit_sizing_and_spacing(ImGuiThemeData* data);
        void render_edit_fonts(ImGuiThemeData* data);
        bool choose_font_family(ImGuiThemeData* data, const std::string& family);
        bool choose_font_size(ImGuiThemeData* data, float size);
        void apply_font_changes();
        ImGuiThemeData* active_data();
        void render_edit_icons(ImGuiThemeData* data);
    };

    ThemeEditorPanel::ThemeEditorPanel() : pimpl(std::make_unique<Pimpl>()) {}
    ThemeEditorPanel::~ThemeEditorPanel() = default;

    void ThemeEditorPanel::initialize(EditorContext* context) {
        if (!context) {
            std::cerr << "ThemeEditorPanel::initialize - EditorContext is NULL!" << std::endl;
            return;
        }
        pimpl->context = context;
        if (!context->gui) return;
        pimpl->icon_manager = dynamic_cast<ImGuiIconManager*>(pimpl->context->gui->get_icon_manager());
        if (!pimpl->icon_manager) { return; }
        pimpl->create_dialog_boxes();
    }
    
    void ThemeEditorPanel::on_gui_update() {
        if (!pimpl->is_visible || !pimpl->context || !pimpl->context->theme_manager) return;

        // The lock control must remain interactive even when the content is disabled.
        if (!pimpl->icon_manager) {
            ImGui::TextUnformatted("Theme icon service is unavailable.");
            return;
        }
        pimpl->render_layout();
    }

#ifdef SALIX_TESTS_ENABLED
    void ThemeEditorPanel::save_theme_for_testing(const std::string& path) {
        if (!pimpl->context || !pimpl->context->theme_manager) return;
        auto* theme = dynamic_cast<ImGuiTheme*>(pimpl->context->theme_manager->get_active_theme());
        strncpy_s(pimpl->theme_path_buffer, sizeof(pimpl->theme_path_buffer), path.c_str(), _TRUNCATE);
        pimpl->save_current_theme(theme ? theme->get_data() : nullptr);
    }
    void ThemeEditorPanel::load_theme_for_testing(const std::string& name, const std::string& path) {
        strncpy_s(pimpl->theme_name_buffer, sizeof(pimpl->theme_name_buffer), name.c_str(), _TRUNCATE);
        strncpy_s(pimpl->theme_path_buffer, sizeof(pimpl->theme_path_buffer), path.c_str(), _TRUNCATE);
        pimpl->load_selected_theme(nullptr);
    }
#endif

    void ThemeEditorPanel::on_gui_render() {

    }

    void ThemeEditorPanel::set_visibility(bool visibility) { pimpl->is_visible = visibility; }
    bool ThemeEditorPanel::get_visibility() const { return pimpl->is_visible; }
    void ThemeEditorPanel::set_name(const std::string& new_name) { pimpl->name = new_name; }
    const std::string& ThemeEditorPanel::get_name() { return pimpl->name; }
    void ThemeEditorPanel::on_event(IEvent& event) { (void) event; }
    void ThemeEditorPanel::on_render() {}

    void ThemeEditorPanel::Pimpl::create_dialog_boxes() {
        // Select Icon Image
        std::string select_icon_key = "SelectIconImage";
        std::string select_icon_title = "Select An Icon Image";

        DialogBox* select_icon_image_dialog = context->gui->create_dialog(
            select_icon_key,
            select_icon_title,
            DialogType::File,
            false);

        if (!select_icon_image_dialog) return;
        select_icon_image_dialog->set_filters(std::string("Image File (*.png){.png},All files (*.*){.*}"));

        

    }

   

    void ThemeEditorPanel::Pimpl::draw_lock_button() {
        if (!icon_manager) return;
        const auto& icon = icon_manager->get_icon_by_name(is_locked ? "Panel Locked" : "Panel Unlocked");
        const ImVec4 tint = is_locked ? ImVec4(.5f, 0, 0, 1) : ImVec4(1, 1, 1, 1);
        if (icon.texture_id != 0 && ImGui::ImageButton("##PanelLockBtn", icon.texture_id,
                ImVec2(16,16), ImVec2(0,0), ImVec2(1,1), ImVec4(0,0,0,0), tint)) {
            is_locked = !is_locked;
            std::cout << "Theme Editor Panel lock toggled to: " << (is_locked ? "LOCKED" : "UNLOCKED") << std::endl;
        }
    }
#ifdef SALIX_TESTS_ENABLED
    void ThemeEditorPanel::draw_lock_button_for_testing() { pimpl->draw_lock_button(); }
#endif

    void ThemeEditorPanel::Pimpl::render_layout() {
        if (ImGui::Begin(name.c_str(), &is_visible)) {
            ImGui::Separator();
            ImGui::AlignTextToFramePadding();
            draw_lock_button();

                if (is_locked) {
                    ImGui::BeginDisabled(); // Disable all interactive widgets below this point
                }

            ITheme* active_theme_base = context->theme_manager->get_active_theme();
            ImGuiTheme* active_imgui_theme = dynamic_cast<ImGuiTheme*>(active_theme_base);

            if (!active_imgui_theme) {
                ImGui::Text("No active ImGui theme to edit.");
                 if (is_locked) {
                    ImGui::EndDisabled(); // <-- This was missing!
                }
                ImGui::End();
                return;
            }
            
            ImGuiThemeData* data = active_imgui_theme->get_data();
            if (!data) {
                ImGui::Text("Active theme has no data.");
                if (is_locked) ImGui::EndDisabled();
                ImGui::End();
                return;
            }

            // Sync UI buffers with theme data if the window is just appearing
            if (ImGui::IsWindowAppearing()) { 
                strncpy_s(theme_name_buffer, sizeof(theme_name_buffer), active_imgui_theme->get_name().c_str(), _TRUNCATE);
                strncpy_s(theme_path_buffer, sizeof(theme_path_buffer), active_imgui_theme->get_file_path().c_str(), _TRUNCATE);
            }

            ImGui::Text("Theme Properties:");
            if (ImGui::InputText("Name", theme_name_buffer, sizeof(theme_name_buffer))) {
                data->name = theme_name_buffer; // Update theme data if user types
            }

            ImGui::InputText("File Path", theme_path_buffer, sizeof(theme_path_buffer));
            ImGui::Separator();

            if (ImGui::Button("Save Theme")) {
                save_current_theme(data);
            }
            ImGui::SameLine();

            if (ImGui::Button("Load Theme")) {
                load_selected_theme(data);
                // Loading can replace the registered theme and invalidate the previous data.
                auto* current_theme = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
                data = current_theme ? current_theme->get_data() : nullptr;
                if (!data) {
                    if (is_locked) ImGui::EndDisabled();
                    ImGui::End();
                    return;
                }
            }
            ImGui::Separator();

            render_edit_colors(data);
            render_edit_sizing_and_spacing(data);
            render_edit_fonts(data); 
            render_edit_icons(data);
            
            if (is_locked) {
                ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }

    void ThemeEditorPanel::Pimpl::load_selected_theme(ImGuiThemeData* data) {
        // The panel lock also applies to programmatically activated widgets.
        if (is_locked || !context || !context->theme_manager) return;

        std::string theme_name = std::string(theme_name_buffer);
        std::string file_path = std::string(theme_path_buffer);

        if (context->theme_manager->load_theme_from_file(theme_name, file_path, false)) {
            context->theme_manager->set_active_theme(theme_name);
            if (context->gui) context->gui->request_theme_reload(); 
            // After loading, get the newly active theme to refresh the UI text fields
            ImGuiTheme* new_active_theme = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
            if (new_active_theme && new_active_theme->get_data()) {
                data = new_active_theme->get_data();
                // Safely copy the new theme's actual name and path into the buffers
                strncpy_s(theme_name_buffer, sizeof(theme_name_buffer), new_active_theme->get_data()->name.c_str(), _TRUNCATE);
                strncpy_s(theme_path_buffer, sizeof(theme_path_buffer), new_active_theme->get_file_path().c_str(), _TRUNCATE);
            }
        } else {
            std::cerr << "ThemeEditorPanel: Failed to load theme '" << theme_name << "'." << std::endl;
        }
    }

    void ThemeEditorPanel::Pimpl::save_current_theme(ImGuiThemeData* data) {
        if (is_locked || !context || !context->theme_manager || !data) return;
        
        std::string file_path = std::string(theme_path_buffer);
        // Editable theme metadata is not the registry identity. Saving a renamed
        // theme must still select the actual active object, never another entry.
        auto* active = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
        if (active && active->get_data() == data)
            context->theme_manager->save_theme_to_file(active->get_name(), file_path);
    }

    void ThemeEditorPanel::Pimpl::render_edit_colors(ImGuiThemeData* data) {
        static const std::map<std::string, ImGuiCol> color_name_map = {
            {"Text", ImGuiCol_Text}, {"TextDisabled", ImGuiCol_TextDisabled}, {"WindowBg", ImGuiCol_WindowBg},
            {"ChildBg", ImGuiCol_ChildBg}, {"PopupBg", ImGuiCol_PopupBg}, {"Border", ImGuiCol_Border},
            {"BorderShadow", ImGuiCol_BorderShadow}, {"FrameBg", ImGuiCol_FrameBg}, {"FrameBgHovered", ImGuiCol_FrameBgHovered},
            {"FrameBgActive", ImGuiCol_FrameBgActive}, {"TitleBg", ImGuiCol_TitleBg}, {"TitleBgActive", ImGuiCol_TitleBgActive},
            {"TitleBgCollapsed", ImGuiCol_TitleBgCollapsed}, {"MenuBarBg", ImGuiCol_MenuBarBg}, {"ScrollbarBg", ImGuiCol_ScrollbarBg},
            {"ScrollbarGrab", ImGuiCol_ScrollbarGrab}, {"ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered},
            {"ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive}, {"CheckMark", ImGuiCol_CheckMark}, {"SliderGrab", ImGuiCol_SliderGrab},
            {"SliderGrabActive", ImGuiCol_SliderGrabActive}, {"Button", ImGuiCol_Button}, {"ButtonHovered", ImGuiCol_ButtonHovered},
            {"ButtonActive", ImGuiCol_ButtonActive}, {"Header", ImGuiCol_Header}, {"HeaderHovered", ImGuiCol_HeaderHovered},
            {"HeaderActive", ImGuiCol_HeaderActive}, {"Separator", ImGuiCol_Separator}, {"SeparatorHovered", ImGuiCol_SeparatorHovered},
            {"SeparatorActive", ImGuiCol_SeparatorActive}, {"ResizeGrip", ImGuiCol_ResizeGrip}, {"ResizeGripHovered", ImGuiCol_ResizeGripHovered},
            {"ResizeGripActive", ImGuiCol_ResizeGripActive}, {"Tab", ImGuiCol_Tab}, {"TabHovered", ImGuiCol_TabHovered},
            {"TabActive", ImGuiCol_TabActive}, {"TabUnfocused", ImGuiCol_TabUnfocused}, {"TabUnfocusedActive", ImGuiCol_TabUnfocusedActive},
            {"DockingPreview", ImGuiCol_DockingPreview}, {"DockingEmptyBg", ImGuiCol_DockingEmptyBg}, {"PlotLines", ImGuiCol_PlotLines},
            {"PlotLinesHovered", ImGuiCol_PlotLinesHovered}, {"PlotHistogram", ImGuiCol_PlotHistogram},
            {"PlotHistogramHovered", ImGuiCol_PlotHistogramHovered}, {"TableHeaderBg", ImGuiCol_TableHeaderBg},
            {"TableBorderStrong", ImGuiCol_TableBorderStrong}, {"TableBorderLight", ImGuiCol_TableBorderLight},
            {"TableRowBg", ImGuiCol_TableRowBg}, {"TableRowBgAlt", ImGuiCol_TableRowBgAlt}, {"TextSelectedBg", ImGuiCol_TextSelectedBg},
            {"DragDropTarget", ImGuiCol_DragDropTarget}, {"NavHighlight", ImGuiCol_NavHighlight},
            {"NavWindowingHighlight", ImGuiCol_NavWindowingHighlight}, {"NavWindowingDimBg", ImGuiCol_NavWindowingDimBg},
            {"ModalWindowDimBg", ImGuiCol_ModalWindowDimBg}
        };

        if (ImGui::TreeNode("Colors")) {
            for (auto& color_pair : data->theme_colors) {
                auto it = color_name_map.find(color_pair.first);
                if (it != color_name_map.end()) {
                    if (ImGui::ColorEdit4(it->first.c_str(), &color_pair.second->r)) {
                        ImGui::GetStyle().Colors[it->second] = color_pair.second->to_imvec4();
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    void ThemeEditorPanel::Pimpl::render_edit_sizing_and_spacing(ImGuiThemeData* data) {
        if (ImGui::TreeNode("Sizing & Spacing")) {
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::Text("Rounding");
            if (ImGui::DragFloat("Window Rounding", &data->window_rounding, 0.1f, 0.0f, 20.0f)) { style.WindowRounding = data->window_rounding; }
            if (ImGui::DragFloat("Frame Rounding", &data->frame_rounding, 0.1f, 0.0f, 20.0f)) { style.FrameRounding = data->frame_rounding; }
            if (ImGui::DragFloat("Scrollbar Rounding", &data->scrollbar_rounding, 0.1f, 0.0f, 20.0f)) { style.ScrollbarRounding = data->scrollbar_rounding; }
            
            ImGui::Separator();
            ImGui::Text("Padding & Spacing");
            if (ImGui::DragFloat2("Window Padding", &data->window_padding.x, 0.1f)) { style.WindowPadding = {data->window_padding.x, data->window_padding.y}; }
            if (ImGui::DragFloat2("Frame Padding", &data->frame_padding.x, 0.1f)) { style.FramePadding = {data->frame_padding.x, data->frame_padding.y}; }
            if (ImGui::DragFloat2("Item Spacing", &data->item_spacing.x, 0.1f)) { style.ItemSpacing = {data->item_spacing.x, data->item_spacing.y}; }

            ImGui::TreePop();
        }
    }

    ImGuiThemeData* ThemeEditorPanel::Pimpl::active_data() {
        if (!context || !context->theme_manager) return nullptr;
        auto* theme = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
        return theme ? theme->get_data() : nullptr;
    }

    bool ThemeEditorPanel::Pimpl::choose_font_family(ImGuiThemeData* data, const std::string& family) {
        if (!data || !context || !context->font_manager || is_locked) return false;
        const auto families = context->font_manager->get_unique_font_families();
        if (std::find(families.begin(), families.end(), family) == families.end()) return false;
        data->font_family = family;
        const auto sizes = context->font_manager->get_font_sizes_for_font_family(family);
        if (!sizes.empty()) data->font_size = sizes.front();
        return true;
    }

    bool ThemeEditorPanel::Pimpl::choose_font_size(ImGuiThemeData* data, float size) {
        if (!data || !context || !context->font_manager || is_locked) return false;
        const auto sizes = context->font_manager->get_font_sizes_for_font_family(data->font_family);
        if (std::find(sizes.begin(), sizes.end(), size) == sizes.end()) return false;
        data->font_size = size;
        return true;
    }

    void ThemeEditorPanel::Pimpl::apply_font_changes() {
        if (!is_locked && context && context->gui && active_data()) context->gui->request_theme_reload();
    }

#ifdef SALIX_TESTS_ENABLED
    bool ThemeEditorPanel::choose_font_family_for_testing(const std::string& family) { return pimpl->choose_font_family(pimpl->active_data(), family); }
    bool ThemeEditorPanel::choose_font_size_for_testing(float size) { return pimpl->choose_font_size(pimpl->active_data(), size); }
    void ThemeEditorPanel::apply_font_changes_for_testing() { pimpl->apply_font_changes(); }
#endif

    void ThemeEditorPanel::Pimpl::render_edit_fonts(ImGuiThemeData* data) {
        if (ImGui::TreeNode("Fonts")) {
            ImGuiFontManager* font_manager = dynamic_cast<ImGuiFontManager*>(context->font_manager);
            if (!font_manager || !data) {
                ImGui::Text("Error: Font Manager or Theme Data not available.");
                ImGui::TreePop();
                return;
            }

            // --- FONT FAMILY DROPDOWN ---
            std::vector<std::string> families = font_manager->get_unique_font_families();
            int family_idx = -1;
            for (int i = 0; i < families.size(); i++) {
                if (families[i] == data->font_family) {
                    family_idx = i;
                    break;
                }
            }

            auto family_getter = [](void* vec, int idx, const char** out_text) {
                auto& vector = *static_cast<std::vector<std::string>*>(vec);
                if (idx < 0 || idx >= vector.size()) { return false; }
                *out_text = vector.at(idx).c_str();
                return true;
            };

            if (ImGui::Combo("Font Family", &family_idx, family_getter, static_cast<void*>(&families), families.size())) {
                if (family_idx >= 0 && static_cast<size_t>(family_idx) < families.size()) choose_font_family(data, families[family_idx]);
            }

            // --- FONT SIZE DROPDOWN ---
            std::vector<float> sizes = font_manager->get_font_sizes_for_font_family(data->font_family);
            int size_idx = -1;
            
            std::vector<std::string> size_strings;
            size_strings.reserve(sizes.size());
            for (int i = 0; i < sizes.size(); i++) {
                size_strings.push_back(std::to_string(static_cast<int>(sizes[i])) + "px");
                if (std::abs(sizes[i] - data->font_size) < 0.001f) {
                    size_idx = i;
                }
            }
            
            std::vector<const char*> size_c_strs;
            size_c_strs.reserve(size_strings.size());
            for(const auto& s : size_strings) {
                size_c_strs.push_back(s.c_str());
            }

            if (ImGui::Combo("Font Size", &size_idx, size_c_strs.data(), size_c_strs.size())) {
                if (size_idx >= 0 && static_cast<size_t>(size_idx) < sizes.size()) choose_font_size(data, sizes[size_idx]);
            }

            // --- APPLY BUTTON ---
            if (ImGui::Button("Apply Font Changes")) apply_font_changes();
            ImGui::TextDisabled("Changes apply on next frame after applying.");
            
            ImGui::TreePop();
        }
    }


    void ThemeEditorPanel::Pimpl::browse_icon(const std::string& type_name) {
        if (!context || !context->gui || !context->theme_manager || type_name.empty()) return;
        auto* theme = context->theme_manager->get_active_theme();
        if (!theme) return;
        auto* dialog = context->gui->get_dialog("SelectIconImage");
        if (!dialog) return;
        // Capture the registry key, not a borrowed ImGuiThemeData pointer.
        const std::string theme_name = theme->get_name();
        const unsigned request = ++icon_request;
        std::weak_ptr<int> lifetime = callback_lifetime;
        icon_type_being_edited = type_name;
        dialog->set_callback([this, lifetime, theme_name, type_name, request](const FileDialogResult& result) {
            if (lifetime.expired() || request != icon_request) return;
            ++icon_request; // Consume this request even when cancelled.
            icon_type_being_edited.clear();
            if (!result.is_ok || result.file_path_name.empty() || !context || !context->theme_manager || !context->gui) return;
            auto* current = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
            // A selection belongs to the theme that was active when Browse was pressed.
            if (!current || current->get_name() != theme_name || !current->get_data()) return;
            auto relative_path = FileManager::convert_to_relative(std::filesystem::current_path().string(), result.file_path_name);
            current->get_data()->icon_paths[type_name] = relative_path;
            last_icon_directory = result.folder_path;
            context->gui->request_theme_reload();
        });
        dialog->set_default_path(last_icon_directory);
        std::string key = "SelectIconImage";
        context->gui->show_dialog_by_key(key);
    }

#ifdef SALIX_TESTS_ENABLED
    void ThemeEditorPanel::browse_icon_for_testing(const std::string& type_name) { pimpl->browse_icon(type_name); }
#endif

    void ThemeEditorPanel::Pimpl::render_edit_icons(ImGuiThemeData* data) {
        
        if (ImGui::CollapsingHeader("Icon Theme")) {
            if (!context->init_context || !context->init_context->icon_manager) {
                ImGui::TextUnformatted("Icon manager unavailable.");
                return;
            }
            if (ImGui::BeginTable("icon_table", 3)) {
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& [type_name, icon_info] : context->init_context->icon_manager->get_icon_registry()) {
                    ImGui::TableNextRow();

                    // Column 1: Type Name
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", type_name.c_str());


                    // Column 2: Icon Preview
                    ImVec2 icon_size = ImVec2(16, 16);
                    ImTextureID icon_tex_id = icon_info.texture_id;
                    ImVec2 top_left = ImVec2(0, 0);
                    ImVec2 bottom_right = ImVec2(1, 1);
                    if (context->init_context->renderer_type == RendererType::OpenGL) {
                        top_left = ImVec2(0, 1);
                        bottom_right = ImVec2 (1, 0);
                    }
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Image(icon_tex_id, icon_size, top_left, bottom_right);
                  

                    // Column 3: Path and Browse Button
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushItemWidth(-70.0f); // Leave space for the button
                    // Create a temporary, larger buffer for ImGui
                    char path_buffer[512];
                    // Safely copy the string into the buffer
                    strncpy_s(path_buffer, sizeof(path_buffer), icon_info.path.c_str(), _TRUNCATE);
                    ImGui::InputText(("##Path" + type_name).c_str(), path_buffer, sizeof(path_buffer), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    if (ImGui::Button(("Browse##" + type_name).c_str())) {
                        browse_icon(type_name);
                    }
                }
                ImGui::EndTable();
            }
        }
    }






    bool ThemeEditorPanel::is_locked() {
        return pimpl->is_locked;
    }


    void ThemeEditorPanel::unlock() {
        pimpl->is_locked = false;
    }


    void ThemeEditorPanel::lock() {
        pimpl->is_locked = true;
    }   
}