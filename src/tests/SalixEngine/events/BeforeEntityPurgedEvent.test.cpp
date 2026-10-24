#include <doctest.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/events/IEvent.h>
#include <Salix/ecs/Entity.h>
#include <memory>
#include <string>

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
