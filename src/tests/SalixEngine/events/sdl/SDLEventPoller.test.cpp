#include <doctest.h>
#include <Salix/events/sdl/SDLEventPoller.h>
#include <Salix/events/IEventPoller.h>
#include <type_traits>
#include <utility>

TEST_SUITE("Salix::events::SDLEventPollerBatch1647") {
    TEST_CASE("SDL event poller derives from event poller interface") {
        CHECK((std::is_base_of_v<Salix::IEventPoller, Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller remains polymorphic through virtual event interface") {
        CHECK((std::is_polymorphic_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller exposes a virtual destructor contract") {
        CHECK((std::has_virtual_destructor_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller remains default constructible inside its owning module") {
        CHECK((std::is_default_constructible_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller remains destructible inside its owning module") {
        CHECK((std::is_destructible_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller is not copy constructible because callback state owns a mutex") {
        CHECK_FALSE((std::is_copy_constructible_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller is not copy assignable because callback state owns a mutex") {
        CHECK_FALSE((std::is_copy_assignable_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller is not move constructible while its mutex remains an object member") {
        CHECK_FALSE((std::is_move_constructible_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("SDL event poller poll method preserves void interface return contract") {
        using Return = decltype(std::declval<Salix::SDLEventPoller&>().poll_events(
            std::declval<const Salix::IEventPoller::event_callback_fn&>()));
        CHECK((std::is_same_v<Return, void>));
    }

    TEST_CASE("SDL event poller raw callback registration and unregister signatures match interface types") {
        using RegisterReturn = decltype(
            std::declval<Salix::SDLEventPoller&>().register_raw_event_callback(
                std::declval<Salix::RawEventCallback>()));
        using UnregisterReturn = decltype(
            std::declval<Salix::SDLEventPoller&>().unregister_raw_event_callback(
                std::declval<Salix::RawEventCallbackHandle>()));
        CHECK((std::is_same_v<RegisterReturn, Salix::RawEventCallbackHandle>));
        CHECK((std::is_same_v<UnregisterReturn, void>));
    }
}

TEST_SUITE("Salix::events::SDLEventPollerBatch2447") {
    TEST_CASE("batch2447 concrete SDL poller remains a non-abstract implementation") {
        CHECK_FALSE((std::is_abstract_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("batch2447 interface remains abstract while SDL implementation is concrete") {
        CHECK((std::is_abstract_v<Salix::IEventPoller>));
        CHECK_FALSE((std::is_abstract_v<Salix::SDLEventPoller>));
    }

    TEST_CASE("batch2447 SDL poller pointer converts implicitly to event poller interface pointer") {
        CHECK((std::is_convertible_v<Salix::SDLEventPoller*, Salix::IEventPoller*>));
    }

    TEST_CASE("batch2447 interface pointer does not implicitly downcast to SDL poller pointer") {
        CHECK_FALSE((std::is_convertible_v<Salix::IEventPoller*, Salix::SDLEventPoller*>));
    }

    TEST_CASE("batch2447 raw event callback handle remains unsigned integral storage") {
        CHECK((std::is_integral_v<Salix::RawEventCallbackHandle>));
        CHECK((std::is_unsigned_v<Salix::RawEventCallbackHandle>));
    }

    TEST_CASE("batch2447 raw event callback handle remains exactly sixty four bits") {
        CHECK(sizeof(Salix::RawEventCallbackHandle) == 8);
    }

    TEST_CASE("batch2447 raw event callback type remains std function accepting one native pointer") {
        CHECK((std::is_same_v<Salix::RawEventCallback, std::function<void(void*)>>));
    }

    TEST_CASE("batch2447 processed event callback remains std function accepting mutable event reference") {
        CHECK((std::is_same_v<
            Salix::IEventPoller::event_callback_fn,
            std::function<void(Salix::IEvent&)>>));
    }

    TEST_CASE("batch2447 SDL poll method member signature exactly matches interface callback contract") {
        using PollMethod = decltype(&Salix::SDLEventPoller::poll_events);
        using Expected = void (Salix::SDLEventPoller::*)(
            const Salix::IEventPoller::event_callback_fn&);
        CHECK((std::is_same_v<PollMethod, Expected>));
    }

    TEST_CASE("batch2447 SDL raw callback member signatures retain handle based registration contract") {
        using RegisterMethod = decltype(&Salix::SDLEventPoller::register_raw_event_callback);
        using UnregisterMethod = decltype(&Salix::SDLEventPoller::unregister_raw_event_callback);
        using ExpectedRegister = Salix::RawEventCallbackHandle (
            Salix::SDLEventPoller::*)(Salix::RawEventCallback);
        using ExpectedUnregister = void (
            Salix::SDLEventPoller::*)(Salix::RawEventCallbackHandle);
        CHECK((std::is_same_v<RegisterMethod, ExpectedRegister>));
        CHECK((std::is_same_v<UnregisterMethod, ExpectedUnregister>));
    }
}


TEST_SUITE("Salix::events::SDLEventPollerBatch3047") {
    TEST_CASE("batch3047 first registered raw callback receives handle one") {
        Salix::SDLEventPoller poller;
        const auto handle = poller.register_raw_event_callback([](void*) {});
        CHECK(handle == static_cast<Salix::RawEventCallbackHandle>(1));
    }

    TEST_CASE("batch3047 raw callback handles increase monotonically from one") {
        Salix::SDLEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        const auto third = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(second == 2);
        CHECK(third == 3);
    }

    TEST_CASE("batch3047 registering an empty raw callback still allocates a handle") {
        Salix::SDLEventPoller poller;
        Salix::RawEventCallback empty{};
        const auto handle = poller.register_raw_event_callback(empty);
        CHECK(handle == 1);
    }

    TEST_CASE("batch3047 unregistering a registered raw callback is safe") {
        Salix::SDLEventPoller poller;
        const auto handle = poller.register_raw_event_callback([](void*) {});
        CHECK_NOTHROW(poller.unregister_raw_event_callback(handle));
    }

    TEST_CASE("batch3047 unregistering an unknown raw callback handle is a no op") {
        Salix::SDLEventPoller poller;
        CHECK_NOTHROW(poller.unregister_raw_event_callback(3047));
    }

    TEST_CASE("batch3047 unregistering the same raw callback twice remains safe") {
        Salix::SDLEventPoller poller;
        const auto handle = poller.register_raw_event_callback([](void*) {});
        poller.unregister_raw_event_callback(handle);
        CHECK_NOTHROW(poller.unregister_raw_event_callback(handle));
    }

    TEST_CASE("batch3047 unregistering a callback never reuses its old handle") {
        Salix::SDLEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        poller.unregister_raw_event_callback(first);
        const auto second = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(second == 2);
    }

    TEST_CASE("batch3047 removing a middle callback preserves monotonic future handle allocation") {
        Salix::SDLEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        const auto third = poller.register_raw_event_callback([](void*) {});
        poller.unregister_raw_event_callback(second);
        const auto fourth = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(third == 3);
        CHECK(fourth == 4);
    }

    TEST_CASE("batch3047 separate pollers own independent callback handle sequences") {
        Salix::SDLEventPoller first;
        Salix::SDLEventPoller second;
        CHECK(first.register_raw_event_callback([](void*) {}) == 1);
        CHECK(first.register_raw_event_callback([](void*) {}) == 2);
        CHECK(second.register_raw_event_callback([](void*) {}) == 1);
    }

    TEST_CASE("batch3047 raw callback registration works through event poller interface dispatch") {
        Salix::SDLEventPoller concrete;
        Salix::IEventPoller& poller = concrete;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(second == 2);
        CHECK_NOTHROW(poller.unregister_raw_event_callback(first));
        CHECK_NOTHROW(poller.unregister_raw_event_callback(second));
    }
}
#include <SDL.h>
#include <vector>

namespace {
struct Batch3447SDLEventQueueFixture {
    Batch3447SDLEventQueueFixture() {
        SDL_InitSubSystem(SDL_INIT_EVENTS);
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }
    ~Batch3447SDLEventQueueFixture() {
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }
};
}

TEST_SUITE("Salix::events::SDLEventPollerBatch3447") {
    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 user event reaches raw callback without producing Salix event") {
        Salix::SDLEventPoller poller;
        int raw_count = 0;
        int processed_count = 0;
        poller.register_raw_event_callback([&](void* raw) {
            REQUIRE(raw != nullptr);
            ++raw_count;
            CHECK(static_cast<SDL_Event*>(raw)->type == SDL_USEREVENT);
        });
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(raw_count == 1);
        CHECK(processed_count == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 raw callbacks execute in monotonically assigned handle order") {
        Salix::SDLEventPoller poller;
        std::vector<int> order;
        poller.register_raw_event_callback([&](void*) { order.push_back(1); });
        poller.register_raw_event_callback([&](void*) { order.push_back(2); });
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        REQUIRE(order.size() == 2);
        CHECK(order[0] == 1);
        CHECK(order[1] == 2);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 unregistered raw callback is absent from subsequent poll") {
        Salix::SDLEventPoller poller;
        int first_count = 0;
        int second_count = 0;
        const auto first = poller.register_raw_event_callback([&](void*) { ++first_count; });
        poller.register_raw_event_callback([&](void*) { ++second_count; });
        poller.unregister_raw_event_callback(first);
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(first_count == 0);
        CHECK(second_count == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 empty raw callback is skipped safely during polling") {
        Salix::SDLEventPoller poller;
        Salix::RawEventCallback empty;
        poller.register_raw_event_callback(empty);
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        CHECK_NOTHROW(poller.poll_events([](Salix::IEvent&) {}));
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 SDL quit event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed_count = 0;
        SDL_Event event{}; event.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(processed_count == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 resized window event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed_count = 0;
        SDL_Event event{};
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_RESIZED;
        event.window.data1 = 1280;
        event.window.data2 = 720;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(processed_count == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 text input remains raw only") {
        Salix::SDLEventPoller poller;
        int raw_count = 0;
        int processed_count = 0;
        poller.register_raw_event_callback([&](void*) { ++raw_count; });
        SDL_Event event{}; event.type = SDL_TEXTINPUT;
        event.text.text[0] = 'A'; event.text.text[1] = '\0';
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(raw_count == 1);
        CHECK(processed_count == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 key down event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed_count = 0;
        SDL_Event event{};
        event.type = SDL_KEYDOWN;
        event.key.keysym.sym = SDLK_a;
        event.key.repeat = 0;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(processed_count == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 mouse motion event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed_count = 0;
        SDL_Event event{};
        event.type = SDL_MOUSEMOTION;
        event.motion.x = 344;
        event.motion.y = 700;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(processed_count == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch3447 mouse wheel event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed_count = 0;
        SDL_Event event{};
        event.type = SDL_MOUSEWHEEL;
        event.wheel.x = -2;
        event.wheel.y = 3;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed_count; });
        CHECK(processed_count == 1);
    }
}

