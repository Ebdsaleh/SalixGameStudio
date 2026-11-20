#include <doctest.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/ITheme.h>
#include <imgui/imgui.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>
#include <Salix/management/FileManager.h>

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

TEST_SUITE("Salix::gui::ImGuiThemeBatch4947") {
    TEST_CASE("batch4947 default theme name and file path reference objects stay stable across load") {
        Batch1947ThemeFile file;
        file.write("name: Loaded4947\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme;
        const auto* name = &theme.get_name();
        const auto* path = &theme.get_file_path();
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(&theme.get_name() == name);
        CHECK(&theme.get_file_path() == path);
        CHECK(*name == "Loaded4947");
        CHECK(*path == file.path.string());
    }

    TEST_CASE("batch4947 explicitly named theme keeps name reference and identity while source path changes") {
        Batch1947ThemeFile file;
        file.write("name: YAML4947\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme("Explicit4947", "before4947.theme");
        const auto* name = &theme.get_name();
        const auto* path = &theme.get_file_path();
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(&theme.get_name() == name);
        CHECK(&theme.get_file_path() == path);
        CHECK(*name == "Explicit4947");
        CHECK(*path == file.path.string());
        CHECK(theme.get_data()->name == "YAML4947");
    }

    TEST_CASE("batch4947 theme data pointer remains stable across two successful YAML loads") {
        Batch1947ThemeFile first;
        Batch1947ThemeFile second;
        first.write("name: First4947\nStyleFloats:\n  alpha: 0.49\n");
        second.write("name: Second4947\nStyleFloats:\n  alpha: 0.47\n");
        Salix::ImGuiTheme theme;
        auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        REQUIRE(theme.load_from_yaml(first.path.string()));
        CHECK(theme.get_data() == data);
        REQUIRE(theme.load_from_yaml(second.path.string()));
        CHECK(theme.get_data() == data);
        CHECK(data->name == "Second4947");
        CHECK(data->alpha == doctest::Approx(0.47f));
    }

    TEST_CASE("batch4947 malformed YAML fails before changing source file path") {
        Batch1947ThemeFile valid;
        Batch1947ThemeFile malformed;
        valid.write("name: Valid4947\ngui_type: ImGui\n");
        malformed.write("name: [unterminated\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(valid.path.string()));
        const auto previous_path = theme.get_file_path();
        const auto previous_name = theme.get_name();
        CHECK_FALSE(theme.load_from_yaml(malformed.path.string()));
        CHECK(theme.get_file_path() == previous_path);
        CHECK(theme.get_name() == previous_name);
    }

    TEST_CASE("batch4947 YAML without name preserves default public identity while updating source path") {
        Batch1947ThemeFile file;
        file.write("gui_type: ImGui\nStyleFloats:\n  alpha: 0.4947\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name() == "Default ImGui Theme");
        CHECK(theme.get_file_path() == file.path.string());
        CHECK(theme.get_data()->alpha == doctest::Approx(0.4947f));
    }

    TEST_CASE("batch4947 partial Colors section updates selected color and preserves unrelated color") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme theme;
        const auto border_before = theme.get_data()->border_color;
        file.write(
            "name: Colors4947\nColors:\n"
            "  text_color: {r: 0.1, g: 0.2, b: 0.3, a: 0.4}\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->text_color == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));
        CHECK(theme.get_data()->border_color == border_before);
    }

    TEST_CASE("batch4947 save and reload preserves anti alias booleans together") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Boolean4947");
        source.get_data()->name = "Boolean4947";
        source.get_data()->anti_aliased_lines = false;
        source.get_data()->anti_aliased_lines_use_tex = false;
        source.get_data()->anti_aliased_fill = true;
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK_FALSE(restored.get_data()->anti_aliased_lines);
        CHECK_FALSE(restored.get_data()->anti_aliased_lines_use_tex);
        CHECK(restored.get_data()->anti_aliased_fill);
    }

    TEST_CASE("batch4947 save and reload preserves hover delay values including negative diagnostic input") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Hover4947");
        source.get_data()->name = "Hover4947";
        source.get_data()->hover_stationary_delay = -0.25f;
        source.get_data()->hover_delay_short = 0.4947f;
        source.get_data()->hover_delay_normal = 4.947f;
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK(restored.get_data()->hover_stationary_delay == doctest::Approx(-0.25f));
        CHECK(restored.get_data()->hover_delay_short == doctest::Approx(0.4947f));
        CHECK(restored.get_data()->hover_delay_normal == doctest::Approx(4.947f));
    }

    TEST_CASE("batch4947 save and reload preserves direction enum selections") {
        Batch1947ThemeFile file;
        Salix::ImGuiTheme source("Enums4947");
        source.get_data()->name = "Enums4947";
        source.get_data()->window_menu_button_position = ImGuiDir_Right;
        source.get_data()->color_button_position = ImGuiDir_Left;
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK(restored.get_data()->window_menu_button_position == ImGuiDir_Right);
        CHECK(restored.get_data()->color_button_position == ImGuiDir_Left);
    }

    TEST_CASE("batch4947 saving to another file never changes theme source file metadata") {
        Batch1947ThemeFile source_file;
        Batch1947ThemeFile destination_file;
        source_file.write("name: Source4947\ngui_type: ImGui\n");
        Salix::ImGuiTheme theme;
        REQUIRE(theme.load_from_yaml(source_file.path.string()));
        const auto source_path = theme.get_file_path();
        REQUIRE(theme.save_to_yaml(destination_file.path.string()));
        CHECK(theme.get_file_path() == source_path);
        CHECK(std::filesystem::is_regular_file(destination_file.path));
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch5647") {
    TEST_CASE("batch5647 default theme exposes stable owned data pointer and identity") {
        Salix::ImGuiTheme theme;
        auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        CHECK(theme.get_data() == data);
        CHECK(theme.get_name() == "Default ImGui Theme");
        CHECK(theme.get_file_path().empty());
        CHECK(data->get_gui_type() == "ImGui");
    }

    TEST_CASE("batch5647 named constructor keeps explicit public identity separate from default data identity") {
        Salix::ImGuiTheme theme("Named5647");
        CHECK(theme.get_name() == "Named5647");
        CHECK(theme.get_file_path().empty());
        REQUIRE(theme.get_data() != nullptr);
        CHECK(theme.get_data()->get_name() == "New ImGui Theme");
    }

    TEST_CASE("batch5647 named path constructor preserves metadata verbatim") {
        Salix::ImGuiTheme theme("Theme 5647", " Themes/Theme 5647.theme ");
        CHECK(theme.get_name() == "Theme 5647");
        CHECK(theme.get_file_path() == " Themes/Theme 5647.theme ");
        CHECK(theme.get_data()->get_name() == "Theme 5647");
    }

    TEST_CASE("batch5647 direct data mutations retain data pointer identity") {
        Salix::ImGuiTheme theme("Mutable5647");
        auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        data->alpha = 0.5647f;
        data->default_font_name = "Font5647";
        CHECK(theme.get_data() == data);
        CHECK(theme.get_data()->alpha == doctest::Approx(0.5647f));
        CHECK(theme.get_data()->default_font_name == "Font5647");
    }

    TEST_CASE("batch5647 saving empty path fails without changing public metadata") {
        Salix::ImGuiTheme theme("NoSave5647", "source5647.theme");
        CHECK_FALSE(theme.save_to_yaml(""));
        CHECK(theme.get_name() == "NoSave5647");
        CHECK(theme.get_file_path() == "source5647.theme");
    }

    TEST_CASE("batch5647 missing load preserves existing data and source metadata") {
        Salix::ImGuiTheme theme("Keep5647", "keep5647.theme");
        theme.get_data()->alpha = 0.47f;
        const auto missing = std::filesystem::temp_directory_path() / "SalixGameStudio_Batch5647_missing_theme.yaml";
        std::error_code ec;
        std::filesystem::remove(missing, ec);
        CHECK_FALSE(theme.load_from_yaml(missing.string()));
        CHECK(theme.get_name() == "Keep5647");
        CHECK(theme.get_file_path() == "keep5647.theme");
        CHECK(theme.get_data()->alpha == doctest::Approx(0.47f));
    }

    TEST_CASE("batch5647 explicitly named theme retains public name while YAML updates data name") {
        Batch1147ThemeTempFile file;
        file.write("name: DataName5647\ngui_type: ImGui\nStyleFloats:\n  alpha: 0.5647\n");
        Salix::ImGuiTheme theme("PublicName5647", "before5647.theme");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_name() == "PublicName5647");
        CHECK(theme.get_file_path() == file.path.string());
        CHECK(theme.get_data()->name == "DataName5647");
        CHECK(theme.get_data()->alpha == doctest::Approx(0.5647f));
    }

    TEST_CASE("batch5647 partial style load updates alpha while retaining unrelated float") {
        Batch1147ThemeTempFile file;
        Salix::ImGuiTheme theme;
        const float before = theme.get_data()->window_rounding;
        file.write("name: Style5647\nStyleFloats:\n  alpha: 0.47\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->alpha == doctest::Approx(0.47f));
        CHECK(theme.get_data()->window_rounding == doctest::Approx(before));
    }

    TEST_CASE("batch5647 partial color load updates text while retaining border color") {
        Batch1147ThemeTempFile file;
        Salix::ImGuiTheme theme;
        const auto border_before = theme.get_data()->border_color;
        file.write(
            "name: Color5647\nColors:\n"
            "  text_color: {r: 0.56, g: 0.47, b: 0.25, a: 0.75}\n");
        REQUIRE(theme.load_from_yaml(file.path.string()));
        CHECK(theme.get_data()->text_color == Salix::Color(0.56f, 0.47f, 0.25f, 0.75f));
        CHECK(theme.get_data()->border_color == border_before);
    }

    TEST_CASE("batch5647 save reload preserves selected style booleans and alpha") {
        Batch1147ThemeTempFile file;
        Salix::ImGuiTheme source("Roundtrip5647");
        source.get_data()->name = "Roundtrip5647";
        source.get_data()->alpha = 0.5647f;
        source.get_data()->anti_aliased_lines = false;
        source.get_data()->anti_aliased_fill = true;
        REQUIRE(source.save_to_yaml(file.path.string()));
        Salix::ImGuiTheme restored;
        REQUIRE(restored.load_from_yaml(file.path.string()));
        CHECK(restored.get_data()->alpha == doctest::Approx(0.5647f));
        CHECK_FALSE(restored.get_data()->anti_aliased_lines);
        CHECK(restored.get_data()->anti_aliased_fill);
    }
}

#include <Salix/input/ImGuiInputManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <imgui.h>
#include <limits>

namespace {
struct Batch6947ThemeContext {
    ImGuiContext* previous_test = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();
    Batch6947ThemeContext() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(owned);
    }
    ~Batch6947ThemeContext() {
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous_test);
    }
};
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch6947") {
    TEST_CASE("batch6947 apply copies representative color float and vector style values") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("Apply6947");
        auto* data = theme.get_data();
        REQUIRE(data != nullptr);
        data->text_color = Salix::Color(0.11f, 0.22f, 0.33f, 0.44f);
        data->alpha = 0.6947f;
        data->window_rounding = 6.947f;
        data->window_padding = Salix::Vector2(12.0f, 34.0f);
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        theme.apply(&gui);
        const ImGuiStyle& style = ImGui::GetStyle();
        CHECK(style.Colors[ImGuiCol_Text].x == doctest::Approx(0.11f));
        CHECK(style.Alpha == doctest::Approx(0.6947f));
        CHECK(style.WindowRounding == doctest::Approx(6.947f));
        CHECK(style.WindowPadding.x == doctest::Approx(12.0f));
        CHECK(style.WindowPadding.y == doctest::Approx(34.0f));
    }

    TEST_CASE("batch6947 apply uses requested registered font variant") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("Requested6947");
        auto* data = theme.get_data();
        data->font_family = "Family6947";
        data->font_size = 18.0f;
        data->default_font_size = 20.0f;
        MockIFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Family6947_18px", "Family6947", 18.0f)));
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 1);
    }

    TEST_CASE("batch6947 missing requested variant falls back to registered default size") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("DefaultFallback6947");
        auto* data = theme.get_data();
        data->font_family = "Family6947";
        data->font_size = 18.0f;
        data->default_font_size = 22.0f;
        MockIFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Family6947_22px", "Family6947", 22.0f)));
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 1);
    }

    TEST_CASE("batch6947 missing family variants falls back to built in safe font") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("BuiltinFallback6947");
        auto* data = theme.get_data();
        data->font_family = "MissingFamily6947";
        data->font_size = 18.0f;
        data->default_font_size = 20.0f;
        MockIFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Roboto-Regular_20px", "Roboto-Regular", 20.0f)));
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 1);
    }

    TEST_CASE("batch6947 no registered safe font leaves font application untouched") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("NoFont6947");
        auto* data = theme.get_data();
        data->font_family = "Missing6947";
        data->font_size = 18.0f;
        data->default_font_size = 20.0f;
        MockIFontManager fonts;
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 0);
    }

    TEST_CASE("batch6947 invalid selected size still permits valid default variant") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("InvalidSelected6947");
        auto* data = theme.get_data();
        data->font_family = "Family6947";
        data->font_size = (std::numeric_limits<float>::quiet_NaN)();
        data->default_font_size = 24.0f;
        MockIFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Family6947_24px", "Family6947", 24.0f)));
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 1);
    }

    TEST_CASE("batch6947 invalid selected and default sizes still permit built in fallback") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("InvalidBoth6947");
        auto* data = theme.get_data();
        data->font_family = "Family6947";
        data->font_size = -1.0f;
        data->default_font_size = (std::numeric_limits<float>::infinity)();
        MockIFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Roboto-Regular_20px", "Roboto-Regular", 20.0f)));
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, &fonts, nullptr);
        theme.apply(&gui);
        CHECK(fonts.apply_calls == 1);
    }

    TEST_CASE("batch6947 absent font manager executes explicit no manager branch") {
        Batch6947ThemeContext context;
        Salix::ImGuiTheme theme("NoManager6947");
        MockIGui gui;
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        CHECK_NOTHROW(theme.apply(&gui));
        CHECK(theme.get_data() != nullptr);
    }

    TEST_CASE("batch6947 save emits Up and Down direction enum branches") {
        Batch1147ThemeTempFile temp;
        Salix::ImGuiTheme theme("Direction6947");
        auto* data = theme.get_data();
        data->window_menu_button_position = ImGuiDir_Up;
        data->color_button_position = ImGuiDir_Down;
        REQUIRE(theme.save_to_yaml(temp.path.string()));
        std::ifstream in(temp.path);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text.find("ImGuiDir_Up") != std::string::npos);
        CHECK(text.find("ImGuiDir_Down") != std::string::npos);
    }

    TEST_CASE("batch6947 save emits enum and hovered flag fallback strings") {
        Batch1147ThemeTempFile temp;
        Salix::ImGuiTheme theme("FallbackEnum6947");
        auto* data = theme.get_data();
        data->window_menu_button_position = static_cast<ImGuiDir>(6947);
        data->color_button_position = static_cast<ImGuiDir>(6947);
        data->hover_flags_for_tooltip_mouse = ImGuiHoveredFlags_None;
        data->hover_flags_for_tooltip_nav = ImGuiHoveredFlags_None;
        REQUIRE(theme.save_to_yaml(temp.path.string()));
        std::ifstream in(temp.path);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text.find("ImGuiDir_None") != std::string::npos);
        CHECK(text.find("ImGuiHoveredFlags_None") != std::string::npos);
    }
}

