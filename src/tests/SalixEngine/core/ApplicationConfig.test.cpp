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