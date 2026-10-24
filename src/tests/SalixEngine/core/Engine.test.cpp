#include <doctest.h>
#include <Salix/core/Engine.h>
#include <Salix/core/EngineMode.h>
#include <memory>

TEST_SUITE("Salix::core::EngineBatch1647") {
    TEST_CASE("fresh engine begins stopped") {
        Salix::Engine engine;
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("fresh engine begins in neutral engine mode") {
        Salix::Engine engine;
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("fresh engine begins with unit time scale") {
        Salix::Engine engine;
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("preinitialized engine exposes null owned subsystem pointers") {
        Salix::Engine engine;
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("engine running setter toggles stopped and running states explicitly") {
        Salix::Engine engine;
        CHECK(engine.is_running(true));
        CHECK(engine.is_running());
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("engine mode setter roundtrips launch editor game options and debug") {
        Salix::Engine engine;
        const Salix::EngineMode modes[] = {
            Salix::EngineMode::Launch,
            Salix::EngineMode::Editor,
            Salix::EngineMode::Game,
            Salix::EngineMode::Options,
            Salix::EngineMode::Debug
        };
        for (auto mode : modes) {
            engine.set_mode(mode);
            CHECK(engine.get_mode() == mode);
        }
    }

    TEST_CASE("engine time scale preserves fractional zero and negative assignments") {
        Salix::Engine engine;
        engine.set_time_scale(0.25f);
        CHECK(engine.get_time_scale() == doctest::Approx(0.25f));
        engine.set_time_scale(0.0f);
        CHECK(engine.get_time_scale() == doctest::Approx(0.0f));
        engine.set_time_scale(-2.0f);
        CHECK(engine.get_time_scale() == doctest::Approx(-2.0f));
    }

    TEST_CASE("engine no-op state stack methods preserve preinitialization state") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(0.5f);
        engine.push_state(nullptr);
        engine.pop_state();
        engine.change_state(nullptr);
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.get_time_scale() == doctest::Approx(0.5f));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("separate engine instances keep independent running mode and time scale state") {
        Salix::Engine first;
        Salix::Engine second;
        first.is_running(true);
        first.set_mode(Salix::EngineMode::Game);
        first.set_time_scale(2.0f);
        CHECK(first.is_running());
        CHECK(first.get_mode() == Salix::EngineMode::Game);
        CHECK(first.get_time_scale() == doctest::Approx(2.0f));
        CHECK_FALSE(second.is_running());
        CHECK(second.get_mode() == Salix::EngineMode::None);
        CHECK(second.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine supports polymorphic ownership through EngineInterface") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        REQUIRE(engine != nullptr);
        CHECK_FALSE(engine->is_running());
        engine->set_mode(Salix::EngineMode::Editor);
        engine->set_time_scale(1.5f);
        CHECK(engine->get_mode() == Salix::EngineMode::Editor);
        CHECK(engine->get_time_scale() == doctest::Approx(1.5f));
    }
}

// ---- Relocated from src/tests/SalixEngine/core/ApplicationConfig.test.cpp; canonical tests mirror production .cpp files ----
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


// ---- Relocated from src/tests/SalixEngine/core/Core.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/Core.h>
#include <filesystem>
#include <string>

namespace {
struct Batch1647ProjectRootGuard {
    std::filesystem::path original = Salix::g_project_root_path;
    ~Batch1647ProjectRootGuard() { Salix::g_project_root_path = original; }
};
}

TEST_SUITE("Salix::core::CoreBatch1647") {
    TEST_CASE("project root global accepts an empty path") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path.clear();
        CHECK(Salix::g_project_root_path.empty());
    }

    TEST_CASE("project root global preserves a relative project directory") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "Example";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Projects") / "Example");
    }

    TEST_CASE("project root global preserves spaces in path components") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("My Projects") / "Space Game";
        CHECK(Salix::g_project_root_path.filename() == "Space Game");
        CHECK(Salix::g_project_root_path.parent_path().filename() == "My Projects");
    }

    TEST_CASE("project root global replacement overwrites complete previous value") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "First/Project";
        Salix::g_project_root_path = "Second/Project";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Second/Project"));
    }

    TEST_CASE("project root global reference remains stable across assignments") {
        Batch1647ProjectRootGuard guard;
        auto* address = &Salix::g_project_root_path;
        Salix::g_project_root_path = "One";
        Salix::g_project_root_path = "Two";
        CHECK(&Salix::g_project_root_path == address);
    }

    TEST_CASE("project root global can be mutated through a retained reference") {
        Batch1647ProjectRootGuard guard;
        auto& root = Salix::g_project_root_path;
        root = "Referenced/Project";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Referenced/Project"));
    }

    TEST_CASE("copying project root path creates independent value storage") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "Source/Project";
        auto copy = Salix::g_project_root_path;
        copy /= "Assets";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Source/Project"));
        CHECK(copy.filename() == "Assets");
        CHECK(copy.parent_path() == Salix::g_project_root_path);
    }

    TEST_CASE("project root global does not silently lexically normalize parent traversal") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / ".." / "Game";
        CHECK(Salix::g_project_root_path != Salix::g_project_root_path.lexically_normal());
        CHECK(Salix::g_project_root_path.lexically_normal() == std::filesystem::path("Game"));
    }

    TEST_CASE("project root global retains project file extension when assigned a file path") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "demo.salixproj";
        CHECK(Salix::g_project_root_path.extension() == ".salixproj");
        CHECK(Salix::g_project_root_path.stem() == "demo");
    }

    TEST_CASE("sequential project root mutations can return to original guarded value") {
        const auto before = Salix::g_project_root_path;
        {
            Batch1647ProjectRootGuard guard;
            Salix::g_project_root_path = "Temporary/Root";
            CHECK(Salix::g_project_root_path == std::filesystem::path("Temporary/Root"));
        }
        CHECK(Salix::g_project_root_path == before);
    }
}


