#include <doctest.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

TEST_SUITE("Salix::core::InitContextCoverage1847") {
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
