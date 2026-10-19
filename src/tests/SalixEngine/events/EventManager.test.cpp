// =================================================================================
// Filename:    src/Tests/SalixEngine/events/EventManager.test.cpp
// Description: Contains unit tests for the EventManager.
// ================================================================================= 

#include <doctest.h>
#include <Salix/events/EventManager.h>
// Include the new reusable mock header file.
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>

namespace Salix {

    TEST_SUITE("Salix::events::EventManager") {

        TEST_CASE("a subscribed listener receives a dispatched event") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto mock_event = std::make_unique<MockEvent>();

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener.event_received == true);
        }

        TEST_CASE("an unsubscribed listener does not receive events") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto mock_event = std::make_unique<MockEvent>();

            event_manager.subscribe(EventCategory::Application, &listener);
            
            // ACT
            event_manager.unsubscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener.event_received == false);
        }

        TEST_CASE("a handled event is not dispatched to other listeners") {
            // ARRANGE
            EventManager event_manager;
            auto mock_event = std::make_unique<MockEvent>();
            MockListener listener1;
            MockListener listener2;

            // --- CORRECTED LOGIC ---
            // Assign a lambda to the handler variable, not the function itself.
            listener1.on_event_handler = [&](IEvent& event) {
                event.handled = true;
                listener1.event_received = true;
            };

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener1);
            event_manager.subscribe(EventCategory::Application, &listener2);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener1.event_received == true);
            CHECK(listener2.event_received == false);
        }

        TEST_CASE("helper methods and overloads work correctly") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            MockEvent mock_event_on_stack;

            // ASSERT
            CHECK(event_manager.is_queue_empty() == true);

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(mock_event_on_stack);

            // ASSERT
            CHECK(event_manager.is_queue_empty() == false);
            event_manager.process_queue();
            CHECK(event_manager.is_queue_empty() == true);
            CHECK(listener.event_received == true);
        }
        TEST_CASE("a blocking event is not added to the queue") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto blocking_event = std::make_unique<MockBlockingEvent>();

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(blocking_event)); // Dispatch the blocking event

            // ASSERT
            // The queue should still be empty because the event was blocked before being added.
            CHECK(event_manager.is_queue_empty() == true);

            // ACT (continued)
            event_manager.process_queue(); // Process the (empty) queue

            // ASSERT
            // The listener should not have received the event.
            CHECK(listener.event_received == false);
        }
    }

} // namespace Salix

// Cross-category and multi-listener queue contracts (no Editor components required).
namespace {
    class AdditionalMultiCategoryEvent final : public Salix::IEvent {
    public:
        Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
        const char* get_name() const override { return "AdditionalMultiCategoryEvent"; }
        int get_category_flags() const override { return Salix::EventCategory::Application | Salix::EventCategory::Editor; }
        std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<AdditionalMultiCategoryEvent>(*this); }
    };
}

TEST_SUITE("Salix::events::EventManagerAdditional") {
    TEST_CASE("event manager two listeners in one category both receive an unhandled event") {
        Salix::EventManager manager;
        Salix::MockListener a, b;
        int ac = 0, bc = 0;
        a.on_event_handler = [&](Salix::IEvent&) { ++ac; };
        b.on_event_handler = [&](Salix::IEvent&) { ++bc; };
        manager.subscribe(Salix::EventCategory::Application, &a);
        manager.subscribe(Salix::EventCategory::Application, &b);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(ac == 1);
        CHECK(bc == 1);
    }

    TEST_CASE("event manager two queued events each reach both eligible listeners") {
        Salix::EventManager manager;
        Salix::MockListener a, b;
        int ac = 0, bc = 0;
        a.on_event_handler = [&](Salix::IEvent&) { ++ac; };
        b.on_event_handler = [&](Salix::IEvent&) { ++bc; };
        manager.subscribe(Salix::EventCategory::Application, &a);
        manager.subscribe(Salix::EventCategory::Application, &b);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(ac == 2);
        CHECK(bc == 2);
    }

    TEST_CASE("event manager unsubscribing an unknown listener does not remove another subscriber") {
        Salix::EventManager manager;
        Salix::MockListener subscribed, stranger;
        manager.subscribe(Salix::EventCategory::Application, &subscribed);
        manager.unsubscribe(Salix::EventCategory::Application, &stranger);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(subscribed.event_received);
        CHECK_FALSE(stranger.event_received);
    }

    TEST_CASE("event manager unsubscribing an unknown category preserves application subscription") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.unsubscribe(Salix::EventCategory::Editor, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(listener.event_received);
    }

    TEST_CASE("event manager multi-category event reaches one listener for each category subscription") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int received = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++received; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Editor, &listener);
        manager.dispatch(std::make_unique<AdditionalMultiCategoryEvent>());
        manager.process_queue();
        CHECK(received == 2);
    }

    TEST_CASE("event manager handled multi-category event does not leak into later categories") {
        Salix::EventManager manager;
        Salix::MockListener first, later;
        int later_received = 0;
        first.on_event_handler = [&](Salix::IEvent& e) { e.handled = true; };
        later.on_event_handler = [&](Salix::IEvent&) { ++later_received; };
        manager.subscribe(Salix::EventCategory::Application, &first);
        manager.subscribe(Salix::EventCategory::Editor, &later);
        manager.dispatch(std::make_unique<AdditionalMultiCategoryEvent>());
        manager.process_queue();
        CHECK(later_received == 0);
    }

    TEST_CASE("event manager handling first queued event leaves later queued events independently handled") {
        Salix::EventManager manager;
        Salix::MockListener handler, observer;
        int count = 0;
        handler.on_event_handler = [&](Salix::IEvent& e) { if (++count == 1) e.handled = true; };
        int observers = 0;
        observer.on_event_handler = [&](Salix::IEvent&) { ++observers; };
        manager.subscribe(Salix::EventCategory::Application, &handler);
        manager.subscribe(Salix::EventCategory::Application, &observer);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(count == 2);
        CHECK(observers == 1);
    }

    TEST_CASE("event manager resubscribing after unsubscribe restores exactly one delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.unsubscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(calls == 1);
    }

    TEST_CASE("event manager separate managers never deliver an event through the other manager") {
        Salix::EventManager alpha, beta;
        Salix::MockListener a, b;
        alpha.subscribe(Salix::EventCategory::Application, &a);
        beta.subscribe(Salix::EventCategory::Application, &b);
        alpha.dispatch(std::make_unique<Salix::MockEvent>());
        alpha.process_queue();
        CHECK(a.event_received);
        CHECK_FALSE(b.event_received);
        CHECK(beta.is_queue_empty());
    }

    TEST_CASE("event manager empty process and subsequently added events preserve queue readiness") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK_FALSE(listener.event_received);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK(listener.event_received);
    }
}
