#include <doctest.h>
#include <Salix/events/IEvent.h>
#include <memory>
#include <string>

namespace {
class Batch1347Event : public Salix::IEvent {
public:
    Batch1347Event(
        Salix::EventType type = Salix::EventType::AppTick,
        std::string name = "Batch1347Event",
        int categories = static_cast<int>(Salix::EventCategory::Application))
        : type_(type), name_(std::move(name)), categories_(categories) {}

    Salix::EventType get_event_type() const override { return type_; }
    const char* get_name() const override { return name_.c_str(); }
    int get_category_flags() const override { return categories_; }
    void* get_native_handle() override { return native_handle_; }
    bool should_block() const override { return block_; }
    void set_block(bool block) override { block_ = block; }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1347Event>(*this); }

    void set_native_handle(void* handle) { native_handle_ = handle; }

private:
    Salix::EventType type_;
    std::string name_;
    int categories_;
    void* native_handle_ = nullptr;
    bool block_ = false;
};

class Batch1347DefaultEvent : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::AppUpdate; }
    const char* get_name() const override { return "DefaultEvent"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1347DefaultEvent>(*this); }
};
}

TEST_SUITE("Salix::events::IEventBatch1347") {
    TEST_CASE("event default to_string delegates to event name") {
        Batch1347DefaultEvent event;
        CHECK(event.to_string() == "DefaultEvent");
    }

    TEST_CASE("event category bitwise helpers combine strongly typed flags") {
        const int flags = Salix::EventCategory::Input | Salix::EventCategory::Keyboard | Salix::EventCategory::Editor;
        CHECK((flags & static_cast<int>(Salix::EventCategory::Input)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Keyboard)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Editor)) != 0);
    }

    TEST_CASE("event application category helper reports configured membership") {
        Batch1347Event event;
        CHECK(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("event input and keyboard helpers can both be true") {
        const int categories = Salix::EventCategory::Input | Salix::EventCategory::Keyboard;
        Batch1347Event event(Salix::EventType::KeyPressed, "KeyPressed", categories);
        CHECK(event.is_input_event());
        CHECK(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("event mouse and mouse button helpers distinguish pointer categories") {
        const int categories = Salix::EventCategory::Input | Salix::EventCategory::Mouse | Salix::EventCategory::MouseButton;
        Batch1347Event event(Salix::EventType::MouseButtonPressed, "MouseButtonPressed", categories);
        CHECK(event.is_input_event());
        CHECK(event.is_mouse_event());
        CHECK(event.is_mouse_button_event());
        CHECK_FALSE(event.is_keyboard_event());
    }

    TEST_CASE("event none category is not reported as any concrete category") {
        Batch1347Event event(Salix::EventType::None, "None", static_cast<int>(Salix::EventCategory::None));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("event interface default native handle and blocking behavior are neutral") {
        Batch1347DefaultEvent event;
        CHECK(event.get_native_handle() == nullptr);
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event implementation can expose native handle and mutable blocking state") {
        Batch1347Event event;
        int native = 42;
        event.set_native_handle(&native);
        event.set_block(true);
        CHECK(event.get_native_handle() == &native);
        CHECK(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event handled flag is independent of category and block state") {
        Batch1347Event event;
        CHECK_FALSE(event.handled);
        event.handled = true;
        CHECK(event.handled);
        CHECK(event.is_application_event());
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event clone preserves dynamic contract and handled state") {
        Batch1347Event event(Salix::EventType::WindowClose, "CloseProbe",
            static_cast<int>(Salix::EventCategory::Application));
        event.handled = true;
        event.set_block(true);
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_event_type() == Salix::EventType::WindowClose);
        CHECK(std::string(clone->get_name()) == "CloseProbe");
        CHECK(clone->handled);
        CHECK(clone->should_block());
    }
}
