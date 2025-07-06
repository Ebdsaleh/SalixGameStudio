// Salix/gui/imgui/ImGuiThemeData.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IThemeData.h> // Inherit from abstract base
#include <Salix/math/Color.h>     // For Salix::Color (assuming it's defined and SALIX_API)
#include <Salix/math/Vector2.h>
#include <string>

namespace Salix {

    // This struct defines the concrete data for an ImGui theme.
    // It inherits from IThemeData and provides ImGui-specific styling properties.
    // This is the data that will be saved to and loaded from .theme files.
    struct SALIX_API ImGuiThemeData : public IThemeData {

        ImGuiThemeData() {
            name =  "Default ImGui Theme";
            gui_type = "ImGui";
        }

        ImGuiThemeData(const std::string theme_name) {
            name = theme_name;
            gui_type = "ImGui";
        }
        std::string gui_type;
        // --- IThemeData overrides ---
        // Implement the pure virtual functions from IThemeData
        const std::string& get_name() const override { return name; }
        const std::string& get_gui_type() const override { return gui_type; } // Specific GUI type

        // --- ImGui-specific properties with default values ---
        std::string name; // Default name for this theme data

        // Colors (mapping to ImGuiCol_ enum values conceptually)
        Salix::Color text_color                        = {1.0f, 1.0f, 1.0f, 1.0f};     // ImGuiCol_Text
        Salix::Color text_disabled_color               = {0.5f, 0.5f, 0.5f, 1.0f};     // ImGuiCol_TextDisabled
        Salix::Color window_bg_color                   = {0.1f, 0.1f, 0.1f, 0.94f};    // ImGuiCol_WindowBg
        Salix::Color child_bg_color                    = {0.0f, 0.0f, 0.0f, 0.0f};     // ImGuiCol_ChildBg
        Salix::Color popup_bg_color                    = {0.08f, 0.08f, 0.08f, 0.94f}; // ImGuiCol_PopupBg
        Salix::Color border_color                      = {0.43f, 0.43f, 0.5f, 0.5f};   // ImGuiCol_Border
        Salix::Color border_shadow_color               = {0.0f, 0.0f, 0.0f, 0.0f};     // ImGuiCol_BorderShadow
        Salix::Color frame_bg_color                    = {0.16f, 0.29f, 0.48f, 0.54f}; // ImGuiCol_FrameBg
        Salix::Color frame_bg_hovered_color            = {0.26f, 0.59f, 0.98f, 0.40f}; // ImGuiCol_FrameBgHovered
        Salix::Color frame_bg_active_color             = {0.26f, 0.59f, 0.98f, 0.67f}; // ImGuiCol_FrameBgActive
        Salix::Color title_bg_color                    = {0.04f, 0.04f, 0.04f, 1.0f};  // ImGuiCol_TitleBg
        Salix::Color title_bg_active_color             = {0.16f, 0.29f, 0.48f, 1.0f};  // ImGuiCol_TitleBgActive
        Salix::Color title_bg_collapsed_color          = {0.0f, 0.0f, 0.0f, 0.51f};    // ImGuiCol_TitleBgCollapsed
        Salix::Color menu_bar_bg_color                 = {0.14f, 0.14f, 0.14f, 1.0f};  // ImGuiCol_MenuBarBg
        Salix::Color scrollbar_bg_color                = {0.02f, 0.02f, 0.02f, 0.53f}; // ImGuiCol_ScrollbarBg
        Salix::Color scrollbar_grab_color              = {0.31f, 0.31f, 0.31f, 1.0f};  // ImGuiCol_ScrollbarGrab
        Salix::Color scrollbar_grab_hovered_color      = {0.41f, 0.41f, 0.41f, 1.0f};  // ImGuiCol_ScrollbarGrabHovered
        Salix::Color scrollbar_grab_active_color       = {0.51f, 0.51f, 0.51f, 1.0f};  // ImGuiCol_ScrollbarGrabActive
        Salix::Color checkmark_color                   = {0.26f, 0.59f, 0.98f, 1.0f};  // ImGuiCol_CheckMark
        Salix::Color slider_grab_color                 = {0.24f, 0.52f, 0.88f, 1.0f};  // ImGuiCol_SliderGrab
        Salix::Color slider_grab_active_color          = {0.26f, 0.59f, 0.98f, 1.0f};  // ImGuiCol_SliderGrabActive
        Salix::Color button_color                      = {0.26f, 0.59f, 0.98f, 0.40f}; // ImGuiCol_Button
        Salix::Color button_hovered_color              = {0.26f, 0.59f, 0.98f, 1.0f};  // ImGuiCol_ButtonHovered
        Salix::Color button_active_color               = {0.06f, 0.53f, 0.98f, 1.0f};  // ImGuiCol_ButtonActive
        Salix::Color header_color                      = {0.26f, 0.59f, 0.98f, 0.31f}; // ImGuiCol_Header
        Salix::Color header_hovered_color              = {0.26f, 0.59f, 0.98f, 0.80f}; // ImGuiCol_HeaderHovered
        Salix::Color header_active_color               = {0.26f, 0.59f, 0.98f, 1.0f};  // ImGuiCol_HeaderActive
        Salix::Color separator_color                   = {0.43f, 0.43f, 0.5f, 0.50f};  // ImGuiCol_Separator
        Salix::Color separator_hovered_color           = {0.10f, 0.40f, 0.75f, 0.78f}; // ImGuiCol_SeparatorHovered
        Salix::Color separator_active_color            = {0.10f, 0.40f, 0.75f, 1.0f};  // ImGuiCol_SeparatorActive
        Salix::Color resize_grip_color                 = {0.26f, 0.59f, 0.98f, 0.25f}; // ImGuiCol_ResizeGrip
        Salix::Color resize_grip_hovered_color         = {0.26f, 0.59f, 0.98f, 0.67f}; // ImGuiCol_ResizeGripHovered
        Salix::Color resize_grip_active_color          = {0.26f, 0.59f, 0.98f, 0.95f}; // ImGuiCol_ResizeGripActive
        Salix::Color tab_color                         = {0.18f, 0.35f, 0.58f, 0.86f}; // ImGuiCol_Tab
        Salix::Color tab_hovered_color                 = {0.26f, 0.59f, 0.98f, 0.80f}; // ImGuiCol_TabHovered
        Salix::Color tab_active_color                  = {0.20f, 0.41f, 0.68f, 1.0f};  // ImGuiCol_TabActive
        Salix::Color tab_unfocused_color               = {0.07f, 0.10f, 0.15f, 0.97f}; // ImGuiCol_TabUnfocused
        Salix::Color tab_unfocused_active_color        = {0.14f, 0.26f, 0.42f, 1.0f};  // ImGuiCol_TabUnfocusedActive
        Salix::Color docking_preview_color             = {0.26f, 0.59f, 0.98f, 0.7f};  // ImGuiCol_DockingPreview
        Salix::Color docking_empty_bg_color            = {0.2f, 0.2f, 0.2f, 1.0f};     // ImGuiCol_DockingEmptyBg
        Salix::Color plot_lines_color                  = {0.61f, 0.61f, 0.61f, 1.0f};  // ImGuiCol_PlotLines
        Salix::Color plot_lines_hovered_color          = {1.0f, 0.43f, 0.35f, 1.0f};   // ImGuiCol_PlotLinesHovered
        Salix::Color plot_histogram_color              = {0.90f, 0.70f, 0.0f, 1.0f};   // ImGuiCol_PlotHistogram
        Salix::Color plot_histogram_hovered_color      = {1.0f, 0.60f, 0.0f, 1.0f};    // ImGuiCol_PlotHistogramHovered
        Salix::Color table_header_bg_color             = {0.19f, 0.19f, 0.2f, 1.0f};   // ImGuiCol_TableHeaderBg
        Salix::Color table_border_strong_color         = {0.31f, 0.31f, 0.35f, 1.0f};  // ImGuiCol_TableBorderStrong
        Salix::Color table_border_light_color          = {0.23f, 0.23f, 0.25f, 1.0f};  // ImGuiCol_TableBorderLight
        Salix::Color table_row_bg_color                = {0.0f, 0.0f, 0.0f, 0.0f};     // ImGuiCol_TableRowBg
        Salix::Color table_row_bg_alt_color            = {1.0f, 1.0f, 1.0f, 0.06f};    // ImGuiCol_TableRowBgAlt
        Salix::Color text_selected_bg_color            = {0.26f, 0.59f, 0.98f, 0.35f}; // ImGuiCol_TextSelectedBg
        Salix::Color drag_drop_target_color            = {1.0f, 1.0f, 0.0f, 0.90f};    // ImGuiCol_DragDropTarget
        Salix::Color nav_highlight_color               = {0.26f, 0.59f, 0.98f, 1.0f};  // ImGuiCol_NavHighlight
        Salix::Color nav_windowing_highlight_color     = {1.0f, 1.0f, 1.0f, 0.70f};    // ImGuiCol_NavWindowingHighlight
        Salix::Color nav_windowing_dim_color           = {0.80f, 0.80f, 0.80f, 0.20f}; // ImGuiCol_NavWindowingDimBg
        Salix::Color modal_window_dim_color            = {0.80f, 0.80f, 0.80f, 0.35f}; // ImGuiCol_ModalWindowDimBg

