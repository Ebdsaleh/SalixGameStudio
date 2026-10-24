#include <doctest.h>
#include <Salix/core/ApplicationConfig.h>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

TEST_SUITE("Salix::core::ApplicationConfigClosure1747") {
    TEST_CASE("batch1747 application config changing renderer leaves other startup selections intact") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.timer_type == Salix::TimerType::SDL);
        CHECK(config.gui_type == Salix::GuiType::ImGui);
        CHECK(config.initial_state == Salix::AppStateType::Launch);
        CHECK(config.target_fps == 60);
    }

    TEST_CASE("batch1747 application config changing initial state does not rewrite window state") {
        Salix::ApplicationConfig config;
        config.window_config.title = "State Test";
        config.window_config.width = 1440;
        config.window_config.height = 900;
        config.initial_state = Salix::AppStateType::Game;
        CHECK(config.initial_state == Salix::AppStateType::Game);
        CHECK(config.window_config.title == "State Test");
        CHECK(config.window_config.width == 1440);
        CHECK(config.window_config.height == 900);
    }

    TEST_CASE("batch1747 application config stores signed FPS integer boundaries without hidden normalization") {
        Salix::ApplicationConfig low, high;
        low.target_fps = std::numeric_limits<int>::min();
        high.target_fps = std::numeric_limits<int>::max();
        CHECK(low.target_fps == std::numeric_limits<int>::min());
        CHECK(high.target_fps == std::numeric_limits<int>::max());
    }

    TEST_CASE("batch1747 application config self assignment preserves nested and top level state") {
        Salix::ApplicationConfig config;
        config.window_config.title = "Self";
        config.window_config.width = 333;
        config.gui_settings.dialog_width_ratio = 0.42f;
        config.renderer_type = Salix::RendererType::Vulkan;
        config = config;
        CHECK(config.window_config.title == "Self");
        CHECK(config.window_config.width == 333);
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.42f));
        CHECK(config.renderer_type == Salix::RendererType::Vulkan);
    }

    TEST_CASE("batch1747 application config move construction transfers the complete configured value") {
        Salix::ApplicationConfig source;
        source.window_config.title = "Moved Config";
        source.window_config.width = 2560;
        source.window_config.height = 1080;
        source.target_fps = 144;
        source.gui_settings.font_scaling = 1.5f;
        Salix::ApplicationConfig moved(std::move(source));
        CHECK(moved.window_config.title == "Moved Config");
        CHECK(moved.window_config.width == 2560);
        CHECK(moved.window_config.height == 1080);
        CHECK(moved.target_fps == 144);
        CHECK(moved.gui_settings.font_scaling == doctest::Approx(1.5f));
    }

    TEST_CASE("batch1747 application config swapping instances exchanges complete nested configuration") {
        Salix::ApplicationConfig first, second;
        first.window_config.title = "First";
        first.target_fps = 30;
        first.gui_settings.global_dpi_scaling = 1.25f;
        second.window_config.title = "Second";
        second.target_fps = 240;
        second.gui_settings.global_dpi_scaling = 2.0f;
        std::swap(first, second);
        CHECK(first.window_config.title == "Second");
        CHECK(first.target_fps == 240);
        CHECK(first.gui_settings.global_dpi_scaling == doctest::Approx(2.0f));
        CHECK(second.window_config.title == "First");
        CHECK(second.target_fps == 30);
    }

    TEST_CASE("batch1747 application config nested GUI copies remain independently mutable") {
        Salix::ApplicationConfig original;
        original.gui_settings.dialog_width_ratio = 0.2f;
        original.gui_settings.dialog_height_ratio = 0.3f;
        Salix::ApplicationConfig copy = original;
        copy.gui_settings.dialog_width_ratio = 0.8f;
        copy.gui_settings.dialog_height_ratio = 0.9f;
        CHECK(original.gui_settings.dialog_width_ratio == doctest::Approx(0.2f));
        CHECK(original.gui_settings.dialog_height_ratio == doctest::Approx(0.3f));
        CHECK(copy.gui_settings.dialog_width_ratio == doctest::Approx(0.8f));
        CHECK(copy.gui_settings.dialog_height_ratio == doctest::Approx(0.9f));
    }

    TEST_CASE("batch1747 application config window title copy preserves embedded NUL bytes") {
        Salix::ApplicationConfig config;
        config.window_config.title = std::string("Salix\0Editor", 12);
        Salix::ApplicationConfig copy = config;
        CHECK(copy.window_config.title.size() == 12);
        CHECK(copy.window_config.title == config.window_config.title);
        CHECK(copy.window_config.title[5] == '\0');
    }

    TEST_CASE("batch1747 application config vector storage preserves independent element values") {
        Salix::ApplicationConfig a, b;
        a.window_config.title = "A";
        a.target_fps = 30;
        b.window_config.title = "B";
        b.target_fps = 120;
        std::vector<Salix::ApplicationConfig> configs{a, b};
        REQUIRE(configs.size() == 2);
        configs[0].target_fps = 75;
        CHECK(configs[0].window_config.title == "A");
        CHECK(configs[0].target_fps == 75);
        CHECK(configs[1].window_config.title == "B");
        CHECK(configs[1].target_fps == 120);
    }

    TEST_CASE("batch1747 application config GUI scale fields remain independent at extreme finite values") {
        Salix::ApplicationConfig config;
        config.gui_settings.font_scaling = -1000.0f;
        config.gui_settings.global_dpi_scaling = 1000.0f;
        config.gui_settings.dialog_width_ratio = 0.0f;
        config.gui_settings.dialog_height_ratio = 2.0f;
        CHECK(config.gui_settings.font_scaling == doctest::Approx(-1000.0f));
        CHECK(config.gui_settings.global_dpi_scaling == doctest::Approx(1000.0f));
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.0f));
        CHECK(config.gui_settings.dialog_height_ratio == doctest::Approx(2.0f));
    }
}
