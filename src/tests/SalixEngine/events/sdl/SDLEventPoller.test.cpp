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

#include <SDL.h>
#include <vector>

namespace {
struct Batch2447SdlEvents {
    Batch2447SdlEvents() {
        SDL_InitSubSystem(SDL_INIT_EVENTS);
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }
    ~Batch2447SdlEvents() {
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        SDL_QuitSubSystem(SDL_INIT_EVENTS);
    }
};
}

TEST_SUITE("Salix::events::SDLEventPollerBatch2447") {
    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 raw callback handles increase monotonically from one") {
        Salix::SDLEventPoller poller;
        const auto first = poller.register_raw_event_callback([](void*) {});
        const auto second = poller.register_raw_event_callback([](void*) {});
        const auto third = poller.register_raw_event_callback([](void*) {});
        CHECK(first == 1);
        CHECK(second == 2);
        CHECK(third == 3);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 unregistering unknown handle preserves registered callback") {
        Salix::SDLEventPoller poller;
        int raw_calls = 0;
        poller.register_raw_event_callback([&](void*) { ++raw_calls; });
        poller.unregister_raw_event_callback(9999);
        SDL_Event event{};
        event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(raw_calls == 1);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 unregistering known handle prevents future raw callback invocation") {
        Salix::SDLEventPoller poller;
        int raw_calls = 0;
        const auto handle = poller.register_raw_event_callback([&](void*) { ++raw_calls; });
        poller.unregister_raw_event_callback(handle);
        SDL_Event event{};
        event.type = SDL_USEREVENT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([](Salix::IEvent&) {});
        CHECK(raw_calls == 0);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 raw callbacks run before processed quit dispatcher") {
        Salix::SDLEventPoller poller;
        int stage = 0;
        poller.register_raw_event_callback([&](void*) {
            CHECK(stage == 0);
            stage = 1;
        });
        SDL_Event event{};
        event.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) {
            CHECK(stage == 1);
            stage = 2;
        });
        CHECK(stage == 2);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 SDL quit produces exactly one Salix event") {
        Salix::SDLEventPoller poller;
        int dispatched = 0;
        SDL_Event event{};
        event.type = SDL_QUIT;
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(dispatched == 1);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 both SDL window resize variants dispatch Salix events") {
        Salix::SDLEventPoller poller;
        int dispatched = 0;
        SDL_Event resized{};
        resized.type = SDL_WINDOWEVENT;
        resized.window.event = SDL_WINDOWEVENT_RESIZED;
        resized.window.data1 = 640;
        resized.window.data2 = 480;
        SDL_Event changed = resized;
        changed.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
        REQUIRE(SDL_PushEvent(&resized) == 1);
        REQUIRE(SDL_PushEvent(&changed) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(dispatched == 2);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 key down and key up each dispatch one Salix event") {
        Salix::SDLEventPoller poller;
        int dispatched = 0;
        SDL_Event down{};
        down.type = SDL_KEYDOWN;
        down.key.keysym.sym = SDLK_a;
        down.key.repeat = 0;
        SDL_Event up{};
        up.type = SDL_KEYUP;
        up.key.keysym.sym = SDLK_a;
        REQUIRE(SDL_PushEvent(&down) == 1);
        REQUIRE(SDL_PushEvent(&up) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(dispatched == 2);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 text input reaches raw callback without creating Salix event") {
        Salix::SDLEventPoller poller;
        int raw_calls = 0;
        int dispatched = 0;
        poller.register_raw_event_callback([&](void*) { ++raw_calls; });
        SDL_Event event{};
        event.type = SDL_TEXTINPUT;
        event.text.text[0] = 'A';
        event.text.text[1] = '\0';
        REQUIRE(SDL_PushEvent(&event) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(raw_calls == 1);
        CHECK(dispatched == 0);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 mouse motion and button transitions dispatch independently") {
        Salix::SDLEventPoller poller;
        int dispatched = 0;
        SDL_Event motion{};
        motion.type = SDL_MOUSEMOTION;
        motion.motion.x = 123;
        motion.motion.y = -45;
        SDL_Event down{};
        down.type = SDL_MOUSEBUTTONDOWN;
        down.button.button = SDL_BUTTON_LEFT;
        SDL_Event up{};
        up.type = SDL_MOUSEBUTTONUP;
        up.button.button = SDL_BUTTON_LEFT;
        REQUIRE(SDL_PushEvent(&motion) == 1);
        REQUIRE(SDL_PushEvent(&down) == 1);
        REQUIRE(SDL_PushEvent(&up) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(dispatched == 3);
    }

    TEST_CASE_FIXTURE(Batch2447SdlEvents, "batch2447 mouse wheel event dispatches exactly once") {
        Salix::SDLEventPoller poller;
        int dispatched = 0;
        SDL_Event wheel{};
        wheel.type = SDL_MOUSEWHEEL;
        wheel.wheel.x = -2;
        wheel.wheel.y = 3;
        REQUIRE(SDL_PushEvent(&wheel) == 1);
        poller.poll_events([&](Salix::IEvent&) { ++dispatched; });
        CHECK(dispatched == 1);
    }
}

