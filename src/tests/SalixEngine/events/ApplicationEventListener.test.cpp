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

TEST_SUITE("Salix::events::ApplicationEventListenerBatch2547") {
    TEST_CASE("batch2547 null initialization on fresh listener reports failure") {
        Salix::ApplicationEventListener listener; CHECK_FALSE(listener.initialize(nullptr));
    }
    TEST_CASE("batch2547 resize before initialization is a harmless no-op") {
        Salix::ApplicationEventListener listener; Salix::WindowResizeEvent resize(1,2);
        CHECK_NOTHROW(listener.on_event(resize)); CHECK(resize.get_width()==1); CHECK(resize.get_height()==2); CHECK_FALSE(resize.handled);
    }
    TEST_CASE("batch2547 close before initialization is repeatably harmless") {
        Salix::ApplicationEventListener listener; Salix::WindowCloseEvent close;
        listener.on_event(close); listener.on_event(close); CHECK_FALSE(close.handled);
    }
    TEST_CASE("batch2547 valid initialization leaves running engine unchanged until event arrives") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine)); CHECK(engine.is_running());
    }
    TEST_CASE("batch2547 close after valid initialization stops engine exactly once semantically") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running()); listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2547 resize never changes running state after initialization") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent a(320,200),b(1920,1080); listener.on_event(a); CHECK(engine.is_running()); listener.on_event(b); CHECK(engine.is_running());
    }
    TEST_CASE("batch2547 tick event never changes running state after initialization") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Batch1847TickEvent tick; listener.on_event(tick); CHECK(engine.is_running()); CHECK_FALSE(tick.handled);
    }
    TEST_CASE("batch2547 retargeting listener isolates shutdown to newest engine") {
        Salix::Engine first,second; first.is_running(true); second.is_running(true); Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first)); REQUIRE(listener.initialize(&second)); Salix::WindowCloseEvent close; listener.on_event(close);
        CHECK(first.is_running()); CHECK_FALSE(second.is_running());
    }
    TEST_CASE("batch2547 failed null retarget preserves current valid engine") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2547 polymorphic listener call leaves application event handled flag untouched") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& base=concrete; Salix::WindowCloseEvent close; close.handled=true; base.on_event(close);
        CHECK(close.handled); CHECK_FALSE(engine.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch2947") {
    TEST_CASE("batch2947 close event never marks event handled while stopping engine") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close; CHECK_FALSE(close.handled); listener.on_event(close); CHECK_FALSE(engine.is_running()); CHECK_FALSE(close.handled);
    }
    TEST_CASE("batch2947 already handled close still stops initialized engine") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close; close.handled=true; listener.on_event(close); CHECK_FALSE(engine.is_running()); CHECK(close.handled);
    }
    TEST_CASE("batch2947 resize with zero dimensions leaves running engine untouched") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(0,0); listener.on_event(resize); CHECK(engine.is_running()); CHECK(resize.get_width()==0); CHECK(resize.get_height()==0);
    }
    TEST_CASE("batch2947 resize with large unsigned diagnostic dimensions leaves running engine untouched") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        constexpr unsigned int width = 4000000000u;
        constexpr unsigned int height = 3000000000u;
        Salix::WindowResizeEvent resize(width,height); listener.on_event(resize); CHECK(engine.is_running()); CHECK(resize.get_width()==width); CHECK(resize.get_height()==height);
    }
    TEST_CASE("batch2947 failed null initialization after valid target preserves close routing") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine)); CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2947 multiple failed null initializations never erase valid target") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr)); CHECK_FALSE(listener.initialize(nullptr)); Salix::WindowCloseEvent close; listener.on_event(close); CHECK_FALSE(engine.is_running());
    }
    TEST_CASE("batch2947 retarget chain shuts down only final engine") {
        Salix::Engine a,b,c; a.is_running(true); b.is_running(true); c.is_running(true); Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&a)); REQUIRE(listener.initialize(&b)); REQUIRE(listener.initialize(&c)); Salix::WindowCloseEvent close; listener.on_event(close);
        CHECK(a.is_running()); CHECK(b.is_running()); CHECK_FALSE(c.is_running());
    }
    TEST_CASE("batch2947 tick preserves already stopped engine state") {
        Salix::Engine engine; engine.is_running(false); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Batch1847TickEvent tick; listener.on_event(tick); CHECK_FALSE(engine.is_running()); CHECK_FALSE(tick.handled);
    }
    TEST_CASE("batch2947 repeated resize events preserve running state and handled flags independently") {
        Salix::Engine engine; engine.is_running(true); Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent first(100,200),second(300,400); first.handled=true; listener.on_event(first); listener.on_event(second);
        CHECK(engine.is_running()); CHECK(first.handled); CHECK_FALSE(second.handled);
    }
    TEST_CASE("batch2947 polymorphic close delivery can stop restarted engine repeatedly") {
        Salix::Engine engine; Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&engine)); Salix::IEventListener& base=concrete;
        Salix::WindowCloseEvent close; engine.is_running(true); base.on_event(close); CHECK_FALSE(engine.is_running()); engine.is_running(true); base.on_event(close); CHECK_FALSE(engine.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch3247") {
 TEST_CASE("batch3247 fresh listener accepts valid engine after repeated null failures") { Salix::ApplicationEventListener l; CHECK_FALSE(l.initialize(nullptr)); CHECK_FALSE(l.initialize(nullptr)); Salix::Engine e; CHECK(l.initialize(&e)); }
 TEST_CASE("batch3247 close before initialization preserves prehandled true state") { Salix::ApplicationEventListener l; Salix::WindowCloseEvent ev; ev.handled=true; l.on_event(ev); CHECK(ev.handled); }
 TEST_CASE("batch3247 valid initialization never changes engine running state by itself") { Salix::Engine e; e.is_running(true); Salix::ApplicationEventListener l; REQUIRE(l.initialize(&e)); CHECK(e.is_running()); }
 TEST_CASE("batch3247 resize preserves payload and running state after initialization") { Salix::Engine e; e.is_running(true); Salix::ApplicationEventListener l; REQUIRE(l.initialize(&e)); Salix::WindowResizeEvent ev(3247,1724); l.on_event(ev); CHECK(e.is_running()); CHECK(ev.get_width()==3247); CHECK(ev.get_height()==1724); CHECK_FALSE(ev.handled); }
 TEST_CASE("batch3247 tick event preserves running and handled states") { Salix::Engine e; e.is_running(true); Salix::ApplicationEventListener l; REQUIRE(l.initialize(&e)); Batch1847TickEvent ev; ev.handled=true; l.on_event(ev); CHECK(e.is_running()); CHECK(ev.handled); }
 TEST_CASE("batch3247 close stops latest target after three engine retargets") { Salix::Engine a,b,c; a.is_running(true); b.is_running(true); c.is_running(true); Salix::ApplicationEventListener l; REQUIRE(l.initialize(&a)); REQUIRE(l.initialize(&b)); REQUIRE(l.initialize(&c)); Salix::WindowCloseEvent ev; l.on_event(ev); CHECK(a.is_running()); CHECK(b.is_running()); CHECK_FALSE(c.is_running()); }
 TEST_CASE("batch3247 null retarget after three valid targets preserves final target") { Salix::Engine a,b,c; a.is_running(true); b.is_running(true); c.is_running(true); Salix::ApplicationEventListener l; REQUIRE(l.initialize(&a)); REQUIRE(l.initialize(&b)); REQUIRE(l.initialize(&c)); CHECK_FALSE(l.initialize(nullptr)); Salix::WindowCloseEvent ev; l.on_event(ev); CHECK(a.is_running()); CHECK(b.is_running()); CHECK_FALSE(c.is_running()); }
 TEST_CASE("batch3247 polymorphic resize delivery does not stop engine") { Salix::Engine e; e.is_running(true); Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&e)); Salix::IEventListener& l=concrete; Salix::WindowResizeEvent ev(10,20); l.on_event(ev); CHECK(e.is_running()); CHECK_FALSE(ev.handled); }
 TEST_CASE("batch3247 polymorphic close delivery preserves already handled flag while stopping engine") { Salix::Engine e; e.is_running(true); Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&e)); Salix::IEventListener& l=concrete; Salix::WindowCloseEvent ev; ev.handled=true; l.on_event(ev); CHECK_FALSE(e.is_running()); CHECK(ev.handled); }
 TEST_CASE("batch3247 same listener can stop restarted target across three close events") { Salix::Engine e; Salix::ApplicationEventListener l; REQUIRE(l.initialize(&e)); for(int i=0;i<3;++i){ e.is_running(true); Salix::WindowCloseEvent ev; l.on_event(ev); CHECK_FALSE(e.is_running()); CHECK_FALSE(ev.handled); } }
}
TEST_SUITE("Salix::events::ApplicationEventListenerBatch3547") {
    TEST_CASE("batch3547 close event keeps an already stopped initialized engine stopped") {
        Salix::Engine engine; engine.is_running(false);
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch3547 resize event keeps an already stopped initialized engine stopped") {
        Salix::Engine engine; engine.is_running(false);
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(640, 480);
        listener.on_event(resize);
        CHECK_FALSE(engine.is_running());
        CHECK(resize.get_width() == 640);
        CHECK(resize.get_height() == 480);
    }

    TEST_CASE("batch3547 prehandled resize preserves handled flag and running state") {
        Salix::Engine engine; engine.is_running(true);
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(123, 456); resize.handled = true;
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.handled);
    }

    TEST_CASE("batch3547 ordinary close preserves false handled flag while stopping target") {
        Salix::Engine engine; engine.is_running(true);
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        REQUIRE_FALSE(close.handled);
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch3547 stopped old target remains stopped after listener retargets and closes running new target") {
        Salix::Engine old_engine, new_engine;
        old_engine.is_running(false); new_engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&old_engine));
        REQUIRE(listener.initialize(&new_engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(old_engine.is_running());
        CHECK_FALSE(new_engine.is_running());
    }

    TEST_CASE("batch3547 running old target remains running when listener retargets to already stopped engine") {
        Salix::Engine old_engine, new_engine;
        old_engine.is_running(true); new_engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&old_engine));
        REQUIRE(listener.initialize(&new_engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(old_engine.is_running());
        CHECK_FALSE(new_engine.is_running());
    }

    TEST_CASE("batch3547 failed null initialization before valid target does not prevent later close routing") {
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::Engine engine; engine.is_running(true);
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch3547 initialized listener can stop target again after external restart") {
        Salix::Engine engine;
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        for (int i = 0; i < 2; ++i) {
            engine.is_running(true);
            Salix::WindowCloseEvent close;
            listener.on_event(close);
            CHECK_FALSE(engine.is_running());
        }
    }

    TEST_CASE("batch3547 polymorphic resize delivery preserves running engine and payload") {
        Salix::Engine engine; engine.is_running(true);
        Salix::ApplicationEventListener concrete; REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener* base = &concrete;
        Salix::WindowResizeEvent resize(3547, 7453);
        base->on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.get_width() == 3547);
        CHECK(resize.get_height() == 7453);
        CHECK_FALSE(resize.handled);
    }

    TEST_CASE("batch3547 resize close resize sequence stops once and never mutates event handled flags") {
        Salix::Engine engine; engine.is_running(true);
        Salix::ApplicationEventListener listener; REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent before(1, 2);
        Salix::WindowCloseEvent close;
        Salix::WindowResizeEvent after(3, 4);
        listener.on_event(before);
        CHECK(engine.is_running());
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        listener.on_event(after);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(before.handled);
        CHECK_FALSE(close.handled);
        CHECK_FALSE(after.handled);
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch3947") {
    TEST_CASE("batch3947 valid initialization preserves an already stopped engine state") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch3947 prehandled close before initialization remains safely handled") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent close;
        close.handled = true;
        CHECK_NOTHROW(listener.on_event(close));
        CHECK(close.handled);
    }

    TEST_CASE("batch3947 resize before initialization preserves payload and handled state") {
        Salix::ApplicationEventListener listener;
        Salix::WindowResizeEvent resize(3947, 7049);
        CHECK_NOTHROW(listener.on_event(resize));
        CHECK(resize.get_width() == 3947);
        CHECK(resize.get_height() == 7049);
        CHECK_FALSE(resize.handled);
    }

    TEST_CASE("batch3947 tick before initialization is a safe no op") {
        Salix::ApplicationEventListener listener;
        Batch1847TickEvent tick;
        tick.handled = false;
        CHECK_NOTHROW(listener.on_event(tick));
        CHECK_FALSE(tick.handled);
    }

    TEST_CASE("batch3947 prehandled resize after initialization keeps engine running and flag intact") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(1, 1);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.handled);
    }

    TEST_CASE("batch3947 resize followed by close preserves resize payload then stops engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(800, 450);
        listener.on_event(resize);
        CHECK(engine.is_running());
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(resize.get_width() == 800);
        CHECK(resize.get_height() == 450);
    }

    TEST_CASE("batch3947 retarget after first engine was stopped affects only newest target") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        Salix::WindowCloseEvent first_close;
        listener.on_event(first_close);
        CHECK_FALSE(first.is_running());
        first.is_running(true);
        REQUIRE(listener.initialize(&second));
        Salix::WindowCloseEvent second_close;
        listener.on_event(second_close);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("batch3947 repeated null initialization failures preserve established target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        CHECK_FALSE(listener.initialize(nullptr));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch3947 polymorphic close delivery keeps already stopped engine stopped") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch3947 repeated initialization to same engine preserves later close routing") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        REQUIRE(listener.initialize(&engine));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch4247") {
    TEST_CASE("batch4247 repeated null initialization attempts remain rejected") {
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        CHECK_FALSE(listener.initialize(nullptr));
    }

    TEST_CASE("batch4247 close before initialization remains safe and unmodified") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent close;
        CHECK_NOTHROW(listener.on_event(close));
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch4247 resize before initialization preserves payload") {
        Salix::ApplicationEventListener listener;
        Salix::WindowResizeEvent resize(4247, 7424);
        listener.on_event(resize);
        CHECK(resize.get_width() == 4247);
        CHECK(resize.get_height() == 7424);
        CHECK_FALSE(resize.handled);
    }

    TEST_CASE("batch4247 valid initialization never changes current running state") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK(engine.is_running());
    }

    TEST_CASE("batch4247 close stops initialized running engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4247 prehandled close remains handled while stopping engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        close.handled = true;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(close.handled);
    }

    TEST_CASE("batch4247 tick event never changes initialized running engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Batch1847TickEvent tick;
        listener.on_event(tick);
        CHECK(engine.is_running());
        CHECK_FALSE(tick.handled);
    }

    TEST_CASE("batch4247 retargeting routes close only to newest engine") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("batch4247 failed null reinitialization preserves established target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4247 polymorphic listener can stop restarted engine repeatedly") {
        Salix::Engine engine;
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        for (int i = 0; i < 2; ++i) {
            engine.is_running(true);
            Salix::WindowCloseEvent close;
            listener.on_event(close);
            CHECK_FALSE(engine.is_running());
            CHECK_FALSE(close.handled);
        }
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch4647") {
    TEST_CASE("batch4647 initialization with stopped engine never starts it") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4647 resize event leaves stopped initialized engine stopped") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(4647, 7464);
        listener.on_event(resize);
        CHECK_FALSE(engine.is_running());
        CHECK(resize.get_width() == 4647);
        CHECK(resize.get_height() == 7464);
    }

    TEST_CASE("batch4647 close stops engine without changing selected engine mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch4647 close stops engine without changing time scale") {
        Salix::Engine engine;
        engine.set_time_scale(-4.647f);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(-4.647f));
    }

    TEST_CASE("batch4647 retarget first second then first routes final close back to first") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        REQUIRE(listener.initialize(&first));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(first.is_running());
        CHECK(second.is_running());
    }

    TEST_CASE("batch4647 failed null initialization after retarget preserves newest valid engine") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("batch4647 listener destruction after initialization never changes engine state") {
        Salix::Engine engine;
        engine.is_running(true);
        {
            Salix::ApplicationEventListener listener;
            REQUIRE(listener.initialize(&engine));
            CHECK(engine.is_running());
        }
        CHECK(engine.is_running());
    }

    TEST_CASE("batch4647 two listeners linked to same engine can process close sequentially") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener first, second;
        REQUIRE(first.initialize(&engine));
        REQUIRE(second.initialize(&engine));
        Salix::WindowCloseEvent close1;
        first.on_event(close1);
        REQUIRE_FALSE(engine.is_running());
        Salix::WindowCloseEvent close2;
        CHECK_NOTHROW(second.on_event(close2));
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close1.handled);
        CHECK_FALSE(close2.handled);
    }

    TEST_CASE("batch4647 prehandled resize preserves handled flag and running state") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(64, 47);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.handled);
        CHECK(resize.get_width() == 64);
        CHECK(resize.get_height() == 47);
    }

    TEST_CASE("batch4647 polymorphic close preserves configured mode and time scale together") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(0.4647f);
        engine.is_running(true);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(0.4647f));
        CHECK_FALSE(close.handled);
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch5147") {
    TEST_CASE("batch5147 close before initialization remains harmless and unhandled") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent close;
        CHECK_NOTHROW(listener.on_event(close));
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch5147 failed null initialization can be followed by successful valid initialization") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5147 failed null reinitialization preserves previous valid engine target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5147 repeated valid initialization remains idempotent for close routing") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        REQUIRE(listener.initialize(&engine));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5147 resize event preserves running engine and dimensions") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(5147, 7415);
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.get_width() == 5147);
        CHECK(resize.get_height() == 7415);
    }

    TEST_CASE("batch5147 prehandled resize remains handled without affecting engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(51, 47);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.handled);
    }

    TEST_CASE("batch5147 prehandled close still stops engine and preserves handled flag") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        close.handled = true;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(close.handled);
    }

    TEST_CASE("batch5147 close preserves configured debug mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch5147 close preserves configured negative time scale") {
        Salix::Engine engine;
        engine.set_time_scale(-5.147f);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(-5.147f));
    }

    TEST_CASE("batch5147 retargeted listener stops only newest engine") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch5547") {
    TEST_CASE("batch5547 null initialization is repeatably rejected") {
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        CHECK_FALSE(listener.initialize(nullptr));
    }

    TEST_CASE("batch5547 resize before initialization preserves event state") {
        Salix::ApplicationEventListener listener;
        Salix::WindowResizeEvent resize(5547, 7455);
        listener.on_event(resize);
        CHECK(resize.get_width() == 5547);
        CHECK(resize.get_height() == 7455);
        CHECK_FALSE(resize.handled);
    }

    TEST_CASE("batch5547 valid initialization leaves stopped engine stopped on resize") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(55, 47);
        listener.on_event(resize);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5547 close stops running engine through direct listener") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch5547 close stops running engine through interface dispatch") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5547 repeated close remains idempotent on already stopped engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5547 close preserves engine time scale and mode together") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(5.547f);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(5.547f));
    }

    TEST_CASE("batch5547 failed null reinitialization preserves current engine target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5547 retargeting twice routes close only to final engine") {
        Salix::Engine first, second, third;
        first.is_running(true); second.is_running(true); third.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        REQUIRE(listener.initialize(&third));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(first.is_running());
        CHECK(second.is_running());
        CHECK_FALSE(third.is_running());
    }

    TEST_CASE("batch5547 prehandled close preserves handled flag while still stopping engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        close.handled = true;
        listener.on_event(close);
        CHECK(close.handled);
        CHECK_FALSE(engine.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch5947") {
    TEST_CASE("batch5947 null initialization is safely rejected repeatedly") {
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        CHECK_FALSE(listener.initialize(nullptr));
    }

    TEST_CASE("batch5947 close before initialization remains a harmless no op") {
        Salix::ApplicationEventListener listener;
        Salix::WindowCloseEvent close;
        CHECK_NOTHROW(listener.on_event(close));
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch5947 valid initialization accepts engine pointer") {
        Salix::Engine engine;
        Salix::ApplicationEventListener listener;
        CHECK(listener.initialize(&engine));
    }

    TEST_CASE("batch5947 resize preserves running engine and exact dimensions") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(5947, 7495);
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.get_width() == 5947);
        CHECK(resize.get_height() == 7495);
    }

    TEST_CASE("batch5947 close stops running engine without changing handled flag") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK_FALSE(close.handled);
    }

    TEST_CASE("batch5947 prehandled close remains handled while stopping engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        close.handled = true;
        listener.on_event(close);
        CHECK(close.handled);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5947 close preserves configured mode and time scale") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(-5.947f);
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.get_time_scale() == doctest::Approx(-5.947f));
    }

    TEST_CASE("batch5947 failed null reinitialization preserves previous valid target") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        CHECK_FALSE(listener.initialize(nullptr));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch5947 retargeting routes shutdown only to newest engine") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }

    TEST_CASE("batch5947 interface dispatch reaches concrete shutdown behavior") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }
}

