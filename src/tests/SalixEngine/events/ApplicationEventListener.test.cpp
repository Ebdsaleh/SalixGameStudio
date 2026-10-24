#include <doctest.h>
#include <Salix/events/ApplicationEventListener.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/events/IEventListener.h>
#include <Salix/core/Engine.h>
#include <Salix/events/IEvent.h>
#include <memory>

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

namespace {
class Batch1847TickEvent : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
    const char* get_name() const override { return "Batch1847TickEvent"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1847TickEvent>(*this); }
};
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch1847") {
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

// ---- Relocated from src/tests/SalixEngine/events/IEventListener.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/events/IEventListener.h>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>
#include <memory>

TEST_SUITE("Salix::events::IEventListenerBatch1347") {
    TEST_CASE("event listener mock begins without a received event") {
        Salix::MockListener listener;
        CHECK_FALSE(listener.event_received);
    }

    TEST_CASE("event listener dispatch through interface reaches concrete listener") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        Salix::IEventListener* interface_ptr = &listener;
        interface_ptr->on_event(event);
        CHECK(listener.event_received);
    }

    TEST_CASE("event listener custom handler receives exact event reference") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        Salix::IEvent* seen = nullptr;
        listener.on_event_handler = [&](Salix::IEvent& received) { seen = &received; };
        listener.on_event(event);
        CHECK(seen == &event);
    }

    TEST_CASE("event listener handler can mark event handled") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        listener.on_event_handler = [](Salix::IEvent& received) { received.handled = true; };
        listener.on_event(event);
        CHECK(event.handled);
    }

    TEST_CASE("event listener can observe blocking event contract") {
        Salix::MockListener listener;
        Salix::MockBlockingEvent event;
        bool saw_blocking = false;
        listener.on_event_handler = [&](Salix::IEvent& received) { saw_blocking = received.should_block(); };
        listener.on_event(event);
        CHECK(saw_blocking);
    }

    TEST_CASE("event listener can receive repeated dispatches of same event") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        listener.on_event(event);
        listener.on_event(event);
        CHECK(calls == 2);
    }

    TEST_CASE("separate event listeners maintain independent handlers") {
        Salix::MockListener first;
        Salix::MockListener second;
        Salix::MockEvent event;
        int first_calls = 0;
        int second_calls = 0;
        first.on_event_handler = [&](Salix::IEvent&) { ++first_calls; };
        second.on_event_handler = [&](Salix::IEvent&) { ++second_calls; };
        first.on_event(event);
        CHECK(first_calls == 1);
        CHECK(second_calls == 0);
    }

    TEST_CASE("event listener handler can inspect event type and name") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        listener.on_event_handler = [](Salix::IEvent& received) {
            CHECK(received.get_event_type() == Salix::EventType::AppTick);
            CHECK(std::string(received.get_name()) == "AppTick");
        };
        listener.on_event(event);
    }

    TEST_CASE("event listener can receive a cloned event independently") {
        Salix::MockListener listener;
        Salix::MockEvent source;
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        clone->handled = false;
        listener.on_event(*clone);
        CHECK(listener.event_received);
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("event listener supports polymorphic destruction") {
        std::unique_ptr<Salix::IEventListener> listener = std::make_unique<Salix::MockListener>();
        Salix::MockEvent event;
        REQUIRE(listener != nullptr);
        listener->on_event(event);
    }
}
