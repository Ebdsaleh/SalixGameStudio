#include <doctest.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/events/EventManager.h>
#include <vector>
#include <Salix/events/sdl/SDLEventPoller.h>
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

TEST_SUITE("Salix::events::SDLEventBatch6547") {
    TEST_CASE("batch6547 key pressed copy preserves key repeat and native handle") {
        SDL_Event native{};
        native.type = SDL_KEYDOWN;
        Salix::KeyPressedEvent source(6547, true, native);
        Salix::KeyPressedEvent copy(source);
        CHECK(copy.get_key_code() == 6547);
        CHECK(copy.was_repeated());
        CHECK(copy.get_native_handle() == &native);
    }

    TEST_CASE("batch6547 key released copy preserves key and native handle") {
        SDL_Event native{};
        native.type = SDL_KEYUP;
        Salix::KeyReleasedEvent source(-6547, native);
        Salix::KeyReleasedEvent copy(source);
        CHECK(copy.get_key_code() == -6547);
        CHECK(copy.get_native_handle() == &native);
    }

    TEST_CASE("batch6547 mouse moved copy preserves signed coordinates") {
        SDL_Event native{};
        Salix::MouseMovedEvent source(-65.47f, 47.65f, native);
        Salix::MouseMovedEvent copy(source);
        CHECK(copy.get_x() == doctest::Approx(-65.47f));
        CHECK(copy.get_y() == doctest::Approx(47.65f));
    }

    TEST_CASE("batch6547 mouse scrolled copy preserves signed offsets") {
        SDL_Event native{};
        Salix::MouseScrolledEvent source(-6.5f, 4.7f, native);
        Salix::MouseScrolledEvent copy(source);
        CHECK(copy.get_x_offset() == doctest::Approx(-6.5f));
        CHECK(copy.get_y_offset() == doctest::Approx(4.7f));
    }

    TEST_CASE("batch6547 mouse button pressed supports zero button identifier") {
        SDL_Event native{};
        Salix::MouseButtonPressedEvent event(0, native);
        CHECK(event.get_mouse_button() == 0);
        CHECK(event.to_string() == "MouseButtonPressedEvent: 0");
    }

    TEST_CASE("batch6547 mouse button released supports negative edge identifier") {
        SDL_Event native{};
        Salix::MouseButtonReleasedEvent event(-1, native);
        CHECK(event.get_mouse_button() == -1);
        CHECK(event.to_string() == "MouseButtonReleasedEvent: -1");
    }

    TEST_CASE("batch6547 zero sized window resize event remains representable") {
        Salix::WindowResizeEvent event(0u, 0u);
        CHECK(event.get_width() == 0u);
        CHECK(event.get_height() == 0u);
        CHECK(event.to_string() == "WindowResizeEvent: 0, 0");
    }

    TEST_CASE("batch6547 window close clone preserves application metadata") {
        Salix::WindowCloseEvent source;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_event_type() == Salix::EventType::WindowClose);
        CHECK(clone->is_application_event());
        CHECK(std::string(clone->get_name()) == "WindowClose");
    }

    TEST_CASE("batch6547 SDL wrapper observes later mutation of shared native event") {
        SDL_Event native{};
        native.type = SDL_KEYDOWN;
        Salix::KeyPressedEvent event(1, false, native);
        native.type = SDL_KEYUP;
        auto* handle = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(handle != nullptr);
        CHECK(handle->type == SDL_KEYUP);
    }

    TEST_CASE("batch6547 cloned mouse event shares original native event identity") {
        SDL_Event native{};
        native.type = SDL_MOUSEMOTION;
        Salix::MouseMovedEvent source(6.0f, 547.0f, native);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_native_handle() == &native);
        CHECK(clone->to_string() == source.to_string());
    }
}

namespace {
struct Batch7578SDLEventQueue {
    Batch7578SDLEventQueue() { SDL_InitSubSystem(SDL_INIT_EVENTS); SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); }
    ~Batch7578SDLEventQueue() { SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); }
};
}