namespace {
class Batch7047RejectingFontManager : public MockIFontManager {
public:
    bool apply_font(const std::string& name) override {
        ++reject_calls; last_rejected=name; return false;
    }
    int reject_calls=0;
    std::string last_rejected;
};
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch7047") {
    TEST_CASE("batch7047 registered requested font failure reaches apply error branch") {
        Batch6947ThemeContext context; Salix::ImGuiTheme theme("Reject7047");
        theme.get_data()->font_family="RejectFamily7047"; theme.get_data()->font_size=18.0f; theme.get_data()->default_font_size=20.0f;
        Batch7047RejectingFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("RejectFamily7047_18px","RejectFamily7047",18.0f)));
        MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,&fonts,nullptr);
        theme.apply(&gui); CHECK(fonts.reject_calls==1); CHECK(fonts.last_rejected=="RejectFamily7047_18px");
    }

    TEST_CASE("batch7047 default fallback font failure also reaches apply error branch") {
        Batch6947ThemeContext context; Salix::ImGuiTheme theme("RejectDefault7047");
        theme.get_data()->font_family="RejectFamily7047"; theme.get_data()->font_size=18.0f; theme.get_data()->default_font_size=20.0f;
        Batch7047RejectingFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("RejectFamily7047_20px","RejectFamily7047",20.0f)));
        MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,&fonts,nullptr);
        theme.apply(&gui); CHECK(fonts.reject_calls==1); CHECK(fonts.last_rejected=="RejectFamily7047_20px");
    }

    TEST_CASE("batch7047 builtin fallback font failure reaches apply error branch") {
        Batch6947ThemeContext context; Salix::ImGuiTheme theme("RejectBuiltin7047");
        theme.get_data()->font_family="Missing7047"; theme.get_data()->font_size=18.0f; theme.get_data()->default_font_size=20.0f;
        Batch7047RejectingFontManager fonts;
        REQUIRE(fonts.register_font(std::make_unique<MockIFont>("Roboto-Regular_20px","Roboto-Regular",20.0f)));
        MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,&fonts,nullptr);
        theme.apply(&gui); CHECK(fonts.reject_calls==1); CHECK(fonts.last_rejected=="Roboto-Regular_20px");
    }

    TEST_CASE("batch7047 save emits Down and Up direction branches explicitly") {
        Batch1147ThemeTempFile file; Salix::ImGuiTheme theme("Directions7047A");
        theme.get_data()->window_menu_button_position=ImGuiDir_Down; theme.get_data()->color_button_position=ImGuiDir_Up;
        REQUIRE(theme.save_to_yaml(file.path.string()));
        const auto text=Salix::FileManager::read_file_content(file.path.string());
        CHECK(text.find("ImGuiDir_Down")!=std::string::npos); CHECK(text.find("ImGuiDir_Up")!=std::string::npos);
    }

    TEST_CASE("batch7047 save emits explicit None direction branches") {
        Batch1147ThemeTempFile file; Salix::ImGuiTheme theme("Directions7047B");
        theme.get_data()->window_menu_button_position=ImGuiDir_None; theme.get_data()->color_button_position=ImGuiDir_None;
        REQUIRE(theme.save_to_yaml(file.path.string()));
        const auto text=Salix::FileManager::read_file_content(file.path.string());
        CHECK(text.find("ImGuiDir_None")!=std::string::npos);
    }

    TEST_CASE("batch7047 save emits Down for both direction fields") {
        Batch1147ThemeTempFile file; Salix::ImGuiTheme theme("Directions7047C");
        theme.get_data()->window_menu_button_position=ImGuiDir_Down; theme.get_data()->color_button_position=ImGuiDir_Down;
        REQUIRE(theme.save_to_yaml(file.path.string()));
        const auto text=Salix::FileManager::read_file_content(file.path.string());
        CHECK(text.find("ImGuiDir_Down")!=std::string::npos);
    }

    TEST_CASE("batch7047 save emits Up for both direction fields") {
        Batch1147ThemeTempFile file; Salix::ImGuiTheme theme("Directions7047D");
        theme.get_data()->window_menu_button_position=ImGuiDir_Up; theme.get_data()->color_button_position=ImGuiDir_Up;
        REQUIRE(theme.save_to_yaml(file.path.string()));
        const auto text=Salix::FileManager::read_file_content(file.path.string());
        CHECK(text.find("ImGuiDir_Up")!=std::string::npos);
    }

    TEST_CASE("batch7047 no font manager remains safe after full style application") {
        Batch6947ThemeContext context; Salix::ImGuiTheme theme("NoManager7047");
        theme.get_data()->alpha=.7047f; MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        CHECK_NOTHROW(theme.apply(&gui)); CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.7047f));
    }

    TEST_CASE("batch7047 no safe registered font leaves current default font pointer unchanged") {
        Batch6947ThemeContext context; Salix::ImGuiTheme theme("NoSafe7047"); MockIFontManager fonts;
        theme.get_data()->font_family="Absent7047"; theme.get_data()->font_size=18.0f; theme.get_data()->default_font_size=20.0f;
        ImFont* before=ImGui::GetIO().FontDefault; MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,&fonts,nullptr);
        theme.apply(&gui); CHECK(ImGui::GetIO().FontDefault==before); CHECK(fonts.apply_calls==0);
    }

    TEST_CASE("batch7047 malformed existing YAML remains a controlled load failure") {
        Batch1147ThemeTempFile file; file.write("Enums: [unterminated\n"); Salix::ImGuiTheme theme("Malformed7047");
        CHECK_FALSE(theme.load_from_yaml(file.path.string())); CHECK(theme.get_name()=="Malformed7047");
    }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch7478") {
    TEST_CASE("batch7478 default theme keeps owned data") { Salix::ImGuiTheme theme; REQUIRE(theme.get_data()!=nullptr); CHECK(theme.get_name()=="Default ImGui Theme"); }
    TEST_CASE("batch7478 explicit theme keeps requested name and source path") { Salix::ImGuiTheme theme("Named7478","Themes/Named7478.yaml"); CHECK(theme.get_name()=="Named7478"); CHECK(theme.get_file_path()=="Themes/Named7478.yaml"); }
    TEST_CASE("batch7478 missing YAML load fails without replacing explicit identity") { Salix::ImGuiTheme theme("Keep7478"); CHECK_FALSE(theme.load_from_yaml("__batch7478_missing_theme__.yaml")); CHECK(theme.get_name()=="Keep7478"); }
    TEST_CASE("batch7478 malformed YAML load remains controlled") { Batch1147ThemeTempFile file; file.write("Style: [unterminated\n"); Salix::ImGuiTheme theme("Malformed7478"); CHECK_FALSE(theme.load_from_yaml(file.path.string())); CHECK(theme.get_name()=="Malformed7478"); }
    TEST_CASE("batch7478 default theme adopts YAML supplied identity") { Batch1147ThemeTempFile file; file.write("name: Loaded7478\ngui_type: ImGui\n"); Salix::ImGuiTheme theme; REQUIRE(theme.load_from_yaml(file.path.string())); CHECK(theme.get_name()=="Loaded7478"); }
    TEST_CASE("batch7478 explicit theme retains caller identity while loading YAML") { Batch1147ThemeTempFile file; file.write("name: Ignored7478\ngui_type: ImGui\n"); Salix::ImGuiTheme theme("Explicit7478"); REQUIRE(theme.load_from_yaml(file.path.string())); CHECK(theme.get_name()=="Explicit7478"); }
    TEST_CASE("batch7478 saving to empty path remains rejected") { Salix::ImGuiTheme theme("NoPath7478"); CHECK_FALSE(theme.save_to_yaml("")); }
    TEST_CASE("batch7478 saving valid theme produces a regular YAML file") { Batch1147ThemeTempFile file; Salix::ImGuiTheme theme("Saved7478"); REQUIRE(theme.save_to_yaml(file.path.string())); CHECK(std::filesystem::is_regular_file(file.path)); }
    TEST_CASE("batch7478 save load roundtrip preserves alpha") { Batch1147ThemeTempFile file; Salix::ImGuiTheme source("Roundtrip7478"); source.get_data()->alpha=.7478f; REQUIRE(source.save_to_yaml(file.path.string())); Salix::ImGuiTheme restored; REQUIRE(restored.load_from_yaml(file.path.string())); CHECK(restored.get_data()->alpha==doctest::Approx(.7478f)); }
    TEST_CASE("batch7478 apply copies alpha into active ImGui style") { Batch6947ThemeContext context; Salix::ImGuiTheme theme("Apply7478"); theme.get_data()->alpha=.7478f; MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr); CHECK_NOTHROW(theme.apply(&gui)); CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.7478f)); }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch7878") {
    TEST_CASE("batch7878 default theme data remains owned after failed missing load") { Salix::ImGuiTheme t;auto* d=t.get_data();REQUIRE(d);CHECK_FALSE(t.load_from_yaml("__batch7878_missing_theme__.yaml"));CHECK(t.get_data()==d); }
    TEST_CASE("batch7878 explicit theme identity survives malformed YAML") { Batch1147ThemeTempFile f;f.write("Style: [unterminated\n");Salix::ImGuiTheme t("Keep7878");CHECK_FALSE(t.load_from_yaml(f.path.string()));CHECK(t.get_name()=="Keep7878"); }
    TEST_CASE("batch7878 alpha roundtrip persists exact finite value") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("Alpha7878");a.get_data()->alpha=.7878f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->alpha==doctest::Approx(.7878f)); }
    TEST_CASE("batch7878 window rounding roundtrip persists") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("Round7878");a.get_data()->window_rounding=7.878f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->window_rounding==doctest::Approx(7.878f)); }
    TEST_CASE("batch7878 explicit source path remains metadata after save") { Batch1147ThemeTempFile f;Salix::ImGuiTheme t("Path7878","original7878.theme");REQUIRE(t.save_to_yaml(f.path.string()));CHECK(t.get_file_path()=="original7878.theme"); }
    TEST_CASE("batch7878 applying finite alpha updates live style") { Batch6947ThemeContext c;Salix::ImGuiTheme t("Apply7878");t.get_data()->alpha=.78f;MockIGui gui;gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);t.apply(&gui);CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.78f)); }
    TEST_CASE("batch7878 applying theme with absent font manager remains safe") { Batch6947ThemeContext c;Salix::ImGuiTheme t("NoFont7878");MockIGui gui;gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);CHECK_NOTHROW(t.apply(&gui)); }
    TEST_CASE("batch7878 save to directory path remains controlled failure") { auto dir=std::filesystem::temp_directory_path()/ "Batch7878ThemeDir";std::error_code ec;std::filesystem::create_directories(dir,ec);Salix::ImGuiTheme t("Dir7878");CHECK_FALSE(t.save_to_yaml(dir.string()));std::filesystem::remove_all(dir,ec); }
    TEST_CASE("batch7878 minimal YAML loads gui identity without losing explicit name") { Batch1147ThemeTempFile f;f.write("name: File7878\ngui_type: ImGui\n");Salix::ImGuiTheme t("Explicit7878");REQUIRE(t.load_from_yaml(f.path.string()));CHECK(t.get_name()=="Explicit7878");CHECK(t.get_data()->get_gui_type()=="ImGui"); }
    TEST_CASE("batch7878 direction enums save both up and down representations") { Batch1147ThemeTempFile f;Salix::ImGuiTheme t("Dirs7878");t.get_data()->window_menu_button_position=ImGuiDir_Up;t.get_data()->color_button_position=ImGuiDir_Down;REQUIRE(t.save_to_yaml(f.path.string()));auto text=Salix::FileManager::read_file_content(f.path.string());CHECK(text.find("ImGuiDir_Up")!=std::string::npos);CHECK(text.find("ImGuiDir_Down")!=std::string::npos); }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch8078") {
    TEST_CASE("batch8078 invalid window direction serializes fallback none") { Batch1147ThemeTempFile f; Salix::ImGuiTheme t("Dir8078A"); t.get_data()->window_menu_button_position=static_cast<ImGuiDir>(8078); REQUIRE(t.save_to_yaml(f.path.string())); auto s=Salix::FileManager::read_file_content(f.path.string()); CHECK(s.find("ImGuiDir_None")!=std::string::npos); }
    TEST_CASE("batch8078 invalid color direction serializes fallback none") { Batch1147ThemeTempFile f; Salix::ImGuiTheme t("Dir8078B"); t.get_data()->color_button_position=static_cast<ImGuiDir>(8078); REQUIRE(t.save_to_yaml(f.path.string())); auto s=Salix::FileManager::read_file_content(f.path.string()); CHECK(s.find("ImGuiDir_None")!=std::string::npos); }
    TEST_CASE("batch8078 unknown tree flags serialize fallback none") { Batch1147ThemeTempFile f; Salix::ImGuiTheme t("Flags8078"); t.get_data()->tree_lines_flags=static_cast<ImGuiTreeNodeFlags>(1u<<30); REQUIRE(t.save_to_yaml(f.path.string())); auto s=Salix::FileManager::read_file_content(f.path.string()); CHECK(s.find("ImGuiTreeNodeFlags_None")!=std::string::npos); }
    TEST_CASE("batch8078 default theme save load preserves gui type") { Batch1147ThemeTempFile f; Salix::ImGuiTheme a("Theme8078"); REQUIRE(a.save_to_yaml(f.path.string())); Salix::ImGuiTheme b; REQUIRE(b.load_from_yaml(f.path.string())); CHECK(b.get_data()->get_gui_type()=="ImGui"); }
    TEST_CASE("batch8078 save load preserves default font name") { Batch1147ThemeTempFile f; Salix::ImGuiTheme a("Font8078"); a.get_data()->default_font_name="Roboto_8078"; REQUIRE(a.save_to_yaml(f.path.string())); Salix::ImGuiTheme b; REQUIRE(b.load_from_yaml(f.path.string())); CHECK(b.get_data()->default_font_name=="Roboto_8078"); }
    TEST_CASE("batch8078 save load preserves mouse cursor scale") { Batch1147ThemeTempFile f; Salix::ImGuiTheme a("Cursor8078"); a.get_data()->mouse_cursor_scale=1.8078f; REQUIRE(a.save_to_yaml(f.path.string())); Salix::ImGuiTheme b; REQUIRE(b.load_from_yaml(f.path.string())); CHECK(b.get_data()->mouse_cursor_scale==doctest::Approx(1.8078f)); }
    TEST_CASE("batch8078 save load preserves anti aliased lines flag") { Batch1147ThemeTempFile f; Salix::ImGuiTheme a("AA8078"); a.get_data()->anti_aliased_lines=false; REQUIRE(a.save_to_yaml(f.path.string())); Salix::ImGuiTheme b; REQUIRE(b.load_from_yaml(f.path.string())); CHECK_FALSE(b.get_data()->anti_aliased_lines); }
    TEST_CASE("batch8078 malformed YAML still preserves explicit theme name") { Batch1147ThemeTempFile f; f.write("Colors: [bad\n"); Salix::ImGuiTheme t("Keep8078"); CHECK_FALSE(t.load_from_yaml(f.path.string())); CHECK(t.get_name()=="Keep8078"); }
    TEST_CASE("batch8078 directory save path remains controlled failure") { auto d=std::filesystem::temp_directory_path()/"Batch8078ThemeDirectory"; std::error_code ec; std::filesystem::create_directories(d,ec); Salix::ImGuiTheme t("DirFail8078"); CHECK_FALSE(t.save_to_yaml(d.string())); std::filesystem::remove_all(d,ec); }
    TEST_CASE("batch8078 apply finite rounding updates live ImGui style") { Batch6947ThemeContext c; Salix::ImGuiTheme t("Apply8078"); t.get_data()->window_rounding=8.078f; MockIGui gui; gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr); t.apply(&gui); CHECK(ImGui::GetStyle().WindowRounding==doctest::Approx(8.078f)); }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch8178") {
    TEST_CASE("batch8178 save load preserves alpha zero") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8178A");a.get_data()->alpha=0.0f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->alpha==doctest::Approx(0.0f)); }
    TEST_CASE("batch8178 save load preserves alpha one") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8178B");a.get_data()->alpha=1.0f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->alpha==doctest::Approx(1.0f)); }
    TEST_CASE("batch8178 save load preserves negative rounding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8178C");a.get_data()->window_rounding=-8.178f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->window_rounding==doctest::Approx(-8.178f)); }
    TEST_CASE("batch8178 save load preserves empty font name") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8178D");a.get_data()->default_font_name.clear();REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->default_font_name.empty()); }
    TEST_CASE("batch8178 missing YAML preserves explicit name") { Salix::ImGuiTheme t("Keep8178");CHECK_FALSE(t.load_from_yaml("__missing8178__.yaml"));CHECK(t.get_name()=="Keep8178"); }
    TEST_CASE("batch8178 malformed YAML remains false") { Batch1147ThemeTempFile f;f.write("Enums: [bad\n");Salix::ImGuiTheme t("Bad8178");CHECK_FALSE(t.load_from_yaml(f.path.string())); }
    TEST_CASE("batch8178 directory save remains false") { auto d=std::filesystem::temp_directory_path()/"Theme8178Dir";std::error_code ec;std::filesystem::create_directories(d,ec);Salix::ImGuiTheme t("Dir8178");CHECK_FALSE(t.save_to_yaml(d.string()));std::filesystem::remove_all(d,ec); }
    TEST_CASE("batch8178 apply alpha updates style") { Batch6947ThemeContext c;Salix::ImGuiTheme t("Apply8178A");t.get_data()->alpha=.8178f;MockIGui g;g.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);t.apply(&g);CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.8178f)); }
    TEST_CASE("batch8178 apply rounding updates style") { Batch6947ThemeContext c;Salix::ImGuiTheme t("Apply8178B");t.get_data()->window_rounding=8.178f;MockIGui g;g.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);t.apply(&g);CHECK(ImGui::GetStyle().WindowRounding==doctest::Approx(8.178f)); }
    TEST_CASE("batch8178 default theme retains owned data") { Salix::ImGuiTheme t; REQUIRE(t.get_data()!=nullptr); CHECK(t.get_data()->get_gui_type()=="ImGui"); }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch8278") {
    TEST_CASE("batch8278 theme roundtrip preserves hover short delay") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8278A");a.get_data()->hover_delay_short=.8278f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->hover_delay_short==doctest::Approx(.8278f)); }
    TEST_CASE("batch8278 theme roundtrip preserves hover normal delay") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8278B");a.get_data()->hover_delay_normal=1.8278f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->hover_delay_normal==doctest::Approx(1.8278f)); }
    TEST_CASE("batch8278 theme roundtrip preserves cursor scale") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8278C");a.get_data()->mouse_cursor_scale=2.8278f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->mouse_cursor_scale==doctest::Approx(2.8278f)); }
    TEST_CASE("batch8278 theme roundtrip preserves anti alias fill false") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8278D");a.get_data()->anti_aliased_fill=false;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK_FALSE(b.get_data()->anti_aliased_fill); }
    TEST_CASE("batch8278 theme roundtrip preserves tab rounding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("T8278E");a.get_data()->tab_rounding=8.278f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->tab_rounding==doctest::Approx(8.278f)); }
    TEST_CASE("batch8278 theme missing file remains controlled") { Salix::ImGuiTheme t("T8278F");CHECK_FALSE(t.load_from_yaml("__missing_theme_8278__.yaml"));CHECK(t.get_name()=="T8278F"); }
    TEST_CASE("batch8278 theme malformed map remains controlled") { Batch1147ThemeTempFile f;f.write("StyleFloats: [bad\n");Salix::ImGuiTheme t("T8278G");CHECK_FALSE(t.load_from_yaml(f.path.string())); }
    TEST_CASE("batch8278 theme directory save remains controlled") { auto d=std::filesystem::temp_directory_path()/"Theme8278Dir";std::error_code ec;std::filesystem::create_directories(d,ec);Salix::ImGuiTheme t("T8278H");CHECK_FALSE(t.save_to_yaml(d.string()));std::filesystem::remove_all(d,ec); }
    TEST_CASE("batch8278 theme apply preserves finite alpha") { Batch6947ThemeContext c;Salix::ImGuiTheme t("T8278I");t.get_data()->alpha=.8278f;MockIGui g;g.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);t.apply(&g);CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.8278f)); }
    TEST_CASE("batch8278 fresh theme always owns data") { Salix::ImGuiTheme t("T8278J");REQUIRE(t.get_data()!=nullptr);CHECK(t.get_data()->get_gui_type()=="ImGui"); }
}

