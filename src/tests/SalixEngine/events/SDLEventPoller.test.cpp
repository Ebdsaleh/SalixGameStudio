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
