#include <doctest.h>
#include <Tests/SalixEngine/mocking/events/MockIEventPoller.h>
#include <cstdlib>
#include <SDL.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <Salix/events/IEventListener.h>
#include <Salix/events/EventManager.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/events/ImGuiInputEvent.h>
#include <Salix/events/IEvent.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::events::ImGuiInputEventBatch1047") {
    TEST_CASE("no-capture event reports neither mouse nor keyboard capture") {
        Salix::ImGuiInputEvent event(false, false);
        CHECK_FALSE(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("mouse-only capture remains independent from keyboard capture") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("keyboard-only capture remains independent from mouse capture") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK_FALSE(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("simultaneous capture reports both channels") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("string representation reports disabled capture channels as zero") {
        Salix::ImGuiInputEvent event(false, false);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 0");
    }

    TEST_CASE("string representation reports enabled capture channels as one") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 1, Keyboard Captured = 1");
    }

    TEST_CASE("static type virtual type and event name remain consistent") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK(Salix::ImGuiInputEvent::get_static_type() == Salix::EventType::ImGuiInput);
        CHECK(event.get_event_type() == Salix::EventType::ImGuiInput);
        CHECK(std::string(event.get_name()) == "ImGuiInput");
    }

    TEST_CASE("ImGui capture event is classified as input but not unrelated categories") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
        CHECK(event.is_input_event());
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("clone preserves capture flags concrete type and handled state") {
        Salix::ImGuiInputEvent source(true, false);
        source.handled = true;
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::ImGuiInputEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->is_mouse_captured());
        CHECK_FALSE(concrete->is_keyboard_captured());
        CHECK(concrete->handled);
        concrete->handled = false;
        CHECK(source.handled);
    }

    TEST_CASE("non-native ImGui capture event retains default blocking and native-handle contracts") {
        Salix::ImGuiInputEvent event(false, true);
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
        CHECK_FALSE(base->should_block());
        base->set_block(true);
        CHECK_FALSE(base->should_block());
        CHECK(base->get_event_type() == Salix::EventType::ImGuiInput);
    }
}

TEST_SUITE("Salix::events::ImGuiInputEventBatch1847") {
    TEST_CASE("mouse-only string form records one and zero capture flags") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 1, Keyboard Captured = 0");
    }

    TEST_CASE("keyboard-only string form records zero and one capture flags") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 1");
    }

    TEST_CASE("fresh ImGui input event starts unhandled independently of capture state") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK_FALSE(event.handled);
        CHECK(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("marking event handled never changes immutable capture flags") {
        Salix::ImGuiInputEvent event(true, false);
        event.handled = true;
        CHECK(event.handled);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("clone of no-capture event remains independently unhandled") {
        Salix::ImGuiInputEvent source(false, false);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        clone->handled = true;
        CHECK_FALSE(source.handled);
        CHECK(clone->handled);
        auto* concrete = dynamic_cast<Salix::ImGuiInputEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK_FALSE(concrete->is_mouse_captured());
        CHECK_FALSE(concrete->is_keyboard_captured());
    }

    TEST_CASE("clone of full-capture event preserves full string representation") {
        Salix::ImGuiInputEvent source(true, true);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->to_string() == source.to_string());
    }

    TEST_CASE("repeated string conversion is stable and side-effect free") {
        Salix::ImGuiInputEvent event(true, false);
        const auto first = event.to_string();
        const auto second = event.to_string();
        CHECK(first == second);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("base interface virtual to_string dispatches to ImGui-specific representation") {
        Salix::ImGuiInputEvent concrete(false, true);
        Salix::IEvent& event = concrete;
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 1");
        CHECK(event.get_event_type() == Salix::EventType::ImGuiInput);
    }

    TEST_CASE("input classification does not leak into editor or mouse-axis categories") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.is_input_event());
        CHECK_FALSE(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_in_category(Salix::EventCategory::MouseAxis));
    }

    TEST_CASE("default blocking contract remains false through base interface and clone") {
        Salix::ImGuiInputEvent source(true, false);
        Salix::IEvent& base = source;
        base.set_block(true);
        CHECK_FALSE(base.should_block());
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK_FALSE(clone->should_block());
    }
}

