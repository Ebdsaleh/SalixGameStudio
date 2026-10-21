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
