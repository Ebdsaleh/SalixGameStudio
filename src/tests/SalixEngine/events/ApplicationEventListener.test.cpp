#include <doctest.h>
#include <Salix/events/ApplicationEventListener.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/events/IEventListener.h>
#include <Salix/core/Engine.h>

TEST_SUITE("Salix::events::ApplicationEventListenerBatch1147") {
    TEST_CASE("listener rejects null engine during initialization") {
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
    }

    TEST_CASE("window close before initialization is safely ignored") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent event;
        CHECK_NOTHROW(listener.on_event(event));
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("non-window application event before initialization is safely ignored") {
        Salix::ApplicationEventListener listener;
        Salix::WindowResizeEvent event(800, 600);
        CHECK_NOTHROW(listener.on_event(event));
        CHECK(event.get_width() == 800);
        CHECK(event.get_height() == 600);
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("listener accepts a valid engine pointer") {
        Salix::Engine engine;
        Salix::ApplicationEventListener listener;
        CHECK(listener.initialize(&engine));
    }

    TEST_CASE("listener can be initialized repeatedly with the same engine") {
        Salix::Engine engine;
        Salix::ApplicationEventListener listener;
        CHECK(listener.initialize(&engine));
        CHECK(listener.initialize(&engine));
    }

    TEST_CASE("window close event changes initialized engine running state to false") {
        Salix::Engine engine;
        engine.is_running(true);
        REQUIRE(engine.is_running());
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("non-window event leaves initialized engine running state unchanged") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent event(1280, 720);
        listener.on_event(event);
        CHECK(engine.is_running());
    }

    TEST_CASE("event delivery dispatches through IEventListener interface") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener* listener = &concrete;
        Salix::WindowCloseEvent event;
        listener->on_event(event);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("reinitializing listener retargets shutdown notification to the newest engine") {
        Salix::Engine first;
        Salix::Engine second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("listener never mutates handled state while processing application events") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        close.handled = true;
        listener.on_event(close);
        CHECK(close.handled);
        CHECK_FALSE(engine.is_running());

        Salix::WindowResizeEvent resize(320, 200);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(resize.handled);
    }
}
