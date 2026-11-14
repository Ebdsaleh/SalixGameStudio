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

TEST_SUITE("Salix::events::IEventListenerBatch6547") {
    TEST_CASE("batch6547 default listener marks event received without changing handled state") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        listener.on_event(event);
        CHECK(listener.event_received);
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("batch6547 custom handler is used instead of default received flag") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        listener.on_event(event);
        CHECK(calls == 1);
        CHECK_FALSE(listener.event_received);
    }

    TEST_CASE("batch6547 handler may toggle handled state in both directions") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        listener.on_event_handler = [](Salix::IEvent& received) {
            received.handled = true;
            received.handled = false;
        };
        listener.on_event(event);
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("batch6547 two listeners observe the same exact event address") {
        Salix::MockListener first;
        Salix::MockListener second;
        Salix::MockEvent event;
        Salix::IEvent* first_seen = nullptr;
        Salix::IEvent* second_seen = nullptr;
        first.on_event_handler = [&](Salix::IEvent& received) { first_seen = &received; };
        second.on_event_handler = [&](Salix::IEvent& received) { second_seen = &received; };
        first.on_event(event);
        second.on_event(event);
        CHECK(first_seen == &event);
        CHECK(second_seen == &event);
    }

    TEST_CASE("batch6547 listener may inspect blocking contract without mutating event") {
        Salix::MockListener listener;
        Salix::MockBlockingEvent event;
        bool blocking = false;
        listener.on_event_handler = [&](Salix::IEvent& received) {
            blocking = received.should_block();
        };
        listener.on_event(event);
        CHECK(blocking);
        CHECK_FALSE(event.handled);
    }

    TEST_CASE("batch6547 listener receives cloned event independently from source handled state") {
        Salix::MockEvent source;
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        clone->handled = false;
        Salix::MockListener listener;
        listener.on_event(*clone);
        CHECK(listener.event_received);
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("batch6547 handler observes null native handle on mock event") {
        Salix::MockListener listener;
        Salix::MockEvent event;
        bool saw_null = false;
        listener.on_event_handler = [&](Salix::IEvent& received) {
            saw_null = received.get_native_handle() == nullptr;
        };
        listener.on_event(event);
        CHECK(saw_null);
    }

    TEST_CASE("batch6547 independent listeners keep received flags independent") {
        Salix::MockListener first;
        Salix::MockListener second;
        Salix::MockEvent event;
        first.on_event(event);
        CHECK(first.event_received);
        CHECK_FALSE(second.event_received);
    }

    TEST_CASE("batch6547 polymorphic listener dispatch reaches configured handler") {
        auto concrete = std::make_unique<Salix::MockListener>();
        int calls = 0;
        concrete->on_event_handler = [&](Salix::IEvent&) { ++calls; };
        std::unique_ptr<Salix::IEventListener> listener = std::move(concrete);
        Salix::MockEvent event;
        listener->on_event(event);
        CHECK(calls == 1);
    }

    TEST_CASE("batch6547 same listener distinguishes two different event object addresses") {
        Salix::MockListener listener;
        Salix::MockEvent first;
        Salix::MockEvent second;
        Salix::IEvent* seen_first = nullptr;
        Salix::IEvent* seen_second = nullptr;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent& received) {
            ++calls;
            if (calls == 1) seen_first = &received;
            if (calls == 2) seen_second = &received;
        };
        listener.on_event(first);
        listener.on_event(second);
        CHECK(seen_first == &first);
        CHECK(seen_second == &second);
        CHECK(seen_first != seen_second);
    }
}
