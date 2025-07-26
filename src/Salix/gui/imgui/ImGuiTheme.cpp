// Salix/gui/imgui/ImGuiTheme.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IFont.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/imgui/ImGuiThemeData.h> 
#include <Salix/gui/imgui/sdl/SDLImGui.h> 
#include <memory>
#include <string>
#include <iostream>           
// YAML parsing library
#include <yaml-cpp/yaml.h>          
#include <imgui/imgui.h>  
#include <imgui/imgui_internal.h>                   
#include <backends/imgui_impl_sdlrenderer2.h>



namespace Salix {
    namespace ThemeParsingHelpers {
        // Helper to parse enum from string, returns a default value if not found
        ImGuiDir parse_imgui_dir(const YAML::Node& node, ImGuiDir default_val) {
            if (!node) return default_val;
            std::string val = node.as<std::string>();
            if (val == "ImGuiDir_Left") return ImGuiDir_Left;
            if (val == "ImGuiDir_Right") return ImGuiDir_Right;
            if (val == "ImGuiDir_Up") return ImGuiDir_Up;
            if (val == "ImGuiDir_Down") return ImGuiDir_Down;
            if (val == "ImGuiDir_None") return ImGuiDir_None;
            return default_val;
        }
        ImGuiTreeNodeFlags parse_imgui_treenode_flags(const YAML::Node& node, ImGuiTreeNodeFlags default_val) {
        if (!node || node.IsNull()) return default_val;
            std::string val = node.as<std::string>();

            if (val == "ImGuiTreeNodeFlags_None") return ImGuiTreeNodeFlags_None;
                else if (val == "ImGuiTreeNodeFlags_Selected") return ImGuiTreeNodeFlags_Selected;
                else if (val == "ImGuiTreeNodeFlags_Framed") return ImGuiTreeNodeFlags_Framed;
                else if (val == "ImGuiTreeNodeFlags_AllowItemOverlap") return ImGuiTreeNodeFlags_AllowItemOverlap;
                else if (val == "ImGuiTreeNodeFlags_NoTreePushOnOpen") return ImGuiTreeNodeFlags_NoTreePushOnOpen;
                else if (val == "ImGuiTreeNodeFlags_NoAutoOpenOnLog") return ImGuiTreeNodeFlags_NoAutoOpenOnLog;
                else if (val == "ImGuiTreeNodeFlags_DefaultOpen") return ImGuiTreeNodeFlags_DefaultOpen;
                else if (val == "ImGuiTreeNodeFlags_CollapsingHeader") return ImGuiTreeNodeFlags_CollapsingHeader;
                else if (val == "ImGuiTreeNodeFlags_Leaf") return ImGuiTreeNodeFlags_Leaf;
                else if (val == "ImGuiTreeNodeFlags_Bullet") return ImGuiTreeNodeFlags_Bullet;
                else if (val == "ImGuiTreeNodeFlags_SpanAvailWidth") return ImGuiTreeNodeFlags_SpanAvailWidth;
                else if (val == "ImGuiTreeNodeFlags_SpanFullWidth") return ImGuiTreeNodeFlags_SpanFullWidth;
                else if (val == "ImGuiTreeNodeFlags_NavLeftJumpsBackHere") return ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
                // Specific drawing flags from imgui_internal.h
                else if (val == "ImGuiTreeNodeFlags_DrawLinesNone") return ImGuiTreeNodeFlags_DrawLinesNone;
                else if (val == "ImGuiTreeNodeFlags_DrawLinesFull") return ImGuiTreeNodeFlags_DrawLinesFull;
                else if (val == "ImGuiTreeNodeFlags_DrawLinesToNodes") return ImGuiTreeNodeFlags_DrawLinesToNodes;
                // Private flags exposed in the header, if I intend to use them directly
                else if (val == "ImGuiTreeNodeFlags_NoNavFocus") return ImGuiTreeNodeFlags_NoNavFocus;
                else if (val == "ImGuiTreeNodeFlags_ClipLabelForTrailingButton") return ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
                else if (val == "ImGuiTreeNodeFlags_UpsideDownArrow") return ImGuiTreeNodeFlags_UpsideDownArrow;
                // ImGuiTreeNodeFlags_OpenOnMask_ and ImGuiTreeNodeFlags_DrawLinesMask_ are masks, not individual flags to parse

                return default_val; // Return default if string doesn't match any known flag
            }
    
    

        ImGuiHoveredFlags parse_imgui_hovered_flags(const YAML::Node& node, ImGuiHoveredFlags default_val) {
            if (!node) return default_val;
            std::string val = node.as<std::string>();; // Assuming inner key "hover_flags" in YAML if it's a sub-node
            if (val == "ImGuiHoveredFlags_ForTooltip") return ImGuiHoveredFlags_ForTooltip;
            // Add other flags if needed
            return default_val;
        }
    } // namespace ThemeParsingHelpers
    

    // Pimpl struct definition (should be in ImGuiTheme.h if using Pimpl)
    struct ImGuiTheme::Pimpl {
        std::string name;
        std::string file_path;
        std::unique_ptr<ImGuiThemeData> theme_data;
    };

