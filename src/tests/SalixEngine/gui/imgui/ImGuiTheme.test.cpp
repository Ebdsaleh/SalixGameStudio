#include <doctest.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/ITheme.h>
#include <imgui/imgui.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
class Batch1147ThemeTempFile {
public:
    Batch1147ThemeTempFile() {
        static std::atomic<unsigned long> counter{0};
        path = std::filesystem::temp_directory_path()
            / ("SalixGameStudio_Batch1147_Theme_" + std::to_string(++counter) + ".yaml");
    }

    ~Batch1147ThemeTempFile() {
        std::error_code error;
        std::filesystem::remove(path, error);
    }

    void write(const std::string& text) const {
        std::ofstream stream(path, std::ios::out | std::ios::trunc);
        stream << text;
        stream.close();
    }

    std::filesystem::path path;
};
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch1147") {
    TEST_CASE("default theme exposes default object identity and owned theme data") {
        Salix::ImGuiTheme theme;
        CHECK(theme.get_name() == "Default ImGui Theme");
        CHECK(theme.get_file_path().empty());
        REQUIRE(theme.get_data() != nullptr);
        CHECK(theme.get_data()->get_gui_type() == "ImGui");
    }

    TEST_CASE("name constructor preserves requested theme identity with empty source path") {
        Salix::ImGuiTheme theme("Midnight");
        CHECK(theme.get_name() == "Midnight");
        CHECK(theme.get_file_path().empty());
        REQUIRE(theme.get_data() != nullptr);
        CHECK(theme.get_data()->get_gui_type() == "ImGui");
    }

    TEST_CASE("name and path constructor preserves both object metadata fields") {
        Salix::ImGuiTheme theme("Ocean", "Themes/Ocean.yaml");
        CHECK(theme.get_name() == "Ocean");
        CHECK(theme.get_file_path() == "Themes/Ocean.yaml");
        REQUIRE(theme.get_data() != nullptr);
        CHECK(theme.get_data()->get_name() == "Ocean");
    }

    TEST_CASE("theme data pointer remains stable and mutations are visible through subsequent access") {
        Salix::ImGuiTheme theme("Mutable");
        Salix::ImGuiThemeData* first = theme.get_data();
        REQUIRE(first != nullptr);
        first->alpha = 0.42f;
        first->default_font_name = "Custom";
        CHECK(theme.get_data() == first);
        CHECK(theme.get_data()->alpha == doctest::Approx(0.42f));
        CHECK(theme.get_data()->default_font_name == "Custom");
    }

    TEST_CASE("saving to an empty path fails without altering theme metadata") {
        Salix::ImGuiTheme theme("NoPath");
        CHECK_FALSE(theme.save_to_yaml(""));
        CHECK(theme.get_name() == "NoPath");
        CHECK(theme.get_file_path().empty());
        REQUIRE(theme.get_data() != nullptr);
    }

    TEST_CASE("loading a missing YAML file fails and preserves existing data") {
        Salix::ImGuiTheme theme("Existing");
        theme.get_data()->alpha = 0.33f;
        const auto missing = std::filesystem::temp_directory_path() / "SalixGameStudio_Batch1147_missing_theme.yaml";
        std::error_code error;
        std::filesystem::remove(missing, error);
        CHECK_FALSE(theme.load_from_yaml(missing.string()));
        CHECK(theme.get_name() == "Existing");
        CHECK(theme.get_data()->alpha == doctest::Approx(0.33f));
    }

    TEST_CASE("loading metadata and font section updates theme data fields") {
        Batch1147ThemeTempFile file;
        file.write(
            "name: Loaded Theme\n"
            "gui_type: ImGui\n"
            "Font:\n"
            "  default_font_name: Mono\n"
            "  default_font_path: Assets/Fonts/Mono.ttf\n"
            "  default_font_size: 96\n"
            "  font_family: MonoFamily\n"
            "  font_size: 128\n"
            "  font_scale_main: 1.5\n");

        Salix::ImGuiTheme theme("Receiver");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        const auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        CHECK(data->name == "Loaded Theme");
        CHECK(data->gui_type == "ImGui");
        CHECK(data->default_font_name == "Mono");
        CHECK(data->default_font_path == "Assets/Fonts/Mono.ttf");
        CHECK(data->default_font_size == doctest::Approx(96.0f));
        CHECK(data->font_family == "MonoFamily");
        CHECK(data->font_size == doctest::Approx(128.0f));
        CHECK(data->font_scale_main == doctest::Approx(1.5f));
    }

    TEST_CASE("loading colors styles icons and enums updates independent theme channels") {
        Batch1147ThemeTempFile file;
        file.write(
            "name: Styled\n"
            "gui_type: ImGui\n"
            "Icons:\n"
            "  Entity: Assets/Icons/entity.png\n"
            "Colors:\n"
            "  text_color: {r: 0.1, g: 0.2, b: 0.3, a: 0.4}\n"
            "StyleFloats:\n"
            "  alpha: 0.75\n"
            "  window_rounding: 7.5\n"
            "StyleBools:\n"
            "  anti_aliased_lines: false\n"
            "  anti_aliased_lines_use_tex: true\n"
            "  anti_aliased_fill: false\n"
            "StyleVectors:\n"
            "  window_padding: {x: 3.0, y: 4.0}\n"
            "Enums:\n"
            "  window_menu_button_position: ImGuiDir_Right\n"
            "  color_button_position: ImGuiDir_Left\n"
            "  tree_lines_flags: ImGuiTreeNodeFlags_Selected\n"
            "  hover_flags_for_tooltip_mouse: ImGuiHoveredFlags_ForTooltip\n"
            "  hover_flags_for_tooltip_nav: ImGuiHoveredFlags_ForTooltip\n");

        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        const auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        CHECK(data->icon_paths.at("Entity") == "Assets/Icons/entity.png");
        CHECK(data->text_color.r == doctest::Approx(0.1f));
        CHECK(data->text_color.a == doctest::Approx(0.4f));
        CHECK(data->alpha == doctest::Approx(0.75f));
        CHECK(data->window_rounding == doctest::Approx(7.5f));
        CHECK_FALSE(data->anti_aliased_lines);
        CHECK(data->anti_aliased_lines_use_tex);
        CHECK_FALSE(data->anti_aliased_fill);
        CHECK(data->window_padding == Salix::Vector2(3.0f, 4.0f));
        CHECK(data->window_menu_button_position == ImGuiDir_Right);
        CHECK(data->color_button_position == ImGuiDir_Left);
        CHECK((data->tree_lines_flags & ImGuiTreeNodeFlags_Selected) != 0);
    }

    TEST_CASE("invalid YAML font sizes retain safe selections while malformed documents still fail") {
        Batch1147ThemeTempFile file;
        Salix::ImGuiTheme theme;
        REQUIRE(theme.get_data() != nullptr);
        REQUIRE(theme.get_data()->default_font_size == doctest::Approx(20.0f));
        REQUIRE(theme.get_data()->font_size == doctest::Approx(20.0f));

        file.write(
            "name: Size Safety\n"
            "gui_type: ImGui\n"
            "Font:\n"
            "  default_font_size: 1\n"
            "  font_family: CustomFamily\n"
            "  font_size: 1.0e30\n");

        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->name == "Size Safety");
        CHECK(theme.get_data()->font_family == "CustomFamily");
        CHECK(theme.get_data()->default_font_size == doctest::Approx(20.0f));
        CHECK(theme.get_data()->font_size == doctest::Approx(20.0f));

        file.write("name: [unterminated\n");
        CHECK_FALSE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_size == doctest::Approx(20.0f));
        CHECK(theme.get_data()->font_size == doctest::Approx(20.0f));
    }

    TEST_CASE("ITheme interface saves and reloads selected persistent theme values") {
        Batch1147ThemeTempFile file;
        Salix::ImGuiTheme source("Roundtrip", file.path.string());
        REQUIRE(source.get_data() != nullptr);
        source.get_data()->name = "Roundtrip Data";
        source.get_data()->font_family = "Family";
        source.get_data()->font_size = 22.0f;
        source.get_data()->default_font_size = 21.0f;
        source.get_data()->font_scale_main = 1.25f;
        source.get_data()->alpha = 0.66f;
        source.get_data()->window_padding = Salix::Vector2(6.0f, 7.0f);
        source.get_data()->text_color = Salix::Color(0.2f, 0.4f, 0.6f, 0.8f);
        source.get_data()->icon_paths["Entity"] = "Assets/Icons/roundtrip.png";

        Salix::ITheme* source_interface = &source;
        REQUIRE(source_interface->save_to_yaml(file.path.string()));
        CHECK(std::filesystem::exists(file.path));

        Salix::ImGuiTheme restored("Restored");
        Salix::ITheme* restored_interface = &restored;
        REQUIRE(restored_interface->load_from_yaml(file.path.string()));
        REQUIRE(restored.get_data() != nullptr);
        CHECK(restored.get_data()->name == "Roundtrip Data");
        CHECK(restored.get_data()->font_family == "Family");
        CHECK(restored.get_data()->font_size == doctest::Approx(22.0f));
        CHECK(restored.get_data()->alpha == doctest::Approx(0.66f));
        CHECK(restored.get_data()->window_padding == Salix::Vector2(6.0f, 7.0f));
        CHECK(restored.get_data()->text_color == Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        CHECK(restored.get_data()->icon_paths.at("Entity") == "Assets/Icons/roundtrip.png");
    }
}

