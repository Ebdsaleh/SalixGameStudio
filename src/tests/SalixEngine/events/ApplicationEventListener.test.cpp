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

TEST_SUITE("Salix::events::ApplicationEventListenerBatch2347") {
    TEST_CASE("batch2347 close event before initialization leaves event unhandled") {
        Salix::ApplicationEventListener listener; Salix::WindowCloseEvent close; CHECK_NOTHROW(listener.on_event(close)); CHECK_FALSE(close.handled);
    }
    TEST_CASE("batch2347 resize event before initialization preserves payload exactly") {
        Salix::ApplicationEventListener listener; Salix::WindowResizeEvent resize(2347,1234); listener.on_event(resize); CHECK(resize.get_width()==2347); CHECK(resize.get_height()==1234); CHECK_FALSE(resize.handled);
    }
    TEST_CASE("batch2347 initialized listener stops a running engine on close") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2347 initialized listener leaves running engine unchanged on resize") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); Salix::WindowResizeEvent resize(800,600); listener.on_event(resize); CHECK(engine.is_running());
    }
    TEST_CASE("batch2347 initialized listener leaves running engine unchanged on tick event") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); Batch1847TickEvent tick; listener.on_event(tick); CHECK(engine.is_running()); CHECK_FALSE(tick.handled);
    }
    TEST_CASE("batch2347 repeated initialization with same engine preserves close behavior") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); REQUIRE(listener.initialize(&engine)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2347 retargeted listener shuts down only latest engine") {
        Salix::Engine first,second; first.is_running(true); second.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&first)); REQUIRE(listener.initialize(&second)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK(first.is_running()); CHECK_FALSE(second.is_running());
    }
    TEST_CASE("batch2347 failed null initialization does not erase prior valid target") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); CHECK_FALSE(listener.initialize(nullptr)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2347 polymorphic listener dispatch preserves handled flag on close") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&engine)); Salix::IEventListener& listener=concrete; Salix::WindowCloseEvent close; close.handled=true; listener.on_event(close); CHECK(close.handled); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2347 two listeners independently control their own engines") {
        Salix::Engine a,b; a.is_running(true); b.is_running(true); Salix::ApplicationEventListener la,lb; REQUIRE(la.initialize(&a)); REQUIRE(lb.initialize(&b));
        Salix::WindowCloseEvent close_a; la.on_event(close_a); CHECK_FALSE(a.is_running()); CHECK(b.is_running()); Salix::WindowCloseEvent close_b; lb.on_event(close_b); CHECK_FALSE(b.is_running());
    }
}

