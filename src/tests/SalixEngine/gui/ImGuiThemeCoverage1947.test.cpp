#include <doctest.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>

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

TEST_SUITE("Salix::gui::ImGuiThemeCoverage1947") {
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
