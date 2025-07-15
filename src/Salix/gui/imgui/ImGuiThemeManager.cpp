// Salix/gui/imgui/ImGuiThemeManager.cpp

#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/core/ApplicationConfig.h>
#include <imgui/imgui.h>
#include <fstream>
#include <filesystem>
#include <Salix/management/FileManager.h>

 

namespace Salix {

    // ---------------------- ImGuiThemeManager ----------------------
    struct ImGuiThemeManager::Pimpl {
        IGui* gui_system = nullptr;
        std::unordered_map<std::string, std::unique_ptr<ITheme>> theme_registry;
        std::string active_theme_name;
    };

    ImGuiThemeManager::ImGuiThemeManager() : pimpl(std::make_unique<Pimpl>()) {}
    
    ImGuiThemeManager::~ImGuiThemeManager() = default;



    bool ImGuiThemeManager::initialize(IGui* gui_system) {
        pimpl->gui_system = gui_system;
        return true;
    }

    void ImGuiThemeManager::shutdown() {
        pimpl->theme_registry.clear();
        pimpl->gui_system = nullptr;
    }

    bool ImGuiThemeManager::register_theme(std::unique_ptr<ITheme> theme) {
    const std::string& name = theme->get_name(); // Get name before moving
    if (pimpl->theme_registry.count(name)) {
        std::cerr << "ImGuiThemeManager Error: Theme '" << name << "' already registered." << std::endl;
        return false;
    }
    pimpl->theme_registry[name] = std::move(theme); // CORRECT: Just std::move(theme)
    std::cout << "ImGuiThemeManager: Theme '" << name << "' registered." << std::endl;
    return true;
}

    bool ImGuiThemeManager::unregister_theme(const std::string& theme_name) {
        return pimpl->theme_registry.erase(theme_name) > 0;
    }

