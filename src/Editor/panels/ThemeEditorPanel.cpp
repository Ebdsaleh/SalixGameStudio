#include <Editor/panels/ThemeEditorPanel.h>
#include <Editor/EditorContext.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h> 
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <imgui/imgui.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace Salix {

    struct ThemeEditorPanel::Pimpl {
        std::string name = "Theme Editor";
        EditorContext* context = nullptr;
        bool is_visible = false;        
        char theme_name_buffer[256] = {0};
        char theme_path_buffer[512] = {0};

        // Helper functions for rendering UI sections
        void render_layout();
        void save_current_theme(ImGuiThemeData* current_theme_data); 
        void load_selected_theme();
        void render_edit_colors(ImGuiThemeData* data);
        void render_edit_sizing_and_spacing(ImGuiThemeData* data);
        void render_edit_fonts(ImGuiThemeData* data);
    };

    ThemeEditorPanel::ThemeEditorPanel() : pimpl(std::make_unique<Pimpl>()) {}
    ThemeEditorPanel::~ThemeEditorPanel() = default;

    void ThemeEditorPanel::initialize(EditorContext* context) {
        if (!context) {
            std::cerr << "ThemeEditorPanel::initialize - EditorContext is NULL!" << std::endl;
            return;
        }
        pimpl->context = context;
    }
    
    void ThemeEditorPanel::on_gui_render() {
        if (!pimpl->is_visible) return;
        pimpl->render_layout();
    }

    void ThemeEditorPanel::set_visibility(bool visibility) { pimpl->is_visible = visibility; }
    bool ThemeEditorPanel::get_visibility() const { return pimpl->is_visible; }
    void ThemeEditorPanel::set_name(const std::string& new_name) { pimpl->name = new_name; }
    const std::string& ThemeEditorPanel::get_name() { return pimpl->name; }
    void ThemeEditorPanel::on_event(IEvent& event) { (void) event; }

    void ThemeEditorPanel::Pimpl::render_layout() {
        if (ImGui::Begin(name.c_str(), &is_visible)) {
            ITheme* active_theme_base = context->theme_manager->get_active_theme();
            ImGuiTheme* active_imgui_theme = dynamic_cast<ImGuiTheme*>(active_theme_base);

            if (!active_imgui_theme) {
                ImGui::Text("No active ImGui theme to edit.");
                ImGui::End();
                return;
            }
            
            ImGuiThemeData* data = active_imgui_theme->get_data();
            if (!data) {
                ImGui::Text("Active theme has no data.");
                ImGui::End();
                return;
            }

            // Sync UI buffers with theme data if the window is just appearing
            if (ImGui::IsWindowAppearing()) { 
                std::strncpy(theme_name_buffer, active_imgui_theme->get_name().c_str(), sizeof(theme_name_buffer) - 1);
                std::strncpy(theme_path_buffer, active_imgui_theme->get_file_path().c_str(), sizeof(theme_path_buffer) - 1);
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
                load_selected_theme();
            }
            ImGui::Separator();

            render_edit_colors(data);
            render_edit_sizing_and_spacing(data);
            render_edit_fonts(data); 
            
            ImGui::End();
        }
    }

    void ThemeEditorPanel::Pimpl::load_selected_theme() {
        if (!context || !context->theme_manager) return;

        std::string theme_name = std::string(theme_name_buffer);
        std::string file_path = std::string(theme_path_buffer);

        if (context->theme_manager->load_theme_from_file(theme_name, file_path, false)) {
            context->theme_manager->set_active_theme(theme_name);
            context->gui->request_theme_reload(); 
        } else {
            std::cerr << "ThemeEditorPanel: Failed to load theme '" << theme_name << "'." << std::endl;
        }
    }

    void ThemeEditorPanel::Pimpl::save_current_theme(ImGuiThemeData* data) {
        if (!context || !context->theme_manager || !data) return;
        
        std::string file_path = std::string(theme_path_buffer);
        context->theme_manager->save_theme_to_file(data->name, file_path);
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
                data->font_family = families[family_idx];
                std::vector<float> available_sizes = font_manager->get_font_sizes_for_font_family(data->font_family);
                if (!available_sizes.empty()) {
                    data->font_size = available_sizes[0];
                }
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
                data->font_size = sizes[size_idx];
            }

            // --- APPLY BUTTON ---
            if (ImGui::Button("Apply Font Changes")) {
                context->gui->request_theme_reload();
            }
            ImGui::TextDisabled("Changes apply on next frame after applying.");
            
            ImGui::TreePop();
        }
    }
}