TEST_SUITE("Salix::events::ApplicationEventListenerBatch6047") {
    TEST_CASE("batch6047 rejected null initialization can recover with later valid engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        CHECK_FALSE(listener.initialize(nullptr));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch6047 repeated initialization with same engine remains functional") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch6047 retargeting back to first engine closes only final target") {
        Salix::Engine first, second;
        first.is_running(true);
        second.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&first));
        REQUIRE(listener.initialize(&second));
        REQUIRE(listener.initialize(&first));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(first.is_running());
        CHECK(second.is_running());
    }

    TEST_CASE("batch6047 close on already stopped engine remains safely stopped") {
        Salix::Engine engine;
        engine.is_running(false);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch6047 repeated close events leave engine stopped") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent first;
        Salix::WindowCloseEvent second;
        listener.on_event(first);
        REQUIRE_FALSE(engine.is_running());
        listener.on_event(second);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch6047 zero sized resize preserves running state and dimensions") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(0, 0);
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.get_width() == 0);
        CHECK(resize.get_height() == 0);
    }

    TEST_CASE("batch6047 large unsigned resize values remain event local and do not stop engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(4000000000u, 3000000000u);
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(resize.get_width() == 4000000000u);
        CHECK(resize.get_height() == 3000000000u);
    }

    TEST_CASE("batch6047 resize after close never restarts engine") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowCloseEvent close;
        listener.on_event(close);
        REQUIRE_FALSE(engine.is_running());
        Salix::WindowResizeEvent resize(6047, 4706);
        listener.on_event(resize);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch6047 prehandled resize retains handled flag and running state") {
        Salix::Engine engine;
        engine.is_running(true);
        Salix::ApplicationEventListener listener;
        REQUIRE(listener.initialize(&engine));
        Salix::WindowResizeEvent resize(60, 47);
        resize.handled = true;
        listener.on_event(resize);
        CHECK(resize.handled);
        CHECK(engine.is_running());
    }

    TEST_CASE("batch6047 interface resize dispatch preserves engine mode and time scale") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(6.047f);
        Salix::ApplicationEventListener concrete;
        REQUIRE(concrete.initialize(&engine));
        Salix::IEventListener& listener = concrete;
        Salix::WindowResizeEvent resize(640, 470);
        listener.on_event(resize);
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(6.047f));
    }
}