// ---- Relocated from src/tests/SalixEngine/core/EngineInterface.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/EngineInterface.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <memory>

TEST_SUITE("Salix::core::EngineInterfaceBatch1347") {
    TEST_CASE("engine interface starts with neutral mock mode and running state") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::None);
        CHECK(interface_ptr->is_running());
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine interface dispatches state switches through base pointer") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->switch_state(Salix::AppStateType::Editor);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
        CHECK(engine.switch_state_calls == 1);
    }

    TEST_CASE("engine interface accepts repeated state transitions") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->switch_state(Salix::AppStateType::Launch);
        interface_ptr->switch_state(Salix::AppStateType::Game);
        CHECK(engine.current_state == Salix::AppStateType::Game);
        CHECK(engine.switch_state_calls == 2);
    }

    TEST_CASE("engine interface mode setter and getter preserve selected mode") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_mode(Salix::EngineMode::Debug);
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.set_mode_calls == 1);
    }

    TEST_CASE("engine interface running setter returns and stores false") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        CHECK_FALSE(interface_ptr->is_running(false));
        CHECK_FALSE(interface_ptr->is_running());
    }

    TEST_CASE("engine interface running setter can restart state") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->is_running(false);
        CHECK(interface_ptr->is_running(true));
        CHECK(interface_ptr->is_running());
    }

    TEST_CASE("engine interface time scale preserves fractional values") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_time_scale(0.25f);
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(0.25f));
        CHECK(engine.set_time_scale_calls == 1);
    }

    TEST_CASE("engine interface time scale permits zero without changing mode") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_mode(Salix::EngineMode::Game);
        interface_ptr->set_time_scale(0.0f);
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(0.0f));
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::Game);
    }

    TEST_CASE("separate engine interface implementations keep independent state") {
        MockEngineInterface first;
        MockEngineInterface second;
        Salix::EngineInterface* a = &first;
        Salix::EngineInterface* b = &second;
        a->set_mode(Salix::EngineMode::Editor);
        b->set_mode(Salix::EngineMode::Game);
        a->set_time_scale(0.5f);
        CHECK(a->get_mode() == Salix::EngineMode::Editor);
        CHECK(b->get_mode() == Salix::EngineMode::Game);
        CHECK(a->get_time_scale() == doctest::Approx(0.5f));
        CHECK(b->get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine interface supports polymorphic destruction") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<MockEngineInterface>();
        REQUIRE(engine != nullptr);
        engine->set_mode(Salix::EngineMode::Launch);
        CHECK(engine->get_mode() == Salix::EngineMode::Launch);
    }
}