    // --- UPDATED: apply_theme method now contains the logic to apply the theme data ---
    bool ImGuiThemeManager::apply_theme(const std::string& theme_name) {
        auto it = pimpl->theme_registry.find(theme_name);
        if (it == pimpl->theme_registry.end()) {
            std::cerr << "ImGuiThemeManager Error: Theme '" << theme_name << "' not found in registry." << std::endl;
            return false;
        }

        // Get the ITheme* from the registry
        ITheme* theme_interface = it->second.get();
        std::cout << "DEBUG: ImGuiThemeManager Current Working Directory: " << std::filesystem::current_path() << std::endl;
        // CRITICAL: Cast ITheme* to ImGuiTheme*
        ImGuiTheme* imgui_theme = dynamic_cast<ImGuiTheme*>(theme_interface);
        if (!imgui_theme) {
            std::cerr << "ImGuiThemeManager Error: Registered theme '" << theme_name << "' is not an ImGuiTheme." << std::endl;
            return false;
        }

        // Get the ImGuiThemeData from the ImGuiTheme
        ImGuiThemeData* data = imgui_theme->get_data();
        if (!data) {
            std::cerr << "ImGuiThemeManager Error: ImGuiTheme '" << theme_name << "' has null theme data." << std::endl;
            return false;
        }

        // Ensure GUI system is valid before applying theme
        if (!pimpl->gui_system) {
            std::cerr << "ImGuiThemeManager Error: GUI system reference is null, cannot apply theme." << std::endl;
            return false;
        }

        // --- Apply ImGuiThemeData to ImGuiStyle ---
        // START OF THE MONSTROSITY (as you called it) - THIS CODE GOES HERE
        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        // Apply Colors
        style.Colors[ImGuiCol_Text]                 = ImVec4(data->text_color.r, data->text_color.g, data->text_color.b, data->text_color.a);
        style.Colors[ImGuiCol_TextDisabled]         = ImVec4(data->text_disabled_color.r, data->text_disabled_color.g, data->text_disabled_color.b, data->text_disabled_color.a);
        style.Colors[ImGuiCol_WindowBg]             = ImVec4(data->window_bg_color.r, data->window_bg_color.g, data->window_bg_color.b, data->window_bg_color.a);
        style.Colors[ImGuiCol_ChildBg]              = ImVec4(data->child_bg_color.r, data->child_bg_color.g, data->child_bg_color.b, data->child_bg_color.a);
        style.Colors[ImGuiCol_PopupBg]              = ImVec4(data->popup_bg_color.r, data->popup_bg_color.g, data->popup_bg_color.b, data->popup_bg_color.a);
        style.Colors[ImGuiCol_Border]               = ImVec4(data->border_color.r, data->border_color.g, data->border_color.b, data->border_color.a);
        style.Colors[ImGuiCol_BorderShadow]         = ImVec4(data->border_shadow_color.r, data->border_shadow_color.g, data->border_shadow_color.b, data->border_shadow_color.a);
        style.Colors[ImGuiCol_FrameBg]              = ImVec4(data->frame_bg_color.r, data->frame_bg_color.g, data->frame_bg_color.b, data->frame_bg_color.a);
        style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(data->frame_bg_hovered_color.r, data->frame_bg_hovered_color.g, data->frame_bg_hovered_color.b, data->frame_bg_hovered_color.a);
        style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(data->frame_bg_active_color.r, data->frame_bg_active_color.g, data->frame_bg_active_color.b, data->frame_bg_active_color.a);
        style.Colors[ImGuiCol_TitleBg]              = ImVec4(data->title_bg_color.r, data->title_bg_color.g, data->title_bg_color.b, data->title_bg_color.a);
        style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(data->title_bg_active_color.r, data->title_bg_active_color.g, data->title_bg_active_color.b, data->title_bg_active_color.a);
        style.Colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(data->title_bg_collapsed_color.r, data->title_bg_collapsed_color.g, data->title_bg_collapsed_color.b, data->title_bg_collapsed_color.a);
        style.Colors[ImGuiCol_MenuBarBg]            = ImVec4(data->menu_bar_bg_color.r, data->menu_bar_bg_color.g, data->menu_bar_bg_color.b, data->menu_bar_bg_color.a);
        style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(data->scrollbar_bg_color.r, data->scrollbar_bg_color.g, data->scrollbar_bg_color.b, data->scrollbar_bg_color.a);
        style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(data->scrollbar_grab_color.r, data->scrollbar_grab_color.g, data->scrollbar_grab_color.b, data->scrollbar_grab_color.a);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(data->scrollbar_grab_hovered_color.r, data->scrollbar_grab_hovered_color.g, data->scrollbar_grab_hovered_color.b, data->scrollbar_grab_hovered_color.a);
        style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(data->scrollbar_grab_active_color.r, data->scrollbar_grab_active_color.g, data->scrollbar_grab_active_color.b, data->scrollbar_grab_active_color.a);
        style.Colors[ImGuiCol_CheckMark]            = ImVec4(data->checkmark_color.r, data->checkmark_color.g, data->checkmark_color.b, data->checkmark_color.a);
        style.Colors[ImGuiCol_SliderGrab]           = ImVec4(data->slider_grab_color.r, data->slider_grab_color.g, data->slider_grab_color.b, data->slider_grab_color.a);
        style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(data->slider_grab_active_color.r, data->slider_grab_active_color.g, data->slider_grab_active_color.b, data->slider_grab_active_color.a);
        style.Colors[ImGuiCol_Button]               = ImVec4(data->button_color.r, data->button_color.g, data->button_color.b, data->button_color.a);
        style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(data->button_hovered_color.r, data->button_hovered_color.g, data->button_hovered_color.b, data->button_hovered_color.a);
        style.Colors[ImGuiCol_ButtonActive]         = ImVec4(data->button_active_color.r, data->button_active_color.g, data->button_active_color.b, data->button_active_color.a);
        style.Colors[ImGuiCol_Header]               = ImVec4(data->header_color.r, data->header_color.g, data->header_color.b, data->header_color.a);
        style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(data->header_hovered_color.r, data->header_hovered_color.g, data->header_hovered_color.b, data->header_hovered_color.a);
        style.Colors[ImGuiCol_HeaderActive]         = ImVec4(data->header_active_color.r, data->header_active_color.g, data->header_active_color.b, data->header_active_color.a);
        style.Colors[ImGuiCol_Separator]            = ImVec4(data->separator_color.r, data->separator_color.g, data->separator_color.b, data->separator_color.a);
        style.Colors[ImGuiCol_SeparatorHovered]     = ImVec4(data->separator_hovered_color.r, data->separator_hovered_color.g, data->separator_hovered_color.b, data->separator_hovered_color.a);
        style.Colors[ImGuiCol_SeparatorActive]      = ImVec4(data->separator_active_color.r, data->separator_active_color.g, data->separator_active_color.b, data->separator_active_color.a);
        style.Colors[ImGuiCol_ResizeGrip]           = ImVec4(data->resize_grip_color.r, data->resize_grip_color.g, data->resize_grip_color.b, data->resize_grip_color.a);
        style.Colors[ImGuiCol_ResizeGripHovered]    = ImVec4(data->resize_grip_hovered_color.r, data->resize_grip_hovered_color.g, data->resize_grip_hovered_color.b, data->resize_grip_hovered_color.a);
        style.Colors[ImGuiCol_ResizeGripActive]     = ImVec4(data->resize_grip_active_color.r, data->resize_grip_active_color.g, data->resize_grip_active_color.b, data->resize_grip_active_color.a);
        style.Colors[ImGuiCol_Tab]                  = ImVec4(data->tab_color.r, data->tab_color.g, data->tab_color.b, data->tab_color.a);
        style.Colors[ImGuiCol_TabHovered]           = ImVec4(data->tab_hovered_color.r, data->tab_hovered_color.g, data->tab_hovered_color.b, data->tab_hovered_color.a);
        style.Colors[ImGuiCol_TabActive]            = ImVec4(data->tab_active_color.r, data->tab_active_color.g, data->tab_active_color.b, data->tab_active_color.a);
        style.Colors[ImGuiCol_TabUnfocused]         = ImVec4(data->tab_unfocused_color.r, data->tab_unfocused_color.g, data->tab_unfocused_color.b, data->tab_unfocused_color.a);
        style.Colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(data->tab_unfocused_active_color.r, data->tab_unfocused_active_color.g, data->tab_unfocused_active_color.b, data->tab_unfocused_active_color.a);
        style.Colors[ImGuiCol_DockingPreview]       = ImVec4(data->docking_preview_color.r, data->docking_preview_color.g, data->docking_preview_color.b, data->docking_preview_color.a);
        style.Colors[ImGuiCol_DockingEmptyBg]       = ImVec4(data->docking_empty_bg_color.r, data->docking_empty_bg_color.g, data->docking_empty_bg_color.b, data->docking_empty_bg_color.a);
        style.Colors[ImGuiCol_PlotLines]            = ImVec4(data->plot_lines_color.r, data->plot_lines_color.g, data->plot_lines_color.b, data->plot_lines_color.a);
        style.Colors[ImGuiCol_PlotLinesHovered]     = ImVec4(data->plot_lines_hovered_color.r, data->plot_lines_hovered_color.g, data->plot_lines_hovered_color.b, data->plot_lines_hovered_color.a);
        style.Colors[ImGuiCol_PlotHistogram]        = ImVec4(data->plot_histogram_color.r, data->plot_histogram_color.g, data->plot_histogram_color.b, data->plot_histogram_color.a);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(data->plot_histogram_hovered_color.r, data->plot_histogram_hovered_color.g, data->plot_histogram_hovered_color.b, data->plot_histogram_hovered_color.a);
        style.Colors[ImGuiCol_TableHeaderBg]        = ImVec4(data->table_header_bg_color.r, data->table_header_bg_color.g, data->table_header_bg_color.b, data->table_header_bg_color.a);
        style.Colors[ImGuiCol_TableBorderStrong]    = ImVec4(data->table_border_strong_color.r, data->table_border_strong_color.g, data->table_border_strong_color.b, data->table_border_strong_color.a);
        style.Colors[ImGuiCol_TableBorderLight]     = ImVec4(data->table_border_light_color.r, data->table_border_light_color.g, data->table_border_light_color.b, data->table_border_light_color.a);
        style.Colors[ImGuiCol_TableRowBg]           = ImVec4(data->table_row_bg_color.r, data->table_row_bg_color.g, data->table_row_bg_color.b, data->table_row_bg_color.a);
        style.Colors[ImGuiCol_TableRowBgAlt]        = ImVec4(data->table_row_bg_alt_color.r, data->table_row_bg_alt_color.g, data->table_row_bg_alt_color.b, data->table_row_bg_alt_color.a);
        style.Colors[ImGuiCol_TextSelectedBg]       = ImVec4(data->text_selected_bg_color.r, data->text_selected_bg_color.g, data->text_selected_bg_color.b, data->text_selected_bg_color.a);
        style.Colors[ImGuiCol_DragDropTarget]       = ImVec4(data->drag_drop_target_color.r, data->drag_drop_target_color.g, data->drag_drop_target_color.b, data->drag_drop_target_color.a);
        style.Colors[ImGuiCol_NavHighlight]         = ImVec4(data->nav_highlight_color.r, data->nav_highlight_color.g, data->nav_highlight_color.b, data->nav_highlight_color.a);
        style.Colors[ImGuiCol_NavWindowingHighlight]= ImVec4(data->nav_windowing_highlight_color.r, data->nav_windowing_highlight_color.g, data->nav_windowing_highlight_color.b, data->nav_windowing_highlight_color.a);
        style.Colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(data->nav_windowing_dim_color.r, data->nav_windowing_dim_color.g, data->nav_windowing_dim_color.b, data->nav_windowing_dim_color.a);
        style.Colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(data->modal_window_dim_color.r, data->modal_window_dim_color.g, data->modal_window_dim_color.b, data->modal_window_dim_color.a);

        // --- Apply float style variables ---
        style.Alpha                                 = data->alpha;
        style.WindowRounding                        = data->window_rounding;
        style.WindowBorderSize                      = data->window_border_size;
        style.WindowMenuButtonPosition              = (ImGuiDir)data->window_menu_button_position; // Cast enum
        style.ChildRounding                         = data->child_rounding;
        style.ChildBorderSize                       = data->child_border_size;
        style.PopupRounding                         = data->popup_rounding;
        style.PopupBorderSize                       = data->popup_border_size;
        style.FrameRounding                         = data->frame_rounding;
        style.FrameBorderSize                       = data->frame_border_size;
        style.IndentSpacing                         = data->indent_spacing;
        style.ColumnsMinSpacing                     = data->columns_min_spacing;
        style.ScrollbarSize                         = data->scrollbar_size;
        style.ScrollbarRounding                     = data->scrollbar_rounding;
        style.GrabMinSize                           = data->grab_min_size;
        style.GrabRounding                          = data->grab_rounding;
        style.LogSliderDeadzone                     = data->log_slider_deadzone;
        style.ImageBorderSize                       = data->image_border_size;
        style.TabRounding                           = data->tab_rounding;
        style.TabBorderSize                         = data->tab_border_size;
        style.TabCloseButtonMinWidthSelected        = data->tab_close_button_min_width_selected;
        style.TabCloseButtonMinWidthUnselected      = data->tab_close_button_min_width_unselected;
        style.TabBarBorderSize                      = data->tab_bar_border_size;
        style.TabBarOverlineSize                    = data->tab_bar_overline_size;
        style.TableAngledHeadersAngle               = data->table_angled_headers_angle;
        style.TreeLinesSize                         = data->tree_lines_size;
        style.TreeLinesRounding                     = data->tree_lines_rounding;
        style.ColorButtonPosition                   = (ImGuiDir)data->color_button_position; // Cast enum
        style.SeparatorTextBorderSize               = data->separator_text_border_size;
        style.DockingSeparatorSize                  = data->docking_separator_size;
        style.MouseCursorScale                      = data->mouse_cursor_scale;
        style.AntiAliasedLines                      = data->anti_aliased_lines;
        style.AntiAliasedLinesUseTex                = data->anti_aliased_lines_use_tex;
        style.AntiAliasedFill                       = data->anti_aliased_fill;
        style.CurveTessellationTol                  = data->curve_tessellation_tol;
        style.CircleTessellationMaxError            = data->circle_tessellation_max_error;
        style.HoverStationaryDelay                  = data->hover_stationary_delay;
        style.HoverDelayShort                       = data->hover_delay_short;
        style.HoverDelayNormal                      = data->hover_delay_normal;
        style.HoverFlagsForTooltipMouse             = (ImGuiHoveredFlags)data->hover_flags_for_tooltip_mouse; // Cast enum
        style.HoverFlagsForTooltipNav               = (ImGuiHoveredFlags)data->hover_flags_for_tooltip_nav;   // Cast enum

        // --- Apply ImVec2 style variables ---
        style.WindowPadding                         = ImVec2(data->window_padding.x, data->window_padding.y);
        style.WindowMinSize                         = ImVec2(data->window_min_size.x, data->window_min_size.y);
        style.WindowTitleAlign                      = ImVec2(data->window_title_align.x, data->window_title_align.y);
        style.FramePadding                          = ImVec2(data->frame_padding.x, data->frame_padding.y);
        style.ItemSpacing                           = ImVec2(data->item_spacing.x, data->item_spacing.y);
        style.ItemInnerSpacing                      = ImVec2(data->item_inner_spacing.x, data->item_inner_spacing.y);
        style.CellPadding                           = ImVec2(data->cell_padding.x, data->cell_padding.y);
        style.TouchExtraPadding                     = ImVec2(data->touch_extra_padding.x, data->touch_extra_padding.y);
        style.TableAngledHeadersTextAlign           = ImVec2(data->table_angled_headers_text_align.x, data->table_angled_headers_text_align.y);
        style.ButtonTextAlign                       = ImVec2(data->button_text_align.x, data->button_text_align.y);
        style.SelectableTextAlign                   = ImVec2(data->selectable_text_align.x, data->selectable_text_align.y);
        style.SeparatorTextAlign                    = ImVec2(data->separator_text_align.x, data->separator_text_align.y);
        style.SeparatorTextPadding                  = ImVec2(data->separator_text_padding.x, data->separator_text_padding.y);
        style.DisplayWindowPadding                  = ImVec2(data->display_window_padding.x, data->display_window_padding.y);
        style.DisplaySafeAreaPadding                = ImVec2(data->display_safe_area_padding.x, data->display_safe_area_padding.y);

        // --- Fonts ---

        // Ensure FontGlobalScale is valid (default to 1.0f)
        style.FontScaleMain = data->font_scale_main;

        // Set style's base font size
        style.FontSizeBase = data->default_font_size;


        ImFont* loaded_font = nullptr;
        if (!data->default_font_path.empty()) {
            loaded_font = io.Fonts->AddFontFromFileTTF(data->default_font_path.c_str(), data->default_font_size);
            if (!loaded_font) {
                std::cerr << "ImGuiThemeManager Error: Failed to load font from file: " << data->default_font_path << ". Falling back to default font." << std::endl;
            }
        }

        if (!loaded_font) {
            loaded_font = io.Fonts->AddFontDefault(); // Fallback to default ImGui font
            std::cout << "ImGuiThemeManager: Using default ImGui font." << std::endl;
        } else {
            std::cout << "ImGuiThemeManager: Loaded font from file: " << data->default_font_path << std::endl;
        }

        io.Fonts->Build(); // Rebuild font atlas

        
        // CRITICAL FIX: After building, explicitly set io.FontDefault and push/pop it.
        // This is the 1.92.x way to ensure the default font is active and internal state is correct.
        if (loaded_font) {
            io.FontDefault = loaded_font; // Set this font as the default for the context

            // Push and Pop the font to ensure ImGui's internal g.Font and g.FontSize are updated.
            // The size parameter for PushFont should be the size at which you want to render the font.
            // Using loaded_font->LegacySize is a common choice for fixed-size fonts,
            // or data->default_font_size if you want to explicitly use that size.
            ImGui::PushFont(loaded_font, data->default_font_size); // Push with desired rendering size
            ImGui::PopFont();
        } else {
            std::cerr << "ImGuiThemeManager Error: No font loaded after build. This is critical." << std::endl;
            return false; // Fatal if no font is loaded
        }

        if (pimpl->gui_system && pimpl->gui_system->get_font_manager()) {
            pimpl->gui_system->get_font_manager()->rebuild_font_atlas_texture();
            std::cout << "ImGuiThemeManager::apply_font - Success on call 'rebuild_font_atlas_texture()', congratulations!" <<
                std::endl;
        } else {
            std::cerr << "ImGuiThemeManager::apply_font - Failed on call 'rebuild_font_atlas_texture()'" <<
                std::endl;
            
        }
        std::cout << "Theme '" << data->name << "' applied to ImGui." << std::endl;
        return true;
    }



