// Realm.test.cpp - Memory-only regression tests for engine realm ownership.
// No user projects, scene files, or external assets are modified by these tests.
#include <doctest.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Realm.h>

TEST_SUITE("Salix::ecs::Realm") {

    TEST_CASE("new realms have defined defaults and reject empty metadata updates") {
        Salix::Realm realm;
        CHECK(realm.get_name() == "New Realm");
        CHECK(realm.get_path() == "Undefined");
        CHECK(realm.get_entities().empty());
        CHECK(realm.get_active_camera() == nullptr);
        CHECK(realm.get_main_camera_entity_id() == Salix::SimpleGuid::invalid());

        realm.set_name("Gameplay");
        realm.set_path("Scenes/Gameplay.json");
        realm.set_name("");
        realm.set_path("");
        CHECK(realm.get_name() == "Gameplay");
        CHECK(realm.get_path() == "Scenes/Gameplay.json");
    }

    TEST_CASE("realm constructors preserve supplied names and paths") {
        Salix::Realm named("Menu");
        CHECK(named.get_name() == "Menu");
        CHECK(named.get_path() == "Undefined");

        Salix::Realm complete("Battle", "Scenes/Battle.json");
        CHECK(complete.get_name() == "Battle");
        CHECK(complete.get_path() == "Scenes/Battle.json");
    }

    TEST_CASE("entity creation owns distinct entities and supports both lookup methods") {
        Salix::Realm realm("Entities");
        Salix::Entity* first = realm.create_entity("First");
        Salix::Entity* second = realm.create_entity("Second");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(first->get_id().is_valid());
        CHECK(second->get_id().is_valid());
        CHECK(first->get_id() != second->get_id());
        CHECK(realm.get_entity_by_id(first->get_id()) == first);
        CHECK(realm.get_entity_by_name("Second") == second);
        CHECK(realm.get_entity_by_name("Absent") == nullptr);
        CHECK(realm.get_entity_by_id(Salix::SimpleGuid::invalid()) == nullptr);

        const auto entities = realm.get_entities();
        REQUIRE(entities.size() == 2);
        CHECK(entities[0] == first);
        CHECK(entities[1] == second);
    }

    TEST_CASE("explicit entity GUID is retained by realm lookup") {
        Salix::Realm realm;
        const Salix::SimpleGuid chosen_id = Salix::SimpleGuid::generate();
        Salix::Entity* entity = realm.create_entity(chosen_id, "Assigned");
        REQUIRE(entity != nullptr);
        CHECK(entity->get_id() == chosen_id);
        CHECK(realm.get_entity_by_id(chosen_id) == entity);
        CHECK(realm.get_entity_by_name("Assigned") == entity);
    }

    TEST_CASE("purging a parent removes only that entity and preserves descendants") {
        Salix::AssetManager asset_manager;
        Salix::InitContext context{};
        context.asset_manager = &asset_manager;

        Salix::Realm realm;
        // set_context currently requires a non-null asset manager.
        // This also gives maintain() a known-null event_manager.
        realm.set_context(context);
        Salix::Entity* parent = realm.create_entity("Parent");
        Salix::Entity* child = realm.create_entity("Child");
        Salix::Entity* grandchild = realm.create_entity("Grandchild");
        Salix::Entity* unrelated = realm.create_entity("Unrelated");
        REQUIRE(parent != nullptr);
        REQUIRE(child != nullptr);
        REQUIRE(grandchild != nullptr);
        REQUIRE(unrelated != nullptr);
        child->set_parent(parent);
        grandchild->set_parent(child);
        const Salix::SimpleGuid parent_id = parent->get_id();
        const Salix::SimpleGuid child_id = child->get_id();
        const Salix::SimpleGuid grandchild_id = grandchild->get_id();
        const Salix::SimpleGuid unrelated_id = unrelated->get_id();
        REQUIRE(child->get_parent() == parent);
        REQUIRE(grandchild->get_parent() == child);

        parent->purge();
        CHECK(parent->is_purged());
        CHECK(child->get_parent() == nullptr);
        CHECK_FALSE(child->is_purged());
        CHECK(grandchild->get_parent() == child);
        realm.maintain();

        CHECK(realm.get_entity_by_id(parent_id) == nullptr);
        CHECK(realm.get_entity_by_id(child_id) == child);
        CHECK(realm.get_entity_by_id(grandchild_id) == grandchild);
        CHECK(realm.get_entity_by_id(unrelated_id) == unrelated);
        CHECK(child->is_root());
        CHECK(grandchild->get_parent() == child);
        CHECK(unrelated->is_root());
        CHECK(realm.get_entities().size() == 3);
    }

    TEST_CASE("simple purge delays orphaning until realm maintenance") {
        Salix::AssetManager asset_manager;
        Salix::InitContext context{};
        context.asset_manager = &asset_manager;
        Salix::Realm realm;
        realm.set_context(context);
        Salix::Entity* parent = realm.create_entity("Parent");
        Salix::Entity* child = realm.create_entity("Child");
        Salix::Entity* other = realm.create_entity("Other");
        REQUIRE(parent != nullptr);
        REQUIRE(child != nullptr);
        REQUIRE(other != nullptr);
        child->set_parent(parent);
        const Salix::SimpleGuid parent_id = parent->get_id();
        const Salix::SimpleGuid other_id = other->get_id();

        parent->simple_purge();
        CHECK(child->get_parent() == parent);
        CHECK_FALSE(child->is_purged());
        realm.maintain();

        CHECK(realm.get_entity_by_id(parent_id) == nullptr);
        CHECK(child->get_parent() == nullptr);
        CHECK_FALSE(child->is_purged());
        CHECK(realm.get_entity_by_id(other_id) == other);
        CHECK(realm.get_entities().size() == 2);
    }

    TEST_CASE("clearing a realm removes every owned entity") {
        Salix::Realm realm;
        Salix::Entity* parent = realm.create_entity("Parent");
        Salix::Entity* child = realm.create_entity("Child");
        REQUIRE(parent != nullptr);
        REQUIRE(child != nullptr);
        child->set_parent(parent);
        const Salix::SimpleGuid parent_id = parent->get_id();
        realm.clear_all_entities();
        CHECK(realm.get_entities().empty());
        CHECK(realm.get_entity_by_id(parent_id) == nullptr);
        CHECK(realm.get_entity_by_name("Child") == nullptr);
        // Don't dereference parent or child after their owning realm erases them.
    }

    TEST_CASE("main camera selection activates the chosen component") {
        Salix::Realm realm;
        Salix::Entity* camera_entity = realm.create_entity("MainCamera");
        REQUIRE(camera_entity != nullptr);
        Salix::Camera* camera = camera_entity->add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        CHECK_FALSE(camera->get_is_active());

        realm.set_main_camera_entity(camera_entity->get_id());
        CHECK(realm.get_main_camera_entity_id() == camera_entity->get_id());
        realm.set_active_camera_entity(camera_entity->get_id());
        CHECK(realm.get_active_camera() == camera);
        CHECK(camera->get_is_active());
    }

    TEST_CASE("switching cameras deactivates old selection and invalid ID clears it") {
        Salix::Realm realm;
        Salix::Entity* first_entity = realm.create_entity("FirstCamera");
        Salix::Entity* second_entity = realm.create_entity("SecondCamera");
        REQUIRE(first_entity != nullptr);
        REQUIRE(second_entity != nullptr);
        Salix::Camera* first = first_entity->add_element<Salix::Camera>();
        Salix::Camera* second = second_entity->add_element<Salix::Camera>();
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);

        realm.set_active_camera_entity(first_entity->get_id());
        REQUIRE(realm.get_active_camera() == first);
        CHECK(first->get_is_active());

        realm.set_active_camera_entity(second_entity->get_id());
        CHECK_FALSE(first->get_is_active());
        REQUIRE(realm.get_active_camera() == second);
        CHECK(second->get_is_active());

        realm.set_active_camera_entity(Salix::SimpleGuid::invalid());
        CHECK(realm.get_active_camera() == nullptr);
        CHECK(realm.get_main_camera_entity_id() == Salix::SimpleGuid::invalid());
        CHECK_FALSE(second->get_is_active());
    }

    TEST_CASE("purging a selected camera clears its non-owning pointer") {
        Salix::Realm realm;
        Salix::Entity* camera_entity = realm.create_entity("CameraToRemove");
        Salix::Entity* survivor = realm.create_entity("Survivor");
        REQUIRE(camera_entity != nullptr);
        REQUIRE(survivor != nullptr);
        Salix::Camera* camera = camera_entity->add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        const Salix::SimpleGuid removed_id = camera_entity->get_id();
        const Salix::SimpleGuid survivor_id = survivor->get_id();
        realm.set_active_camera_entity(removed_id);
        REQUIRE(realm.get_active_camera() == camera);

        camera_entity->simple_purge();
        realm.maintain();
        CHECK(realm.get_entity_by_id(removed_id) == nullptr);
        CHECK(realm.get_active_camera() == nullptr);
        CHECK(realm.get_main_camera_entity_id() == Salix::SimpleGuid::invalid());
        CHECK(realm.get_entity_by_id(survivor_id) == survivor);
    }

    TEST_CASE("clearing all entities also resets active camera state") {
        Salix::Realm realm;
        Salix::Entity* camera_entity = realm.create_entity("MainCamera");
        REQUIRE(camera_entity != nullptr);
        Salix::Camera* camera = camera_entity->add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        realm.set_active_camera_entity(camera_entity->get_id());
        REQUIRE(realm.get_active_camera() == camera);

        realm.clear_all_entities();
        CHECK(realm.get_entities().empty());
        CHECK(realm.get_active_camera() == nullptr);
        CHECK(realm.get_main_camera_entity_id() == Salix::SimpleGuid::invalid());
    }

}
