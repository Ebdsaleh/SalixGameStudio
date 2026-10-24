#include <doctest.h>
#include <Salix/events/IEvent.h>
#include <memory>
#include <string>
#include <utility>

namespace {
class Batch1847Event : public Salix::IEvent {
public:
    Batch1847Event(int flags, std::string name = "Batch1847Event") : flags_(flags), name_(std::move(name)) {}
    Salix::EventType get_event_type() const override { return Salix::EventType::AppUpdate; }
    const char* get_name() const override { return name_.c_str(); }
    int get_category_flags() const override { return flags_; }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1847Event>(*this); }
private:
    int flags_;
    std::string name_;
};
}

TEST_SUITE("Salix::events::IEventCoverage1847") {
    TEST_CASE("two strongly typed categories combine into the expected integer mask") {
        const int flags = Salix::EventCategory::Application | Salix::EventCategory::Input;
        CHECK(flags == (static_cast<int>(Salix::EventCategory::Application) | static_cast<int>(Salix::EventCategory::Input)));
    }

    TEST_CASE("integer-left category operator supports chaining a third category") {
        const int first = Salix::EventCategory::Input | Salix::EventCategory::Keyboard;
        const int flags = first | Salix::EventCategory::Editor;
        CHECK((flags & static_cast<int>(Salix::EventCategory::Editor)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Keyboard)) != 0);
    }

    TEST_CASE("mouse-axis membership is queryable through generic category helper") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::MouseAxis));
        CHECK(event.is_in_category(Salix::EventCategory::MouseAxis));
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("editor and input categories can coexist without implying keyboard input") {
        const int flags = Salix::EventCategory::Input | Salix::EventCategory::Editor;
        Batch1847Event event(flags);
        CHECK(event.is_input_event());
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_keyboard_event());
    }

    TEST_CASE("mouse category alone does not imply mouse-button category") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Mouse));
        CHECK(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("combined concrete categories report each requested membership") {
        const int flags = Salix::EventCategory::Application | Salix::EventCategory::Input |
            Salix::EventCategory::Keyboard | Salix::EventCategory::Mouse | Salix::EventCategory::MouseButton;
        Batch1847Event event(flags);
        CHECK(event.is_application_event()); CHECK(event.is_input_event()); CHECK(event.is_keyboard_event());
        CHECK(event.is_mouse_event()); CHECK(event.is_mouse_button_event());
    }

    TEST_CASE("base blocking setter remains an intentional no-op") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("base native handle remains null across unrelated state mutations") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        event.handled = true;
        event.set_block(true);
        CHECK(event.get_native_handle() == nullptr);
    }

    TEST_CASE("clone preserves name category membership and handled flag") {
        Batch1847Event event(Salix::EventCategory::Input | Salix::EventCategory::Editor, "CloneProbe");
        event.handled = true;
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        CHECK(std::string(clone->get_name()) == "CloneProbe");
        CHECK(clone->is_input_event());
        CHECK(clone->is_in_category(Salix::EventCategory::Editor));
        CHECK(clone->handled);
    }

    TEST_CASE("cloned handled state is independent from the source after cloning") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        clone->handled = true;
        CHECK_FALSE(event.handled);
        CHECK(clone->handled);
    }
}
