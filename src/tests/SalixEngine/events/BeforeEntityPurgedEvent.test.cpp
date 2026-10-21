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