// ---- Relocated from src/tests/SalixEngine/core/EngineMode.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/EngineMode.h>
#include <array>
#include <set>
#include <type_traits>

TEST_SUITE("Salix::core::EngineModeBatch1647") {
    TEST_CASE("value initialized engine mode is None") {
        Salix::EngineMode mode{};
        CHECK(mode == Salix::EngineMode::None);
    }

    TEST_CASE("all current engine modes are distinct") {
        const std::array<Salix::EngineMode, 6> modes{
            Salix::EngineMode::None,
            Salix::EngineMode::Launch,
            Salix::EngineMode::Editor,
            Salix::EngineMode::Game,
            Salix::EngineMode::Options,
            Salix::EngineMode::Debug
        };
        std::set<Salix::EngineMode> unique(modes.begin(), modes.end());
        CHECK(unique.size() == modes.size());
    }

    TEST_CASE("launch mode remains distinct from editor and game") {
        CHECK(Salix::EngineMode::Launch != Salix::EngineMode::Editor);
        CHECK(Salix::EngineMode::Launch != Salix::EngineMode::Game);
    }

    TEST_CASE("editor and game modes remain independently assignable") {
        Salix::EngineMode mode = Salix::EngineMode::Editor;
        CHECK(mode == Salix::EngineMode::Editor);
        mode = Salix::EngineMode::Game;
        CHECK(mode == Salix::EngineMode::Game);
    }

    TEST_CASE("options and debug modes remain independently assignable") {
        Salix::EngineMode mode = Salix::EngineMode::Options;
        CHECK(mode == Salix::EngineMode::Options);
        mode = Salix::EngineMode::Debug;
        CHECK(mode == Salix::EngineMode::Debug);
    }

    TEST_CASE("engine mode is strongly typed and not implicitly convertible to int") {
        CHECK_FALSE((std::is_convertible_v<Salix::EngineMode, int>));
        CHECK((std::is_enum_v<Salix::EngineMode>));
    }

    TEST_CASE("engine mode underlying representation is integral") {
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::EngineMode>>));
    }

    TEST_CASE("engine mode copy construction preserves exact selected mode") {
        const Salix::EngineMode source = Salix::EngineMode::Debug;
        const Salix::EngineMode copy = source;
        CHECK(copy == source);
    }

    TEST_CASE("engine mode copy assignment replaces previous mode") {
        Salix::EngineMode destination = Salix::EngineMode::Launch;
        destination = Salix::EngineMode::Options;
        CHECK(destination == Salix::EngineMode::Options);
    }

    TEST_CASE("engine mode can be used as an ordered set key without aliasing values") {
        std::set<Salix::EngineMode> modes;
        modes.insert(Salix::EngineMode::Game);
        modes.insert(Salix::EngineMode::Game);
        modes.insert(Salix::EngineMode::Editor);
        CHECK(modes.size() == 2);
        CHECK(modes.count(Salix::EngineMode::Game) == 1);
        CHECK(modes.count(Salix::EngineMode::Editor) == 1);
    }
}


// ---- Relocated from src/tests/SalixEngine/core/IPlugin.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/IPlugin.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/core/MockIPlugin.h>
#include <memory>
#include <string>

