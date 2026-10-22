#include <doctest.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <string>

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
