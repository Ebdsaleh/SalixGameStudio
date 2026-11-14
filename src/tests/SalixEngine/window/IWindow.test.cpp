#include <doctest.h>
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
