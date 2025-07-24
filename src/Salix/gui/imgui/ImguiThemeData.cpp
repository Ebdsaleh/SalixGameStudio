// Salix/gui/imgui/ImGuiThemeData.cpp
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <imgui.h> // Only include ImGui here for default style values
#include <array>   // For std::array if you need to copy ImGuiStyle::Colors directly

namespace Salix {

    ImGuiThemeData::~ImGuiThemeData() = default;

    const std::string& ImGuiThemeData::get_name() const {return name;}


    const std::string& ImGuiThemeData::get_gui_type() const { return gui_type; }

    
    ImGuiThemeData::ImGuiThemeData(const std::string theme_name) : ImGuiThemeData() {
        name = theme_name;
        gui_type = "ImGui";
    }
    ImGuiThemeData::ImGuiThemeData() : name ("New ImGui Theme"), gui_type("ImGui") {
        // Temporarily create an ImGui context to get default style values.
        // This is a common practice for populating data structs with ImGui defaults.
        // Ensure ImGui::CreateContext() is handled carefully elsewhere (e.g., in SDLImGui::initialize)
        // This context is temporary and will be destroyed.
        //ImGui::CreateContext();
        ImGui::StyleColorsDark(); // Apply default dark style to this temporary context
        ImGuiStyle& default_style = ImGui::GetStyle();

        // --- Populate Colors (All ImGuiCol_ are present in 1.89.9) ---
        text_color                     = { default_style.Colors[ImGuiCol_Text].x, default_style.Colors[ImGuiCol_Text].y, default_style.Colors[ImGuiCol_Text].z, default_style.Colors[ImGuiCol_Text].w };
        text_disabled_color            = { default_style.Colors[ImGuiCol_TextDisabled].x, default_style.Colors[ImGuiCol_TextDisabled].y, default_style.Colors[ImGuiCol_TextDisabled].z, default_style.Colors[ImGuiCol_TextDisabled].w };
        window_bg_color                = { default_style.Colors[ImGuiCol_WindowBg].x, default_style.Colors[ImGuiCol_WindowBg].y, default_style.Colors[ImGuiCol_WindowBg].z, default_style.Colors[ImGuiCol_WindowBg].w };
        child_bg_color                 = { default_style.Colors[ImGuiCol_ChildBg].x, default_style.Colors[ImGuiCol_ChildBg].y, default_style.Colors[ImGuiCol_ChildBg].z, default_style.Colors[ImGuiCol_ChildBg].w };
        popup_bg_color                 = { default_style.Colors[ImGuiCol_PopupBg].x, default_style.Colors[ImGuiCol_PopupBg].y, default_style.Colors[ImGuiCol_PopupBg].z, default_style.Colors[ImGuiCol_PopupBg].w };
        border_color                   = { default_style.Colors[ImGuiCol_Border].x, default_style.Colors[ImGuiCol_Border].y, default_style.Colors[ImGuiCol_Border].z, default_style.Colors[ImGuiCol_Border].w };
        border_shadow_color            = { default_style.Colors[ImGuiCol_BorderShadow].x, default_style.Colors[ImGuiCol_BorderShadow].y, default_style.Colors[ImGuiCol_BorderShadow].z, default_style.Colors[ImGuiCol_BorderShadow].w };
        frame_bg_color                 = { default_style.Colors[ImGuiCol_FrameBg].x, default_style.Colors[ImGuiCol_FrameBg].y, default_style.Colors[ImGuiCol_FrameBg].z, default_style.Colors[ImGuiCol_FrameBg].w };
        frame_bg_hovered_color         = { default_style.Colors[ImGuiCol_FrameBgHovered].x, default_style.Colors[ImGuiCol_FrameBgHovered].y, default_style.Colors[ImGuiCol_FrameBgHovered].z, default_style.Colors[ImGuiCol_FrameBgHovered].w };
        frame_bg_active_color          = { default_style.Colors[ImGuiCol_FrameBgActive].x, default_style.Colors[ImGuiCol_FrameBgActive].y, default_style.Colors[ImGuiCol_FrameBgActive].z, default_style.Colors[ImGuiCol_FrameBgActive].w };
        title_bg_color                 = { default_style.Colors[ImGuiCol_TitleBg].x, default_style.Colors[ImGuiCol_TitleBg].y, default_style.Colors[ImGuiCol_TitleBg].z, default_style.Colors[ImGuiCol_TitleBg].w };
        title_bg_active_color          = { default_style.Colors[ImGuiCol_TitleBgActive].x, default_style.Colors[ImGuiCol_TitleBgActive].y, default_style.Colors[ImGuiCol_TitleBgActive].z, default_style.Colors[ImGuiCol_TitleBgActive].w };
        title_bg_collapsed_color       = { default_style.Colors[ImGuiCol_TitleBgCollapsed].x, default_style.Colors[ImGuiCol_TitleBgCollapsed].y, default_style.Colors[ImGuiCol_TitleBgCollapsed].z, default_style.Colors[ImGuiCol_TitleBgCollapsed].w };
        menu_bar_bg_color              = { default_style.Colors[ImGuiCol_MenuBarBg].x, default_style.Colors[ImGuiCol_MenuBarBg].y, default_style.Colors[ImGuiCol_MenuBarBg].z, default_style.Colors[ImGuiCol_MenuBarBg].w };
        scrollbar_bg_color             = { default_style.Colors[ImGuiCol_ScrollbarBg].x, default_style.Colors[ImGuiCol_ScrollbarBg].y, default_style.Colors[ImGuiCol_ScrollbarBg].z, default_style.Colors[ImGuiCol_ScrollbarBg].w };
        scrollbar_grab_color           = { default_style.Colors[ImGuiCol_ScrollbarGrab].x, default_style.Colors[ImGuiCol_ScrollbarGrab].y, default_style.Colors[ImGuiCol_ScrollbarGrab].z, default_style.Colors[ImGuiCol_ScrollbarGrab].w };
        scrollbar_grab_hovered_color   = { default_style.Colors[ImGuiCol_ScrollbarGrabHovered].x, default_style.Colors[ImGuiCol_ScrollbarGrabHovered].y, default_style.Colors[ImGuiCol_ScrollbarGrabHovered].z, default_style.Colors[ImGuiCol_ScrollbarGrabHovered].w };
        scrollbar_grab_active_color    = { default_style.Colors[ImGuiCol_ScrollbarGrabActive].x, default_style.Colors[ImGuiCol_ScrollbarGrabActive].y, default_style.Colors[ImGuiCol_ScrollbarGrabActive].z, default_style.Colors[ImGuiCol_ScrollbarGrabActive].w };
        checkmark_color                = { default_style.Colors[ImGuiCol_CheckMark].x, default_style.Colors[ImGuiCol_CheckMark].y, default_style.Colors[ImGuiCol_CheckMark].z, default_style.Colors[ImGuiCol_CheckMark].w };
        slider_grab_color              = { default_style.Colors[ImGuiCol_SliderGrab].x, default_style.Colors[ImGuiCol_SliderGrab].y, default_style.Colors[ImGuiCol_SliderGrab].z, default_style.Colors[ImGuiCol_SliderGrab].w };
        slider_grab_active_color       = { default_style.Colors[ImGuiCol_SliderGrabActive].x, default_style.Colors[ImGuiCol_SliderGrabActive].y, default_style.Colors[ImGuiCol_SliderGrabActive].z, default_style.Colors[ImGuiCol_SliderGrabActive].w };
        button_color                   = { default_style.Colors[ImGuiCol_Button].x, default_style.Colors[ImGuiCol_Button].y, default_style.Colors[ImGuiCol_Button].z, default_style.Colors[ImGuiCol_Button].w };
        button_hovered_color           = { default_style.Colors[ImGuiCol_ButtonHovered].x, default_style.Colors[ImGuiCol_ButtonHovered].y, default_style.Colors[ImGuiCol_ButtonHovered].z, default_style.Colors[ImGuiCol_ButtonHovered].w };
        button_active_color            = { default_style.Colors[ImGuiCol_ButtonActive].x, default_style.Colors[ImGuiCol_ButtonActive].y, default_style.Colors[ImGuiCol_ButtonActive].z, default_style.Colors[ImGuiCol_ButtonActive].w };
        header_color                   = { default_style.Colors[ImGuiCol_Header].x, default_style.Colors[ImGuiCol_Header].y, default_style.Colors[ImGuiCol_Header].z, default_style.Colors[ImGuiCol_Header].w };
        header_hovered_color           = { default_style.Colors[ImGuiCol_HeaderHovered].x, default_style.Colors[ImGuiCol_HeaderHovered].y, default_style.Colors[ImGuiCol_HeaderHovered].z, default_style.Colors[ImGuiCol_HeaderHovered].w };
        header_active_color            = { default_style.Colors[ImGuiCol_HeaderActive].x, default_style.Colors[ImGuiCol_HeaderActive].y, default_style.Colors[ImGuiCol_HeaderActive].z, default_style.Colors[ImGuiCol_HeaderActive].w };
        separator_color                = { default_style.Colors[ImGuiCol_Separator].x, default_style.Colors[ImGuiCol_Separator].y, default_style.Colors[ImGuiCol_Separator].z, default_style.Colors[ImGuiCol_Separator].w };
        separator_hovered_color        = { default_style.Colors[ImGuiCol_SeparatorHovered].x, default_style.Colors[ImGuiCol_SeparatorHovered].y, default_style.Colors[ImGuiCol_SeparatorHovered].z, default_style.Colors[ImGuiCol_SeparatorHovered].w };
        separator_active_color         = { default_style.Colors[ImGuiCol_SeparatorActive].x, default_style.Colors[ImGuiCol_SeparatorActive].y, default_style.Colors[ImGuiCol_SeparatorActive].z, default_style.Colors[ImGuiCol_SeparatorActive].w };
        resize_grip_color              = { default_style.Colors[ImGuiCol_ResizeGrip].x, default_style.Colors[ImGuiCol_ResizeGrip].y, default_style.Colors[ImGuiCol_ResizeGrip].z, default_style.Colors[ImGuiCol_ResizeGrip].w };
        resize_grip_hovered_color      = { default_style.Colors[ImGuiCol_ResizeGripHovered].x, default_style.Colors[ImGuiCol_ResizeGripHovered].y, default_style.Colors[ImGuiCol_ResizeGripHovered].z, default_style.Colors[ImGuiCol_ResizeGripHovered].w };
        resize_grip_active_color       = { default_style.Colors[ImGuiCol_ResizeGripActive].x, default_style.Colors[ImGuiCol_ResizeGripActive].y, default_style.Colors[ImGuiCol_ResizeGripActive].z, default_style.Colors[ImGuiCol_ResizeGripActive].w };
        tab_color                      = { default_style.Colors[ImGuiCol_Tab].x, default_style.Colors[ImGuiCol_Tab].y, default_style.Colors[ImGuiCol_Tab].z, default_style.Colors[ImGuiCol_Tab].w };
        tab_hovered_color              = { default_style.Colors[ImGuiCol_TabHovered].x, default_style.Colors[ImGuiCol_TabHovered].y, default_style.Colors[ImGuiCol_TabHovered].z, default_style.Colors[ImGuiCol_TabHovered].w };
        tab_active_color               = { default_style.Colors[ImGuiCol_TabActive].x, default_style.Colors[ImGuiCol_TabActive].y, default_style.Colors[ImGuiCol_TabActive].z, default_style.Colors[ImGuiCol_TabActive].w };
        tab_unfocused_color            = { default_style.Colors[ImGuiCol_TabUnfocused].x, default_style.Colors[ImGuiCol_TabUnfocused].y, default_style.Colors[ImGuiCol_TabUnfocused].z, default_style.Colors[ImGuiCol_TabUnfocused].w };
        tab_unfocused_active_color     = { default_style.Colors[ImGuiCol_TabUnfocusedActive].x, default_style.Colors[ImGuiCol_TabUnfocusedActive].y, default_style.Colors[ImGuiCol_TabUnfocusedActive].z, default_style.Colors[ImGuiCol_TabUnfocusedActive].w };
        docking_preview_color          = { default_style.Colors[ImGuiCol_DockingPreview].x, default_style.Colors[ImGuiCol_DockingPreview].y, default_style.Colors[ImGuiCol_DockingPreview].z, default_style.Colors[ImGuiCol_DockingPreview].w };
        docking_empty_bg_color         = { default_style.Colors[ImGuiCol_DockingEmptyBg].x, default_style.Colors[ImGuiCol_DockingEmptyBg].y, default_style.Colors[ImGuiCol_DockingEmptyBg].z, default_style.Colors[ImGuiCol_DockingEmptyBg].w };
        plot_lines_color               = { default_style.Colors[ImGuiCol_PlotLines].x, default_style.Colors[ImGuiCol_PlotLines].y, default_style.Colors[ImGuiCol_PlotLines].z, default_style.Colors[ImGuiCol_PlotLines].w };
        plot_lines_hovered_color       = { default_style.Colors[ImGuiCol_PlotLinesHovered].x, default_style.Colors[ImGuiCol_PlotLinesHovered].y, default_style.Colors[ImGuiCol_PlotLinesHovered].z, default_style.Colors[ImGuiCol_PlotLinesHovered].w };
        plot_histogram_color           = { default_style.Colors[ImGuiCol_PlotHistogram].x, default_style.Colors[ImGuiCol_PlotHistogram].y, default_style.Colors[ImGuiCol_PlotHistogram].z, default_style.Colors[ImGuiCol_PlotHistogram].w };
        plot_histogram_hovered_color   = { default_style.Colors[ImGuiCol_PlotHistogramHovered].x, default_style.Colors[ImGuiCol_PlotHistogramHovered].y, default_style.Colors[ImGuiCol_PlotHistogramHovered].z, default_style.Colors[ImGuiCol_PlotHistogramHovered].w };
        table_header_bg_color          = { default_style.Colors[ImGuiCol_TableHeaderBg].x, default_style.Colors[ImGuiCol_TableHeaderBg].y, default_style.Colors[ImGuiCol_TableHeaderBg].z, default_style.Colors[ImGuiCol_TableHeaderBg].w };
        table_border_strong_color      = { default_style.Colors[ImGuiCol_TableBorderStrong].x, default_style.Colors[ImGuiCol_TableBorderStrong].y, default_style.Colors[ImGuiCol_TableBorderStrong].z, default_style.Colors[ImGuiCol_TableBorderStrong].w };
        table_border_light_color       = { default_style.Colors[ImGuiCol_TableBorderLight].x, default_style.Colors[ImGuiCol_TableBorderLight].y, default_style.Colors[ImGuiCol_TableBorderLight].z, default_style.Colors[ImGuiCol_TableBorderLight].w };
        table_row_bg_color             = { default_style.Colors[ImGuiCol_TableRowBg].x, default_style.Colors[ImGuiCol_TableRowBg].y, default_style.Colors[ImGuiCol_TableRowBg].z, default_style.Colors[ImGuiCol_TableRowBg].w };
        table_row_bg_alt_color         = { default_style.Colors[ImGuiCol_TableRowBgAlt].x, default_style.Colors[ImGuiCol_TableRowBgAlt].y, default_style.Colors[ImGuiCol_TableRowBgAlt].z, default_style.Colors[ImGuiCol_TableRowBgAlt].w };
        text_selected_bg_color         = { default_style.Colors[ImGuiCol_TextSelectedBg].x, default_style.Colors[ImGuiCol_TextSelectedBg].y, default_style.Colors[ImGuiCol_TextSelectedBg].z, default_style.Colors[ImGuiCol_TextSelectedBg].w };
        drag_drop_target_color         = { default_style.Colors[ImGuiCol_DragDropTarget].x, default_style.Colors[ImGuiCol_DragDropTarget].y, default_style.Colors[ImGuiCol_DragDropTarget].z, default_style.Colors[ImGuiCol_DragDropTarget].w };
        nav_highlight_color            = { default_style.Colors[ImGuiCol_NavHighlight].x, default_style.Colors[ImGuiCol_NavHighlight].y, default_style.Colors[ImGuiCol_NavHighlight].z, default_style.Colors[ImGuiCol_NavHighlight].w };
        nav_windowing_highlight_color  = { default_style.Colors[ImGuiCol_NavWindowingHighlight].x, default_style.Colors[ImGuiCol_NavWindowingHighlight].y, default_style.Colors[ImGuiCol_NavWindowingHighlight].z, default_style.Colors[ImGuiCol_NavWindowingHighlight].w };
        nav_windowing_dim_color        = { default_style.Colors[ImGuiCol_NavWindowingDimBg].x, default_style.Colors[ImGuiCol_NavWindowingDimBg].y, default_style.Colors[ImGuiCol_NavWindowingDimBg].z, default_style.Colors[ImGuiCol_NavWindowingDimBg].w };
        modal_window_dim_color         = { default_style.Colors[ImGuiCol_ModalWindowDimBg].x, default_style.Colors[ImGuiCol_ModalWindowDimBg].y, default_style.Colors[ImGuiCol_ModalWindowDimBg].z, default_style.Colors[ImGuiCol_ModalWindowDimBg].w };

        // Style variables
        alpha                          = default_style.Alpha;
        window_padding                 = { default_style.WindowPadding.x, default_style.WindowPadding.y };
        window_rounding                = default_style.WindowRounding;
        window_border_size             = default_style.WindowBorderSize;
        window_border_hover_padding    = default_style.WindowBorderHoverPadding;
        window_min_size                = { default_style.WindowMinSize.x, default_style.WindowMinSize.y };
        window_title_align             = { default_style.WindowTitleAlign.x, default_style.WindowTitleAlign.y };
        window_menu_button_position    = ImGuiDir_Left; // Enum, needs special handling if exposed
        child_rounding                 = default_style.ChildRounding;
        child_border_size              = default_style.ChildBorderSize;
        popup_rounding                 = default_style.PopupRounding;
        popup_border_size              = default_style.PopupBorderSize;
        frame_padding                  = { default_style.FramePadding.x, default_style.FramePadding.y };
        frame_rounding                 = default_style.FrameRounding;
        frame_border_size              = default_style.FrameBorderSize;
        item_spacing                   = { default_style.ItemSpacing.x, default_style.ItemSpacing.y };
        item_inner_spacing             = { default_style.ItemInnerSpacing.x, default_style.ItemInnerSpacing.y };
        cell_padding                   = { default_style.CellPadding.x, default_style.CellPadding.y };
        touch_extra_padding            = { default_style.TouchExtraPadding.x, default_style.TouchExtraPadding.y };
        indent_spacing                 = default_style.IndentSpacing;
        columns_min_spacing            = default_style.ColumnsMinSpacing;
        scrollbar_size                 = default_style.ScrollbarSize;
        scrollbar_rounding             = default_style.ScrollbarRounding;
        grab_min_size                  = default_style.GrabMinSize;
        grab_rounding                  = default_style.GrabRounding;
        log_slider_deadzone            = default_style.LogSliderDeadzone;
        image_border_size              = default_style.ImageBorderSize;
        tab_rounding                   = default_style.TabRounding;
        tab_border_size                = default_style.TabBorderSize;
        tab_close_button_min_width_selected = default_style.TabCloseButtonMinWidthSelected;
        tab_close_button_min_width_unselected = default_style.TabCloseButtonMinWidthUnselected;
        tab_bar_border_size            = default_style.TabBarBorderSize;
        tab_bar_overline_size          = default_style.TabBarOverlineSize;
        table_angled_headers_angle     = default_style.TableAngledHeadersAngle;
        table_angled_headers_text_align = { default_style.TableAngledHeadersTextAlign.x, default_style.TableAngledHeadersTextAlign.y };
        tree_lines_flags                = ImGuiTreeNodeFlags_DrawLinesNone; // Enum, needs special handling if expose    = (ImGuiDir_Left)d
        tree_lines_size                = default_style.TreeLinesSize;
        tree_lines_rounding            = default_style.TreeLinesRounding;
        color_button_position          = ImGuiDir_Right; // Enum, needs special handling if expose       = (ImGuiDir_Left)d
        button_text_align              = { default_style.ButtonTextAlign.x, default_style.ButtonTextAlign.y };
        selectable_text_align          = { default_style.SelectableTextAlign.x, default_style.SelectableTextAlign.y };
        separator_text_border_size     = default_style.SeparatorTextBorderSize;
        separator_text_align           = { default_style.SeparatorTextAlign.x, default_style.SeparatorTextAlign.y };
        separator_text_padding         = { default_style.SeparatorTextPadding.x, default_style.SeparatorTextPadding.y };
        display_window_padding         = { default_style.DisplayWindowPadding.x, default_style.DisplayWindowPadding.y };
        display_safe_area_padding      = { default_style.DisplaySafeAreaPadding.x, default_style.DisplaySafeAreaPadding.y };
        docking_separator_size         = default_style.DockingSeparatorSize;
        mouse_cursor_scale             = default_style.MouseCursorScale;
        anti_aliased_lines             = default_style.AntiAliasedLines;
        anti_aliased_lines_use_tex     = default_style.AntiAliasedLinesUseTex;
        anti_aliased_fill              = default_style.AntiAliasedFill;
        curve_tessellation_tol         = default_style.CurveTessellationTol;
        circle_tessellation_max_error  = default_style.CircleTessellationMaxError;

        // Behaviors
        hover_stationary_delay         = default_style.HoverStationaryDelay;
        hover_delay_short              = default_style.HoverDelayShort;
        hover_delay_normal             = default_style.HoverDelayNormal;
        hover_flags_for_tooltip_mouse  = ImGuiHoveredFlags_ForTooltip; // Enum, needs special handling
        hover_flags_for_tooltip_nav    = ImGuiHoveredFlags_ForTooltip;   // Enum, needs special handling

        // Fonts
        default_font_name = "Roboto-Regular";
        default_font_path = "Assets/Fonts/Roboto-Regular.ttf";
        default_font_size = 20.0f;
        font_family = font_family;
        font_size = font_size;

        font_scale_main = 1.0f;  // Name changed to match the new API was 'default_font_scale', now 'font_scale_main'.


        // Vector style vars for looping
        // Initialize these vectors with pointers to the members of 'data'
        theme_colors = {
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

        float_style_vars = {
            {"Alpha", &alpha },
            {"WindowRounding", &window_rounding },
            {"WindowBorderSize", &window_border_size },
            {"WindowBorderHoverPadding", &window_border_hover_padding },
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
            { "ImageBorderSize", &image_border_size },
            { "TabRounding", &tab_rounding },
            { "TabBorderSize", &tab_border_size },
            { "TabCloseButtonMinWidthSelected", &tab_close_button_min_width_selected },
            { "TabCloseButtonMinWidthUnselected", &tab_close_button_min_width_unselected },
            { "TabBarBorderSize", &tab_bar_border_size },
            { "TabBarOverlineSize", &tab_bar_overline_size },
            { "TableAngledHeadersAngle", &table_angled_headers_angle },
            { "TreeLinesSize", &tree_lines_size },
            { "TreeLinesRounding", &tree_lines_rounding },
            { "SeparatorTextBorderSize", &separator_text_border_size },
            { "DockingSeparatorSize", &docking_separator_size },
            { "MouseCursorScale", &mouse_cursor_scale },
            { "AntiAliasedLines", reinterpret_cast<float*>(&anti_aliased_lines) }, // bool to float
            { "AntiAliasedLinesUseTex", reinterpret_cast<float*>(&anti_aliased_lines_use_tex) }, // bool to float
            { "AntiAliasedFill", reinterpret_cast<float*>(&anti_aliased_fill) }, // bool to float
            { "CurveTessellationTol", &curve_tessellation_tol },
            { "CircleTessellationMaxError", &circle_tessellation_max_error },
            { "HoverStationaryDelay", &hover_stationary_delay },
            { "HoverDelayShort", &hover_delay_short },
            { "HoverDelayNormal", &hover_delay_normal }
        };

        vector2_style_vars = {
            {"WindowPadding", &window_padding },
            {"WindowMinSize", &window_min_size },
            {"WindowTitleAlign", &window_title_align },
            {"FramePadding", &frame_padding },
            {"ItemSpacing", &item_spacing },
            {"ItemInnerSpacing", &item_inner_spacing },
            {"CellPadding", &cell_padding },
            {"TouchExtraPadding", &touch_extra_padding },
            {"TableAngledHeadersTextAlign", &table_angled_headers_text_align },
            {"ButtonTextAlign", &button_text_align },
            {"SelectableTextAlign", &selectable_text_align },
            {"SeparatorTextAlign", &separator_text_align },
            {"SeparatorTextPadding", &separator_text_padding },
            {"DisplayWindowPadding", &display_window_padding},
            {"DisplaySafeAreaPadding", &display_safe_area_padding}
        };

    }
}