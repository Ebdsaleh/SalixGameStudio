// EventManagerQueue.test.cpp - deferred event queue behavioural regressions.
// No application windows, project files, assets or graphics backends are used.
#include <doctest.h>
#include <Salix/events/EventManager.h>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>

#include <memory>
#include <vector>

namespace {
    // Encodes ordering explicitly: different events of the same type can be
    // distinguished without relying on pointer identity or execution timing.
    class NumberedApplicationEvent final : public Salix::IEvent {
    public:
        explicit NumberedApplicationEvent(int number) : sequence(number) {}

        Salix::EventType get_event_type() const override {
            return Salix::EventType::AppTick;
        }
        const char* get_name() const override { return "NumberedApplicationEvent"; }
        int get_category_flags() const override {
            return static_cast<int>(Salix::EventCategory::Application);
        }
        std::unique_ptr<Salix::IEvent> clone() const override {
            return std::make_unique<NumberedApplicationEvent>(*this);
        }

        int sequence;
    };
}

TEST_SUITE("Salix::events::EventManagerQueue") {

    TEST_CASE("duplicate subscription delivers each event just once to that listener") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };

        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("dispatch defers delivery until process_queue and drains pending events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        CHECK(manager.is_queue_empty());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        CHECK(deliveries == 0);

        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK(deliveries == 1);

        manager.process_queue();
        CHECK(deliveries == 1);
    }

    TEST_CASE("queued events reach subscribers in FIFO order") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        std::vector<int> received;
        listener.on_event_handler = [&](Salix::IEvent& event) {
            received.push_back(static_cast<NumberedApplicationEvent&>(event).sequence);
        };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<NumberedApplicationEvent>(41));
        manager.dispatch(std::make_unique<NumberedApplicationEvent>(7));
        manager.dispatch(std::make_unique<NumberedApplicationEvent>(99));
        CHECK(received.empty());

        manager.process_queue();
        REQUIRE(received.size() == 3);
        CHECK(received[0] == 41);
        CHECK(received[1] == 7);
        CHECK(received[2] == 99);
    }

    TEST_CASE("unsubscribing before processing prevents delivery of queued events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.unsubscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();

        CHECK(deliveries == 0);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("a listener subscribed after dispatch receives the still-queued event") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unrelated event categories are not delivered to subscribers") {
        Salix::EventManager manager;
        Salix::MockListener editor_listener;
        Salix::MockListener application_listener;
        int editor_calls = 0;
        int application_calls = 0;
        editor_listener.on_event_handler = [&](Salix::IEvent&) { ++editor_calls; };
        application_listener.on_event_handler = [&](Salix::IEvent&) { ++application_calls; };
        manager.subscribe(Salix::EventCategory::Editor, &editor_listener);
        manager.subscribe(Salix::EventCategory::Application, &application_listener);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(editor_calls == 0);
        CHECK(application_calls == 1);
    }

    TEST_CASE("handled events stop notifying later listeners but not later events") {
        Salix::EventManager manager;
        Salix::MockListener first;
        Salix::MockListener second;
        int first_calls = 0;
        int second_calls = 0;
        first.on_event_handler = [&](Salix::IEvent& event) {
            ++first_calls;
            event.handled = true;
        };
        second.on_event_handler = [&](Salix::IEvent&) { ++second_calls; };
        manager.subscribe(Salix::EventCategory::Application, &first);
        manager.subscribe(Salix::EventCategory::Application, &second);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(first_calls == 2);
        CHECK(second_calls == 0);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("blocking an event does not block subsequent normal events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<Salix::MockBlockingEvent>());
        CHECK(manager.is_queue_empty());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("dispatching by const reference clones the event for later delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        std::vector<int> received;
        listener.on_event_handler = [&](Salix::IEvent& event) {
            received.push_back(static_cast<NumberedApplicationEvent&>(event).sequence);
        };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        {
            const NumberedApplicationEvent stack_event(1234);
            manager.dispatch(stack_event);
        }

        CHECK(received.empty());
        manager.process_queue();
        REQUIRE(received.size() == 1);
        CHECK(received.front() == 1234);
    }

    TEST_CASE("processing an event without listeners still drains the queue") {
        Salix::EventManager manager;
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());

        manager.process_queue();
        CHECK(manager.is_queue_empty());
    }
}


