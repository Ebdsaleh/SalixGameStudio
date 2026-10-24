#include <doctest.h>
#include <Salix/events/IEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/SimpleGuid.h>
#include <memory>
#include <string>
#include <utility>

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

TEST_SUITE("Salix::events::IEventBatch1847") {
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

TEST_SUITE("Salix::events::BeforeElementPurgedEventBatch1147") {
    TEST_CASE("event preserves element and parent identifiers exactly") {
        const auto element = Salix::SimpleGuid::from_value(101);
        const auto parent = Salix::SimpleGuid::from_value(202);
        Salix::BeforeElementPurgedEvent event(element, parent);
        CHECK(event.element_id == element);
        CHECK(event.parent_id == parent);
    }

    TEST_CASE("event accepts invalid element identifier without altering valid parent identity") {
        const auto parent = Salix::SimpleGuid::from_value(22);
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::invalid(), parent);
        CHECK_FALSE(event.element_id.is_valid());
        CHECK(event.parent_id == parent);
    }

    TEST_CASE("event accepts invalid parent identifier without altering valid element identity") {
        const auto element = Salix::SimpleGuid::from_value(33);
        Salix::BeforeElementPurgedEvent event(element, Salix::SimpleGuid::invalid());
        CHECK(event.element_id == element);
        CHECK_FALSE(event.parent_id.is_valid());
    }

    TEST_CASE("static and virtual event type identities agree") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(Salix::BeforeElementPurgedEvent::get_static_type() == Salix::EventType::BeforeElementPurged);
        CHECK(event.get_event_type() == Salix::BeforeElementPurgedEvent::get_static_type());
    }

    TEST_CASE("event name and default string representation identify BeforeElementPurged") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(std::string(event.get_name()) == "BeforeElementPurged");
        CHECK(event.to_string() == "BeforeElementPurged");
    }

    TEST_CASE("purge notification exposes exactly the editor category") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("clone preserves both identifiers and concrete event type") {
        const auto element = Salix::SimpleGuid::from_value(77);
        const auto parent = Salix::SimpleGuid::from_value(88);
        Salix::BeforeElementPurgedEvent source(element, parent);
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->element_id == element);
        CHECK(concrete->parent_id == parent);
    }

    TEST_CASE("clone copies handled state while remaining independently mutable") {
        Salix::BeforeElementPurgedEvent source({}, {});
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->handled);
        clone->handled = false;
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("editor purge event uses default null native handle") {
        Salix::BeforeElementPurgedEvent event({}, {});
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
    }

    TEST_CASE("editor purge event retains default nonblocking contract") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }
}


TEST_SUITE("Salix::events::BeforeElementPurgedEventBatch2047") {
    TEST_CASE("event payload identifiers can be retargeted independently after construction") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(1), Salix::SimpleGuid::from_value(2));
        event.element_id = Salix::SimpleGuid::from_value(3);
        CHECK(event.element_id == Salix::SimpleGuid::from_value(3));
        CHECK(event.parent_id == Salix::SimpleGuid::from_value(2));
    }
    TEST_CASE("copy construction preserves payload and handled state") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(10), Salix::SimpleGuid::from_value(20));
        source.handled = true;
        Salix::BeforeElementPurgedEvent copy(source);
        CHECK(copy.element_id == source.element_id); CHECK(copy.parent_id == source.parent_id); CHECK(copy.handled);
    }
    TEST_CASE("copy assignment replaces both identifiers and handled state") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(30), Salix::SimpleGuid::from_value(40));
        source.handled = true;
        Salix::BeforeElementPurgedEvent destination(Salix::SimpleGuid::from_value(1), Salix::SimpleGuid::from_value(2));
        destination = source;
        CHECK(destination.element_id == Salix::SimpleGuid::from_value(30));
        CHECK(destination.parent_id == Salix::SimpleGuid::from_value(40)); CHECK(destination.handled);
    }
    TEST_CASE("clone preserves two invalid payload identifiers") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::invalid(), Salix::SimpleGuid::invalid());
        auto clone = source.clone(); REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        CHECK_FALSE(concrete->element_id.is_valid()); CHECK_FALSE(concrete->parent_id.is_valid());
    }
    TEST_CASE("mutating cloned identifiers never changes source payload") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(50), Salix::SimpleGuid::from_value(60));
        auto clone = source.clone(); auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        concrete->element_id = Salix::SimpleGuid::from_value(500); concrete->parent_id = Salix::SimpleGuid::from_value(600);
        CHECK(source.element_id == Salix::SimpleGuid::from_value(50)); CHECK(source.parent_id == Salix::SimpleGuid::from_value(60));
    }
    TEST_CASE("base reference dispatch preserves purge type name and editor category") {
        Salix::BeforeElementPurgedEvent event({}, {}); Salix::IEvent& base = event;
        CHECK(base.get_event_type() == Salix::EventType::BeforeElementPurged);
        CHECK(std::string(base.get_name()) == "BeforeElementPurged");
        CHECK(base.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }
    TEST_CASE("handled state changes do not mutate purge payload identifiers") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(70), Salix::SimpleGuid::from_value(80));
        event.handled = true; event.handled = false;
        CHECK(event.element_id == Salix::SimpleGuid::from_value(70)); CHECK(event.parent_id == Salix::SimpleGuid::from_value(80));
    }
    TEST_CASE("string representation remains stable after payload retargeting") {
        Salix::BeforeElementPurgedEvent event({}, {});
        event.element_id = Salix::SimpleGuid::from_value(90); event.parent_id = Salix::SimpleGuid::from_value(91);
        CHECK(event.to_string() == "BeforeElementPurged");
    }
    TEST_CASE("separate purge events own independent identifier values") {
        Salix::BeforeElementPurgedEvent first(Salix::SimpleGuid::from_value(100), Salix::SimpleGuid::from_value(200));
        Salix::BeforeElementPurgedEvent second = first; second.element_id = Salix::SimpleGuid::from_value(101);
        CHECK(first.element_id == Salix::SimpleGuid::from_value(100)); CHECK(second.element_id == Salix::SimpleGuid::from_value(101));
        CHECK(first.parent_id == second.parent_id);
    }
    TEST_CASE("self assignment preserves purge payload and handled state") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(111), Salix::SimpleGuid::from_value(222));
        event.handled = true; event = event;
        CHECK(event.element_id == Salix::SimpleGuid::from_value(111)); CHECK(event.parent_id == Salix::SimpleGuid::from_value(222)); CHECK(event.handled);
    }
}

