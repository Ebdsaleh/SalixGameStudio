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
        std::string last_icon_directory = ".";
        // Helper functions for rendering UI sections
        void create_dialog_boxes();
        void render_layout();
        void save_current_theme(ImGuiThemeData* current_theme_data); 
        void load_selected_theme(ImGuiThemeData* data);
        void render_edit_colors(ImGuiThemeData* data);
        void render_edit_sizing_and_spacing(ImGuiThemeData* data);
        void render_edit_fonts(ImGuiThemeData* data);
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
        pimpl->icon_manager = dynamic_cast<ImGuiIconManager*>(pimpl->context->gui->get_icon_manager());
        if (!pimpl->icon_manager) { return; }
        pimpl->create_dialog_boxes();
    }
    
    void ThemeEditorPanel::on_gui_update() {
        if (!pimpl->is_visible) return;

        // Use RAII to handle disabled state safely
        struct DisabledGuard {
            bool active;
            DisabledGuard(bool should_disable) : active(should_disable) { 
                if (active) ImGui::BeginDisabled(); 
            }
            ~DisabledGuard() { 
                if (active) ImGui::EndDisabled(); 
            }
        } disabled_guard(pimpl->is_locked);

        // Early return checks AFTER setting up the guard
        ITheme* active_theme_base = pimpl->context->theme_manager->get_active_theme();
        ImGuiTheme* active_imgui_theme = dynamic_cast<ImGuiTheme*>(active_theme_base);

        if (!active_imgui_theme) {
            ImGui::Text("No active ImGui theme to edit.");
            return; // Guard will auto-cleanup disabled state
        }

        ImGuiThemeData* data = active_imgui_theme->get_data();
        if (!data) {
            ImGui::Text("Active theme has no data.");
            return; // Guard will auto-cleanup disabled state
        }
        
        
        
        // Main panel rendering
        pimpl->render_layout();
    }

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

        select_icon_image_dialog->set_filters(std::string("Image File (*.png){.png},All files (*.*){.*}"));

        

    }

   

    void ThemeEditorPanel::Pimpl::render_layout() {
        if (ImGui::Begin(name.c_str(), &is_visible)) {
            // handle icon adjustment for OpenGL rendering
                ImVec2 icon_size = ImVec2(16, 16);
                ImVec2 top_left = ImVec2(0, 0);
                ImVec2 bottom_right = ImVec2(1, 1);

                ImGui::Separator();

                // --- PANEL LOCK/UNLOCK BUTTON ---
                ImGui::AlignTextToFramePadding();
                // ImGui::SameLine(

                const IconInfo& lock_icon = is_locked ? 
                    icon_manager->get_icon_by_name("Panel Locked") :
                    icon_manager->get_icon_by_name("Panel Unlocked");

                ImVec4 tint_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white (no tint)
                if (is_locked) {
                    tint_color = ImVec4(0.50f, 0.0f, 0.0f, 1.0f); // Red tint when locked
                }

                // Only attempt to draw if the icon texture ID is valid
                if (lock_icon.texture_id != 0) {
                    if (ImGui::ImageButton("##PanelLockBtn", lock_icon.texture_id, icon_size, top_left, bottom_right, ImVec4(0,0,0,0), tint_color)) {
                        // Button was clicked, toggle the lock state
                        is_locked = !is_locked; 
                        std::cout << "Theme Editor Panel lock toggled to: " << (is_locked ? "LOCKED" : "UNLOCKED") << std::endl;
                    }
                } 
                
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
                load_selected_theme(data);
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
        if (!context || !context->theme_manager) return;

        std::string theme_name = std::string(theme_name_buffer);
        std::string file_path = std::string(theme_path_buffer);

        if (context->theme_manager->load_theme_from_file(theme_name, file_path, false)) {
            context->theme_manager->set_active_theme(theme_name);
            context->gui->request_theme_reload(); 
            // After loading, get the newly active theme to refresh the UI text fields
            ImGuiTheme* new_active_theme = dynamic_cast<ImGuiTheme*>(context->theme_manager->get_active_theme());
            if (new_active_theme) {
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


    void ThemeEditorPanel::Pimpl::render_edit_icons(ImGuiThemeData* data) {
        
        if (ImGui::CollapsingHeader("Icon Theme")) {
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
                        // 1. Set the context for which icon we are editing.
                        icon_type_being_edited = type_name;

                        DialogBox* dialog = context->gui->get_dialog("SelectIconImage");
                        if (dialog) {
                            // 1. Set the context for which icon is being edited
                            icon_type_being_edited = type_name;

                            // 2. Set the callback HERE, capturing the current 'data' pointer
                            dialog->set_callback([this, data](const FileDialogResult& result) {
                            if (!icon_type_being_edited.empty()) {
                                if (result.is_ok) {
                                    // 1. Get the absolute path from the dialog.
                                    std::string absolute_path = result.file_path_name;

                                    // 2. Get the project's current working directory directly.
                                    std::string project_root = std::filesystem::current_path().string();

                                    // 3. Convert the absolute path to a clean, relative one using your new utility.
                                    std::string relative_path = FileManager::convert_to_relative(project_root, absolute_path);

                                    // 4. Update the theme data with the portable, RELATIVE path.
                                    data->icon_paths[icon_type_being_edited] = relative_path;
                                    
                                    // 5. Update the last-used directory for a better user experience.
                                    last_icon_directory = result.folder_path;

                                    // 6. Request a theme reload to make the UI update automatically.
                                    context->gui->request_theme_reload();
                                }
                                
                                // 7. Clear the state variable.
                                icon_type_being_edited.clear();
                            }
                            });
                            // 3. Set the dialog's starting path
                            dialog->set_default_path(last_icon_directory);

                            // 4. Show the dialog
                            std::string dialog_key = "SelectIconImage";
                            context->gui->show_dialog_by_key(dialog_key);
                        }
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