TEST_SUITE("Salix::events::ImGuiInputEventBatch6547") {
    TEST_CASE("batch6547 all four capture combinations preserve exact input category") {
        const Salix::ImGuiInputEvent a(false, false);
        const Salix::ImGuiInputEvent b(true, false);
        const Salix::ImGuiInputEvent c(false, true);
        const Salix::ImGuiInputEvent d(true, true);
        CHECK(a.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
        CHECK(b.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
        CHECK(c.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
        CHECK(d.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
    }

    TEST_CASE("batch6547 mouse only capture survives copy construction") {
        const Salix::ImGuiInputEvent source(true, false);
        const Salix::ImGuiInputEvent copy(source);
        CHECK(copy.is_mouse_captured());
        CHECK_FALSE(copy.is_keyboard_captured());
    }

    TEST_CASE("batch6547 keyboard only capture survives copy construction") {
        const Salix::ImGuiInputEvent source(false, true);
        const Salix::ImGuiInputEvent copy(source);
        CHECK_FALSE(copy.is_mouse_captured());
        CHECK(copy.is_keyboard_captured());
    }

    TEST_CASE("batch6547 copy assignment replaces capture flags and handled state") {
        Salix::ImGuiInputEvent source(true, true);
        source.handled = true;
        Salix::ImGuiInputEvent destination(false, false);
        destination = source;
        CHECK(destination.is_mouse_captured());
        CHECK(destination.is_keyboard_captured());
        CHECK(destination.handled);
    }

    TEST_CASE("batch6547 self assignment preserves capture flags and handled state") {
        Salix::ImGuiInputEvent event(true, false);
        event.handled = true;
        event = event;
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
        CHECK(event.handled);
    }

    TEST_CASE("batch6547 clone type metadata remains ImGuiInput") {
        Salix::ImGuiInputEvent source(false, true);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_event_type() == Salix::EventType::ImGuiInput);
        CHECK(std::string(clone->get_name()) == "ImGuiInput");
    }

    TEST_CASE("batch6547 clone capture flags are independent from clone handled mutation") {
        Salix::ImGuiInputEvent source(true, true);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        clone->handled = true;
        auto* concrete = dynamic_cast<Salix::ImGuiInputEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->is_mouse_captured());
        CHECK(concrete->is_keyboard_captured());
        CHECK_FALSE(source.handled);
    }

    TEST_CASE("batch6547 base interface classification remains input only") {
        Salix::ImGuiInputEvent concrete(true, true);
        Salix::IEvent& event = concrete;
        CHECK(event.is_input_event());
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("batch6547 native handle remains null for every capture combination") {
        Salix::ImGuiInputEvent none(false, false);
        Salix::ImGuiInputEvent both(true, true);
        CHECK(none.get_native_handle() == nullptr);
        CHECK(both.get_native_handle() == nullptr);
    }

    TEST_CASE("batch6547 handled mutation never changes string representation") {
        Salix::ImGuiInputEvent event(true, false);
        const auto before = event.to_string();
        event.handled = true;
        CHECK(event.to_string() == before);
    }
}

namespace {
struct Batch7678ImGuiEventBackend {
    SDL_Window* raw_window = nullptr;
    SDL_Renderer* raw_renderer = nullptr;
    MockIWindow window;
    MockIRenderer renderer;
    MockIEventPoller poller;
    Salix::ApplicationConfig config;
    Salix::SDLImGui gui;
    Salix::EventManager events;
    bool ready = false;

    Batch7678ImGuiEventBackend() {
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return;
        raw_window = SDL_CreateWindow("Batch7678 ImGui Event", 0, 0, 64, 48, SDL_WINDOW_HIDDEN);
        if (!raw_window) return;
        raw_renderer = SDL_CreateRenderer(raw_window, -1, SDL_RENDERER_SOFTWARE);
        if (!raw_renderer) return;
        window.native_handle = raw_window;
        renderer.native_handle = raw_renderer;
        config.renderer_type = Salix::RendererType::SDL;
        gui.set_app_config(&config);
        ready = gui.initialize(&window, &renderer, nullptr, nullptr, nullptr);
        if (ready) {
            gui.setup_event_polling(&poller, &events);
        }
    }

    ~Batch7678ImGuiEventBackend() {
        if (ready) gui.shutdown();
        if (raw_renderer) SDL_DestroyRenderer(raw_renderer);
        if (raw_window) SDL_DestroyWindow(raw_window);
        SDL_Quit();
        _putenv_s("SDL_VIDEODRIVER", "");
        _putenv_s("SDL_RENDER_DRIVER", "");
    }
};

class Batch7678CaptureInspector final : public Salix::IEventListener {
public:
    void on_event(Salix::IEvent& event) override {
        auto* input = dynamic_cast<Salix::ImGuiInputEvent*>(&event);
        if (!input) return;
        ++count;
        mouse = input->is_mouse_captured();
        keyboard = input->is_keyboard_captured();
        text = input->to_string();
        type = input->get_event_type();
        name = input->get_name();
        flags = input->get_category_flags();
        native_handle = input->get_native_handle();
        blocked = input->should_block();
        clone = input->clone();
    }

    int count = 0;
    bool mouse = false;
    bool keyboard = false;
    std::string text;
    Salix::EventType type = Salix::EventType::None;
    std::string name;
    int flags = 0;
    void* native_handle = reinterpret_cast<void*>(1);
    bool blocked = true;
    std::unique_ptr<Salix::IEvent> clone;
};

void batch7678_emit_imgui_input(
    Batch7678ImGuiEventBackend& fixture,
    Batch7678CaptureInspector& inspector) {
    fixture.events.subscribe(Salix::EventCategory::Input, &inspector);
    fixture.gui.new_frame();
    fixture.gui.render();
    fixture.events.process_queue();
}
}

TEST_SUITE("Salix::events::ImGuiInputEventBatch7678") {
    TEST_CASE("batch7678 DLL generated ImGui event dispatches exactly once") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.count == 1);
    }
    TEST_CASE("batch7678 DLL generated ImGui event reports default mouse capture false") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK_FALSE(inspector.mouse);
    }
    TEST_CASE("batch7678 DLL generated ImGui event reports default keyboard capture false") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK_FALSE(inspector.keyboard);
    }
    TEST_CASE("batch7678 DLL generated ImGui event exposes expected type") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.type == Salix::EventType::ImGuiInput);
    }
    TEST_CASE("batch7678 DLL generated ImGui event exposes expected name") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.name == "ImGuiInput");
    }
    TEST_CASE("batch7678 DLL generated ImGui event exposes input category") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.flags == static_cast<int>(Salix::EventCategory::Input));
    }
    TEST_CASE("batch7678 DLL generated ImGui event string reports default capture state") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.text == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 0");
    }
    TEST_CASE("batch7678 DLL generated ImGui event clone preserves type") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        REQUIRE(inspector.clone != nullptr);
        CHECK(inspector.clone->get_event_type() == Salix::EventType::ImGuiInput);
    }
    TEST_CASE("batch7678 DLL generated ImGui event retains null native handle") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK(inspector.native_handle == nullptr);
    }
    TEST_CASE("batch7678 DLL generated ImGui event retains nonblocking contract") {
        Batch7678ImGuiEventBackend fixture; REQUIRE(fixture.ready); Batch7678CaptureInspector inspector;
        batch7678_emit_imgui_input(fixture, inspector);
        CHECK_FALSE(inspector.blocked);
    }
}
