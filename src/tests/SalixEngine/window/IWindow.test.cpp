#include <doctest.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/core/Engine.h>
#include <filesystem>
#include <cstdlib>
#include <SDL.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/window/IWindow.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <memory>

TEST_SUITE("Salix::window::IWindowBatch1447") {
    TEST_CASE("window interface initialization captures complete window configuration") {
        MockIWindow window;
        Salix::WindowConfig config;
        config.title = "Unit Window";
        config.width = 1280;
        config.height = 800;
        Salix::IWindow* interface_ptr = &window;
        CHECK(interface_ptr->initialize(config));
        CHECK(window.initialized);
        CHECK(window.config.title == "Unit Window");
        CHECK(window.get_width() == 1280);
        CHECK(window.get_height() == 800);
    }

    TEST_CASE("window interface controlled initialization failure remains observable") {
        MockIWindow window;
        window.initialize_result = false;
        Salix::IWindow* interface_ptr = &window;
        CHECK_FALSE(interface_ptr->initialize(Salix::WindowConfig{}));
        CHECK_FALSE(window.initialized);
        CHECK(window.initialize_calls == 1);
    }

    TEST_CASE("window native handle transports exact opaque pointer") {
        MockIWindow window;
        int native = 5;
        window.native_handle = &native;
        Salix::IWindow* interface_ptr = &window;
        CHECK(interface_ptr->get_native_handle() == &native);
    }

    TEST_CASE("window dimension query mirrors direct width and height accessors") {
        MockIWindow window;
        window.width = 640;
        window.height = 360;
        int width = 0;
        int height = 0;
        window.query_dimensions(width, height);
        CHECK(width == window.get_width());
        CHECK(height == window.get_height());
        CHECK(width == 640);
        CHECK(height == 360);
    }

    TEST_CASE("window set size changes both dimensions through interface") {
        MockIWindow window;
        Salix::IWindow* interface_ptr = &window;
        interface_ptr->set_size(1920, 1080);
        CHECK(interface_ptr->get_width() == 1920);
        CHECK(interface_ptr->get_height() == 1080);
        CHECK(window.set_size_calls == 1);
    }

    TEST_CASE("window set size preserves zero sized surface state") {
        MockIWindow window;
        window.set_size(0, 0);
        CHECK(window.get_width() == 0);
        CHECK(window.get_height() == 0);
    }

    TEST_CASE("window set size preserves negative values for validation edge tests") {
        MockIWindow window;
        window.set_size(-10, -20);
        CHECK(window.get_width() == -10);
        CHECK(window.get_height() == -20);
    }

    TEST_CASE("window DPI scale is transported independently of pixel dimensions") {
        MockIWindow window;
        window.width = 800;
        window.height = 600;
        window.dpi_scale = 1.75f;
        CHECK(window.get_dpi_scale() == doctest::Approx(1.75f));
        CHECK(window.get_width() == 800);
        CHECK(window.get_height() == 600);
    }

    TEST_CASE("window shutdown clears initialized state and records lifecycle call") {
        MockIWindow window;
        REQUIRE(window.initialize(Salix::WindowConfig{}));
        window.shutdown();
        CHECK_FALSE(window.initialized);
        CHECK(window.shutdown_calls == 1);
    }

    TEST_CASE("window interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IWindow> window = std::make_unique<MockIWindow>();
        REQUIRE(window != nullptr);
        window->set_size(320, 200);
        CHECK(window->get_width() == 320);
        CHECK(window->get_height() == 200);
    }
}