namespace {
class Batch1247DestructionPlugin : public MockIPlugin {
public:
    explicit Batch1247DestructionPlugin(bool* destroyed) : destroyed_(destroyed) {}
    ~Batch1247DestructionPlugin() override {
        if (destroyed_) {
            *destroyed_ = true;
        }
    }

private:
    bool* destroyed_;
};
}

TEST_SUITE("Salix::core::IPluginBatch1247") {
    TEST_CASE("plugin construction performs no lifecycle callbacks implicitly") {
        MockIPlugin plugin;
        CHECK(plugin.register_calls == 0);
        CHECK(plugin.startup_calls == 0);
        CHECK(plugin.shutdown_calls == 0);
        CHECK(plugin.call_order.empty());
    }

    TEST_CASE("plugin interface dispatches register types callback") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        CHECK(plugin.register_calls == 1);
        REQUIRE(plugin.call_order.size() == 1);
        CHECK(plugin.call_order[0] == "register");
    }

    TEST_CASE("plugin interface startup receives the exact mutable InitContext reference") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(context);
        CHECK(plugin.startup_calls == 1);
        CHECK(plugin.last_context == &context);
    }

    TEST_CASE("plugin interface dispatches shutdown callback without requiring startup") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_shutdown();
        CHECK(plugin.shutdown_calls == 1);
        REQUIRE(plugin.call_order.size() == 1);
        CHECK(plugin.call_order[0] == "shutdown");
    }

    TEST_CASE("plugin lifecycle preserves explicit register startup shutdown ordering") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        interface_ptr->on_startup(context);
        interface_ptr->on_shutdown();
        REQUIRE(plugin.call_order.size() == 3);
        CHECK(plugin.call_order[0] == "register");
        CHECK(plugin.call_order[1] == "startup");
        CHECK(plugin.call_order[2] == "shutdown");
    }

    TEST_CASE("plugin interface allows repeated type registration requests to remain observable") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        interface_ptr->on_register_types();
        CHECK(plugin.register_calls == 2);
        CHECK(plugin.startup_calls == 0);
    }

    TEST_CASE("plugin repeated startup updates the most recently supplied context reference") {
        MockIPlugin plugin;
        Salix::InitContext first{};
        Salix::InitContext second{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(first);
        CHECK(plugin.last_context == &first);
        interface_ptr->on_startup(second);
        CHECK(plugin.last_context == &second);
        CHECK(plugin.startup_calls == 2);
    }

    TEST_CASE("plugin startup callback can intentionally configure shared initialization context") {
        MockIPlugin plugin;
        plugin.startup_handler = [](Salix::InitContext& context) {
            context.engine_mode = Salix::EngineMode::Game;
            context.project_dll_path = "PluginGame.dll";
        };

        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(context);

        CHECK(context.engine_mode == Salix::EngineMode::Game);
        CHECK(context.project_dll_path == "PluginGame.dll");
    }

    TEST_CASE("separate plugin instances maintain independent lifecycle accounting") {
        MockIPlugin first;
        MockIPlugin second;
        Salix::InitContext context{};
        first.on_register_types();
        first.on_startup(context);
        second.on_shutdown();
        CHECK(first.register_calls == 1);
        CHECK(first.startup_calls == 1);
        CHECK(first.shutdown_calls == 0);
        CHECK(second.register_calls == 0);
        CHECK(second.startup_calls == 0);
        CHECK(second.shutdown_calls == 1);
    }

    TEST_CASE("plugin interface virtual destructor destroys concrete implementation through base ownership") {
        bool destroyed = false;
        {
            std::unique_ptr<Salix::IPlugin> plugin =
                std::make_unique<Batch1247DestructionPlugin>(&destroyed);
            CHECK_FALSE(destroyed);
        }
        CHECK(destroyed);
    }
}


// ---- Relocated from src/tests/SalixEngine/core/InitContext.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <string>
#include <cmath>
#include <limits>
#include <utility>

