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

// ---- Relocated from src/tests/SalixEngine/events/IEventPoller.test.cpp; canonical tests mirror production .cpp files ----
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


// ---- Relocated from src/tests/SalixEngine/events/SDLEvent.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <SDL.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::events::SDLEventBatch1547") {
    TEST_CASE("key pressed event exposes key code repeat state type name and categories") {
        SDL_Event native{};
        native.type = SDL_KEYDOWN;
        Salix::KeyPressedEvent event(42, true, native);
        CHECK(event.get_key_code() == 42);
        CHECK(event.was_repeated());
        CHECK(event.get_event_type() == Salix::EventType::KeyPressed);
        CHECK(std::string(event.get_name()) == "KeyPressed");
        CHECK(event.is_keyboard_event());
        CHECK(event.is_input_event());
    }

    TEST_CASE("key pressed event string describes key and repeat flag") {
        SDL_Event native{};
        Salix::KeyPressedEvent event(7, false, native);
        CHECK(event.to_string() == "KeyPressedEvent: 7 (repeat = 0)");
    }

    TEST_CASE("key released event exposes key identity and native event address") {
        SDL_Event native{};
        native.type = SDL_KEYUP;
        Salix::KeyReleasedEvent event(99, native);
        CHECK(event.get_key_code() == 99);
        CHECK(event.get_event_type() == Salix::EventType::KeyReleased);
        CHECK(std::string(event.get_name()) == "KeyReleased");
        CHECK(event.get_native_handle() == &native);
        CHECK(event.to_string() == "KeyReleasedEvent: 99");
    }

    TEST_CASE("mouse moved event preserves coordinates categories and text") {
        SDL_Event native{};
        Salix::MouseMovedEvent event(12.5f, -3.25f, native);
        CHECK(event.get_x() == doctest::Approx(12.5f));
        CHECK(event.get_y() == doctest::Approx(-3.25f));
        CHECK(event.get_event_type() == Salix::EventType::MouseMoved);
        CHECK(event.is_mouse_event());
        CHECK(event.is_input_event());
        CHECK(event.to_string().find("12.5") != std::string::npos);
    }

    TEST_CASE("mouse scrolled event preserves both offsets and mouse axis category") {
        SDL_Event native{};
        Salix::MouseScrolledEvent event(-1.0f, 2.0f, native);
        CHECK(event.get_x_offset() == doctest::Approx(-1.0f));
        CHECK(event.get_y_offset() == doctest::Approx(2.0f));
        CHECK(event.get_event_type() == Salix::EventType::MouseScrolled);
        CHECK(event.is_mouse_event());
        CHECK(event.is_input_event());
        CHECK((event.get_category_flags() & static_cast<int>(Salix::EventCategory::MouseAxis)) != 0);
    }

    TEST_CASE("mouse button pressed event exposes button identity and button category") {
        SDL_Event native{};
        Salix::MouseButtonPressedEvent event(1, native);
        CHECK(event.get_mouse_button() == 1);
        CHECK(event.get_event_type() == Salix::EventType::MouseButtonPressed);
        CHECK(event.is_mouse_event());
        CHECK(event.is_mouse_button_event());
        CHECK(event.to_string() == "MouseButtonPressedEvent: 1");
    }

    TEST_CASE("mouse button released event preserves button and native pointer") {
        SDL_Event native{};
        Salix::MouseButtonReleasedEvent event(3, native);
        CHECK(event.get_mouse_button() == 3);
        CHECK(event.get_event_type() == Salix::EventType::MouseButtonReleased);
        CHECK(event.get_native_handle() == &native);
        CHECK(event.to_string() == "MouseButtonReleasedEvent: 3");
    }

    TEST_CASE("window resize event exposes dimensions application category and text") {
        Salix::WindowResizeEvent event(1920, 1080);
        CHECK(event.get_width() == 1920u);
        CHECK(event.get_height() == 1080u);
        CHECK(event.get_event_type() == Salix::EventType::WindowResize);
        CHECK(event.is_application_event());
        CHECK(event.to_string() == "WindowResizeEvent: 1920, 1080");
    }

    TEST_CASE("window close event exposes application contract without native SDL handle") {
        Salix::WindowCloseEvent event;
        CHECK(event.get_event_type() == Salix::EventType::WindowClose);
        CHECK(std::string(event.get_name()) == "WindowClose");
        CHECK(event.is_application_event());
        CHECK(event.get_native_handle() == nullptr);
    }

    TEST_CASE("SDL event clone preserves concrete event state and shared native source identity") {
        SDL_Event native{};
        native.type = SDL_MOUSEMOTION;
        Salix::MouseMovedEvent source(4.0f, 8.0f, native);
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_event_type() == Salix::EventType::MouseMoved);
        CHECK(clone->handled);
        CHECK(clone->get_native_handle() == &native);
        CHECK(clone->to_string() == source.to_string());
    }
}