TEST_SUITE("Salix::events::SDLEventBatch7578") {
    TEST_CASE("batch7578 polled key press exercises string category clone and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_KEYDOWN; e.key.keysym.sym=SDLK_F7; e.key.repeat=1; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.to_string().find("KeyPressedEvent")!=std::string::npos); CHECK(event.is_keyboard_event()); CHECK(event.is_input_event()); CHECK(event.get_native_handle()!=nullptr); auto copy=event.clone(); REQUIRE(copy!=nullptr); CHECK(copy->get_event_type()==event.get_event_type()); });
    }
    TEST_CASE("batch7578 polled key release exercises string clone and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_KEYUP; e.key.keysym.sym=SDLK_F8; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.to_string().find("KeyReleasedEvent")!=std::string::npos); CHECK(event.get_native_handle()!=nullptr); CHECK(event.clone()!=nullptr); });
    }
    TEST_CASE("batch7578 polled mouse move exercises string axis category and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.x=75; e.motion.y=-78; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.to_string().find("MouseMovedEvent")!=std::string::npos); CHECK(event.is_mouse_event()); CHECK((event.get_category_flags()&static_cast<int>(Salix::EventCategory::MouseAxis))!=0); CHECK(event.get_native_handle()!=nullptr); });
    }
    TEST_CASE("batch7578 polled mouse wheel exercises offsets string clone and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_MOUSEWHEEL; e.wheel.x=7; e.wheel.y=-5; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ auto* wheel=dynamic_cast<Salix::MouseScrolledEvent*>(&event); REQUIRE(wheel!=nullptr); CHECK(wheel->get_x_offset()==doctest::Approx(7.0f)); CHECK(event.to_string().find("MouseScrolledEvent")!=std::string::npos); CHECK(event.clone()!=nullptr); CHECK(event.get_native_handle()!=nullptr); });
    }
    TEST_CASE("batch7578 polled mouse press exercises string button category and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_MOUSEBUTTONDOWN; e.button.button=SDL_BUTTON_LEFT; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.to_string()=="MouseButtonPressedEvent: 1"); CHECK(event.is_mouse_button_event()); CHECK(event.clone()!=nullptr); CHECK(event.get_native_handle()!=nullptr); });
    }
    TEST_CASE("batch7578 polled mouse release exercises string button category and native handle") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_MOUSEBUTTONUP; e.button.button=SDL_BUTTON_RIGHT; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.to_string().find("MouseButtonReleasedEvent")!=std::string::npos); CHECK(event.is_mouse_button_event()); CHECK(event.clone()!=nullptr); CHECK(event.get_native_handle()!=nullptr); });
    }
    TEST_CASE("batch7578 polled resize exercises dimensions string category and clone") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_WINDOWEVENT; e.window.event=SDL_WINDOWEVENT_RESIZED; e.window.data1=757; e.window.data2=578; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ auto* resize=dynamic_cast<Salix::WindowResizeEvent*>(&event); REQUIRE(resize!=nullptr); CHECK(resize->get_width()==757u); CHECK(resize->get_height()==578u); CHECK(event.to_string().find("WindowResizeEvent")!=std::string::npos); CHECK(event.clone()!=nullptr); CHECK(event.is_application_event()); });
    }
    TEST_CASE("batch7578 polled quit exercises close type category and clone") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_QUIT; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ CHECK(event.get_event_type()==Salix::EventType::WindowClose); CHECK(event.is_application_event()); CHECK(event.clone()!=nullptr); });
    }
    TEST_CASE("batch7578 mixed keyboard and mouse queue preserves category operators") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event key{}; key.type=SDL_KEYDOWN; key.key.keysym.sym=SDLK_a; SDL_Event mouse{}; mouse.type=SDL_MOUSEMOTION; REQUIRE(SDL_PushEvent(&key)==1); REQUIRE(SDL_PushEvent(&mouse)==1);
        int calls=0; poller.poll_events([&](Salix::IEvent& event){ ++calls; CHECK(event.is_input_event()); CHECK(event.get_category_flags()!=0); }); CHECK(calls==2);
    }
    TEST_CASE("batch7578 clone inside callback preserves concrete event name") {
        Batch7578SDLEventQueue queue; Salix::SDLEventPoller poller; SDL_Event e{}; e.type=SDL_KEYDOWN; e.key.keysym.sym=SDLK_RETURN; REQUIRE(SDL_PushEvent(&e)==1);
        poller.poll_events([&](Salix::IEvent& event){ auto copy=event.clone(); REQUIRE(copy!=nullptr); CHECK(std::string(copy->get_name())==std::string(event.get_name())); CHECK(copy->get_category_flags()==event.get_category_flags()); });
    }
}