TEST_SUITE("Salix::core::InitContextBatch1247") {
    TEST_CASE("value initialized context starts with neutral renderer GUI and engine modes") {
        Salix::InitContext context{};
        CHECK(context.renderer_type == Salix::RendererType::None);
        CHECK(context.gui_type == Salix::GuiType::None);
        CHECK(context.engine_mode == Salix::EngineMode::None);
    }

    TEST_CASE("value initialized context uses SDL as TimerType zero value") {
        Salix::InitContext context{};
        CHECK(context.timer_type == Salix::TimerType::SDL);
    }

    TEST_CASE("value initialized context nulls all optional service pointers") {
        Salix::InitContext context{};
        CHECK(context.app_config == nullptr);
        CHECK(context.gui == nullptr);
        CHECK(context.timer == nullptr);
        CHECK(context.engine == nullptr);
        CHECK(context.asset_manager == nullptr);
        CHECK(context.input_manager == nullptr);
        CHECK(context.renderer == nullptr);
        CHECK(context.window == nullptr);
        CHECK(context.event_poller == nullptr);
        CHECK(context.event_manager == nullptr);
        CHECK(context.project_manager == nullptr);
        CHECK(context.theme_manager == nullptr);
        CHECK(context.font_manager == nullptr);
        CHECK(context.icon_manager == nullptr);
    }

    TEST_CASE("value initialized context starts with empty project DLL path and zero DPI scale") {
        Salix::InitContext context{};
        CHECK(context.project_dll_path.empty());
        CHECK(context.dpi_scale == doctest::Approx(0.0f));
    }

    TEST_CASE("context retains renderer GUI timer and engine mode selections independently") {
        Salix::InitContext context{};
        context.renderer_type = Salix::RendererType::OpenGL;
        context.gui_type = Salix::GuiType::ImGui;
        context.timer_type = Salix::TimerType::Chrono;
        context.engine_mode = Salix::EngineMode::Editor;
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
        CHECK(context.gui_type == Salix::GuiType::ImGui);
        CHECK(context.timer_type == Salix::TimerType::Chrono);
        CHECK(context.engine_mode == Salix::EngineMode::Editor);
    }

    TEST_CASE("context transports exact engine interface pointer without ownership") {
        Salix::InitContext context{};
        MockEngineInterface engine;
        context.engine = &engine;
        CHECK(context.engine == &engine);
        context.engine->set_mode(Salix::EngineMode::Debug);
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("context preserves project DLL path including nested directories") {
        Salix::InitContext context{};
        context.project_dll_path = "C:/Projects/My Game/bin/Game.dll";
        CHECK(context.project_dll_path == "C:/Projects/My Game/bin/Game.dll");
    }

    TEST_CASE("context DPI scale accepts fractional and large logical scale values") {
        Salix::InitContext context{};
        context.dpi_scale = 1.25f;
        CHECK(context.dpi_scale == doctest::Approx(1.25f));
        context.dpi_scale = 3.0f;
        CHECK(context.dpi_scale == doctest::Approx(3.0f));
    }

    TEST_CASE("context copy construction preserves service identity and scalar state") {
        Salix::InitContext source{};
        MockEngineInterface engine;
        source.engine = &engine;
        source.renderer_type = Salix::RendererType::OpenGL;
        source.engine_mode = Salix::EngineMode::Game;
        source.project_dll_path = "Game.dll";
        source.dpi_scale = 2.0f;

        Salix::InitContext copy(source);
        CHECK(copy.engine == &engine);
        CHECK(copy.renderer_type == Salix::RendererType::OpenGL);
        CHECK(copy.engine_mode == Salix::EngineMode::Game);
        CHECK(copy.project_dll_path == "Game.dll");
        CHECK(copy.dpi_scale == doctest::Approx(2.0f));
    }

    TEST_CASE("context copies own independent project path storage while preserving nonowning pointers") {
        Salix::InitContext source{};
        MockEngineInterface engine;
        source.engine = &engine;
        source.project_dll_path = "Original.dll";

        Salix::InitContext copy = source;
        copy.project_dll_path = "Copy.dll";

        CHECK(source.engine == &engine);
        CHECK(copy.engine == &engine);
        CHECK(source.project_dll_path == "Original.dll");
        CHECK(copy.project_dll_path == "Copy.dll");
    }
}

TEST_SUITE("Salix::core::InitContextBatch1847") {
    TEST_CASE("project DLL path preserves embedded NUL bytes as owned string data") {
        Salix::InitContext context{};
        context.project_dll_path = std::string("Game\0Debug.dll", 14);
        CHECK(context.project_dll_path.size() == 14);
        CHECK(context.project_dll_path[4] == '\0');
        CHECK(context.project_dll_path.substr(5) == "Debug.dll");
    }

    TEST_CASE("DPI scale preserves negative diagnostic values without normalization") {
        Salix::InitContext context{};
        context.dpi_scale = -2.5f;
        CHECK(context.dpi_scale == doctest::Approx(-2.5f));
    }

    TEST_CASE("DPI scale can transport positive infinity unchanged") {
        Salix::InitContext context{};
        context.dpi_scale = std::numeric_limits<float>::infinity();
        CHECK(std::isinf(context.dpi_scale));
        CHECK(context.dpi_scale > 0.0f);
    }

    TEST_CASE("renderer selection roundtrips every declared renderer mode") {
        Salix::InitContext context{};
        const Salix::RendererType modes[] = {Salix::RendererType::None, Salix::RendererType::SDL,
            Salix::RendererType::OpenGL, Salix::RendererType::Vulkan, Salix::RendererType::DirectX};
        for (auto mode : modes) { context.renderer_type = mode; CHECK(context.renderer_type == mode); }
    }

    TEST_CASE("GUI selection can move from none to ImGui and back independently") {
        Salix::InitContext context{};
        context.gui_type = Salix::GuiType::ImGui;
        CHECK(context.gui_type == Salix::GuiType::ImGui);
        CHECK(context.renderer_type == Salix::RendererType::None);
        context.gui_type = Salix::GuiType::None;
        CHECK(context.gui_type == Salix::GuiType::None);
    }

    TEST_CASE("timer selection toggles between SDL and Chrono without touching engine mode") {
        Salix::InitContext context{};
        context.engine_mode = Salix::EngineMode::Debug;
        context.timer_type = Salix::TimerType::Chrono;
        CHECK(context.timer_type == Salix::TimerType::Chrono);
        context.timer_type = Salix::TimerType::SDL;
        CHECK(context.timer_type == Salix::TimerType::SDL);
        CHECK(context.engine_mode == Salix::EngineMode::Debug);
    }

    TEST_CASE("engine mode field transports every declared mode without hidden transitions") {
        Salix::InitContext context{};
        const Salix::EngineMode modes[] = {Salix::EngineMode::None, Salix::EngineMode::Launch,
            Salix::EngineMode::Editor, Salix::EngineMode::Game, Salix::EngineMode::Options, Salix::EngineMode::Debug};
        for (auto mode : modes) { context.engine_mode = mode; CHECK(context.engine_mode == mode); }
    }

    TEST_CASE("copy assignment replaces scalar configuration and keeps independent path storage") {
        Salix::InitContext source{};
        source.renderer_type = Salix::RendererType::DirectX;
        source.gui_type = Salix::GuiType::ImGui;
        source.timer_type = Salix::TimerType::Chrono;
        source.engine_mode = Salix::EngineMode::Options;
        source.project_dll_path = "Configured.dll";
        source.dpi_scale = 1.75f;
        Salix::InitContext copy{};
        copy = source;
        copy.project_dll_path = "Copy.dll";
        CHECK(copy.renderer_type == Salix::RendererType::DirectX);
        CHECK(copy.gui_type == Salix::GuiType::ImGui);
        CHECK(copy.timer_type == Salix::TimerType::Chrono);
        CHECK(copy.engine_mode == Salix::EngineMode::Options);
        CHECK(source.project_dll_path == "Configured.dll");
        CHECK(copy.dpi_scale == doctest::Approx(1.75f));
    }

    TEST_CASE("move construction transfers configured value and nonowning engine pointer") {
        MockEngineInterface engine;
        Salix::InitContext source{};
        source.engine = &engine;
        source.project_dll_path = "Moved.dll";
        source.engine_mode = Salix::EngineMode::Game;
        source.dpi_scale = 2.25f;
        Salix::InitContext moved(std::move(source));
        CHECK(moved.engine == &engine);
        CHECK(moved.project_dll_path == "Moved.dll");
        CHECK(moved.engine_mode == Salix::EngineMode::Game);
        CHECK(moved.dpi_scale == doctest::Approx(2.25f));
    }

    TEST_CASE("self assignment preserves configured context state") {
        Salix::InitContext context{};
        MockEngineInterface engine;
        context.engine = &engine;
        context.renderer_type = Salix::RendererType::OpenGL;
        context.project_dll_path = "Self.dll";
        context.dpi_scale = 1.5f;
        context = context;
        CHECK(context.engine == &engine);
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
        CHECK(context.project_dll_path == "Self.dll");
        CHECK(context.dpi_scale == doctest::Approx(1.5f));
    }
}


// ---- Relocated from src/tests/SalixEngine/core/InitEnums.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/core/InitEnums.h>
#include <array>
#include <set>
#include <type_traits>

TEST_SUITE("Salix::core::InitEnumsBatch1547") {
    TEST_CASE("renderer type neutral value is zero for value initialized startup state") {
        CHECK(static_cast<int>(Salix::RendererType::None) == 0);
    }

    TEST_CASE("renderer type defines five unique startup backend selections") {
        const std::array<Salix::RendererType, 5> values{
            Salix::RendererType::None,
            Salix::RendererType::SDL,
            Salix::RendererType::OpenGL,
            Salix::RendererType::Vulkan,
            Salix::RendererType::DirectX
        };
        std::set<int> unique;
        for (auto value : values) unique.insert(static_cast<int>(value));
        CHECK(unique.size() == values.size());
    }

    TEST_CASE("timer type SDL remains zero default and Chrono remains distinct") {
        CHECK(static_cast<int>(Salix::TimerType::SDL) == 0);
        CHECK(Salix::TimerType::SDL != Salix::TimerType::Chrono);
    }

    TEST_CASE("GUI type neutral startup mode is zero and ImGui is distinct") {
        CHECK(static_cast<int>(Salix::GuiType::None) == 0);
        CHECK(Salix::GuiType::None != Salix::GuiType::ImGui);
    }

    TEST_CASE("application state neutral value is zero for unselected state") {
        CHECK(static_cast<int>(Salix::AppStateType::None) == 0);
    }

    TEST_CASE("application state defines five unique lifecycle destinations") {
        const std::array<Salix::AppStateType, 5> values{
            Salix::AppStateType::None,
            Salix::AppStateType::Launch,
            Salix::AppStateType::Editor,
            Salix::AppStateType::Game,
            Salix::AppStateType::Options
        };
        std::set<int> unique;
        for (auto value : values) unique.insert(static_cast<int>(value));
        CHECK(unique.size() == values.size());
    }

    TEST_CASE("renderer timer GUI and app state remain distinct enum types") {
        CHECK_FALSE((std::is_same_v<Salix::RendererType, Salix::TimerType>));
        CHECK_FALSE((std::is_same_v<Salix::RendererType, Salix::GuiType>));
        CHECK_FALSE((std::is_same_v<Salix::GuiType, Salix::AppStateType>));
    }

    TEST_CASE("renderer selections compare only inside renderer domain") {
        Salix::RendererType renderer = Salix::RendererType::OpenGL;
        CHECK(renderer == Salix::RendererType::OpenGL);
        CHECK(renderer != Salix::RendererType::SDL);
        renderer = Salix::RendererType::Vulkan;
        CHECK(renderer == Salix::RendererType::Vulkan);
    }

    TEST_CASE("application state assignment can represent every current transition target") {
        Salix::AppStateType state = Salix::AppStateType::Launch;
        CHECK(state == Salix::AppStateType::Launch);
        state = Salix::AppStateType::Editor;
        CHECK(state == Salix::AppStateType::Editor);
        state = Salix::AppStateType::Game;
        CHECK(state == Salix::AppStateType::Game);
        state = Salix::AppStateType::Options;
        CHECK(state == Salix::AppStateType::Options);
    }

    TEST_CASE("all initialization enums use integral underlying representations") {
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::RendererType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::TimerType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::GuiType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::AppStateType>>));
    }
}