TEST_SUITE("Salix::window::IWindowBatch6447") {
    TEST_CASE("batch6447 default mock begins uninitialized with zero dimensions") {
        MockIWindow window;
        CHECK_FALSE(window.initialized);
        CHECK(window.get_width() == 0);
        CHECK(window.get_height() == 0);
        CHECK(window.initialize_calls == 0);
    }

    TEST_CASE("batch6447 initialization with empty title preserves empty title") {
        MockIWindow window;
        Salix::WindowConfig config{"", 640, 447};
        REQUIRE(window.initialize(config));
        CHECK(window.config.title.empty());
        CHECK(window.get_width() == 640);
        CHECK(window.get_height() == 447);
    }

    TEST_CASE("batch6447 repeated initialization increments accounting and keeps latest config") {
        MockIWindow window;
        REQUIRE(window.initialize(Salix::WindowConfig{"First6447", 100, 200}));
        REQUIRE(window.initialize(Salix::WindowConfig{"Second6447", 300, 400}));
        CHECK(window.initialize_calls == 2);
        CHECK(window.config.title == "Second6447");
        CHECK(window.get_width() == 300);
        CHECK(window.get_height() == 400);
    }

    TEST_CASE("batch6447 controlled failed initialization still captures requested dimensions") {
        MockIWindow window;
        window.initialize_result = false;
        CHECK_FALSE(window.initialize(Salix::WindowConfig{"Fail6447", 6447, 7446}));
        CHECK_FALSE(window.initialized);
        CHECK(window.get_width() == 6447);
        CHECK(window.get_height() == 7446);
    }

    TEST_CASE("batch6447 repeated set size increments accounting and keeps latest dimensions") {
        MockIWindow window;
        window.set_size(100, 200);
        window.set_size(6447, 7446);
        CHECK(window.set_size_calls == 2);
        CHECK(window.get_width() == 6447);
        CHECK(window.get_height() == 7446);
    }

    TEST_CASE("batch6447 query dimensions overwrites both output variables") {
        MockIWindow window;
        window.width = -64;
        window.height = 47;
        int width = 999;
        int height = 999;
        window.query_dimensions(width, height);
        CHECK(width == -64);
        CHECK(height == 47);
    }

    TEST_CASE("batch6447 native handle can transition from null to exact pointer") {
        MockIWindow window;
        Salix::IWindow& interface = window;
        CHECK(interface.get_native_handle() == nullptr);
        int native = 6447;
        window.native_handle = &native;
        CHECK(interface.get_native_handle() == &native);
    }

    TEST_CASE("batch6447 DPI scale may be zero or negative independently of dimensions") {
        MockIWindow window;
        window.width = 1280;
        window.height = 720;
        window.dpi_scale = 0.0f;
        CHECK(window.get_dpi_scale() == doctest::Approx(0.0f));
        window.dpi_scale = -1.5f;
        CHECK(window.get_dpi_scale() == doctest::Approx(-1.5f));
        CHECK(window.get_width() == 1280);
        CHECK(window.get_height() == 720);
    }

    TEST_CASE("batch6447 repeated shutdown calls remain observable and uninitialized") {
        MockIWindow window;
        REQUIRE(window.initialize(Salix::WindowConfig{}));
        window.shutdown();
        window.shutdown();
        CHECK_FALSE(window.initialized);
        CHECK(window.shutdown_calls == 2);
    }

    TEST_CASE("batch6447 polymorphic window ownership preserves latest set size") {
        auto concrete = std::make_unique<MockIWindow>();
        concrete->set_size(6447, 7446);
        std::unique_ptr<Salix::IWindow> window = std::move(concrete);
        CHECK(window->get_width() == 6447);
        CHECK(window->get_height() == 7446);
    }
}

namespace {
struct Batch7578WindowDriverGuard {
    bool had_video = false;
    std::string old_video;
    Batch7578WindowDriverGuard() {
        if (const char* v = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = v; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_InitSubSystem(SDL_INIT_VIDEO);
    }
    ~Batch7578WindowDriverGuard() {
        SDL_Quit();
        std::error_code ec; std::filesystem::remove("gl_init.log", ec);
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
    }
};

struct Batch7578FailedOpenGLWindow {
    Batch7578WindowDriverGuard driver;
    Salix::OpenGLRenderer renderer;
    bool initialized = true;
    explicit Batch7578FailedOpenGLWindow(const Salix::WindowConfig& config) { initialized = renderer.initialize(config); }
    Salix::IWindow* window() { return renderer.get_window(); }
};
}

TEST_SUITE("Salix::window::IWindowBatch7578") {
    TEST_CASE("batch7578 failed OpenGL initialization retains SDL window wrapper") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578A", 757, 578});
        REQUIRE_FALSE(run.initialized); CHECK(run.window() != nullptr);
    }
    TEST_CASE("batch7578 failed wrapper native handle remains null") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578B", 757, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_native_handle() == nullptr);
    }
    TEST_CASE("batch7578 failed wrapper retains requested width") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578C", 757, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_width() == 757);
    }
    TEST_CASE("batch7578 failed wrapper retains requested height") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578D", 757, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_height() == 578);
    }
    TEST_CASE("batch7578 failed wrapper query overwrites dimensions with zero") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578E", 757, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); int w=1,h=1; window->query_dimensions(w,h); CHECK(w==0); CHECK(h==0);
    }
    TEST_CASE("batch7578 failed wrapper returns unit DPI without native window") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578F", 757, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_dpi_scale() == doctest::Approx(1.0f));
    }
    TEST_CASE("batch7578 failed wrapper preserves zero width config") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578G", 0, 578});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_width() == 0);
    }
    TEST_CASE("batch7578 failed wrapper preserves zero height config") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578H", 757, 0});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_height() == 0);
    }
    TEST_CASE("batch7578 failed wrapper preserves negative dimensions for diagnostics") {
        Batch7578FailedOpenGLWindow run(Salix::WindowConfig{"Window7578I", -75, -78});
        auto* window = run.window(); REQUIRE(window != nullptr); CHECK(window->get_width() == -75); CHECK(window->get_height() == -78);
    }
    TEST_CASE("batch7578 repeated failed initialization replaces stored window config") {
        Batch7578WindowDriverGuard driver;
        Salix::OpenGLRenderer renderer;
        REQUIRE_FALSE(renderer.initialize(Salix::WindowConfig{"First7578", 100, 200}));
        REQUIRE_FALSE(renderer.initialize(Salix::WindowConfig{"Second7578", 300, 400}));
        auto* window = renderer.get_window(); REQUIRE(window != nullptr);
        CHECK(window->get_width() == 300); CHECK(window->get_height() == 400);
    }
}

