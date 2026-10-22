#include <doctest.h>
#include <Salix/events/IEventPoller.h>
#include <Tests/SalixEngine/mocking/events/MockIEventPoller.h>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>
#include <memory>

TEST_SUITE("Salix::events::IEventPollerBatch1347") {
    TEST_CASE("event poller empty queue invokes no processed callbacks") {
        MockIEventPoller poller;
        int calls = 0;
        poller.poll_events([&](Salix::IEvent&) { ++calls; });
        CHECK(calls == 0);
        CHECK(poller.poll_calls == 1);
    }

    TEST_CASE("event poller dispatches queued event through interface callback") {
        MockIEventPoller poller;
        poller.queue_event(std::make_unique<Salix::MockEvent>());
        int calls = 0;
        Salix::IEventPoller* interface_ptr = &poller;
        interface_ptr->poll_events([&](Salix::IEvent& event) {
            ++calls;
            CHECK(event.get_event_type() == Salix::EventType::AppTick);
        });
        CHECK(calls == 1);
    }

    TEST_CASE("event poller dispatches multiple queued events in insertion order") {
        MockIEventPoller poller;
        poller.queue_event(std::make_unique<Salix::MockEvent>());
        poller.queue_event(std::make_unique<Salix::MockBlockingEvent>());
        int index = 0;
        poller.poll_events([&](Salix::IEvent& event) {
            if (index == 0) CHECK_FALSE(event.should_block());
            if (index == 1) CHECK(event.should_block());
            ++index;
        });
        CHECK(index == 2);
    }

    TEST_CASE("event poller callback can mutate queued event handled state") {
        MockIEventPoller poller;
        poller.queue_event(std::make_unique<Salix::MockEvent>());
        poller.poll_events([](Salix::IEvent& event) { event.handled = true; });
        int handled = 0;
        poller.poll_events([&](Salix::IEvent& event) { if (event.handled) ++handled; });
        CHECK(handled == 1);
    }

    TEST_CASE("event poller raw callback registration returns unique nonzero handles") {
        MockIEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        CHECK(first != 0);
        CHECK(second != 0);
        CHECK(first != second);
        CHECK(poller.raw_callback_count() == 2);
    }

    TEST_CASE("event poller raw callback receives exact native pointer") {
        MockIEventPoller poller;
        void* seen = nullptr;
        poller.register_raw_event_callback([&](void* native) { seen = native; });
        int native = 73;
        poller.emit_raw(&native);
        CHECK(seen == &native);
    }

    TEST_CASE("event poller unregister prevents future raw callback dispatch") {
        MockIEventPoller poller;
        int calls = 0;
        const auto handle = poller.register_raw_event_callback([&](void*) { ++calls; });
        poller.emit_raw(nullptr);
        poller.unregister_raw_event_callback(handle);
        poller.emit_raw(nullptr);
        CHECK(calls == 1);
        CHECK(poller.raw_callback_count() == 0);
    }

    TEST_CASE("event poller multiple raw callbacks receive same native event") {
        MockIEventPoller poller;
        int first = 0;
        int second = 0;
        poller.register_raw_event_callback([&](void*) { ++first; });
        poller.register_raw_event_callback([&](void*) { ++second; });
        poller.emit_raw(nullptr);
        CHECK(first == 1);
        CHECK(second == 1);
    }

    TEST_CASE("event poller clearing queued events leaves raw callbacks intact") {
        MockIEventPoller poller;
        poller.queue_event(std::make_unique<Salix::MockEvent>());
        int raw_calls = 0;
        poller.register_raw_event_callback([&](void*) { ++raw_calls; });
        poller.clear_events();
        int event_calls = 0;
        poller.poll_events([&](Salix::IEvent&) { ++event_calls; });
        poller.emit_raw(nullptr);
        CHECK(event_calls == 0);
        CHECK(raw_calls == 1);
    }

    TEST_CASE("event poller supports polymorphic destruction") {
        std::unique_ptr<Salix::IEventPoller> poller = std::make_unique<MockIEventPoller>();
        REQUIRE(poller != nullptr);
        int calls = 0;
        poller->poll_events([&](Salix::IEvent&) { ++calls; });
        CHECK(calls == 0);
    }
}
