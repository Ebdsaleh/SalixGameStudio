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
