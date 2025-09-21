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