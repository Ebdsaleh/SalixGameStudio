#include <doctest.h>
#include <Salix/events/ApplicationEventListener.h>
#include <Salix/events/IEvent.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/core/Engine.h>
#include <memory>

namespace {
class Batch1847TickEvent : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
    const char* get_name() const override { return "Batch1847TickEvent"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1847TickEvent>(*this); }
};
}

TEST_SUITE("Salix::events::ApplicationEventListenerCoverage1847") {
    TEST_CASE("repeated close events before initialization remain safe no-ops") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent event;
        CHECK_NOTHROW(listener.on_event(event));
        CHECK_NOTHROW(listener.on_event(event));
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("close event keeps an already stopped initialized engine stopped") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("same listener can stop an engine again after caller restarts it") {
        Salix::Engine engine;
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent event;
        engine.is_running(true); listener.on_event(event); CHECK_FALSE(engine.is_running());
        engine.is_running(true); listener.on_event(event); CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("failed null reinitialization preserves the previously valid engine target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("listener can recover from an initial null initialization attempt") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("null initialization after retargeting does not fall back to the older engine") {
        Salix::Engine first, second;
        first.is_running(true); second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent event;
        listener.on_event(event);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("resize event preserves an already stopped engine state") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(1, 1);
        listener.on_event(resize);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("resize event preserves handled state and running engine state") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(1920, 1080);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.handled);
    }

    TEST_CASE("other application-category events do not request engine shutdown") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Batch1847TickEvent tick;
        listener.on_event(tick);
        CHECK(engine.is_running());
        CHECK_FALSE(tick.handled);
    }

    TEST_CASE("interface dispatch can deliver repeated close events to one initialized listener") {
        Salix::Engine engine;
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowCloseEvent close;
        engine.is_running(true); listener.on_event(close); CHECK_FALSE(engine.is_running());
        engine.is_running(true); listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
}
