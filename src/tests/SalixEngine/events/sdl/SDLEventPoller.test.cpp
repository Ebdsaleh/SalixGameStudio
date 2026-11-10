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

TEST_SUITE("Salix::events::SDLEventPollerBatch4147") {
    TEST_CASE("batch4147 callback handles begin at one and remain monotonic") {
        Salix::SDLEventPoller poller;
        CHECK(poller.register_raw_event_callback([](void*) {}) == 1);
        CHECK(poller.register_raw_event_callback([](void*) {}) == 2);
        CHECK(poller.register_raw_event_callback([](void*) {}) == 3);
    }

    TEST_CASE("batch4147 unregistering unknown handle before registration is harmless") {
        Salix::SDLEventPoller poller;
        CHECK_NOTHROW(poller.unregister_raw_event_callback(9999));
        CHECK(poller.register_raw_event_callback([](void*) {}) == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 user event reaches raw callback only") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void* native) {
            REQUIRE(native != nullptr);
            CHECK(static_cast<SDL_Event*>(native)->type == SDL_USEREVENT);
            ++raw;
        });
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 1);
        CHECK(processed == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 unregistering second callback leaves first active") {
        Salix::SDLEventPoller poller;
        int first_calls = 0, second_calls = 0;
        poller.register_raw_event_callback([&](void*) { ++first_calls; });
        const auto second = poller.register_raw_event_callback([&](void*) { ++second_calls; });
        poller.unregister_raw_event_callback(second);
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(first_calls == 1);
        CHECK(second_calls == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 quit event reaches raw and processed layers once") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        SDL_Event event{}; event.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 1);
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 size changed window event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed = 0;
        SDL_Event event{};
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
        event.window.data1 = 4147;
        event.window.data2 = 714;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 key up event dispatches one Salix event") {
        Salix::SDLEventPoller poller;
        int processed = 0;
        SDL_Event event{};
        event.type = SDL_KEYUP;
        event.key.keysym.sym = SDLK_z;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 mouse button down and up each dispatch") {
        Salix::SDLEventPoller poller;
        int processed = 0;
        SDL_Event down{}; down.type = SDL_MOUSEBUTTONDOWN; down.button.button = SDL_BUTTON_LEFT;
        SDL_Event up{}; up.type = SDL_MOUSEBUTTONUP; up.button.button = SDL_BUTTON_LEFT;
        REQUIRE(SDL_PushEvent(&down) == 1);
        REQUIRE(SDL_PushEvent(&up) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(processed == 2);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 text input remains raw while following wheel dispatches") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        SDL_Event text{}; text.type = SDL_TEXTINPUT; text.text.text[0] = 'Q'; text.text.text[1] = '\0';
        SDL_Event wheel{}; wheel.type = SDL_MOUSEWHEEL; wheel.wheel.x = 1; wheel.wheel.y = -1;
        REQUIRE(SDL_PushEvent(&text) == 1);
        REQUIRE(SDL_PushEvent(&wheel) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 2);
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4147 empty raw callback registration remains safe for native event") {
        Salix::SDLEventPoller poller;
        Salix::RawEventCallback empty;
        CHECK(poller.register_raw_event_callback(empty) == 1);
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        CHECK_NOTHROW(poller.poll_events([](Salix::IEvent&) {}));
    }
}

TEST_SUITE("Salix::events::SDLEventPollerBatch4547") {
    TEST_CASE("batch4547 unregistered callback handles are never reused") {
        Salix::SDLEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        poller.unregister_raw_event_callback(first);
        const auto third = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(second == 2);
        CHECK(third == 3);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 polling an empty SDL queue invokes neither callback layer") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 0);
        CHECK(processed == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 two user events reach raw layer twice and processed layer never") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        SDL_Event first{}; first.type = SDL_USEREVENT;
        SDL_Event second{}; second.type = SDL_USEREVENT + 1;
        REQUIRE(SDL_PushEvent(&first) == 1);
        REQUIRE(SDL_PushEvent(&second) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 2);
        CHECK(processed == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 unhandled window event remains raw only") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        SDL_Event event{};
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 1);
        CHECK(processed == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 repeated key down event still dispatches exactly one Salix event") {
        Salix::SDLEventPoller poller;
        int processed = 0;
        SDL_Event event{};
        event.type = SDL_KEYDOWN;
        event.key.keysym.sym = SDLK_x;
        event.key.repeat = 1;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 three raw callbacks run once for one quit event") {
        Salix::SDLEventPoller poller;
        int a = 0, b = 0, c = 0;
        poller.register_raw_event_callback([&](void*) { ++a; });
        poller.register_raw_event_callback([&](void*) { ++b; });
        poller.register_raw_event_callback([&](void*) { ++c; });
        SDL_Event event{}; event.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(a == 1); CHECK(b == 1); CHECK(c == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 unregistering every raw callback leaves native event unobserved") {
        Salix::SDLEventPoller poller;
        int calls = 0;
        const auto a = poller.register_raw_event_callback([&](void*) { ++calls; });
        const auto b = poller.register_raw_event_callback([&](void*) { ++calls; });
        poller.unregister_raw_event_callback(a);
        poller.unregister_raw_event_callback(b);
        SDL_Event event{}; event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(calls == 0);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 motion button and wheel events each dispatch once") {
        Salix::SDLEventPoller poller;
        int processed = 0;
        SDL_Event motion{}; motion.type = SDL_MOUSEMOTION; motion.motion.x = -45; motion.motion.y = 47;
        SDL_Event button{}; button.type = SDL_MOUSEBUTTONDOWN; button.button.button = SDL_BUTTON_RIGHT;
        SDL_Event wheel{}; wheel.type = SDL_MOUSEWHEEL; wheel.wheel.x = 0; wheel.wheel.y = -4;
        REQUIRE(SDL_PushEvent(&motion) == 1);
        REQUIRE(SDL_PushEvent(&button) == 1);
        REQUIRE(SDL_PushEvent(&wheel) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(processed == 3);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 text input then quit reaches raw layer twice but processed layer once") {
        Salix::SDLEventPoller poller;
        int raw = 0, processed = 0;
        poller.register_raw_event_callback([&](void*) { ++raw; });
        SDL_Event text{}; text.type = SDL_TEXTINPUT; text.text.text[0] = 'Z'; text.text.text[1] = '\0';
        SDL_Event quit{}; quit.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&text) == 1);
        REQUIRE(SDL_PushEvent(&quit) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++processed; });
        CHECK(raw == 2);
        CHECK(processed == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4547 raw callback observes queued native event types in order") {
        Salix::SDLEventPoller poller;
        std::vector<Uint32> types;
        poller.register_raw_event_callback([&](void* native) {
            REQUIRE(native != nullptr);
            types.push_back(static_cast<SDL_Event*>(native)->type);
        });
        SDL_Event key{}; key.type = SDL_KEYUP; key.key.keysym.sym = SDLK_q;
        SDL_Event user{}; user.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&key) == 1);
        REQUIRE(SDL_PushEvent(&user) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        REQUIRE(types.size() == 2);
        CHECK(types[0] == SDL_KEYUP);
        CHECK(types[1] == SDL_USEREVENT);
    }
}

#include <Salix/events/sdl/SDLEvent.h>

TEST_SUITE("Salix::events::SDLEventPollerBatch4747") {
    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 quit converts to WindowClose event with application category") {
        Salix::SDLEventPoller poller;
        SDL_Event native{}; native.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&native) == 1);
        int calls = 0;
        poller.poll_events([&](Salix::IEvent& event) {
            ++calls;
            CHECK(event.get_event_type() == Salix::EventType::WindowClose);
            CHECK(event.is_in_category(Salix::EventCategory::Application));
            CHECK(dynamic_cast<Salix::WindowCloseEvent*>(&event) != nullptr);
        });
        CHECK(calls == 1);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 resized window converts exact dimensions") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_WINDOWEVENT;
        native.window.event = SDL_WINDOWEVENT_RESIZED;
        native.window.data1 = 4747;
        native.window.data2 = 7474;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* resize = dynamic_cast<Salix::WindowResizeEvent*>(&event);
            REQUIRE(resize != nullptr);
            CHECK(resize->get_width() == 4747u);
            CHECK(resize->get_height() == 7474u);
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 size changed window converts signed native values through unsigned payload contract") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_WINDOWEVENT;
        native.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
        native.window.data1 = 640;
        native.window.data2 = 480;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* resize = dynamic_cast<Salix::WindowResizeEvent*>(&event);
            REQUIRE(resize != nullptr);
            CHECK(resize->get_width() == 640u);
            CHECK(resize->get_height() == 480u);
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 nonrepeat keydown preserves key code and repeat flag") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_KEYDOWN;
        native.key.keysym.sym = SDLK_f;
        native.key.repeat = 0;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* key = dynamic_cast<Salix::KeyPressedEvent*>(&event);
            REQUIRE(key != nullptr);
            CHECK(key->get_key_code() == SDLK_f);
            CHECK_FALSE(key->was_repeated());
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 repeated keydown preserves repeat flag") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_KEYDOWN;
        native.key.keysym.sym = SDLK_F12;
        native.key.repeat = 1;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* key = dynamic_cast<Salix::KeyPressedEvent*>(&event);
            REQUIRE(key != nullptr);
            CHECK(key->get_key_code() == SDLK_F12);
            CHECK(key->was_repeated());
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 keyup preserves exact key code") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_KEYUP;
        native.key.keysym.sym = SDLK_SEMICOLON;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* key = dynamic_cast<Salix::KeyReleasedEvent*>(&event);
            REQUIRE(key != nullptr);
            CHECK(key->get_key_code() == SDLK_SEMICOLON);
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 mouse motion preserves signed coordinates") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_MOUSEMOTION;
        native.motion.x = -47;
        native.motion.y = 74;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* moved = dynamic_cast<Salix::MouseMovedEvent*>(&event);
            REQUIRE(moved != nullptr);
            CHECK(moved->get_x() == doctest::Approx(-47.0f));
            CHECK(moved->get_y() == doctest::Approx(74.0f));
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 mouse button down and up preserve button identity") {
        Salix::SDLEventPoller poller;
        SDL_Event down{}; down.type = SDL_MOUSEBUTTONDOWN; down.button.button = SDL_BUTTON_MIDDLE;
        SDL_Event up{}; up.type = SDL_MOUSEBUTTONUP; up.button.button = SDL_BUTTON_RIGHT;
        REQUIRE(SDL_PushEvent(&down) == 1);
        REQUIRE(SDL_PushEvent(&up) == 1);
        int calls = 0;
        poller.poll_events([&](Salix::IEvent& event) {
            ++calls;
            if (calls == 1) {
                auto* pressed = dynamic_cast<Salix::MouseButtonPressedEvent*>(&event);
                REQUIRE(pressed != nullptr);
                CHECK(pressed->get_mouse_button() == SDL_BUTTON_MIDDLE);
            } else {
                auto* released = dynamic_cast<Salix::MouseButtonReleasedEvent*>(&event);
                REQUIRE(released != nullptr);
                CHECK(released->get_mouse_button() == SDL_BUTTON_RIGHT);
            }
        });
        CHECK(calls == 2);
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 mouse wheel preserves both axis offsets") {
        Salix::SDLEventPoller poller;
        SDL_Event native{};
        native.type = SDL_MOUSEWHEEL;
        native.wheel.x = -4;
        native.wheel.y = 7;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent& event) {
            auto* wheel = dynamic_cast<Salix::MouseScrolledEvent*>(&event);
            REQUIRE(wheel != nullptr);
            CHECK(wheel->get_x_offset() == doctest::Approx(-4.0f));
            CHECK(wheel->get_y_offset() == doctest::Approx(7.0f));
        });
    }

    TEST_CASE_FIXTURE(Batch3447SDLEventQueueFixture, "batch4747 raw callback layer executes before Salix event dispatcher") {
        Salix::SDLEventPoller poller;
        std::vector<int> order;
        poller.register_raw_event_callback([&](void*) { order.push_back(1); });
        SDL_Event native{}; native.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&native) == 1);
        poller.poll_events([&](Salix::IEvent&) { order.push_back(2); });
        REQUIRE(order.size() == 2);
        CHECK(order[0] == 1);
        CHECK(order[1] == 2);
    }
}