TEST_SUITE("Salix::gui::ImGuiThemeBatch8378") {
    TEST_CASE("batch8378 theme roundtrip preserves window border hover padding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("A8378");a.get_data()->window_border_hover_padding=.8378f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->window_border_hover_padding==doctest::Approx(.8378f)); }
    TEST_CASE("batch8378 theme roundtrip preserves child rounding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("B8378");a.get_data()->child_rounding=8.378f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->child_rounding==doctest::Approx(8.378f)); }
    TEST_CASE("batch8378 theme roundtrip preserves frame rounding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("C8378");a.get_data()->frame_rounding=3.878f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->frame_rounding==doctest::Approx(3.878f)); }
    TEST_CASE("batch8378 theme roundtrip preserves popup rounding") { Batch1147ThemeTempFile f;Salix::ImGuiTheme a("D8378");a.get_data()->popup_rounding=2.378f;REQUIRE(a.save_to_yaml(f.path.string()));Salix::ImGuiTheme b;REQUIRE(b.load_from_yaml(f.path.string()));CHECK(b.get_data()->popup_rounding==doctest::Approx(2.378f)); }
    TEST_CASE("batch8378 theme malformed YAML remains controlled") { Batch1147ThemeTempFile f;f.write("StyleVec2: [bad\n");Salix::ImGuiTheme t("E8378");CHECK_FALSE(t.load_from_yaml(f.path.string())); }
    TEST_CASE("batch8378 theme missing YAML keeps identity") { Salix::ImGuiTheme t("F8378");CHECK_FALSE(t.load_from_yaml("__missing8378__.yaml"));CHECK(t.get_name()=="F8378"); }
    TEST_CASE("batch8378 theme save directory remains false") { auto d=std::filesystem::temp_directory_path()/"Theme8378";std::error_code ec;std::filesystem::create_directories(d,ec);Salix::ImGuiTheme t("G8378");CHECK_FALSE(t.save_to_yaml(d.string()));std::filesystem::remove_all(d,ec); }
    TEST_CASE("batch8378 theme apply preserves alpha") { Batch6947ThemeContext c;Salix::ImGuiTheme t("H8378");t.get_data()->alpha=.8378f;MockIGui g;g.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);t.apply(&g);CHECK(ImGui::GetStyle().Alpha==doctest::Approx(.8378f)); }
    TEST_CASE("batch8378 theme explicit path remains metadata") { Salix::ImGuiTheme t("I8378","Themes/I8378.yaml");CHECK(t.get_file_path()=="Themes/I8378.yaml"); }
    TEST_CASE("batch8378 fresh theme owns ImGui data") { Salix::ImGuiTheme t("J8378");REQUIRE(t.get_data());CHECK(t.get_data()->get_gui_type()=="ImGui"); }
}

