#include <doctest.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/IThemeData.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <string>

namespace {
bool batch1147_has_color_binding(const Salix::ImGuiThemeData& data, const std::string& name, const Salix::Color* address) {
    return std::any_of(data.theme_colors.begin(), data.theme_colors.end(), [&](const auto& item) {
        return item.first == name && item.second == address;
    });
}

bool batch1147_has_float_binding(const Salix::ImGuiThemeData& data, const std::string& name, const float* address) {
    return std::any_of(data.float_style_vars.begin(), data.float_style_vars.end(), [&](const auto& item) {
        return item.first == name && item.second == address;
    });
}

bool batch1147_has_vector_binding(const Salix::ImGuiThemeData& data, const std::string& name, const Salix::Vector2* address) {
    return std::any_of(data.vector2_style_vars.begin(), data.vector2_style_vars.end(), [&](const auto& item) {
        return item.first == name && item.second == address;
    });
}
}

TEST_SUITE("Salix::gui::ImGuiThemeDataBatch1147") {
    TEST_CASE("default theme data exposes stable name and GUI type") {
        Salix::ImGuiThemeData data;
        CHECK(data.get_name() == "New ImGui Theme");
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("named theme data replaces only theme name while keeping ImGui identity") {
        Salix::ImGuiThemeData data("Midnight");
        CHECK(data.get_name() == "Midnight");
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("theme data color snapshot matches ImGui dark style values") {
        Salix::ImGuiThemeData data;
        ImGuiStyle style;
        ImGui::StyleColorsDark(&style);
        CHECK(data.text_color.r == doctest::Approx(style.Colors[ImGuiCol_Text].x));
        CHECK(data.text_color.g == doctest::Approx(style.Colors[ImGuiCol_Text].y));
        CHECK(data.text_color.b == doctest::Approx(style.Colors[ImGuiCol_Text].z));
        CHECK(data.text_color.a == doctest::Approx(style.Colors[ImGuiCol_Text].w));
        CHECK(data.window_bg_color.r == doctest::Approx(style.Colors[ImGuiCol_WindowBg].x));
        CHECK(data.button_color.r == doctest::Approx(style.Colors[ImGuiCol_Button].x));
    }

    TEST_CASE("theme data vector style snapshot matches current ImGui style") {
        Salix::ImGuiThemeData data;
        ImGuiStyle style;
        ImGui::StyleColorsDark(&style);
        CHECK(data.window_padding.x == doctest::Approx(style.WindowPadding.x));
        CHECK(data.window_padding.y == doctest::Approx(style.WindowPadding.y));
        CHECK(data.frame_padding.x == doctest::Approx(style.FramePadding.x));
        CHECK(data.frame_padding.y == doctest::Approx(style.FramePadding.y));
        CHECK(data.item_spacing.x == doctest::Approx(style.ItemSpacing.x));
        CHECK(data.item_spacing.y == doctest::Approx(style.ItemSpacing.y));
    }

    TEST_CASE("theme data scalar style snapshot matches current ImGui style") {
        Salix::ImGuiThemeData data;
        ImGuiStyle style;
        ImGui::StyleColorsDark(&style);
        CHECK(data.alpha == doctest::Approx(style.Alpha));
        CHECK(data.window_rounding == doctest::Approx(style.WindowRounding));
        CHECK(data.frame_rounding == doctest::Approx(style.FrameRounding));
        CHECK(data.scrollbar_size == doctest::Approx(style.ScrollbarSize));
        CHECK(data.mouse_cursor_scale == doctest::Approx(style.MouseCursorScale));
    }

    TEST_CASE("theme data antialiasing booleans mirror current ImGui defaults") {
        Salix::ImGuiThemeData data;
        ImGuiStyle style;
        ImGui::StyleColorsDark(&style);
        CHECK(data.anti_aliased_lines == style.AntiAliasedLines);
        CHECK(data.anti_aliased_lines_use_tex == style.AntiAliasedLinesUseTex);
        CHECK(data.anti_aliased_fill == style.AntiAliasedFill);
    }

    TEST_CASE("theme data establishes explicit directional and tree flag defaults") {
        Salix::ImGuiThemeData data;
        CHECK(data.window_menu_button_position == ImGuiDir_Left);
        CHECK(data.color_button_position == ImGuiDir_Right);
        CHECK(data.tree_lines_flags == ImGuiTreeNodeFlags_DrawLinesNone);
        CHECK(data.hover_flags_for_tooltip_mouse == ImGuiHoveredFlags_ForTooltip);
        CHECK(data.hover_flags_for_tooltip_nav == ImGuiHoveredFlags_ForTooltip);
    }

    TEST_CASE("theme data exposes deterministic built-in font fallback without preloading theme YAML") {
        Salix::ImGuiThemeData data;
        CHECK(data.default_font_name == "Roboto-Regular");
        CHECK(data.default_font_path == "Assets/Fonts/Roboto-Regular.ttf");
        CHECK(data.default_font_size == doctest::Approx(20.0f));
        CHECK(data.font_family == "Roboto-Regular");
        CHECK(data.font_size == doctest::Approx(20.0f));
        CHECK(data.font_scale_main == doctest::Approx(1.0f));
        CHECK(data.icon_paths.empty());
    }

    TEST_CASE("theme color registry binds canonical labels directly to owned color fields") {
        Salix::ImGuiThemeData data;
        CHECK_FALSE(data.theme_colors.empty());
        CHECK(batch1147_has_color_binding(data, "Text", &data.text_color));
        CHECK(batch1147_has_color_binding(data, "WindowBg", &data.window_bg_color));
        CHECK(batch1147_has_color_binding(data, "Button", &data.button_color));
        CHECK(batch1147_has_color_binding(data, "ModalWindowDimBg", &data.modal_window_dim_color));
    }

    TEST_CASE("float and vector registries bind canonical labels to owned style fields") {
        Salix::ImGuiThemeData data;
        CHECK_FALSE(data.float_style_vars.empty());
        CHECK_FALSE(data.vector2_style_vars.empty());
        CHECK(batch1147_has_float_binding(data, "Alpha", &data.alpha));
        CHECK(batch1147_has_float_binding(data, "WindowRounding", &data.window_rounding));
        CHECK(batch1147_has_vector_binding(data, "WindowPadding", &data.window_padding));
        CHECK(batch1147_has_vector_binding(data, "ButtonTextAlign", &data.button_text_align));
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeDataBatch2547") {
    TEST_CASE("batch2547 named theme constructor preserves exact theme name including spaces") {
        Salix::ImGuiThemeData data("Midnight Blue 2547"); CHECK(data.get_name()=="Midnight Blue 2547"); CHECK(data.get_gui_type()=="ImGui");
    }
    TEST_CASE("batch2547 default theme copies disabled and popup colors from dark style") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.text_disabled_color.r==doctest::Approx(style.Colors[ImGuiCol_TextDisabled].x));
        CHECK(data.popup_bg_color.g==doctest::Approx(style.Colors[ImGuiCol_PopupBg].y));
        CHECK(data.popup_bg_color.a==doctest::Approx(style.Colors[ImGuiCol_PopupBg].w));
    }
    TEST_CASE("batch2547 default theme copies hovered and active frame colors") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.frame_bg_hovered_color.r==doctest::Approx(style.Colors[ImGuiCol_FrameBgHovered].x));
        CHECK(data.frame_bg_active_color.b==doctest::Approx(style.Colors[ImGuiCol_FrameBgActive].z));
    }
    TEST_CASE("batch2547 default theme copies table and navigation colors") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.table_header_bg_color.r==doctest::Approx(style.Colors[ImGuiCol_TableHeaderBg].x));
        CHECK(data.nav_highlight_color.g==doctest::Approx(style.Colors[ImGuiCol_NavHighlight].y));
        CHECK(data.modal_window_dim_color.a==doctest::Approx(style.Colors[ImGuiCol_ModalWindowDimBg].w));
    }
    TEST_CASE("batch2547 scalar style snapshot includes border and tab metrics") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.window_border_size==doctest::Approx(style.WindowBorderSize)); CHECK(data.popup_border_size==doctest::Approx(style.PopupBorderSize));
        CHECK(data.tab_rounding==doctest::Approx(style.TabRounding)); CHECK(data.tab_bar_border_size==doctest::Approx(style.TabBarBorderSize));
    }
    TEST_CASE("batch2547 vector style snapshot includes table and separator alignments") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.table_angled_headers_text_align.x==doctest::Approx(style.TableAngledHeadersTextAlign.x));
        CHECK(data.separator_text_align.y==doctest::Approx(style.SeparatorTextAlign.y));
        CHECK(data.display_safe_area_padding.x==doctest::Approx(style.DisplaySafeAreaPadding.x));
    }
    TEST_CASE("batch2547 theme color registry binds additional canonical endpoints") {
        Salix::ImGuiThemeData data;
        CHECK(batch1147_has_color_binding(data,"TextDisabled",&data.text_disabled_color));
        CHECK(batch1147_has_color_binding(data,"DockingPreview",&data.docking_preview_color));
        CHECK(batch1147_has_color_binding(data,"NavWindowingDimBg",&data.nav_windowing_dim_color));
    }
    TEST_CASE("batch2547 float registry binds newer style metrics to owned storage") {
        Salix::ImGuiThemeData data;
        CHECK(batch1147_has_float_binding(data,"WindowBorderHoverPadding",&data.window_border_hover_padding));
        CHECK(batch1147_has_float_binding(data,"TabBarOverlineSize",&data.tab_bar_overline_size));
        CHECK(batch1147_has_float_binding(data,"HoverDelayNormal",&data.hover_delay_normal));
    }
    TEST_CASE("batch2547 vector registry binds separator and display padding fields") {
        Salix::ImGuiThemeData data;
        CHECK(batch1147_has_vector_binding(data,"SeparatorTextPadding",&data.separator_text_padding));
        CHECK(batch1147_has_vector_binding(data,"DisplayWindowPadding",&data.display_window_padding));
        CHECK(batch1147_has_vector_binding(data,"DisplaySafeAreaPadding",&data.display_safe_area_padding));
    }
    TEST_CASE("batch2547 constructor never aliases mutable metadata between separate themes") {
        Salix::ImGuiThemeData first("First"),second("Second"); first.font_family="Changed"; first.icon_paths["Play"]="play.svg";
        CHECK(second.font_family=="Roboto-Regular"); CHECK(second.icon_paths.empty()); CHECK(first.get_name()=="First"); CHECK(second.get_name()=="Second");
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeDataBatch3347") {
    TEST_CASE("batch3347 named constructor preserves punctuation rich name and ImGui identity") {
        Salix::ImGuiThemeData data("Theme #3347 / Dark"); CHECK(data.get_name()=="Theme #3347 / Dark"); CHECK(data.get_gui_type()=="ImGui");
    }
    TEST_CASE("batch3347 title and menu colors match deterministic dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.title_bg_color.r==doctest::Approx(style.Colors[ImGuiCol_TitleBg].x)); CHECK(data.title_bg_active_color.g==doctest::Approx(style.Colors[ImGuiCol_TitleBgActive].y)); CHECK(data.menu_bar_bg_color.b==doctest::Approx(style.Colors[ImGuiCol_MenuBarBg].z));
    }
    TEST_CASE("batch3347 scrollbar and slider colors match dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.scrollbar_bg_color.r==doctest::Approx(style.Colors[ImGuiCol_ScrollbarBg].x)); CHECK(data.scrollbar_grab_active_color.a==doctest::Approx(style.Colors[ImGuiCol_ScrollbarGrabActive].w)); CHECK(data.slider_grab_color.g==doctest::Approx(style.Colors[ImGuiCol_SliderGrab].y));
    }
    TEST_CASE("batch3347 header separator and resize colors match dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.header_hovered_color.r==doctest::Approx(style.Colors[ImGuiCol_HeaderHovered].x)); CHECK(data.separator_active_color.b==doctest::Approx(style.Colors[ImGuiCol_SeparatorActive].z)); CHECK(data.resize_grip_hovered_color.a==doctest::Approx(style.Colors[ImGuiCol_ResizeGripHovered].w));
    }
    TEST_CASE("batch3347 tab and docking colors match dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.tab_active_color.r==doctest::Approx(style.Colors[ImGuiCol_TabActive].x)); CHECK(data.tab_unfocused_color.g==doctest::Approx(style.Colors[ImGuiCol_TabUnfocused].y)); CHECK(data.docking_preview_color.a==doctest::Approx(style.Colors[ImGuiCol_DockingPreview].w));
    }
    TEST_CASE("batch3347 plot and table colors match dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.plot_lines_hovered_color.r==doctest::Approx(style.Colors[ImGuiCol_PlotLinesHovered].x)); CHECK(data.plot_histogram_color.g==doctest::Approx(style.Colors[ImGuiCol_PlotHistogram].y)); CHECK(data.table_border_strong_color.b==doctest::Approx(style.Colors[ImGuiCol_TableBorderStrong].z));
    }
    TEST_CASE("batch3347 selection navigation and modal colors match dark style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.text_selected_bg_color.a==doctest::Approx(style.Colors[ImGuiCol_TextSelectedBg].w)); CHECK(data.nav_windowing_highlight_color.r==doctest::Approx(style.Colors[ImGuiCol_NavWindowingHighlight].x)); CHECK(data.modal_window_dim_color.a==doctest::Approx(style.Colors[ImGuiCol_ModalWindowDimBg].w));
    }
    TEST_CASE("batch3347 child popup and frame scalar metrics match style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.child_rounding==doctest::Approx(style.ChildRounding)); CHECK(data.popup_rounding==doctest::Approx(style.PopupRounding)); CHECK(data.frame_border_size==doctest::Approx(style.FrameBorderSize));
    }
    TEST_CASE("batch3347 spacing vectors match deterministic style snapshot") {
        Salix::ImGuiThemeData data; ImGuiStyle style; ImGui::StyleColorsDark(&style);
        CHECK(data.item_inner_spacing.x==doctest::Approx(style.ItemInnerSpacing.x)); CHECK(data.cell_padding.y==doctest::Approx(style.CellPadding.y)); CHECK(data.touch_extra_padding.x==doctest::Approx(style.TouchExtraPadding.x));
    }
    TEST_CASE("batch3347 registry pointer mutation updates the exact owned alpha field") {
        Salix::ImGuiThemeData data; auto it=std::find_if(data.float_style_vars.begin(),data.float_style_vars.end(),[](const auto& item){return item.first=="Alpha";});
        REQUIRE(it!=data.float_style_vars.end()); REQUIRE(it->second==&data.alpha); *it->second=0.3347f; CHECK(data.alpha==doctest::Approx(0.3347f));
    }
}
