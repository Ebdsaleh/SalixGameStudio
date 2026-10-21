#include <doctest.h>
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