TEST_SUITE("Salix::events::BeforeEntityPurgedEventBatch1047") {
    TEST_CASE("event preserves the exact entity pointer supplied at construction") {
        Salix::Entity entity;
        Salix::BeforeEntityPurgedEvent event(&entity);
        CHECK(event.entity == &entity);
    }

    TEST_CASE("event deliberately supports a null entity payload without changing metadata") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.entity == nullptr);
        CHECK(event.get_event_type() == Salix::EventType::BeforeEntityPurged);
        CHECK(std::string(event.get_name()) == "BeforeEntityPurged");
    }

    TEST_CASE("static and virtual event type identities agree") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(Salix::BeforeEntityPurgedEvent::get_static_type() == Salix::EventType::BeforeEntityPurged);
        CHECK(event.get_event_type() == Salix::BeforeEntityPurgedEvent::get_static_type());
    }

    TEST_CASE("default event string representation uses the concrete event name") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(std::string(event.get_name()) == "BeforeEntityPurged");
        CHECK(event.to_string() == "BeforeEntityPurged");
    }

    TEST_CASE("event exposes exactly the editor category flag") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }

    TEST_CASE("category helpers classify purge notification as editor only") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("clone preserves concrete type and entity pointer payload") {
        Salix::Entity entity;
        Salix::BeforeEntityPurgedEvent source(&entity);
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->entity == &entity);
        CHECK(concrete->get_event_type() == Salix::EventType::BeforeEntityPurged);
    }

    TEST_CASE("clone copies handled state but remains independently mutable") {
        Salix::BeforeEntityPurgedEvent source(nullptr);
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->handled);
        clone->handled = false;
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("event inherits null native handle contract for non-native editor events") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
    }

    TEST_CASE("editor purge event uses default nonblocking contract") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }
}


TEST_SUITE("Salix::events::BeforeEntityPurgedEventBatch2047") {
    TEST_CASE("entity payload pointer can be retargeted after construction") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent event(&first); event.entity = &second; CHECK(event.entity == &second);
    }
    TEST_CASE("copy construction preserves entity pointer and handled state") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent source(&entity); source.handled = true;
        Salix::BeforeEntityPurgedEvent copy(source); CHECK(copy.entity == &entity); CHECK(copy.handled);
    }
    TEST_CASE("copy assignment replaces entity pointer and handled state") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent source(&first); source.handled = true;
        Salix::BeforeEntityPurgedEvent destination(&second); destination = source; CHECK(destination.entity == &first); CHECK(destination.handled);
    }
    TEST_CASE("clone preserves an explicit null entity payload") {
        Salix::BeforeEntityPurgedEvent source(nullptr); auto clone = source.clone(); REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr); CHECK(concrete->entity == nullptr);
    }
    TEST_CASE("retargeting cloned entity pointer does not alter source event") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent source(&first);
        auto clone = source.clone(); auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        concrete->entity = &second; CHECK(source.entity == &first); CHECK(concrete->entity == &second);
    }
    TEST_CASE("base reference dispatch preserves entity purge metadata") {
        Salix::BeforeEntityPurgedEvent event(nullptr); Salix::IEvent& base = event;
        CHECK(base.get_event_type() == Salix::EventType::BeforeEntityPurged); CHECK(std::string(base.get_name()) == "BeforeEntityPurged");
        CHECK(base.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }
    TEST_CASE("handled state remains independent from entity pointer payload") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(&entity); event.handled = true; CHECK(event.entity == &entity);
        event.handled = false; CHECK(event.entity == &entity);
    }
    TEST_CASE("two purge events may reference the same entity while tracking handled state separately") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent first(&entity); Salix::BeforeEntityPurgedEvent second(&entity);
        first.handled = true; CHECK(first.entity == second.entity); CHECK(first.handled); CHECK_FALSE(second.handled);
    }
    TEST_CASE("self assignment preserves entity pointer and handled state") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(&entity); event.handled = true; event = event; CHECK(event.entity == &entity); CHECK(event.handled);
    }
    TEST_CASE("string representation remains stable after entity pointer changes") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(nullptr); event.entity = &entity; CHECK(event.to_string() == "BeforeEntityPurged");
        event.entity = nullptr; CHECK(event.to_string() == "BeforeEntityPurged");
    }
}