namespace {
void batch7678_dispatch_polled_event_through_dll(
    SDL_Event event,
    Salix::EventCategory category,
    Salix::OpenGLRenderer& renderer,
    Salix::EventManager& manager) {
    Salix::SDLEventPoller poller;
    manager.subscribe(category, &renderer);
    REQUIRE(SDL_PushEvent(&event) == 1);
    poller.poll_events([&](Salix::IEvent& translated) {
        manager.dispatch(translated);
    });
    manager.process_queue();
    CHECK(manager.is_queue_empty());
}
}

TEST_SUITE("Salix::events::SDLEventBatch7678") {
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse moved virtuals execute through DLL listener") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.x=76; e.motion.y=78;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Mouse, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse scrolled virtuals execute through DLL listener") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEWHEEL; e.wheel.x=7; e.wheel.y=8;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Mouse, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse pressed virtuals execute through DLL listener") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEBUTTONDOWN; e.button.button=SDL_BUTTON_LEFT;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Mouse, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse released virtuals execute through DLL listener") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEBUTTONUP; e.button.button=SDL_BUTTON_RIGHT;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Mouse, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 zero width resize virtuals execute without GL viewport") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_WINDOWEVENT; e.window.event=SDL_WINDOWEVENT_RESIZED; e.window.data1=0; e.window.data2=78;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Application, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 zero height resize virtuals execute without GL viewport") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_WINDOWEVENT; e.window.event=SDL_WINDOWEVENT_SIZE_CHANGED; e.window.data1=76; e.window.data2=0;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Application, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 negative resize converts to unsigned and avoids viewport through zero height") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_WINDOWEVENT; e.window.event=SDL_WINDOWEVENT_RESIZED; e.window.data1=-1; e.window.data2=0;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Application, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse moved clone path supports negative coordinates") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.x=-76; e.motion.y=-78;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Input, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mouse wheel clone path supports negative offsets") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager;
        SDL_Event e{}; e.type=SDL_MOUSEWHEEL; e.wheel.x=-7; e.wheel.y=-8;
        batch7678_dispatch_polled_event_through_dll(e, Salix::EventCategory::Input, renderer, manager);
    }
    TEST_CASE_FIXTURE(Batch7578SDLEventQueue, "batch7678 mixed mouse category dispatch remains queue clean") {
        Salix::OpenGLRenderer renderer; Salix::EventManager manager; Salix::SDLEventPoller poller;
        manager.subscribe(Salix::EventCategory::Mouse, &renderer);
        SDL_Event a{}; a.type=SDL_MOUSEBUTTONDOWN; a.button.button=SDL_BUTTON_MIDDLE;
        SDL_Event b{}; b.type=SDL_MOUSEMOTION; b.motion.x=1; b.motion.y=2;
        REQUIRE(SDL_PushEvent(&a)==1); REQUIRE(SDL_PushEvent(&b)==1);
        poller.poll_events([&](Salix::IEvent& event){ manager.dispatch(event); });
        manager.process_queue();
        CHECK(manager.is_queue_empty());
    }
}