    // Constructors
    ImGuiTheme::ImGuiTheme() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = "Default ImGui Theme"; // Default name
        pimpl->file_path = "";
        pimpl->theme_data = std::make_unique<ImGuiThemeData>(); // Default data
    }

    ImGuiTheme::ImGuiTheme(const std::string& theme_name) : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = theme_name;
        pimpl->file_path = "";
        pimpl->theme_data = std::make_unique<ImGuiThemeData>(); 
    }

    ImGuiTheme::ImGuiTheme(const std::string& theme_name, const std::string& file_path)
        : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = theme_name;
        pimpl->file_path = file_path;
        pimpl->theme_data = std::make_unique<ImGuiThemeData>(theme_name); // Pass name to data constructor
    }

    // Destructor (must be defined in .cpp for unique_ptr Pimpl)
    ImGuiTheme::~ImGuiTheme() = default;


    // Getters
    const std::string& ImGuiTheme::get_name() const {
        return pimpl->name;
    }

    const std::string& ImGuiTheme::get_file_path() const {
        return pimpl->file_path;
    }

    ImGuiThemeData* ImGuiTheme::get_data() { 
        return pimpl->theme_data.get();
    }

    void Salix::ImGuiTheme::apply(IGui* gui_system) {
        if (!pimpl->theme_data) {
            std::cerr << "ImGuiTheme Error: Cannot apply theme '" << pimpl->name << "' with null data." << std::endl;
            return;
        }

        ImGuiThemeData* data = pimpl->theme_data.get();
        ImGuiStyle& style = ImGui::GetStyle();
        

        // --- Colors ---
        // This is the biggest block, but it's just a simple pattern.
        auto& colors = style.Colors;
        colors[ImGuiCol_Text]                   = {data->text_color.r, data->text_color.g, data->text_color.b, data->text_color.a};
        colors[ImGuiCol_TextDisabled]           = {data->text_disabled_color.r, data->text_disabled_color.g, data->text_disabled_color.b, data->text_disabled_color.a};
        colors[ImGuiCol_WindowBg]               = {data->window_bg_color.r, data->window_bg_color.g, data->window_bg_color.b, data->window_bg_color.a};
        colors[ImGuiCol_ChildBg]                = {data->child_bg_color.r, data->child_bg_color.g, data->child_bg_color.b, data->child_bg_color.a};
        colors[ImGuiCol_PopupBg]                = {data->popup_bg_color.r, data->popup_bg_color.g, data->popup_bg_color.b, data->popup_bg_color.a};
        colors[ImGuiCol_Border]                 = {data->border_color.r, data->border_color.g, data->border_color.b, data->border_color.a};
        colors[ImGuiCol_BorderShadow]           = {data->border_shadow_color.r, data->border_shadow_color.g, data->border_shadow_color.b, data->border_shadow_color.a};
        colors[ImGuiCol_FrameBg]                = {data->frame_bg_color.r, data->frame_bg_color.g, data->frame_bg_color.b, data->frame_bg_color.a};
        colors[ImGuiCol_FrameBgHovered]         = {data->frame_bg_hovered_color.r, data->frame_bg_hovered_color.g, data->frame_bg_hovered_color.b, data->frame_bg_hovered_color.a};
        colors[ImGuiCol_FrameBgActive]          = {data->frame_bg_active_color.r, data->frame_bg_active_color.g, data->frame_bg_active_color.b, data->frame_bg_active_color.a};
        colors[ImGuiCol_TitleBg]                = {data->title_bg_color.r, data->title_bg_color.g, data->title_bg_color.b, data->title_bg_color.a};
        colors[ImGuiCol_TitleBgActive]          = {data->title_bg_active_color.r, data->title_bg_active_color.g, data->title_bg_active_color.b, data->title_bg_active_color.a};
        colors[ImGuiCol_TitleBgCollapsed]       = {data->title_bg_collapsed_color.r, data->title_bg_collapsed_color.g, data->title_bg_collapsed_color.b, data->title_bg_collapsed_color.a};
        colors[ImGuiCol_MenuBarBg]              = {data->menu_bar_bg_color.r, data->menu_bar_bg_color.g, data->menu_bar_bg_color.b, data->menu_bar_bg_color.a};
        colors[ImGuiCol_ScrollbarBg]            = {data->scrollbar_bg_color.r, data->scrollbar_bg_color.g, data->scrollbar_bg_color.b, data->scrollbar_bg_color.a};
        colors[ImGuiCol_ScrollbarGrab]          = {data->scrollbar_grab_color.r, data->scrollbar_grab_color.g, data->scrollbar_grab_color.b, data->scrollbar_grab_color.a};
        colors[ImGuiCol_ScrollbarGrabHovered]   = {data->scrollbar_grab_hovered_color.r, data->scrollbar_grab_hovered_color.g, data->scrollbar_grab_hovered_color.b, data->scrollbar_grab_hovered_color.a};
        colors[ImGuiCol_ScrollbarGrabActive]    = {data->scrollbar_grab_active_color.r, data->scrollbar_grab_active_color.g, data->scrollbar_grab_active_color.b, data->scrollbar_grab_active_color.a};
        colors[ImGuiCol_CheckMark]              = {data->checkmark_color.r, data->checkmark_color.g, data->checkmark_color.b, data->checkmark_color.a};
        colors[ImGuiCol_SliderGrab]             = {data->slider_grab_color.r, data->slider_grab_color.g, data->slider_grab_color.b, data->slider_grab_color.a};
        colors[ImGuiCol_SliderGrabActive]       = {data->slider_grab_active_color.r, data->slider_grab_active_color.g, data->slider_grab_active_color.b, data->slider_grab_active_color.a};
        colors[ImGuiCol_Button]                 = {data->button_color.r, data->button_color.g, data->button_color.b, data->button_color.a};
        colors[ImGuiCol_ButtonHovered]          = {data->button_hovered_color.r, data->button_hovered_color.g, data->button_hovered_color.b, data->button_hovered_color.a};
        colors[ImGuiCol_ButtonActive]           = {data->button_active_color.r, data->button_active_color.g, data->button_active_color.b, data->button_active_color.a};
        colors[ImGuiCol_Header]                 = {data->header_color.r, data->header_color.g, data->header_color.b, data->header_color.a};
        colors[ImGuiCol_HeaderHovered]          = {data->header_hovered_color.r, data->header_hovered_color.g, data->header_hovered_color.b, data->header_hovered_color.a};
        colors[ImGuiCol_HeaderActive]           = {data->header_active_color.r, data->header_active_color.g, data->header_active_color.b, data->header_active_color.a};
        colors[ImGuiCol_Separator]              = {data->separator_color.r, data->separator_color.g, data->separator_color.b, data->separator_color.a};
        colors[ImGuiCol_SeparatorHovered]       = {data->separator_hovered_color.r, data->separator_hovered_color.g, data->separator_hovered_color.b, data->separator_hovered_color.a};
        colors[ImGuiCol_SeparatorActive]        = {data->separator_active_color.r, data->separator_active_color.g, data->separator_active_color.b, data->separator_active_color.a};
        colors[ImGuiCol_ResizeGrip]             = {data->resize_grip_color.r, data->resize_grip_color.g, data->resize_grip_color.b, data->resize_grip_color.a};
        colors[ImGuiCol_ResizeGripHovered]      = {data->resize_grip_hovered_color.r, data->resize_grip_hovered_color.g, data->resize_grip_hovered_color.b, data->resize_grip_hovered_color.a};
        colors[ImGuiCol_ResizeGripActive]       = {data->resize_grip_active_color.r, data->resize_grip_active_color.g, data->resize_grip_active_color.b, data->resize_grip_active_color.a};
        colors[ImGuiCol_Tab]                    = {data->tab_color.r, data->tab_color.g, data->tab_color.b, data->tab_color.a};
        colors[ImGuiCol_TabHovered]             = {data->tab_hovered_color.r, data->tab_hovered_color.g, data->tab_hovered_color.b, data->tab_hovered_color.a};
        colors[ImGuiCol_TabActive]              = {data->tab_active_color.r, data->tab_active_color.g, data->tab_active_color.b, data->tab_active_color.a};
        colors[ImGuiCol_TabUnfocused]           = {data->tab_unfocused_color.r, data->tab_unfocused_color.g, data->tab_unfocused_color.b, data->tab_unfocused_color.a};
        colors[ImGuiCol_TabUnfocusedActive]     = {data->tab_unfocused_active_color.r, data->tab_unfocused_active_color.g, data->tab_unfocused_active_color.b, data->tab_unfocused_active_color.a};
        colors[ImGuiCol_DockingPreview]         = {data->docking_preview_color.r, data->docking_preview_color.g, data->docking_preview_color.b, data->docking_preview_color.a};
        colors[ImGuiCol_DockingEmptyBg]         = {data->docking_empty_bg_color.r, data->docking_empty_bg_color.g, data->docking_empty_bg_color.b, data->docking_empty_bg_color.a};
        colors[ImGuiCol_PlotLines]              = {data->plot_lines_color.r, data->plot_lines_color.g, data->plot_lines_color.b, data->plot_lines_color.a};
        colors[ImGuiCol_PlotLinesHovered]       = {data->plot_lines_hovered_color.r, data->plot_lines_hovered_color.g, data->plot_lines_hovered_color.b, data->plot_lines_hovered_color.a};
        colors[ImGuiCol_PlotHistogram]          = {data->plot_histogram_color.r, data->plot_histogram_color.g, data->plot_histogram_color.b, data->plot_histogram_color.a};
        colors[ImGuiCol_PlotHistogramHovered]   = {data->plot_histogram_hovered_color.r, data->plot_histogram_hovered_color.g, data->plot_histogram_hovered_color.b, data->plot_histogram_hovered_color.a};
        colors[ImGuiCol_TableHeaderBg]          = {data->table_header_bg_color.r, data->table_header_bg_color.g, data->table_header_bg_color.b, data->table_header_bg_color.a};
        colors[ImGuiCol_TableBorderStrong]      = {data->table_border_strong_color.r, data->table_border_strong_color.g, data->table_border_strong_color.b, data->table_border_strong_color.a};
        colors[ImGuiCol_TableBorderLight]       = {data->table_border_light_color.r, data->table_border_light_color.g, data->table_border_light_color.b, data->table_border_light_color.a};
        colors[ImGuiCol_TableRowBg]             = {data->table_row_bg_color.r, data->table_row_bg_color.g, data->table_row_bg_color.b, data->table_row_bg_color.a};
        colors[ImGuiCol_TableRowBgAlt]          = {data->table_row_bg_alt_color.r, data->table_row_bg_alt_color.g, data->table_row_bg_alt_color.b, data->table_row_bg_alt_color.a};
        colors[ImGuiCol_TextSelectedBg]         = {data->text_selected_bg_color.r, data->text_selected_bg_color.g, data->text_selected_bg_color.b, data->text_selected_bg_color.a};
        colors[ImGuiCol_DragDropTarget]         = {data->drag_drop_target_color.r, data->drag_drop_target_color.g, data->drag_drop_target_color.b, data->drag_drop_target_color.a};
        colors[ImGuiCol_NavHighlight]           = {data->nav_highlight_color.r, data->nav_highlight_color.g, data->nav_highlight_color.b, data->nav_highlight_color.a};
        colors[ImGuiCol_NavWindowingHighlight]  = {data->nav_windowing_highlight_color.r, data->nav_windowing_highlight_color.g, data->nav_windowing_highlight_color.b, data->nav_windowing_highlight_color.a};
        colors[ImGuiCol_NavWindowingDimBg]      = {data->nav_windowing_dim_color.r, data->nav_windowing_dim_color.g, data->nav_windowing_dim_color.b, data->nav_windowing_dim_color.a};
        colors[ImGuiCol_ModalWindowDimBg]       = {data->modal_window_dim_color.r, data->modal_window_dim_color.g, data->modal_window_dim_color.b, data->modal_window_dim_color.a};

        // --- Style Variables (floats, bools, enums) ---
        style.Alpha                         = data->alpha;
        style.WindowRounding                = data->window_rounding;
        style.WindowBorderSize              = data->window_border_size;
        style.WindowMenuButtonPosition      = data->window_menu_button_position;
        style.ChildRounding                 = data->child_rounding;
        style.ChildBorderSize               = data->child_border_size;
        style.PopupRounding                 = data->popup_rounding;
        style.PopupBorderSize               = data->popup_border_size;
        style.FrameRounding                 = data->frame_rounding;
        style.FrameBorderSize               = data->frame_border_size;
        style.IndentSpacing                 = data->indent_spacing;
        style.ScrollbarSize                 = data->scrollbar_size;
        style.ScrollbarRounding             = data->scrollbar_rounding;
        style.GrabMinSize                   = data->grab_min_size;
        style.GrabRounding                  = data->grab_rounding;
        style.TabRounding                   = data->tab_rounding;
        style.TabBorderSize                 = data->tab_border_size;
        style.ColorButtonPosition           = data->color_button_position;
        style.MouseCursorScale              = data->mouse_cursor_scale;
        style.AntiAliasedLines              = data->anti_aliased_lines;
        style.AntiAliasedLinesUseTex        = data->anti_aliased_lines_use_tex;
        style.AntiAliasedFill               = data->anti_aliased_fill;
        style.CurveTessellationTol          = data->curve_tessellation_tol;
        style.CircleTessellationMaxError    = data->circle_tessellation_max_error;

        // --- Style Variables (ImVec2) ---
        style.WindowPadding         = {data->window_padding.x, data->window_padding.y};
        style.WindowMinSize         = {data->window_min_size.x, data->window_min_size.y};
        style.WindowTitleAlign      = {data->window_title_align.x, data->window_title_align.y};
        style.FramePadding          = {data->frame_padding.x, data->frame_padding.y};
        style.ItemSpacing           = {data->item_spacing.x, data->item_spacing.y};
        style.ItemInnerSpacing      = {data->item_inner_spacing.x, data->item_inner_spacing.y};
        style.CellPadding           = {data->cell_padding.x, data->cell_padding.y};
        style.ButtonTextAlign       = {data->button_text_align.x, data->button_text_align.y};
        style.SelectableTextAlign   = {data->selectable_text_align.x, data->selectable_text_align.y};

        // --- Fonts ---
        IFontManager* font_manager = gui_system->get_font_manager();
        if (font_manager) {
            std::string family = data->font_family;
            float size_to_use = data->font_size; // Start by trying to use the primary font_size

            // 1. Construct the primary font variant name
            std::string variant_name = family + "_" + std::to_string(static_cast<int>(size_to_use)) + "px";

            // 2. Check if a font with that name is actually registered
            if (!font_manager->get_font(variant_name)) {
                std::cout << "[Theme] Warning: Font '" << variant_name << "' not found. Falling back to default size." << std::endl;
                
                // 3. If not found, fall back to the default_font_size
                size_to_use = data->default_font_size;
                variant_name = family + "_" + std::to_string(static_cast<int>(size_to_use)) + "px";
            }
            
            // 4. Apply the final, validated font variant
            font_manager->apply_font(variant_name);
        }
        else {
                    std::cerr << "ImGuiTheme Error: Cannot get Font Manager from GUI system." << std::endl;
            
        }
    }

    

    bool ImGuiTheme::load_from_yaml(const std::string& file_path) {
        if (!std::filesystem::exists(file_path)) {
            std::cerr << "ImGuiTheme Error: Theme file not found at '" << file_path << "'" << std::endl;
            return false;
        }

        try {
            YAML::Node root = YAML::LoadFile(file_path);

            // Ensure theme_data is initialized before loading into it
            if (!pimpl->theme_data) {
                pimpl->theme_data = std::make_unique<ImGuiThemeData>(); // Initialize with defaults if null
            }

            // --- Load Metadata ---
            if (root["name"]) pimpl->theme_data->name = root["name"].as<std::string>();
            if (root["gui_type"]) pimpl->theme_data->gui_type = root["gui_type"].as<std::string>();

            // --- Load Font Properties ---
            if (root["Font"]) {
                if (root["Font"]["default_font_name"]) pimpl->theme_data->default_font_name = root["Font"]["default_font_name"].as<std::string>();
                if (root["Font"]["default_font_path"]) pimpl->theme_data->default_font_path = root["Font"]["default_font_path"].as<std::string>();
                if (root["Font"]["default_font_size"]) pimpl->theme_data->default_font_size = root["Font"]["default_font_size"].as<float>();
                if (root["Font"]["font_family"]) pimpl->theme_data->font_family = root["Font"]["font_family"].as<std::string>();
                if (root["Font"]["font_size"]) pimpl->theme_data->font_size = root["Font"]["font_size"].as<float>();
                if (root["Font"]["font_scale_main"]) pimpl->theme_data->font_scale_main = root["Font"]["font_scale_main"].as<float>();
            }

            // --- Load Icon Properties ---
                if (root["Icons"]) {
                    const YAML::Node& icons_node = root["Icons"];
                    // Clear any previous icon paths before loading new ones
                    pimpl->theme_data->icon_paths.clear();
                    for (const auto& it : icons_node) {
                        pimpl->theme_data->icon_paths[it.first.as<std::string>()] = it.second.as<std::string>();
                    }
                }

            // --- Load Color Properties ---
            if (root["Colors"]) {
                #define LOAD_COLOR_IF_EXISTS(key, member) if (root["Colors"][key]) root["Colors"][key] >> pimpl->theme_data->member;
                LOAD_COLOR_IF_EXISTS("text_color", text_color);
                LOAD_COLOR_IF_EXISTS("text_disabled_color", text_disabled_color);
                LOAD_COLOR_IF_EXISTS("window_bg_color", window_bg_color);
                LOAD_COLOR_IF_EXISTS("child_bg_color", child_bg_color);
                LOAD_COLOR_IF_EXISTS("popup_bg_color", popup_bg_color);
                LOAD_COLOR_IF_EXISTS("border_color", border_color);
                LOAD_COLOR_IF_EXISTS("border_shadow_color", border_shadow_color);
                LOAD_COLOR_IF_EXISTS("frame_bg_color", frame_bg_color);
                LOAD_COLOR_IF_EXISTS("frame_bg_hovered_color", frame_bg_hovered_color);
                LOAD_COLOR_IF_EXISTS("frame_bg_active_color", frame_bg_active_color);
                LOAD_COLOR_IF_EXISTS("title_bg_color", title_bg_color);
                LOAD_COLOR_IF_EXISTS("title_bg_active_color", title_bg_active_color);
                LOAD_COLOR_IF_EXISTS("title_bg_collapsed_color", title_bg_collapsed_color);
                LOAD_COLOR_IF_EXISTS("menu_bar_bg_color", menu_bar_bg_color);
                LOAD_COLOR_IF_EXISTS("scrollbar_bg_color", scrollbar_bg_color);
                LOAD_COLOR_IF_EXISTS("scrollbar_grab_color", scrollbar_grab_color);
                LOAD_COLOR_IF_EXISTS("scrollbar_grab_hovered_color", scrollbar_grab_hovered_color);
                LOAD_COLOR_IF_EXISTS("scrollbar_grab_active_color", scrollbar_grab_active_color);
                LOAD_COLOR_IF_EXISTS("checkmark_color", checkmark_color);
                LOAD_COLOR_IF_EXISTS("slider_grab_color", slider_grab_color);
                LOAD_COLOR_IF_EXISTS("slider_grab_active_color", slider_grab_active_color);
                LOAD_COLOR_IF_EXISTS("button_color", button_color);
                LOAD_COLOR_IF_EXISTS("button_hovered_color", button_hovered_color);
                LOAD_COLOR_IF_EXISTS("button_active_color", button_active_color);
                LOAD_COLOR_IF_EXISTS("header_color", header_color);
                LOAD_COLOR_IF_EXISTS("header_hovered_color", header_hovered_color);
                LOAD_COLOR_IF_EXISTS("header_active_color", header_active_color);
                LOAD_COLOR_IF_EXISTS("separator_color", separator_color);
                LOAD_COLOR_IF_EXISTS("separator_hovered_color", separator_hovered_color);
                LOAD_COLOR_IF_EXISTS("separator_active_color", separator_active_color);
                LOAD_COLOR_IF_EXISTS("resize_grip_color", resize_grip_color);
                LOAD_COLOR_IF_EXISTS("resize_grip_hovered_color", resize_grip_hovered_color);
                LOAD_COLOR_IF_EXISTS("resize_grip_active_color", resize_grip_active_color);
                LOAD_COLOR_IF_EXISTS("tab_color", tab_color);
                LOAD_COLOR_IF_EXISTS("tab_hovered_color", tab_hovered_color);
                LOAD_COLOR_IF_EXISTS("tab_active_color", tab_active_color);
                LOAD_COLOR_IF_EXISTS("tab_unfocused_color", tab_unfocused_color);
                LOAD_COLOR_IF_EXISTS("tab_unfocused_active_color", tab_unfocused_active_color);
                LOAD_COLOR_IF_EXISTS("docking_preview_color", docking_preview_color);
                LOAD_COLOR_IF_EXISTS("docking_empty_bg_color", docking_empty_bg_color);
                LOAD_COLOR_IF_EXISTS("plot_lines_color", plot_lines_color);
                LOAD_COLOR_IF_EXISTS("plot_lines_hovered_color", plot_lines_hovered_color);
                LOAD_COLOR_IF_EXISTS("plot_histogram_color", plot_histogram_color);
                LOAD_COLOR_IF_EXISTS("plot_histogram_hovered_color", plot_histogram_hovered_color);
                LOAD_COLOR_IF_EXISTS("table_header_bg_color", table_header_bg_color);
                LOAD_COLOR_IF_EXISTS("table_border_strong_color", table_border_strong_color);
                LOAD_COLOR_IF_EXISTS("table_border_light_color", table_border_light_color);
                LOAD_COLOR_IF_EXISTS("table_row_bg_color", table_row_bg_color);
                LOAD_COLOR_IF_EXISTS("table_row_bg_alt_color", table_row_bg_alt_color);
                LOAD_COLOR_IF_EXISTS("text_selected_bg_color", text_selected_bg_color);
                LOAD_COLOR_IF_EXISTS("drag_drop_target_color", drag_drop_target_color);
                LOAD_COLOR_IF_EXISTS("nav_highlight_color", nav_highlight_color);
                LOAD_COLOR_IF_EXISTS("nav_windowing_highlight_color", nav_windowing_highlight_color);
                LOAD_COLOR_IF_EXISTS("nav_windowing_dim_color", nav_windowing_dim_color);
                LOAD_COLOR_IF_EXISTS("modal_window_dim_color", modal_window_dim_color);
                #undef LOAD_COLOR_IF_EXISTS
            }

            // --- Load Style Floats ---
            if (root["StyleFloats"]) {
                #define LOAD_FLOAT_IF_EXISTS(key, member) if (root["StyleFloats"][key]) pimpl->theme_data->member = root["StyleFloats"][key].as<float>();
                LOAD_FLOAT_IF_EXISTS("alpha", alpha);
                LOAD_FLOAT_IF_EXISTS("window_rounding", window_rounding);
                LOAD_FLOAT_IF_EXISTS("window_border_size", window_border_size);
                LOAD_FLOAT_IF_EXISTS("window_border_hover_padding", window_border_hover_padding);
                LOAD_FLOAT_IF_EXISTS("child_rounding", child_rounding);
                LOAD_FLOAT_IF_EXISTS("child_border_size", child_border_size);
                LOAD_FLOAT_IF_EXISTS("popup_rounding", popup_rounding);
                LOAD_FLOAT_IF_EXISTS("popup_border_size", popup_border_size);
                LOAD_FLOAT_IF_EXISTS("frame_rounding", frame_rounding);
                LOAD_FLOAT_IF_EXISTS("frame_border_size", frame_border_size);
                LOAD_FLOAT_IF_EXISTS("indent_spacing", indent_spacing);
                LOAD_FLOAT_IF_EXISTS("columns_min_spacing", columns_min_spacing);
                LOAD_FLOAT_IF_EXISTS("scrollbar_size", scrollbar_size);
                LOAD_FLOAT_IF_EXISTS("scrollbar_rounding", scrollbar_rounding);
                LOAD_FLOAT_IF_EXISTS("grab_min_size", grab_min_size);
                LOAD_FLOAT_IF_EXISTS("grab_rounding", grab_rounding);
                LOAD_FLOAT_IF_EXISTS("log_slider_deadzone", log_slider_deadzone);
                LOAD_FLOAT_IF_EXISTS("image_border_size", image_border_size);
                LOAD_FLOAT_IF_EXISTS("tab_rounding", tab_rounding);
                LOAD_FLOAT_IF_EXISTS("tab_border_size", tab_border_size);
                LOAD_FLOAT_IF_EXISTS("tab_close_button_min_width_selected", tab_close_button_min_width_selected);
                LOAD_FLOAT_IF_EXISTS("tab_close_button_min_width_unselected", tab_close_button_min_width_unselected);
                LOAD_FLOAT_IF_EXISTS("tab_bar_border_size", tab_bar_border_size);
                LOAD_FLOAT_IF_EXISTS("tab_bar_overline_size", tab_bar_overline_size);
                LOAD_FLOAT_IF_EXISTS("table_angled_headers_angle", table_angled_headers_angle);
                LOAD_FLOAT_IF_EXISTS("tree_lines_size", tree_lines_size);
                LOAD_FLOAT_IF_EXISTS("tree_lines_rounding", tree_lines_rounding);
                LOAD_FLOAT_IF_EXISTS("separator_text_border_size", separator_text_border_size);
                LOAD_FLOAT_IF_EXISTS("docking_separator_size", docking_separator_size);
                LOAD_FLOAT_IF_EXISTS("mouse_cursor_scale", mouse_cursor_scale);
                LOAD_FLOAT_IF_EXISTS("curve_tessellation_tol", curve_tessellation_tol);
                LOAD_FLOAT_IF_EXISTS("circle_tessellation_max_error", circle_tessellation_max_error);
                LOAD_FLOAT_IF_EXISTS("hover_stationary_delay", hover_stationary_delay);
                LOAD_FLOAT_IF_EXISTS("hover_delay_short", hover_delay_short);
                LOAD_FLOAT_IF_EXISTS("hover_delay_normal", hover_delay_normal);
                #undef LOAD_FLOAT_IF_EXISTS
            }

            // --- Load Style Booleans ---
            if (root["StyleBools"]) {
                #define LOAD_BOOL_IF_EXISTS(key, member) if (root["StyleBools"][key]) pimpl->theme_data->member = root["StyleBools"][key].as<bool>();
                LOAD_BOOL_IF_EXISTS("anti_aliased_lines", anti_aliased_lines);
                LOAD_BOOL_IF_EXISTS("anti_aliased_lines_use_tex", anti_aliased_lines_use_tex);
                LOAD_BOOL_IF_EXISTS("anti_aliased_fill", anti_aliased_fill);
                #undef LOAD_BOOL_IF_EXISTS
            }

            // --- Load Style Vectors ---
            if (root["StyleVectors"]) {
                #define LOAD_VECTOR2_IF_EXISTS(key, member) if (root["StyleVectors"][key]) root["StyleVectors"][key] >> pimpl->theme_data->member;
                LOAD_VECTOR2_IF_EXISTS("window_padding", window_padding);
                LOAD_VECTOR2_IF_EXISTS("window_min_size", window_min_size);
                LOAD_VECTOR2_IF_EXISTS("window_title_align", window_title_align);
                LOAD_VECTOR2_IF_EXISTS("frame_padding", frame_padding);
                LOAD_VECTOR2_IF_EXISTS("item_spacing", item_spacing);
                LOAD_VECTOR2_IF_EXISTS("item_inner_spacing", item_inner_spacing);
                LOAD_VECTOR2_IF_EXISTS("cell_padding", cell_padding);
                LOAD_VECTOR2_IF_EXISTS("touch_extra_padding", touch_extra_padding);
                LOAD_VECTOR2_IF_EXISTS("table_angled_headers_text_align", table_angled_headers_text_align);
                LOAD_VECTOR2_IF_EXISTS("button_text_align", button_text_align);
                LOAD_VECTOR2_IF_EXISTS("selectable_text_align", selectable_text_align);
                LOAD_VECTOR2_IF_EXISTS("separator_text_align", separator_text_align);
                LOAD_VECTOR2_IF_EXISTS("separator_text_padding", separator_text_padding);
                LOAD_VECTOR2_IF_EXISTS("display_window_padding", display_window_padding);
                LOAD_VECTOR2_IF_EXISTS("display_safe_area_padding", display_safe_area_padding);
                #undef LOAD_VECTOR2_IF_EXISTS
            }
            
            // --- Load Enums ---
            if (root["Enums"]) {
                // Helper functions are inside ThemeParsingHelpers namespace
                pimpl->theme_data->window_menu_button_position = ThemeParsingHelpers::parse_imgui_dir(root["Enums"]["window_menu_button_position"], ImGuiDir_None);
                pimpl->theme_data->color_button_position = ThemeParsingHelpers::parse_imgui_dir(root["Enums"]["color_button_position"], ImGuiDir_None);
                pimpl->theme_data->tree_lines_flags = ThemeParsingHelpers::parse_imgui_treenode_flags(root["Enums"]["tree_lines_flags"], ImGuiTreeNodeFlags_None);
                pimpl->theme_data->hover_flags_for_tooltip_mouse = ThemeParsingHelpers::parse_imgui_hovered_flags(root["Enums"]["hover_flags_for_tooltip_mouse"], ImGuiHoveredFlags_None);
                pimpl->theme_data->hover_flags_for_tooltip_nav = ThemeParsingHelpers::parse_imgui_hovered_flags(root["Enums"]["hover_flags_for_tooltip_nav"], ImGuiHoveredFlags_None);
            }

            std::cout << "ImGuiTheme: Loaded from YAML file '" << file_path << "'" << std::endl;
            return true;

        } catch (const YAML::Exception& e) {
            std::cerr << "ImGuiTheme Error: Failed to parse YAML file '" << file_path << "'. Error: " << e.what() << std::endl;
            return false;
        }
    }


     bool ImGuiTheme::save_to_yaml(const std::string& file_path) const{
        if (!pimpl->theme_data) {
            std::cerr << "ImGuiTheme Error: No theme data to save." << std::endl;
            return false;
        }
        if (file_path.empty()) {
            std::cerr << "ImGuiTheme Error: YAML path is empty for saving theme." << std::endl;
            return false;
        }

        try {
            YAML::Emitter emitter;
            emitter << YAML::BeginMap; // Root map

            // --- Metadata ---
            // Corresponds to 'name' and 'gui_type' at the top level of YAML
            emitter << YAML::Key << "name" << YAML::Value << pimpl->theme_data->name;
            emitter << YAML::Key << "gui_type" << YAML::Value << pimpl->theme_data->gui_type;

            // --- Font Properties ---
            // Corresponds to the 'Font' section in YAML
            emitter << YAML::Key << "Font";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "default_font_name" << YAML::Value << pimpl->theme_data->default_font_name;
            emitter << YAML::Key << "default_font_path" << YAML::Value << pimpl->theme_data->default_font_path;
            emitter << YAML::Key << "default_font_size" << YAML::Value << pimpl->theme_data->default_font_size;
            emitter << YAML::Key << "font_family" << YAML::Value << pimpl->theme_data->font_family;
            emitter << YAML::Key << "font_size" << YAML::Value << pimpl->theme_data->font_size;
            emitter << YAML::Key << "font_scale_main" << YAML::Value << pimpl->theme_data->font_scale_main;
            emitter << YAML::EndMap; // End Font map

            
            // --- Icon Properties ---
            emitter << YAML::Key << "Icons";
            emitter << YAML::BeginMap;
            for (const auto& [type_name, path] : pimpl->theme_data->icon_paths) {
                emitter << YAML::Key << type_name << YAML::Value << path;
            }
            emitter << YAML::EndMap; // End Icon map.


            // --- Color Properties ---
            // Corresponds to the 'Colors' section in YAML
            emitter << YAML::Key << "Colors";
            emitter << YAML::BeginMap;
            // Emit each color using the operator<< overload (Salix::Color -> YAML Map {r,g,b,a})
            emitter << YAML::Key << "text_color" << YAML::Value << pimpl->theme_data->text_color;
            emitter << YAML::Key << "text_disabled_color" << YAML::Value << pimpl->theme_data->text_disabled_color;
            emitter << YAML::Key << "window_bg_color" << YAML::Value << pimpl->theme_data->window_bg_color;
            emitter << YAML::Key << "child_bg_color" << YAML::Value << pimpl->theme_data->child_bg_color;
            emitter << YAML::Key << "popup_bg_color" << YAML::Value << pimpl->theme_data->popup_bg_color;
            emitter << YAML::Key << "border_color" << YAML::Value << pimpl->theme_data->border_color;
            emitter << YAML::Key << "border_shadow_color" << YAML::Value << pimpl->theme_data->border_shadow_color;
            emitter << YAML::Key << "frame_bg_color" << YAML::Value << pimpl->theme_data->frame_bg_color;
            emitter << YAML::Key << "frame_bg_hovered_color" << YAML::Value << pimpl->theme_data->frame_bg_hovered_color;
            emitter << YAML::Key << "frame_bg_active_color" << YAML::Value << pimpl->theme_data->frame_bg_active_color;
            emitter << YAML::Key << "title_bg_color" << YAML::Value << pimpl->theme_data->title_bg_color;
            emitter << YAML::Key << "title_bg_active_color" << YAML::Value << pimpl->theme_data->title_bg_active_color;
            emitter << YAML::Key << "title_bg_collapsed_color" << YAML::Value << pimpl->theme_data->title_bg_collapsed_color;
            emitter << YAML::Key << "menu_bar_bg_color" << YAML::Value << pimpl->theme_data->menu_bar_bg_color;
            emitter << YAML::Key << "scrollbar_bg_color" << YAML::Value << pimpl->theme_data->scrollbar_bg_color;
            emitter << YAML::Key << "scrollbar_grab_color" << YAML::Value << pimpl->theme_data->scrollbar_grab_color;
            emitter << YAML::Key << "scrollbar_grab_hovered_color" << YAML::Value << pimpl->theme_data->scrollbar_grab_hovered_color;
            emitter << YAML::Key << "scrollbar_grab_active_color" << YAML::Value << pimpl->theme_data->scrollbar_grab_active_color;
            emitter << YAML::Key << "checkmark_color" << YAML::Value << pimpl->theme_data->checkmark_color;
            emitter << YAML::Key << "slider_grab_color" << YAML::Value << pimpl->theme_data->slider_grab_color;
            emitter << YAML::Key << "slider_grab_active_color" << YAML::Value << pimpl->theme_data->slider_grab_active_color;
            emitter << YAML::Key << "button_color" << YAML::Value << pimpl->theme_data->button_color;
            emitter << YAML::Key << "button_hovered_color" << YAML::Value << pimpl->theme_data->button_hovered_color;
            emitter << YAML::Key << "button_active_color" << YAML::Value << pimpl->theme_data->button_active_color;
            emitter << YAML::Key << "header_color" << YAML::Value << pimpl->theme_data->header_color;
            emitter << YAML::Key << "header_hovered_color" << YAML::Value << pimpl->theme_data->header_hovered_color;
            emitter << YAML::Key << "header_active_color" << YAML::Value << pimpl->theme_data->header_active_color;
            emitter << YAML::Key << "separator_color" << YAML::Value << pimpl->theme_data->separator_color;
            emitter << YAML::Key << "separator_hovered_color" << YAML::Value << pimpl->theme_data->separator_hovered_color;
            emitter << YAML::Key << "separator_active_color" << YAML::Value << pimpl->theme_data->separator_active_color;
            emitter << YAML::Key << "resize_grip_color" << YAML::Value << pimpl->theme_data->resize_grip_color;
            emitter << YAML::Key << "resize_grip_hovered_color" << YAML::Value << pimpl->theme_data->resize_grip_hovered_color;
            emitter << YAML::Key << "resize_grip_active_color" << YAML::Value << pimpl->theme_data->resize_grip_active_color;
            emitter << YAML::Key << "tab_color" << YAML::Value << pimpl->theme_data->tab_color;
            emitter << YAML::Key << "tab_hovered_color" << YAML::Value << pimpl->theme_data->tab_hovered_color;
            emitter << YAML::Key << "tab_active_color" << YAML::Value << pimpl->theme_data->tab_active_color;
            emitter << YAML::Key << "tab_unfocused_color" << YAML::Value << pimpl->theme_data->tab_unfocused_color;
            emitter << YAML::Key << "tab_unfocused_active_color" << YAML::Value << pimpl->theme_data->tab_unfocused_active_color;
            emitter << YAML::Key << "docking_preview_color" << YAML::Value << pimpl->theme_data->docking_preview_color;
            emitter << YAML::Key << "docking_empty_bg_color" << YAML::Value << pimpl->theme_data->docking_empty_bg_color;
            emitter << YAML::Key << "plot_lines_color" << YAML::Value << pimpl->theme_data->plot_lines_color;
            emitter << YAML::Key << "plot_lines_hovered_color" << YAML::Value << pimpl->theme_data->plot_lines_hovered_color;
            emitter << YAML::Key << "plot_histogram_color" << YAML::Value << pimpl->theme_data->plot_histogram_color;
            emitter << YAML::Key << "plot_histogram_hovered_color" << YAML::Value << pimpl->theme_data->plot_histogram_hovered_color;
            emitter << YAML::Key << "table_header_bg_color" << YAML::Value << pimpl->theme_data->table_header_bg_color;
            emitter << YAML::Key << "table_border_strong_color" << YAML::Value << pimpl->theme_data->table_border_strong_color;
            emitter << YAML::Key << "table_border_light_color" << YAML::Value << pimpl->theme_data->table_border_light_color;
            emitter << YAML::Key << "table_row_bg_color" << YAML::Value << pimpl->theme_data->table_row_bg_color;
            emitter << YAML::Key << "table_row_bg_alt_color" << YAML::Value << pimpl->theme_data->table_row_bg_alt_color;
            emitter << YAML::Key << "text_selected_bg_color" << YAML::Value << pimpl->theme_data->text_selected_bg_color;
            emitter << YAML::Key << "drag_drop_target_color" << YAML::Value << pimpl->theme_data->drag_drop_target_color;
            emitter << YAML::Key << "nav_highlight_color" << YAML::Value << pimpl->theme_data->nav_highlight_color;
            emitter << YAML::Key << "nav_windowing_highlight_color" << YAML::Value << pimpl->theme_data->nav_windowing_highlight_color;
            emitter << YAML::Key << "nav_windowing_dim_color" << YAML::Value << pimpl->theme_data->nav_windowing_dim_color;
            emitter << YAML::Key << "modal_window_dim_color" << YAML::Value << pimpl->theme_data->modal_window_dim_color;
            emitter << YAML::EndMap; // End Colors map

            // --- Style Floats ---
            // Corresponds to the 'StyleFloats' section in YAML
            emitter << YAML::Key << "StyleFloats";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "alpha" << YAML::Value << pimpl->theme_data->alpha;
            emitter << YAML::Key << "window_rounding" << YAML::Value << pimpl->theme_data->window_rounding;
            emitter << YAML::Key << "window_border_size" << YAML::Value << pimpl->theme_data->window_border_size;
            emitter << YAML::Key << "window_border_hover_padding" << YAML::Value << pimpl->theme_data->window_border_hover_padding;
            emitter << YAML::Key << "child_rounding" << YAML::Value << pimpl->theme_data->child_rounding;
            emitter << YAML::Key << "child_border_size" << YAML::Value << pimpl->theme_data->child_border_size;
            emitter << YAML::Key << "popup_rounding" << YAML::Value << pimpl->theme_data->popup_rounding;
            emitter << YAML::Key << "popup_border_size" << YAML::Value << pimpl->theme_data->popup_border_size;
            emitter << YAML::Key << "frame_rounding" << YAML::Value << pimpl->theme_data->frame_rounding;
            emitter << YAML::Key << "frame_border_size" << YAML::Value << pimpl->theme_data->frame_border_size;
            emitter << YAML::Key << "indent_spacing" << YAML::Value << pimpl->theme_data->indent_spacing;
            emitter << YAML::Key << "columns_min_spacing" << YAML::Value << pimpl->theme_data->columns_min_spacing;
            emitter << YAML::Key << "scrollbar_size" << YAML::Value << pimpl->theme_data->scrollbar_size;
            emitter << YAML::Key << "scrollbar_rounding" << YAML::Value << pimpl->theme_data->scrollbar_rounding;
            emitter << YAML::Key << "grab_min_size" << YAML::Value << pimpl->theme_data->grab_min_size;
            emitter << YAML::Key << "grab_rounding" << YAML::Value << pimpl->theme_data->grab_rounding;
            emitter << YAML::Key << "log_slider_deadzone" << YAML::Value << pimpl->theme_data->log_slider_deadzone;
            emitter << YAML::Key << "image_border_size" << YAML::Value << pimpl->theme_data->image_border_size;
            emitter << YAML::Key << "tab_rounding" << YAML::Value << pimpl->theme_data->tab_rounding;
            emitter << YAML::Key << "tab_border_size" << YAML::Value << pimpl->theme_data->tab_border_size;
            emitter << YAML::Key << "tab_close_button_min_width_selected" << YAML::Value << pimpl->theme_data->tab_close_button_min_width_selected;
            emitter << YAML::Key << "tab_close_button_min_width_unselected" << YAML::Value << pimpl->theme_data->tab_close_button_min_width_unselected;
            emitter << YAML::Key << "tab_bar_border_size" << YAML::Value << pimpl->theme_data->tab_bar_border_size;
            emitter << YAML::Key << "tab_bar_overline_size" << YAML::Value << pimpl->theme_data->tab_bar_overline_size;
            emitter << YAML::Key << "table_angled_headers_angle" << YAML::Value << pimpl->theme_data->table_angled_headers_angle;
            emitter << YAML::Key << "tree_lines_size" << YAML::Value << pimpl->theme_data->tree_lines_size;
            emitter << YAML::Key << "tree_lines_rounding" << YAML::Value << pimpl->theme_data->tree_lines_rounding;
            emitter << YAML::Key << "separator_text_border_size" << YAML::Value << pimpl->theme_data->separator_text_border_size;
            emitter << YAML::Key << "docking_separator_size" << YAML::Value << pimpl->theme_data->docking_separator_size;
            emitter << YAML::Key << "mouse_cursor_scale" << YAML::Value << pimpl->theme_data->mouse_cursor_scale;
            emitter << YAML::Key << "curve_tessellation_tol" << YAML::Value << pimpl->theme_data->curve_tessellation_tol;
            emitter << YAML::Key << "circle_tessellation_max_error" << YAML::Value << pimpl->theme_data->circle_tessellation_max_error;
            emitter << YAML::Key << "hover_stationary_delay" << YAML::Value << pimpl->theme_data->hover_stationary_delay;
            emitter << YAML::Key << "hover_delay_short" << YAML::Value << pimpl->theme_data->hover_delay_short;
            emitter << YAML::Key << "hover_delay_normal" << YAML::Value << pimpl->theme_data->hover_delay_normal;
            emitter << YAML::EndMap; // End StyleFloats map

            // --- Style Booleans ---
            // Corresponds to the 'StyleBools' section in YAML
            emitter << YAML::Key << "StyleBools";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "anti_aliased_lines" << YAML::Value << pimpl->theme_data->anti_aliased_lines;
            emitter << YAML::Key << "anti_aliased_lines_use_tex" << YAML::Value << pimpl->theme_data->anti_aliased_lines_use_tex;
            emitter << YAML::Key << "anti_aliased_fill" << YAML::Value << pimpl->theme_data->anti_aliased_fill;
            emitter << YAML::EndMap; // End StyleBools map

            // --- Style Vectors ---
            // Corresponds to the 'StyleVectors' section in YAML
            emitter << YAML::Key << "StyleVectors";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "window_padding" << YAML::Value << pimpl->theme_data->window_padding;
            emitter << YAML::Key << "window_min_size" << YAML::Value << pimpl->theme_data->window_min_size;
            emitter << YAML::Key << "window_title_align" << YAML::Value << pimpl->theme_data->window_title_align;
            emitter << YAML::Key << "frame_padding" << YAML::Value << pimpl->theme_data->frame_padding;
            emitter << YAML::Key << "item_spacing" << YAML::Value << pimpl->theme_data->item_spacing;
            emitter << YAML::Key << "item_inner_spacing" << YAML::Value << pimpl->theme_data->item_inner_spacing;
            emitter << YAML::Key << "cell_padding" << YAML::Value << pimpl->theme_data->cell_padding;
            emitter << YAML::Key << "touch_extra_padding" << YAML::Value << pimpl->theme_data->touch_extra_padding;
            emitter << YAML::Key << "table_angled_headers_text_align" << YAML::Value << pimpl->theme_data->table_angled_headers_text_align;
            emitter << YAML::Key << "button_text_align" << YAML::Value << pimpl->theme_data->button_text_align;
            emitter << YAML::Key << "selectable_text_align" << YAML::Value << pimpl->theme_data->selectable_text_align;
            emitter << YAML::Key << "separator_text_align" << YAML::Value << pimpl->theme_data->separator_text_align;
            emitter << YAML::Key << "separator_text_padding" << YAML::Value << pimpl->theme_data->separator_text_padding;
            emitter << YAML::Key << "display_window_padding" << YAML::Value << pimpl->theme_data->display_window_padding;
            emitter << YAML::Key << "display_safe_area_padding" << YAML::Value << pimpl->theme_data->display_safe_area_padding;
            emitter << YAML::EndMap; // End StyleVectors map

            // --- Enums ---
            // Corresponds to the 'Enums' section in YAML
            emitter << YAML::Key << "Enums";
            emitter << YAML::BeginMap;
            // Emit each enum, converting its value to a string
            emitter << YAML::Key << "window_menu_button_position" << YAML::Value;
            switch (pimpl->theme_data->window_menu_button_position) {
                case ImGuiDir_Left: emitter << "ImGuiDir_Left"; break;
                case ImGuiDir_Right: emitter << "ImGuiDir_Right"; break;
                case ImGuiDir_Up: emitter << "ImGuiDir_Up"; break;
                case ImGuiDir_Down: emitter << "ImGuiDir_Down"; break;
                case ImGuiDir_None: emitter << "ImGuiDir_None"; break;
                default: emitter << "ImGuiDir_None"; break; // Fallback
            }
            emitter << YAML::Key << "color_button_position" << YAML::Value;
            switch (pimpl->theme_data->color_button_position) {
                case ImGuiDir_Left: emitter << "ImGuiDir_Left"; break;
                case ImGuiDir_Right: emitter << "ImGuiDir_Right"; break;
                case ImGuiDir_Up: emitter << "ImGuiDir_Up"; break;
                case ImGuiDir_Down: emitter << "ImGuiDir_Down"; break;
                case ImGuiDir_None: emitter << "ImGuiDir_None"; break;
                default: emitter << "ImGuiDir_None"; break; // Fallback
            }
            emitter << YAML::Key << "tree_lines_flags" << YAML::Value;
            // Handle ImGuiTreeNodeFlags as a bitmask, prioritizing specific single flags
            if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_DrawLinesFull) emitter << "ImGuiTreeNodeFlags_DrawLinesFull";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_DrawLinesToNodes) emitter << "ImGuiTreeNodeFlags_DrawLinesToNodes";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_None) emitter << "ImGuiTreeNodeFlags_None"; // Check explicit None
            // Add other general flags if you want to save them
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_Selected) emitter << "ImGuiTreeNodeFlags_Selected";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_Framed) emitter << "ImGuiTreeNodeFlags_Framed";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_AllowItemOverlap) emitter << "ImGuiTreeNodeFlags_AllowItemOverlap";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_NoTreePushOnOpen) emitter << "ImGuiTreeNodeFlags_NoTreePushOnOpen";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_NoAutoOpenOnLog) emitter << "ImGuiTreeNodeFlags_NoAutoOpenOnLog";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_DefaultOpen) emitter << "ImGuiTreeNodeFlags_DefaultOpen";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_CollapsingHeader) emitter << "ImGuiTreeNodeFlags_CollapsingHeader";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_Leaf) emitter << "ImGuiTreeNodeFlags_Leaf";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_Bullet) emitter << "ImGuiTreeNodeFlags_Bullet";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_SpanAvailWidth) emitter << "ImGuiTreeNodeFlags_SpanAvailWidth";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_SpanFullWidth) emitter << "ImGuiTreeNodeFlags_SpanFullWidth";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) emitter << "ImGuiTreeNodeFlags_NavLeftJumpsBackHere";
            // Private flags (if you want to save them)
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_NoNavFocus) emitter << "ImGuiTreeNodeFlags_NoNavFocus";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton) emitter << "ImGuiTreeNodeFlags_ClipLabelForTrailingButton";
            else if (pimpl->theme_data->tree_lines_flags & ImGuiTreeNodeFlags_UpsideDownArrow) emitter << "ImGuiTreeNodeFlags_UpsideDownArrow";
            else emitter << "ImGuiTreeNodeFlags_None"; // Fallback for any other combination or if nothing specific matched
            
            emitter << YAML::Key << "hover_flags_for_tooltip_mouse" << YAML::Value;
            // Assuming this is a single flag like ImGuiHoveredFlags_ForTooltip
            if (pimpl->theme_data->hover_flags_for_tooltip_mouse & ImGuiHoveredFlags_ForTooltip) emitter << "ImGuiHoveredFlags_ForTooltip";
            else emitter << "ImGuiHoveredFlags_None"; // Fallback
            
            emitter << YAML::Key << "hover_flags_for_tooltip_nav" << YAML::Value;
            if (pimpl->theme_data->hover_flags_for_tooltip_nav & ImGuiHoveredFlags_ForTooltip) emitter << "ImGuiHoveredFlags_ForTooltip";
            else emitter << "ImGuiHoveredFlags_None"; // Fallback

            emitter << YAML::EndMap; // End Enums map

            emitter << YAML::EndMap; // End root map

            // Write to file
            std::ofstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "ImGuiTheme Error: Could not open file for writing: '" << file_path << "'" << std::endl;
                return false;
            }
            file << emitter.c_str();
            file.close();

            std::cout << "ImGuiTheme: Saved to YAML file '" << file_path << "'" << std::endl;
            return true;

        } catch (const YAML::Exception& e) {
            std::cerr << "ImGuiTheme Error: Failed to save YAML file '" << file_path << "'. Error: " << e.what() << std::endl;
            return false;
        }
    }

}