// =================================================================================
// Filename:    src/Tests/SalixEngine/core/ApplicationConfig.test.cpp
// Description: Contains unit tests for the ApplicationConfig struct.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/ApplicationConfig.h>


TEST_SUITE("Salix::core::ApplicationConfig") {

    TEST_CASE("default constructor initializes with correct default values") {
        // ARRANGE: Create a default-constructed config object/
        Salix::ApplicationConfig config;

        // ASSERT: Check that all members have the expected default values.
        // Check top-level ApplicationConfig members.
        CHECK(config.target_fps == 60);
        CHECK(config.renderer_type == Salix::RendererType::SDL);
        CHECK(config.initial_state == Salix::AppStateType::Launch);
        CHECK(config.gui_type == Salix::GuiType::ImGui);
        CHECK(config.timer_type == Salix::TimerType::SDL);

        // Check nested WindowConfig members
        CHECK(config.window_config.title == "SalixGameStudio");
        CHECK(config.window_config.width == 1280);
        CHECK(config.window_config.height == 720);

        // Check nested GuiSettings members
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.7f));
        CHECK(config.gui_settings.dialog_height_ratio == doctest::Approx(0.75f));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(1.0f));
        CHECK(config.gui_settings.global_dpi_scaling == doctest::Approx(1.0f));
    }
}

// Window and GUI configuration value semantics (no OS windows opened).
TEST_SUITE("Salix::core::ApplicationConfigExtended") {
    TEST_CASE("application config fresh instances hold independent window title storage") {
        Salix::ApplicationConfig first, second;
        first.window_config.title = "Editor session";
        CHECK(first.window_config.title == "Editor session");
        CHECK(second.window_config.title == "SalixGameStudio");
    }

    TEST_CASE("application config window dimensions can be configured independently of rendering mode") {
        Salix::ApplicationConfig config;
        config.window_config.width = 3840;
        config.window_config.height = 2160;
        CHECK(config.window_config.width == 3840);
        CHECK(config.window_config.height == 2160);
        CHECK(config.renderer_type == Salix::RendererType::SDL);
    }

    TEST_CASE("application config copy construction retains GUI scale and window dimensions") {
        Salix::ApplicationConfig config;
        config.window_config.title = "Inspection";
        config.window_config.width = 1600;
        config.gui_settings.font_scaling = 1.75f;
        const Salix::ApplicationConfig copied(config);
        CHECK(copied.window_config.title == "Inspection");
        CHECK(copied.window_config.width == 1600);
        CHECK(copied.gui_settings.font_scaling == doctest::Approx(1.75f));
    }

    TEST_CASE("application config copying a configuration never aliases its nested window title") {
        Salix::ApplicationConfig original;
        original.window_config.title = "Original";
        Salix::ApplicationConfig copy = original;
        copy.window_config.title = "Copy";
        CHECK(original.window_config.title == "Original");
        CHECK(copy.window_config.title == "Copy");
    }

    TEST_CASE("application config renderer and GUI preferences are independent choices") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        config.gui_type = Salix::GuiType::None;
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.gui_type == Salix::GuiType::None);
        CHECK(config.timer_type == Salix::TimerType::SDL);
    }

    TEST_CASE("application config startup editor state does not implicitly alter target frame rate") {
        Salix::ApplicationConfig config;
        config.initial_state = Salix::AppStateType::Editor;
        CHECK(config.initial_state == Salix::AppStateType::Editor);
        CHECK(config.target_fps == 60);
        config.target_fps = 144;
        CHECK(config.initial_state == Salix::AppStateType::Editor);
        CHECK(config.target_fps == 144);
    }

    TEST_CASE("application config timer mode switch retains renderer state and game startup") {
        Salix::ApplicationConfig config;
        config.initial_state = Salix::AppStateType::Game;
        config.renderer_type = Salix::RendererType::Vulkan;
        config.timer_type = Salix::TimerType::Chrono;
        CHECK(config.initial_state == Salix::AppStateType::Game);
        CHECK(config.renderer_type == Salix::RendererType::Vulkan);
        CHECK(config.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("application config dialog width height ratios remain separately adjustable") {
        Salix::GuiSettings ui;
        ui.dialog_width_ratio = 0.45f;
        ui.dialog_height_ratio = 0.8f;
        CHECK(ui.dialog_width_ratio == doctest::Approx(0.45f));
        CHECK(ui.dialog_height_ratio == doctest::Approx(0.8f));
        CHECK(ui.font_scaling == doctest::Approx(1.f));
    }

    TEST_CASE("application config DPI and font scales are independent configuration knobs") {
        Salix::ApplicationConfig config;
        config.gui_settings.global_dpi_scaling = 2.f;
        config.gui_settings.font_scaling = 1.25f;
        CHECK(config.gui_settings.global_dpi_scaling == doctest::Approx(2.f));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(1.25f));
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.7f));
    }

    TEST_CASE("application config configuration assignment replaces full nested and top-level state") {
        Salix::ApplicationConfig saved;
        saved.window_config.title = "Recovered project";
        saved.window_config.width = 1024;
        saved.initial_state = Salix::AppStateType::Options;
        saved.renderer_type = Salix::RendererType::DirectX;
        saved.target_fps = 30;
        saved.gui_settings.global_dpi_scaling = 1.5f;
        Salix::ApplicationConfig restored;
        restored = saved;
        CHECK(restored.window_config.title == saved.window_config.title);
        CHECK(restored.window_config.width == 1024);
        CHECK(restored.initial_state == Salix::AppStateType::Options);
        CHECK(restored.renderer_type == Salix::RendererType::DirectX);
        CHECK(restored.target_fps == 30);
        CHECK(restored.gui_settings.global_dpi_scaling == doctest::Approx(1.5f));
    }
}
