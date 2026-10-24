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
