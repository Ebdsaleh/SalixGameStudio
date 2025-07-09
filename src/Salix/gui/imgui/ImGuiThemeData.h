// Salix/gui/imgui/ImGuiThemeData.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IThemeData.h> // Inherit from abstract base
#include <Salix/math/Color.h>     // For Salix::Color
#include <Salix/math/Vector2.h>   // For Salix::Vector2
#include <imgui/imgui.h>
#include <string>
#include <vector> // For std::vector (declarations)
#include <utility> // For std::pair

namespace Salix {

    struct SALIX_API ImGuiThemeData : public IThemeData {

        std::string name;
        std::string gui_type;
        ~ImGuiThemeData() override;
        // Colors (from ImGuiCol_ enum, based on your ImGuiStyle struct)
        Salix::Color text_color;
        Salix::Color text_disabled_color;
        Salix::Color window_bg_color;
        Salix::Color child_bg_color;
        Salix::Color popup_bg_color;
        Salix::Color border_color;
        Salix::Color border_shadow_color;
        Salix::Color frame_bg_color;
        Salix::Color frame_bg_hovered_color;
        Salix::Color frame_bg_active_color;
        Salix::Color title_bg_color;
        Salix::Color title_bg_active_color;
        Salix::Color title_bg_collapsed_color;
        Salix::Color menu_bar_bg_color;
        Salix::Color scrollbar_bg_color;
        Salix::Color scrollbar_grab_color;
        Salix::Color scrollbar_grab_hovered_color;
        Salix::Color scrollbar_grab_active_color;
        Salix::Color checkmark_color;
        Salix::Color slider_grab_color;
        Salix::Color slider_grab_active_color;
        Salix::Color button_color;
        Salix::Color button_hovered_color;
        Salix::Color button_active_color;
        Salix::Color header_color;
        Salix::Color header_hovered_color;
        Salix::Color header_active_color;
        Salix::Color separator_color;
        Salix::Color separator_hovered_color;
        Salix::Color separator_active_color;
        Salix::Color resize_grip_color;
        Salix::Color resize_grip_hovered_color;
        Salix::Color resize_grip_active_color;
        Salix::Color tab_color;
        Salix::Color tab_hovered_color;
        Salix::Color tab_active_color;
        Salix::Color tab_unfocused_color;
        Salix::Color tab_unfocused_active_color;

        // All these colors ARE present in your ImGuiStyle (1.89.9)
        Salix::Color docking_preview_color;
        Salix::Color docking_empty_bg_color;
        Salix::Color plot_lines_color;
        Salix::Color plot_lines_hovered_color;
        Salix::Color plot_histogram_color;
        Salix::Color plot_histogram_hovered_color;
        Salix::Color table_header_bg_color;
        Salix::Color table_border_strong_color;
        Salix::Color table_border_light_color;
        Salix::Color table_row_bg_color;
        Salix::Color table_row_bg_alt_color;
        Salix::Color text_selected_bg_color;
        Salix::Color drag_drop_target_color;
        Salix::Color nav_highlight_color;
        Salix::Color nav_windowing_highlight_color;
        Salix::Color nav_windowing_dim_color;
        Salix::Color modal_window_dim_color;


        // Style variables (from ImGuiStyle struct)
        float alpha;
        Salix::Vector2 window_padding;
        float window_rounding;
        float window_border_size;
        float window_border_hover_padding; // NEW: Added from your ImGuiStyle
        Salix::Vector2 window_min_size;
        Salix::Vector2 window_title_align;
        ImGuiDir window_menu_button_position; // Enum, might need specific handling if you want to expose
        float child_rounding;
        float child_border_size;
        float popup_rounding;
        float popup_border_size;
        Salix::Vector2 frame_padding;
        float frame_rounding;
        float frame_border_size;
        Salix::Vector2 item_spacing;
        Salix::Vector2 item_inner_spacing;
        Salix::Vector2 cell_padding;
        Salix::Vector2 touch_extra_padding;
        float indent_spacing;
        float columns_min_spacing;
        float scrollbar_size;
        float scrollbar_rounding;
        float grab_min_size;
        float grab_rounding;
        float log_slider_deadzone;
        float image_border_size; // NEW: Added from your ImGuiStyle
        float tab_rounding;
        float tab_border_size;
        float tab_close_button_min_width_selected; // NEW: Renamed from your ImGuiStyle
        float tab_close_button_min_width_unselected; // NEW: Renamed from your ImGuiStyle
        float tab_bar_border_size; // NEW: Added from your ImGuiStyle
        float tab_bar_overline_size; // NEW: Added from your ImGuiStyle
        float table_angled_headers_angle; // NEW: Added from your ImGuiStyle
        Salix::Vector2 table_angled_headers_text_align; // NEW: Added from your ImGuiStyle
        ImGuiTreeNodeFlags tree_lines_flags; // Enum, might need specific handling
        float tree_lines_size; // NEW: Added from your ImGuiStyle
        float tree_lines_rounding; // NEW: Added from your ImGuiStyle
        ImGuiDir color_button_position; // Enum, might need specific handling
        Salix::Vector2 button_text_align;
        Salix::Vector2 selectable_text_align;
        float separator_text_border_size;
        Salix::Vector2 separator_text_align;
        Salix::Vector2 separator_text_padding; // CRITICAL FIX: This is ImVec2 in your ImGuiStyle, not float
        Salix::Vector2 display_window_padding;
        Salix::Vector2 display_safe_area_padding;
        float docking_separator_size; // NEW: Added from your ImGuiStyle
        float mouse_cursor_scale; // NEW: Added from your ImGuiStyle
        bool anti_aliased_lines; // NEW: Added from your ImGuiStyle
        bool anti_aliased_lines_use_tex; // NEW: Added from your ImGuiStyle
        bool anti_aliased_fill; // NEW: Added from your ImGuiStyle
        float curve_tessellation_tol; // NEW: Added from your ImGuiStyle
        float circle_tessellation_max_error; // NEW: Added from your ImGuiStyle

        // Behaviors (from your ImGuiStyle)
        float hover_stationary_delay;
        float hover_delay_short;
        float hover_delay_normal;
        ImGuiHoveredFlags hover_flags_for_tooltip_mouse; // Enum, might need specific handling
        ImGuiHoveredFlags hover_flags_for_tooltip_nav;   // Enum, might need specific handling


        // Fonts
        std::string default_font_path;
        float default_font_size;
        float font_scale_main;
        // Vector style vars for looping (declared here, initialized in .cpp)
        std::vector<std::pair<std::string, Salix::Color*>> theme_colors;
        std::vector<std::pair<std::string, float*>> float_style_vars;
        std::vector<std::pair<std::string, Salix::Vector2*>> vector2_style_vars;

        // --- Constructors ---
        ImGuiThemeData();
        ImGuiThemeData(const std::string theme_name);

        // --- IThemeData overrides ---
        const std::string& get_name() const override;
        const std::string& get_gui_type() const override;
        
        
    };
}