        std::vector<std::pair<std::string, Salix::Color*>> theme_colors = {
            { "Text", &text_color },
            { "TextDisabled", &text_disabled_color },
            { "WindowBg", &window_bg_color },
            { "ChildBg", &child_bg_color },
            { "PopupBg", &popup_bg_color },
            { "Border", &border_color },
            { "BorderShadow", &border_shadow_color },
            { "FrameBg", &frame_bg_color },
            { "FrameBgHovered", &frame_bg_hovered_color },
            { "FrameBgActive", &frame_bg_active_color },
            { "TitleBg", &title_bg_color },
            { "TitleBgActive", &title_bg_active_color },
            { "TitleBgCollapsed", &title_bg_collapsed_color },
            { "MenuBarBg", &menu_bar_bg_color },
            { "ScrollbarBg", &scrollbar_bg_color },
            { "ScrollbarGrab", &scrollbar_grab_color },
            { "ScrollbarGrabHovered", &scrollbar_grab_hovered_color },
            { "ScrollbarGrabActive", &scrollbar_grab_active_color },
            { "CheckMark", &checkmark_color },
            { "SliderGrab", &slider_grab_color },
            { "SliderGrabActive", &slider_grab_active_color },
            { "Button", &button_color },
            { "ButtonHovered", &button_hovered_color },
            { "ButtonActive", &button_active_color },
            { "Header", &header_color },
            { "HeaderHovered", &header_hovered_color },
            { "HeaderActive", &header_active_color },
            { "Separator", &separator_color },
            { "SeparatorHovered", &separator_hovered_color },
            { "SeparatorActive", &separator_active_color },
            { "ResizeGrip", &resize_grip_color },
            { "ResizeGripHovered", &resize_grip_hovered_color },
            { "ResizeGripActive", &resize_grip_active_color },
            { "Tab", &tab_color },
            { "TabHovered", &tab_hovered_color },
            { "TabActive", &tab_active_color },
            { "TabUnfocused", &tab_unfocused_color },
            { "TabUnfocusedActive", &tab_unfocused_active_color },
            { "DockingPreview", &docking_preview_color },
            { "DockingEmptyBg", &docking_empty_bg_color },
            { "PlotLines", &plot_lines_color },
            { "PlotLinesHovered", &plot_lines_hovered_color },
            { "PlotHistogram", &plot_histogram_color },
            { "PlotHistogramHovered", &plot_histogram_hovered_color },
            { "TableHeaderBg", &table_header_bg_color },
            { "TableBorderStrong", &table_border_strong_color },
            { "TableBorderLight", &table_border_light_color },
            { "TableRowBg", &table_row_bg_color },
            { "TableRowBgAlt", &table_row_bg_alt_color },
            { "TextSelectedBg", &text_selected_bg_color },
            { "DragDropTarget", &drag_drop_target_color },
            { "NavHighlight", &nav_highlight_color },
            { "NavWindowingHighlight", &nav_windowing_highlight_color },
            { "NavWindowingDimBg", &nav_windowing_dim_color },
            { "ModalWindowDimBg", &modal_window_dim_color }
        };

