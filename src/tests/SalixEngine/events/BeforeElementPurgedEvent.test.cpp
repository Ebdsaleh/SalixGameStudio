#include <doctest.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/events/IEvent.h>
#include <Salix/core/SimpleGuid.h>
#include <memory>
#include <string>

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