namespace {
struct Batch7678WindowNativeGuard {
    bool had_video = false;
    std::string old_video;
    Batch7678WindowNativeGuard() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "windows", 1);
        _putenv_s("SDL_RENDER_DRIVER", "");
    }
    ~Batch7678WindowNativeGuard() {
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
    }
};

struct Batch7678NativeWindow {
    Batch7678WindowNativeGuard driver;
    Salix::Engine engine;
    bool initialized = false;
    Batch7678NativeWindow() {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::Launch;
        config.window_config.title = "Batch7678 IWindow";
        config.window_config.width = 96;
        config.window_config.height = 64;
        initialized = engine.initialize(config);
    }
    ~Batch7678NativeWindow() { engine.shutdown(); }
    Salix::IWindow* window() { return engine.get_renderer() ? engine.get_renderer()->get_window() : nullptr; }
};
}

TEST_SUITE("Salix::window::IWindowBatch7678") {
    TEST_CASE("batch7678 native SDL window exposes nonnull native handle") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        CHECK(run.window()->get_native_handle() != nullptr);
    }
    TEST_CASE("batch7678 native SDL window retains initial configured dimensions") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        CHECK(run.window()->get_width() == 96);
        CHECK(run.window()->get_height() == 64);
    }
    TEST_CASE("batch7678 native SDL window query returns live dimensions") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        int width = 0, height = 0;
        run.window()->query_dimensions(width, height);
        CHECK(width > 0);
        CHECK(height > 0);
    }
    TEST_CASE("batch7678 native SDL window set size updates stored dimensions") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        run.window()->set_size(80, 48);
        CHECK(run.window()->get_width() == 80);
        CHECK(run.window()->get_height() == 48);
    }
    TEST_CASE("batch7678 native SDL window repeated set size keeps latest dimensions") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        run.window()->set_size(70, 50);
        run.window()->set_size(90, 60);
        CHECK(run.window()->get_width() == 90);
        CHECK(run.window()->get_height() == 60);
    }
    TEST_CASE("batch7678 native SDL window query reflects resize request") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        run.window()->set_size(88, 58);
        int width = 0, height = 0;
        run.window()->query_dimensions(width, height);
        CHECK(width > 0);
        CHECK(height > 0);
    }
    TEST_CASE("batch7678 native SDL window DPI scale is positive") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        CHECK(run.window()->get_dpi_scale() > 0.0f);
    }
    TEST_CASE("batch7678 native SDL renderer window accessor remains separate from SDL window shortcut") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        CHECK(run.window()->get_native_handle() != nullptr);
        CHECK(run.engine.get_renderer()->get_sdl_window() == nullptr);
    }
    TEST_CASE("batch7678 native SDL window survives small positive resize") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        CHECK_NOTHROW(run.window()->set_size(16, 16));
        CHECK(run.window()->get_width() == 16);
        CHECK(run.window()->get_height() == 16);
    }
    TEST_CASE("batch7678 native SDL window remains usable after renderer logical resize") {
        Batch7678NativeWindow run; REQUIRE(run.initialized); REQUIRE(run.window() != nullptr);
        run.engine.get_renderer()->on_window_resize(72, 54);
        int width = 0, height = 0;
        run.window()->query_dimensions(width, height);
        CHECK(width > 0);
        CHECK(height > 0);
    }
}