// Integration boundaries for the 847+ coverage batch.
TEST_SUITE("Salix::events::EventManagerQueueBatch847") {
    TEST_CASE("processing an already empty queue is idempotent") {
        Salix::EventManager manager; CHECK(manager.is_queue_empty()); manager.process_queue(); CHECK(manager.is_queue_empty()); manager.process_queue(); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unsubscribing an unknown listener from an unused category is harmless") {
        Salix::EventManager manager; Salix::MockListener listener; CHECK_NOTHROW(manager.unsubscribe(Salix::EventCategory::Editor,&listener)); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("two listeners receive an unhandled event in subscription order") {
        Salix::EventManager manager; Salix::MockListener a,b; std::vector<int> order; a.on_event_handler=[&](Salix::IEvent&){order.push_back(1);}; b.on_event_handler=[&](Salix::IEvent&){order.push_back(2);}; manager.subscribe(Salix::EventCategory::Application,&a); manager.subscribe(Salix::EventCategory::Application,&b); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); REQUIRE(order.size()==2); CHECK(order[0]==1); CHECK(order[1]==2);
    }

    TEST_CASE("a listener can unsubscribe itself during delivery and misses the next queued event") {
        Salix::EventManager manager; Salix::MockListener self; int calls=0; self.on_event_handler=[&](Salix::IEvent&){++calls; manager.unsubscribe(Salix::EventCategory::Application,&self);}; manager.subscribe(Salix::EventCategory::Application,&self); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unsubscribing a later listener during delivery affects the next event not the current copy") {
        Salix::EventManager manager; Salix::MockListener first,second; int a=0,b=0; first.on_event_handler=[&](Salix::IEvent&){++a; manager.unsubscribe(Salix::EventCategory::Application,&second);}; second.on_event_handler=[&](Salix::IEvent&){++b;}; manager.subscribe(Salix::EventCategory::Application,&first); manager.subscribe(Salix::EventCategory::Application,&second); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(a==2); CHECK(b==1);
    }

    TEST_CASE("subscribing a new listener during delivery begins with the next queued event") {
        Salix::EventManager manager; Salix::MockListener first,late; int first_calls=0,late_calls=0; late.on_event_handler=[&](Salix::IEvent&){++late_calls;}; first.on_event_handler=[&](Salix::IEvent&){++first_calls; manager.subscribe(Salix::EventCategory::Application,&late);}; manager.subscribe(Salix::EventCategory::Application,&first); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(first_calls==2); CHECK(late_calls==1);
    }

    TEST_CASE("blocked events interleaved with queued events never disturb FIFO payload order") {
        Salix::EventManager manager; Salix::MockListener listener; std::vector<int> values; listener.on_event_handler=[&](Salix::IEvent& e){values.push_back(static_cast<NumberedApplicationEvent&>(e).sequence);}; manager.subscribe(Salix::EventCategory::Application,&listener); manager.dispatch(std::make_unique<NumberedApplicationEvent>(1)); manager.dispatch(std::make_unique<Salix::MockBlockingEvent>()); manager.dispatch(std::make_unique<NumberedApplicationEvent>(2)); manager.process_queue(); REQUIRE(values.size()==2); CHECK(values[0]==1); CHECK(values[1]==2);
    }

    TEST_CASE("const reference dispatch preserves the source object for reuse") {
        Salix::EventManager manager; Salix::MockListener listener; int sum=0; listener.on_event_handler=[&](Salix::IEvent& e){sum+=static_cast<NumberedApplicationEvent&>(e).sequence;}; manager.subscribe(Salix::EventCategory::Application,&listener); const NumberedApplicationEvent event(9); manager.dispatch(event); manager.dispatch(event); CHECK(event.sequence==9); manager.process_queue(); CHECK(sum==18);
    }

    TEST_CASE("removing and resubscribing a listener does not duplicate future delivery") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;}; manager.subscribe(Salix::EventCategory::Application,&listener); manager.unsubscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1);
    }

    TEST_CASE("handled state on one queued event does not leak into the next event object") {
        Salix::EventManager manager; Salix::MockListener first,second; int first_calls=0,second_calls=0; first.on_event_handler=[&](Salix::IEvent& e){++first_calls; if(first_calls==1)e.handled=true;}; second.on_event_handler=[&](Salix::IEvent&){++second_calls;}; manager.subscribe(Salix::EventCategory::Application,&first); manager.subscribe(Salix::EventCategory::Application,&second); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(first_calls==2); CHECK(second_calls==1);
    }
}
