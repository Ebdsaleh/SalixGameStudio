// =================================================================================
// Filename:    src/Tests/SalixEngine/core/ApplicationConfig.test.cpp
// Description: Contains unit tests for the ApplicationConfig struct.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/ApplicationConfig.h>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>


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


// Additional stable contracts for the 847+ coverage batch.
TEST_SUITE("Salix::core::ApplicationConfigBatch847") {
    TEST_CASE("application config zero target FPS leaves all backend selections untouched") {
        Salix::ApplicationConfig c; c.target_fps=0; CHECK(c.target_fps==0); CHECK(c.renderer_type==Salix::RendererType::SDL); CHECK(c.gui_type==Salix::GuiType::ImGui); CHECK(c.timer_type==Salix::TimerType::SDL);
    }

    TEST_CASE("application config negative target FPS is stored without mutating startup state") {
        Salix::ApplicationConfig c; c.initial_state=Salix::AppStateType::Game; c.target_fps=-1; CHECK(c.target_fps==-1); CHECK(c.initial_state==Salix::AppStateType::Game);
    }

    TEST_CASE("window width and height can independently represent zero sized startup surfaces") {
        Salix::ApplicationConfig c; c.window_config.width=0; c.window_config.height=0; CHECK(c.window_config.width==0); CHECK(c.window_config.height==0); CHECK(c.window_config.title=="SalixGameStudio");
    }

    TEST_CASE("window title supports empty and multiline values without altering dimensions") {
        Salix::ApplicationConfig c; c.window_config.width=640; c.window_config.height=480; c.window_config.title=""; CHECK(c.window_config.title.empty()); c.window_config.title="Line1\nLine2"; CHECK(c.window_config.title=="Line1\nLine2"); CHECK(c.window_config.width==640); CHECK(c.window_config.height==480);
    }

    TEST_CASE("GUI ratios may be configured beyond unit range without cross modifying scales") {
        Salix::GuiSettings g; g.dialog_width_ratio=1.5f; g.dialog_height_ratio=-0.25f; CHECK(g.dialog_width_ratio==doctest::Approx(1.5f)); CHECK(g.dialog_height_ratio==doctest::Approx(-0.25f)); CHECK(g.font_scaling==doctest::Approx(1.f)); CHECK(g.global_dpi_scaling==doctest::Approx(1.f));
    }

    TEST_CASE("GUI scaling values preserve zero and negative assignments independently") {
        Salix::GuiSettings g; g.font_scaling=0.f; g.global_dpi_scaling=-2.f; CHECK(g.font_scaling==0.f); CHECK(g.global_dpi_scaling==-2.f); CHECK(g.dialog_width_ratio==doctest::Approx(0.7f));
    }

    TEST_CASE("copy assignment replaces every window dimension and GUI ratio") {
        Salix::ApplicationConfig a,b; a.window_config.width=333; a.window_config.height=777; a.gui_settings.dialog_width_ratio=0.2f; a.gui_settings.dialog_height_ratio=0.9f; b=a; CHECK(b.window_config.width==333); CHECK(b.window_config.height==777); CHECK(b.gui_settings.dialog_width_ratio==doctest::Approx(0.2f)); CHECK(b.gui_settings.dialog_height_ratio==doctest::Approx(0.9f));
    }

    TEST_CASE("mutating copied GUI settings never aliases the source configuration") {
        Salix::ApplicationConfig source; source.gui_settings.font_scaling=1.25f; Salix::ApplicationConfig copy=source; copy.gui_settings.font_scaling=3.f; copy.gui_settings.global_dpi_scaling=2.f; CHECK(source.gui_settings.font_scaling==doctest::Approx(1.25f)); CHECK(source.gui_settings.global_dpi_scaling==doctest::Approx(1.f));
    }

    TEST_CASE("all top level enum selections survive copy construction together") {
        Salix::ApplicationConfig source; source.renderer_type=Salix::RendererType::OpenGL; source.initial_state=Salix::AppStateType::Options; source.gui_type=Salix::GuiType::None; source.timer_type=Salix::TimerType::Chrono; const Salix::ApplicationConfig copy(source); CHECK(copy.renderer_type==source.renderer_type); CHECK(copy.initial_state==source.initial_state); CHECK(copy.gui_type==source.gui_type); CHECK(copy.timer_type==source.timer_type);
    }

    TEST_CASE("fresh configuration remains unchanged after another instance is fully mutated") {
        Salix::ApplicationConfig a,b; a.window_config.title="Mutated"; a.window_config.width=1; a.window_config.height=2; a.target_fps=240; a.renderer_type=Salix::RendererType::OpenGL; a.initial_state=Salix::AppStateType::Editor; a.gui_type=Salix::GuiType::None; a.timer_type=Salix::TimerType::Chrono; CHECK(b.window_config.title=="SalixGameStudio"); CHECK(b.window_config.width==1280); CHECK(b.window_config.height==720); CHECK(b.target_fps==60); CHECK(b.renderer_type==Salix::RendererType::SDL);
    }
}

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