TEST_SUITE("Salix::core::InitEnumsBatch2047") {
    TEST_CASE("value initialized renderer type selects None") { Salix::RendererType value{}; CHECK(value == Salix::RendererType::None); }
    TEST_CASE("value initialized timer type selects SDL") { Salix::TimerType value{}; CHECK(value == Salix::TimerType::SDL); }
    TEST_CASE("value initialized GUI type selects None") { Salix::GuiType value{}; CHECK(value == Salix::GuiType::None); }
    TEST_CASE("value initialized application state selects None") { Salix::AppStateType value{}; CHECK(value == Salix::AppStateType::None); }
    TEST_CASE("renderer values survive conversion through their underlying type") {
        using U = std::underlying_type_t<Salix::RendererType>;
        const std::array<Salix::RendererType, 5> values{Salix::RendererType::None, Salix::RendererType::SDL, Salix::RendererType::OpenGL, Salix::RendererType::Vulkan, Salix::RendererType::DirectX};
        for (auto value : values) CHECK(static_cast<Salix::RendererType>(static_cast<U>(value)) == value);
    }
    TEST_CASE("timer values survive conversion through their underlying type") {
        using U = std::underlying_type_t<Salix::TimerType>; const std::array<Salix::TimerType, 2> values{Salix::TimerType::SDL, Salix::TimerType::Chrono};
        for (auto value : values) CHECK(static_cast<Salix::TimerType>(static_cast<U>(value)) == value);
    }
    TEST_CASE("GUI values survive conversion through their underlying type") {
        using U = std::underlying_type_t<Salix::GuiType>; const std::array<Salix::GuiType, 2> values{Salix::GuiType::None, Salix::GuiType::ImGui};
        for (auto value : values) CHECK(static_cast<Salix::GuiType>(static_cast<U>(value)) == value);
    }
    TEST_CASE("application state values survive conversion through their underlying type") {
        using U = std::underlying_type_t<Salix::AppStateType>;
        const std::array<Salix::AppStateType, 5> values{Salix::AppStateType::None, Salix::AppStateType::Launch, Salix::AppStateType::Editor, Salix::AppStateType::Game, Salix::AppStateType::Options};
        for (auto value : values) CHECK(static_cast<Salix::AppStateType>(static_cast<U>(value)) == value);
    }
    TEST_CASE("copying renderer and timer arrays preserves every selection") {
        const std::array<Salix::RendererType, 3> renderers{Salix::RendererType::SDL, Salix::RendererType::OpenGL, Salix::RendererType::DirectX};
        const auto renderer_copy = renderers; CHECK(renderer_copy == renderers);
        const std::array<Salix::TimerType, 2> timers{Salix::TimerType::SDL, Salix::TimerType::Chrono};
        const auto timer_copy = timers; CHECK(timer_copy == timers);
    }
    TEST_CASE("assignment between application states changes only the assigned value") {
        Salix::AppStateType first = Salix::AppStateType::Editor; Salix::AppStateType second = Salix::AppStateType::Game;
        first = second; CHECK(first == Salix::AppStateType::Game); CHECK(second == Salix::AppStateType::Game);
    }
}