        // theme_color iterator:
        /*
        for (size_t i = 0; i < theme_colors.size(); ++i) {
        const auto& [name, color] = theme_colors[i];
        ImGui::GetStyle().Colors[i] = ImVec4(color->r, color->g, color->b, color->a);
        }
        /*/


        // Style variables (mapping to ImGuiStyle::VarIdx)
        float alpha                             = 1.0f;                 // ImGuiStyleVar_Alpha
        Salix::Vector2 window_padding           = {8.0f, 8.0f};         // ImGuiStyleVar_WindowPadding
        float window_rounding                   = 7.0f;                 // ImGuiStyleVar_WindowRounding
        float window_border_size                = 1.0f;                 // ImGuiStyleVar_WindowBorderSize
        Salix::Vector2 window_min_size          = {32.0f, 32.0f};       // ImGuiStyleVar_WindowMinSize
        Salix::Vector2 window_title_align       = {0.0f, 0.5f};         // ImGuiStyleVar_WindowTitleAlign
        float child_rounding                    = 0.0f;                 // ImGuiStyleVar_ChildRounding
        float child_border_size                 = 1.0f;                 // ImGuiStyleVar_ChildBorderSize
        float popup_rounding                    = 0.0f;                 // ImGuiStyleVar_PopupRounding
        float popup_border_size                 = 1.0f;                 // ImGuiStyleVar_PopupBorderSize
        Salix::Vector2 frame_padding            = {4.0f, 3.0f};         // ImGuiStyleVar_FramePadding
        float frame_rounding                    = 0.0f;                 // ImGuiStyleVar_FrameRounding
        float frame_border_size                 = 0.0f;                 // ImGuiStyleVar_FrameBorderSize
        Salix::Vector2 item_spacing             = {8.0f, 4.0f};         // ImGuiStyleVar_ItemSpacing
        Salix::Vector2 item_inner_spacing       = {4.0f, 4.0f};         // ImGuiStyleVar_ItemInnerSpacing
        Salix::Vector2 cell_padding             = {4.0f, 2.0f};         // ImGuiStyleVar_CellPadding
        Salix::Vector2 touch_extra_padding      = {0.0f, 0.0f};         // ImGuiStyleVar_TouchExtraPadding
        float indent_spacing                    = 21.0f;                // ImGuiStyleVar_IndentSpacing
        float columns_min_spacing               = 6.0f;                 // ImGuiStyleVar_ColumnsMinSpacing
        float scrollbar_size                    = 14.0f;                // ImGuiStyleVar_ScrollbarSize
        float scrollbar_rounding                = 9.0f;                 // ImGuiStyleVar_ScrollbarRounding
        float grab_min_size                     = 10.0f;                // ImGuiStyleVar_GrabMinSize
        float grab_rounding                     = 0.0f;                 // ImGuiStyleVar_GrabRounding
        float log_slider_deadzone               = 4.0f;                 // ImGuiStyleVar_LogSliderDeadzone
        float tab_rounding                      = 4.0f;                 // ImGuiStyleVar_TabRounding
        float tab_border_size                   = 0.0f;                 // ImGuiStyleVar_TabBorderSize
        float tab_min_width_for_close_button    = 0.0f;                 // ImGuiStyleVar_TabMinWidthForCloseButton
        Salix::Vector2 selectable_text_align    = {0.0f, 0.0f};         // ImGuiStyleVar_SelectableTextAlign
        Salix::Vector2 button_text_align        = {0.5f, 0.5f};         // ImGuiStyleVar_ButtonTextAlign
        Salix::Vector2 separator_text_align     = {0.0f, 0.5f};         // ImGuiStyleVar_SeparatorTextAlign
        float separator_text_border_size        = 3.0f;                 // ImGuiStyleVar_SeparatorTextBorderSize
        float separator_text_align_spacing      = 0.0f;                 // ImGuiStyleVar_SeparatorTextAlignSpacing
        float separator_text_padding            = 20.0f;                // ImGuiStyleVar_SeparatorTextPadding
        float display_window_padding_x          = 10.0f;                // (Not part of VarIdx; manual apply)
        float display_window_padding_y          = 10.0f;                //
        float display_safe_area_padding_x       = 3.0f;                 //
        float display_safe_area_padding_y       = 3.0f;                 //

