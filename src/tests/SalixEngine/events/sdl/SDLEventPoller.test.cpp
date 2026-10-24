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