// ---- Relocated from src/tests/SalixEngine/events/SDLEventBase.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/events/sdl/SDLEventBase.h>
#include <Salix/events/IEvent.h>
#include <SDL.h>
#include <memory>
#include <string>

namespace {
class Batch1047SDLEventProbe : public Salix::SDLEventBase {
public:
    explicit Batch1047SDLEventProbe(const SDL_Event& event)
        : Salix::SDLEventBase(event) {}

    Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
    const char* get_name() const override { return "Batch1047SDLEventProbe"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override {
        return std::make_unique<Batch1047SDLEventProbe>(*this);
    }
};
}

TEST_SUITE("Salix::events::SDLEventBaseBatch1047") {
    TEST_CASE("native handle points at the exact SDL event object supplied to constructor") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        Batch1047SDLEventProbe event(raw);
        CHECK(event.get_native_handle() == &raw);
    }

    TEST_CASE("native handle exposes original event type without copying SDL event storage") {
        SDL_Event raw{};
        raw.type = SDL_KEYDOWN;
        Batch1047SDLEventProbe event(raw);
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        CHECK(native->type == SDL_KEYDOWN);
    }

    TEST_CASE("wrapper observes mutations made to the original SDL event after construction") {
        SDL_Event raw{};
        raw.type = SDL_KEYDOWN;
        Batch1047SDLEventProbe event(raw);
        raw.type = SDL_KEYUP;
        raw.key.keysym.sym = SDLK_ESCAPE;
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        CHECK(native->type == SDL_KEYUP);
        CHECK(native->key.keysym.sym == SDLK_ESCAPE);
    }

    TEST_CASE("mutable native handle aliases original SDL event storage") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        raw.user.code = 1;
        Batch1047SDLEventProbe event(raw);
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        native->user.code = 77;
        CHECK(raw.user.code == 77);
    }

    TEST_CASE("wrappers around different SDL events retain distinct native identities") {
        SDL_Event first{};
        SDL_Event second{};
        first.type = SDL_MOUSEMOTION;
        second.type = SDL_MOUSEWHEEL;
        Batch1047SDLEventProbe first_wrapper(first);
        Batch1047SDLEventProbe second_wrapper(second);
        CHECK(first_wrapper.get_native_handle() == &first);
        CHECK(second_wrapper.get_native_handle() == &second);
        CHECK(first_wrapper.get_native_handle() != second_wrapper.get_native_handle());
    }

    TEST_CASE("multiple wrappers around one SDL event expose the same native handle") {
        SDL_Event raw{};
        raw.type = SDL_QUIT;
        Batch1047SDLEventProbe first(raw);
        Batch1047SDLEventProbe second(raw);
        CHECK(first.get_native_handle() == second.get_native_handle());
        CHECK(first.get_native_handle() == &raw);
    }

    TEST_CASE("native handle dispatches correctly through IEvent interface") {
        SDL_Event raw{};
        raw.type = SDL_WINDOWEVENT;
        Batch1047SDLEventProbe concrete(raw);
        Salix::IEvent* base = &concrete;
        CHECK(base->get_native_handle() == &raw);
        CHECK(base->get_event_type() == Salix::EventType::AppTick);
        CHECK(std::string(base->get_name()) == "Batch1047SDLEventProbe");
    }

    TEST_CASE("SDL event base inherits default unhandled and nonblocking event state") {
        SDL_Event raw{};
        Batch1047SDLEventProbe event(raw);
        CHECK_FALSE(event.handled);
        CHECK_FALSE(event.should_block());
        CHECK(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("default set block implementation does not change SDL event blocking behavior") {
        SDL_Event raw{};
        Batch1047SDLEventProbe event(raw);
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("cloned SDL wrapper keeps reference to same native event and copies handled state") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        Batch1047SDLEventProbe source(raw);
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_native_handle() == &raw);
        CHECK(clone->handled);
        raw.type = SDL_QUIT;
        auto* clone_native = static_cast<SDL_Event*>(clone->get_native_handle());
        REQUIRE(clone_native != nullptr);
        CHECK(clone_native->type == SDL_QUIT);
        clone->handled = false;
        CHECK(source.handled);
    }
}