        // Vector style vars for looping
        std::vector<std::pair<std::string, float*>> float_style_vars = {
            {"Alpha", &alpha },
            {"WindowRounding", &window_rounding },
            {"WindowBorderSize", &window_border_size },
            {"ChildRounding", &child_rounding },
            {"ChildBorderSize", &child_border_size },
            {"PopupRounding", &popup_rounding },
            { "PopupBorderSize", &popup_border_size },
            { "FrameRounding", &frame_rounding },
            { "FrameBorderSize", &frame_border_size },
            { "IndentSpacing", &indent_spacing },
            { "ColumnsMinSpacing", &columns_min_spacing },
            { "ScrollbarSize", &scrollbar_size },
            { "ScrollbarRounding", &scrollbar_rounding },
            { "GrabMinSize", &grab_min_size },
            { "GrabRounding", &grab_rounding },
            { "LogSliderDeadzone", &log_slider_deadzone },
            { "TabRounding", &tab_rounding },
            { "TabBorderSize", &tab_border_size },
            { "TabMinWidthForCloseButton", &tab_min_width_for_close_button },
            { "SeparatorTextBorderSize", &separator_text_border_size },
            { "SeparatorTextAlignSpacing", &separator_text_align_spacing },
            { "SeparatorTextPadding", &separator_text_padding }
            // Add manual ones too if needed
        };


    };
}  // namespace Salixfloat 