    std::vector<std::string> ImGuiThemeManager::get_registered_themes() const {
        std::vector<std::string> names;
        for (const auto& [key, _] : pimpl->theme_registry)
            names.push_back(key);
        return names;
    }

    ITheme* ImGuiThemeManager::get_theme(const std::string& theme_name) const {
        auto it = pimpl->theme_registry.find(theme_name);
        return (it != pimpl->theme_registry.end()) ? it->second.get() : nullptr;
    }

    // Not implemented yet as serialization format is undecided (e.g., JSON, YAML, custom, etc.).
    bool ImGuiThemeManager::load_theme_from_file(const std::string& file_path, bool apply_immediately) {
        if (!FileManager::path_exists(file_path)) {
            return false; // File doesn't exist
        }
        if(apply_immediately) {}


        // TODO: Implement theme deserialization when format is chosen
        // For now, this stub always fails.
        return false;
    }

    bool ImGuiThemeManager::load_themes_from_directory(const std::string& directory_path) {
        if (!FileManager::path_exists(directory_path)) return false;

        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".theme") {
                load_theme_from_file(entry.path().string());
            }
        }
        return true;
    }

    bool ImGuiThemeManager::set_active_theme(const std::string& theme_name) {
        return apply_theme(theme_name);
    }

    bool ImGuiThemeManager::purge_theme(const std::string& theme_name) {
        if (pimpl->active_theme_name == theme_name)
            pimpl->active_theme_name.clear();
        return unregister_theme(theme_name);
    }

} // namespace Salix