namespace {
class Batch1947ThemeFile {
public:
    Batch1947ThemeFile() {
        static std::atomic<unsigned long> counter{0};
        path = std::filesystem::temp_directory_path() /
            ("SalixGameStudio_Batch1947_Theme_" + std::to_string(++counter) + ".yaml");
    }
    ~Batch1947ThemeFile() { std::error_code ec; std::filesystem::remove(path, ec); }
    void write(const std::string& text) const {
        std::ofstream out(path, std::ios::trunc); out << text;
    }
    std::filesystem::path path;
};
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch1947") {
    TEST_CASE("minimal YAML updates metadata while preserving omitted defaults") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme theme;
        const float original_alpha = theme.get_data()->alpha;
        file.write("name: Minimal\ngui_type: ImGui\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->name == "Minimal");
        CHECK(theme.get_data()->gui_type == "ImGui");
        CHECK(theme.get_data()->alpha == doctest::Approx(original_alpha));
    }

    TEST_CASE("recognized up and down ImGui directions are parsed from YAML") {
        Batch1947ThemeFile file;
        file.write(
            "name: Directions\ngui_type: ImGui\nEnums:\n"
            "  window_menu_button_position: ImGuiDir_Up\n"
            "  color_button_position: ImGuiDir_Down\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_menu_button_position == ImGuiDir_Up);
        CHECK(theme.get_data()->color_button_position == ImGuiDir_Down);
    }

    TEST_CASE("recognized none ImGui direction is parsed explicitly") {
        Batch1947ThemeFile file;
        file.write(
            "name: NoDirection\ngui_type: ImGui\nEnums:\n"
            "  window_menu_button_position: ImGuiDir_None\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_menu_button_position == ImGuiDir_None);
    }

    TEST_CASE("unknown direction strings preserve previous enum defaults") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme theme;
        const auto before_window = theme.get_data()->window_menu_button_position;
        const auto before_color = theme.get_data()->color_button_position;
        file.write(
            "name: UnknownDirection\ngui_type: ImGui\nEnums:\n"
            "  window_menu_button_position: NotARealDirection\n"
            "  color_button_position: AlsoUnknown\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_menu_button_position == before_window);
        CHECK(theme.get_data()->color_button_position == before_color);
    }

    TEST_CASE("tree node bullet flag is parsed through theme enum helpers") {
        Batch1947ThemeFile file;
        file.write(
            "name: TreeFlag\ngui_type: ImGui\nEnums:\n"
            "  tree_lines_flags: ImGuiTreeNodeFlags_Bullet\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_Bullet);
    }

    TEST_CASE("unknown tooltip hover flag preserves previous value") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme theme;
        const auto mouse_before = theme.get_data()->hover_flags_for_tooltip_mouse;
        file.write(
            "name: HoverFallback\ngui_type: ImGui\nEnums:\n"
            "  hover_flags_for_tooltip_mouse: UnknownHoverFlag\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->hover_flags_for_tooltip_mouse == mouse_before);
    }

    TEST_CASE("minimum accepted theme font size of six is retained") {
        Batch1947ThemeFile file;
        file.write(
            "name: MinimumFont\ngui_type: ImGui\nFont:\n"
            "  default_font_size: 6\n"
            "  font_size: 6\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_size == doctest::Approx(6.0f));
        CHECK(theme.get_data()->font_size == doctest::Approx(6.0f));
    }

    TEST_CASE("font sizes just below minimum are rejected independently") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme theme;
        const float default_before = theme.get_data()->default_font_size;
        const float size_before = theme.get_data()->font_size;
        file.write(
            "name: BelowMinimum\ngui_type: ImGui\nFont:\n"
            "  default_font_size: 5.99\n"
            "  font_size: -8\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_size == doctest::Approx(default_before));
        CHECK(theme.get_data()->font_size == doctest::Approx(size_before));
    }

    TEST_CASE("multiple style vectors and colors load independently in one document") {
        Batch1947ThemeFile file;
        file.write(
            "name: MultiStyle\ngui_type: ImGui\n"
            "Colors:\n  button_color: {r: 0.2, g: 0.3, b: 0.4, a: 0.5}\n"
            "StyleVectors:\n"
            "  item_spacing: {x: 8.0, y: 9.0}\n"
            "  separator_text_padding: {x: 3.0, y: 4.0}\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->button_color == Salix::Color(0.2f,0.3f,0.4f,0.5f));
        CHECK(theme.get_data()->item_spacing == Salix::Vector2(8.0f,9.0f));
        CHECK(theme.get_data()->separator_text_padding == Salix::Vector2(3.0f,4.0f));
    }

    TEST_CASE("saving into a nonexistent parent directory fails without changing theme identity") {
        const auto missing_parent = std::filesystem::temp_directory_path() / "SalixGameStudio_Batch1947_missing_parent" / "theme.yaml";
        std::error_code ec; std::filesystem::remove_all(missing_parent.parent_path(), ec);
        Salix::ImGuiTheme theme("StableIdentity");
        CHECK_FALSE(theme.save_to_yaml(missing_parent.string()));
        CHECK(theme.get_name() == "StableIdentity");
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch2447") {
    TEST_CASE("batch2447 tree none flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeNone\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_None\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_None);
    }

    TEST_CASE("batch2447 tree framed flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeFramed\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_Framed\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_Framed);
    }

    TEST_CASE("batch2447 tree allow item overlap flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeOverlap\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_AllowItemOverlap\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_AllowItemOverlap);
    }

    TEST_CASE("batch2447 tree no push flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeNoPush\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_NoTreePushOnOpen\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_NoTreePushOnOpen);
    }

    TEST_CASE("batch2447 tree no auto open on log flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeNoAuto\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_NoAutoOpenOnLog\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_NoAutoOpenOnLog);
    }

    TEST_CASE("batch2447 tree default open flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeOpen\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_DefaultOpen\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_DefaultOpen);
    }

    TEST_CASE("batch2447 tree collapsing header flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeCollapse\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_CollapsingHeader\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_CollapsingHeader);
    }

    TEST_CASE("batch2447 tree leaf flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeLeaf\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_Leaf\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_Leaf);
    }

    TEST_CASE("batch2447 tree span available width flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeSpanAvail\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_SpanAvailWidth\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_SpanAvailWidth);
    }

    TEST_CASE("batch2447 tree span full width flag parses explicitly") {
        Batch1947ThemeFile file;
        file.write("name: TreeSpanFull\ngui_type: ImGui\nEnums:\n  tree_lines_flags: ImGuiTreeNodeFlags_SpanFullWidth\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == ImGuiTreeNodeFlags_SpanFullWidth);
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch3347") {
    TEST_CASE("batch3347 default theme public identity follows loaded YAML name") {
        Batch1147ThemeTempFile file; file.write("name: Loaded3347\ngui_type: ImGui\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name()=="Loaded3347"); CHECK(theme.get_data()->name=="Loaded3347");
    }
    TEST_CASE("batch3347 explicitly named theme keeps registry identity while data accepts YAML name") {
        Batch1147ThemeTempFile file; file.write("name: Data3347\ngui_type: ImGui\n"); Salix::ImGuiTheme theme("Registry3347"); REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name()=="Registry3347"); CHECK(theme.get_data()->name=="Data3347");
    }
    TEST_CASE("batch3347 YAML GUI type replaces only data GUI identity") {
        Batch1147ThemeTempFile file; file.write("name: Theme3347\ngui_type: Custom3347\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->gui_type=="Custom3347"); CHECK(theme.get_name()=="Theme3347");
    }
    TEST_CASE("batch3347 Icons section replaces existing icon map completely") {
        Batch1147ThemeTempFile file; file.write("name: Icons3347\nIcons:\n  Play: play3347.png\n  Stop: stop3347.png\n"); Salix::ImGuiTheme theme; theme.get_data()->icon_paths["Old"]="old.png"; REQUIRE(theme.load_from_yaml(file.path.string()));
        REQUIRE(theme.get_data()->icon_paths.size()==2); CHECK(theme.get_data()->icon_paths.at("Play")=="play3347.png"); CHECK(theme.get_data()->icon_paths.at("Stop")=="stop3347.png"); CHECK(theme.get_data()->icon_paths.count("Old")==0);
    }
    TEST_CASE("batch3347 omitted Icons section preserves existing icon map") {
        Batch1147ThemeTempFile file; file.write("name: NoIcons3347\nStyleFloats:\n  alpha: 0.75\n"); Salix::ImGuiTheme theme; theme.get_data()->icon_paths["Keep"]="keep3347.png"; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->icon_paths.size()==1); CHECK(theme.get_data()->icon_paths.at("Keep")=="keep3347.png");
    }
    TEST_CASE("batch3347 StyleBools section updates all antialiasing switches independently") {
        Batch1147ThemeTempFile file; file.write("name: Bools3347\nStyleBools:\n  anti_aliased_lines: false\n  anti_aliased_lines_use_tex: false\n  anti_aliased_fill: true\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK_FALSE(theme.get_data()->anti_aliased_lines); CHECK_FALSE(theme.get_data()->anti_aliased_lines_use_tex); CHECK(theme.get_data()->anti_aliased_fill);
    }
    TEST_CASE("batch3347 StyleFloats section updates independent metrics in one load") {
        Batch1147ThemeTempFile file; file.write("name: Floats3347\nStyleFloats:\n  alpha: 0.3347\n  window_rounding: 7.5\n  scrollbar_size: 19.25\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->alpha==doctest::Approx(0.3347f)); CHECK(theme.get_data()->window_rounding==doctest::Approx(7.5f)); CHECK(theme.get_data()->scrollbar_size==doctest::Approx(19.25f));
    }
    TEST_CASE("batch3347 StyleVectors section updates independent vectors in one load") {
        Batch1147ThemeTempFile file; file.write("name: Vectors3347\nStyleVectors:\n  window_padding: {x: 3.0, y: 4.0}\n  item_spacing: {x: 5.0, y: 6.0}\n  button_text_align: {x: 0.25, y: 0.75}\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_padding==Salix::Vector2(3.f,4.f)); CHECK(theme.get_data()->item_spacing==Salix::Vector2(5.f,6.f)); CHECK(theme.get_data()->button_text_align==Salix::Vector2(.25f,.75f));
    }
    TEST_CASE("batch3347 direction enums load left and right independently") {
        Batch1147ThemeTempFile file; file.write("name: Enums3347\nEnums:\n  window_menu_button_position: ImGuiDir_Right\n  color_button_position: ImGuiDir_Left\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_menu_button_position==ImGuiDir_Right); CHECK(theme.get_data()->color_button_position==ImGuiDir_Left);
    }
    TEST_CASE("batch3347 successful load records exact source file path") {
        Batch1147ThemeTempFile file; file.write("name: Path3347\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string())); CHECK(theme.get_file_path()==file.path.string());
    }
}
TEST_SUITE("Salix::gui::ImGuiThemeBatch3847") {
    TEST_CASE("batch3847 missing YAML load fails without changing object identity or source path") {
        Salix::ImGuiTheme theme("Keep3847", "original3847.theme");
        auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        const float alpha = data->alpha;
        CHECK_FALSE(theme.load_from_yaml("__batch3847_missing_theme__.yaml"));
        CHECK(theme.get_name() == "Keep3847");
        CHECK(theme.get_file_path() == "original3847.theme");
        CHECK(theme.get_data() == data);
        CHECK(theme.get_data()->alpha == doctest::Approx(alpha));
    }

    TEST_CASE("batch3847 unknown direction strings preserve existing enum defaults") {
        Batch1947ThemeFile file;
        file.write(
            "name: UnknownDirs3847\nEnums:\n"
            "  window_menu_button_position: FutureDirection3847\n"
            "  color_button_position: OtherFutureDirection3847\n");
        Salix::ImGuiTheme theme;
        const auto window_before = theme.get_data()->window_menu_button_position;
        const auto color_before = theme.get_data()->color_button_position;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->window_menu_button_position == window_before);
        CHECK(theme.get_data()->color_button_position == color_before);
    }

    TEST_CASE("batch3847 unknown tree flag preserves existing default") {
        Batch1947ThemeFile file;
        file.write(
            "name: UnknownTree3847\nEnums:\n"
            "  tree_lines_flags: ImGuiTreeNodeFlags_Future3847\n");
        Salix::ImGuiTheme theme;
        const auto before = theme.get_data()->tree_lines_flags;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->tree_lines_flags == before);
    }

    TEST_CASE("batch3847 font size below safe minimum is ignored") {
        Batch1947ThemeFile file;
        file.write("name: SmallFont3847\nFont:\n  font_size: 5.0\n");
        Salix::ImGuiTheme theme;
        const float before = theme.get_data()->font_size;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->font_size == doctest::Approx(before));
    }

    TEST_CASE("batch3847 font size at safe minimum is accepted") {
        Batch1947ThemeFile file;
        file.write("name: MinFont3847\nFont:\n  font_size: 6.0\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->font_size == doctest::Approx(6.0f));
    }

    TEST_CASE("batch3847 invalid default font size does not block valid regular font size") {
        Batch1947ThemeFile file;
        file.write(
            "name: MixedFont3847\nFont:\n"
            "  default_font_size: -2.0\n"
            "  font_size: 18.5\n");
        Salix::ImGuiTheme theme;
        const float default_before = theme.get_data()->default_font_size;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_size == doctest::Approx(default_before));
        CHECK(theme.get_data()->font_size == doctest::Approx(18.5f));
    }

    TEST_CASE("batch3847 second partial load preserves style values omitted by later YAML") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write("name: First3847\nStyleFloats:\n  alpha: 0.42\n");
        second.write("name: Second3847\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(first.path.string()));
        REQUIRE(theme.get_data()->alpha == doctest::Approx(0.42f));
        REQUIRE(theme.load_from_yaml(second.path.string()));
        CHECK(theme.get_data()->alpha == doctest::Approx(0.42f));
        CHECK(theme.get_data()->name == "Second3847");
    }

    TEST_CASE("batch3847 empty Icons section clears a previously populated icon map") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write(
            "name: IconsFirst3847\nIcons:\n"
            "  Entity: Assets/Icons/entity3847.png\n"
            "  Camera: Assets/Icons/camera3847.png\n");
        second.write("name: IconsSecond3847\nIcons: {}\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(first.path.string()));
        REQUIRE(theme.get_data()->icon_paths.size() == 2);
        REQUIRE(theme.load_from_yaml(second.path.string()));
        CHECK(theme.get_data()->icon_paths.empty());
    }

    TEST_CASE("batch3847 font metadata with spaces and punctuation is preserved exactly") {
        Batch1947ThemeFile file;
        file.write(
            "name: FontMeta3847\nFont:\n"
            "  default_font_path: \"Assets/My Fonts/Font-3847.ttf\"\n"
            "  default_font_name: \"Font 3847 Regular\"\n"
            "  font_family: \"Family-3847 Mono\"\n"
            "  default_font_size: 17.0\n"
            "  font_size: 19.0\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_path == "Assets/My Fonts/Font-3847.ttf");
        CHECK(theme.get_data()->default_font_name == "Font 3847 Regular");
        CHECK(theme.get_data()->font_family == "Family-3847 Mono");
        CHECK(theme.get_data()->default_font_size == doctest::Approx(17.0f));
        CHECK(theme.get_data()->font_size == doctest::Approx(19.0f));
    }

    TEST_CASE("batch3847 save and reload preserves independent font sizes and icon path") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Roundtrip3847");
        source.get_data()->default_font_size = 14.0f;
        source.get_data()->font_size = 22.0f;
        source.get_data()->font_family = "Roundtrip Family 3847";
        source.get_data()->icon_paths["Entity"] = "Assets/Icons/round trip 3847.png";
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK(restored.get_data()->default_font_size == doctest::Approx(14.0f));
        CHECK(restored.get_data()->font_size == doctest::Approx(22.0f));
        CHECK(restored.get_data()->font_family == "Roundtrip Family 3847");
        CHECK(restored.get_data()->icon_paths.at("Entity") == "Assets/Icons/round trip 3847.png");
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch4447") {
    TEST_CASE("batch4447 default theme public identity follows YAML supplied name") {
        Batch1947ThemeFile file;
        file.write("name: Loaded4447\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name() == "Loaded4447");
        CHECK(theme.get_data()->name == "Loaded4447");
    }

    TEST_CASE("batch4447 explicitly named theme keeps registry identity while data takes YAML name") {
        Batch1947ThemeFile file;
        file.write("name: YAML4447\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme("Explicit4447");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name() == "Explicit4447");
        CHECK(theme.get_data()->name == "YAML4447");
    }

    TEST_CASE("batch4447 explicit name and path constructor keeps name but updates loaded source path") {
        Batch1947ThemeFile file;
        file.write("name: Source4447\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme("Registry4447", "before4447.theme");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name() == "Registry4447");
        CHECK(theme.get_file_path() == file.path.string());
        CHECK(theme.get_data()->name == "Source4447");
    }

    TEST_CASE("batch4447 second Icons section replaces rather than merges prior icon map") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write("name: First4447\nIcons:\n  Entity: entity4447.png\n  Camera: camera4447.png\n");
        second.write("name: Second4447\nIcons:\n  Transform: transform4447.png\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(first.path.string()));
        REQUIRE(theme.get_data()->icon_paths.size() == 2);
        REQUIRE(theme.load_from_yaml(second.path.string()));
        REQUIRE(theme.get_data()->icon_paths.size() == 1);
        CHECK(theme.get_data()->icon_paths.at("Transform") == "transform4447.png");
        CHECK(theme.get_data()->icon_paths.count("Entity") == 0);
    }

    TEST_CASE("batch4447 omitted Icons section preserves previously loaded icon map") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write("name: Icons4447\nIcons:\n  Entity: entity4447.png\n");
        second.write("name: NoIcons4447\nStyleFloats:\n  alpha: 0.4447\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(first.path.string()));
        REQUIRE(theme.load_from_yaml(second.path.string()));
        CHECK(theme.get_data()->icon_paths.at("Entity") == "entity4447.png");
        CHECK(theme.get_data()->alpha == doctest::Approx(0.4447f));
    }

    TEST_CASE("batch4447 empty Icons map explicitly clears previously loaded icons") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write("name: Icons4447\nIcons:\n  Entity: entity4447.png\n");
        second.write("name: ClearIcons4447\nIcons: {}\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(first.path.string()));
        REQUIRE_FALSE(theme.get_data()->icon_paths.empty());
        REQUIRE(theme.load_from_yaml(second.path.string()));
        CHECK(theme.get_data()->icon_paths.empty());
    }

    TEST_CASE("batch4447 gui type metadata accepts an arbitrary future string verbatim") {
        Batch1947ThemeFile file;
        file.write("name: FutureGui4447\ngui_type: FutureGUI-4447\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->gui_type == "FutureGUI-4447");
    }

    TEST_CASE("batch4447 large but safe font sizes are accepted independently") {
        Batch1947ThemeFile file;
        file.write(
            "name: LargeFonts4447\nFont:\n"
            "  default_font_size: 512\n"
            "  font_size: 1024\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->default_font_size == doctest::Approx(512.0f));
        CHECK(theme.get_data()->font_size == doctest::Approx(1024.0f));
    }

    TEST_CASE("batch4447 save and reload preserves multiple icon entries") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Source4447");
        source.get_data()->name = "Saved4447";
        source.get_data()->icon_paths["Entity"] = "entity4447.png";
        source.get_data()->icon_paths["Camera"] = "camera4447.png";
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        REQUIRE(restored.get_data()->icon_paths.size() == 2);
        CHECK(restored.get_data()->icon_paths.at("Entity") == "entity4447.png");
        CHECK(restored.get_data()->icon_paths.at("Camera") == "camera4447.png");
    }

    TEST_CASE("batch4447 save and reload preserves independent style vectors") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Vectors4447");
        source.get_data()->name = "Vectors4447";
        source.get_data()->window_padding = Salix::Vector2(4.0f, 47.0f);
        source.get_data()->item_spacing = Salix::Vector2(-4.0f, 7.0f);
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK(restored.get_data()->window_padding == Salix::Vector2(4.0f, 47.0f));
        CHECK(restored.get_data()->item_spacing == Salix::Vector2(-4.0f, 7.0f));
    }
}
