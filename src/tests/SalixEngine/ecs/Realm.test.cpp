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

// Additional cases preserved from the separate regression suite.
// Realm lifecycle and ownership semantics, using only memory-owned entities.
#include <doctest.h>
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Camera.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/core/InitContext.h>

namespace {
    class LifecycleProbe : public Salix::Element {
    public:
        int loads = 0;
        int updates = 0;
        const char* get_class_name() const override { return "LifecycleProbe"; }
        void on_load(const Salix::InitContext&) override { ++loads; }
        void update(float) override { ++updates; }
    };
}

TEST_SUITE("Salix::ecs::RealmLifecycleRegression") {
    TEST_CASE("realm lifecycle empty realm metadata updates never erase valid scene identifiers") {
        Salix::Realm realm("Level", "Scenes/Level.json");
        realm.set_name("");
        realm.set_path("");
        CHECK(realm.get_name() == "Level");
        CHECK(realm.get_path() == "Scenes/Level.json");
        realm.set_name("Level2");
        realm.set_path("Scenes/Level2.json");
        CHECK(realm.get_name() == "Level2");
        CHECK(realm.get_path() == "Scenes/Level2.json");
    }

    TEST_CASE("realm lifecycle duplicate entity names return first inserted while GUIDs disambiguate") {
        Salix::Realm realm;
        Salix::Entity* first = realm.create_entity("Duplicate");
        Salix::Entity* second = realm.create_entity("Duplicate");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(realm.get_entity_by_name("Duplicate") == first);
        CHECK(realm.get_entity_by_id(first->get_id()) == first);
        CHECK(realm.get_entity_by_id(second->get_id()) == second);
    }

    TEST_CASE("realm lifecycle maintain destroys every flagged entity while preserving survivors") {
        Salix::Realm realm;
        Salix::Entity* doomed_a = realm.create_entity("DoomedA");
        Salix::Entity* survivor = realm.create_entity("Survivor");
        Salix::Entity* doomed_b = realm.create_entity("DoomedB");
        REQUIRE(doomed_a != nullptr);
        REQUIRE(survivor != nullptr);
        REQUIRE(doomed_b != nullptr);
        const auto first_id = doomed_a->get_id();
        const auto last_id = doomed_b->get_id();
        const auto survivor_id = survivor->get_id();
        doomed_a->simple_purge();
        doomed_b->simple_purge();
        realm.maintain();
        CHECK(realm.get_entity_by_id(first_id) == nullptr);
        CHECK(realm.get_entity_by_id(last_id) == nullptr);
        CHECK(realm.get_entity_by_id(survivor_id) == survivor);
        CHECK(realm.get_entities().size() == 1);
    }

    TEST_CASE("realm lifecycle calling maintain twice is idempotent after clearing flagged entities") {
        Salix::Realm realm;
        Salix::Entity* doomed = realm.create_entity("Doomed");
        Salix::Entity* survivor = realm.create_entity("Survivor");
        REQUIRE(doomed != nullptr);
        REQUIRE(survivor != nullptr);
        doomed->simple_purge();
        realm.maintain();
        const auto snapshot = realm.get_entities();
        realm.maintain();
        CHECK(realm.get_entities() == snapshot);
        CHECK(realm.get_entity_by_name("Survivor") == survivor);
    }

    TEST_CASE("realm lifecycle entity snapshots are non owning copies and preserve insertion order") {
        Salix::Realm realm;
        Salix::Entity* first = realm.create_entity("First");
        Salix::Entity* second = realm.create_entity("Second");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        auto snapshot = realm.get_entities();
        REQUIRE(snapshot.size() == 2);
        snapshot.clear();
        const auto newer = realm.get_entities();
        REQUIRE(newer.size() == 2);
        CHECK(newer[0] == first);
        CHECK(newer[1] == second);
    }

    TEST_CASE("realm lifecycle realm on_load visits each owned element exactly once per call") {
        Salix::Realm realm;
        Salix::Entity* entity = realm.create_entity("Probe");
        REQUIRE(entity != nullptr);
        auto* probe = entity->add_element<LifecycleProbe>();
        REQUIRE(probe != nullptr);
        Salix::InitContext context{};
        realm.on_load(context);
        CHECK(probe->loads == 1);
        realm.on_load(context);
        CHECK(probe->loads == 2);
    }

    TEST_CASE("realm lifecycle realm update skips flagged entities while continuing live entities") {
        Salix::Realm realm;
        Salix::Entity* live = realm.create_entity("Live");
        Salix::Entity* dead = realm.create_entity("Flagged");
        REQUIRE(live != nullptr);
        REQUIRE(dead != nullptr);
        auto* live_probe = live->add_element<LifecycleProbe>();
        auto* dead_probe = dead->add_element<LifecycleProbe>();
        REQUIRE(live_probe != nullptr);
        REQUIRE(dead_probe != nullptr);
        realm.update(0.016f);
        CHECK(live_probe->updates == 1);
        CHECK(dead_probe->updates == 1);
        dead->simple_purge();
        realm.update(0.016f);
        CHECK(live_probe->updates == 2);
        CHECK(dead_probe->updates == 1);
    }

    TEST_CASE("realm lifecycle load_assets only propagates on_load when asset manager exists") {
        Salix::Realm realm;
        Salix::Entity* entity = realm.create_entity("Probe");
        REQUIRE(entity != nullptr);
        auto* probe = entity->add_element<LifecycleProbe>();
        REQUIRE(probe != nullptr);
        Salix::InitContext empty{};
        realm.load_assets(empty);
        CHECK(probe->loads == 0);
        Salix::AssetManager asset_manager;
        Salix::InitContext context{};
        context.asset_manager = &asset_manager;
        realm.load_assets(context);
        CHECK(probe->loads == 1);
    }

    TEST_CASE("realm lifecycle direct camera pointer assignment ignores null and keeps selected camera") {
        Salix::Realm realm;
        Salix::Entity* entity = realm.create_entity("Camera");
        REQUIRE(entity != nullptr);
        auto* camera = entity->add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        realm.set_active_camera(camera);
        CHECK(realm.get_active_camera() == camera);
        realm.set_active_camera(nullptr);
        CHECK(realm.get_active_camera() == camera);
    }

    TEST_CASE("realm lifecycle selecting an entity without a camera clears previous camera pointer") {
        Salix::Realm realm;
        Salix::Entity* with_camera = realm.create_entity("Camera");
        Salix::Entity* without_camera = realm.create_entity("NotCamera");
        REQUIRE(with_camera != nullptr);
        REQUIRE(without_camera != nullptr);
        auto* camera = with_camera->add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        realm.set_active_camera_entity(with_camera->get_id());
        REQUIRE(realm.get_active_camera() == camera);
        realm.set_active_camera_entity(without_camera->get_id());
        CHECK(realm.get_active_camera() == nullptr);
        CHECK_FALSE(camera->get_is_active());
        CHECK(realm.get_main_camera_entity_id() == without_camera->get_id());
    }
}

// Realm entity identity and maintenance; no camera-selection or Editor changes.
TEST_SUITE("Salix::ecs::RealmAdditional20261019") {
    TEST_CASE("realm extended create assigns separate stable IDs even when labels are identical") {
        Salix::Realm realm;
        auto* a=realm.create_entity("Opponent");
        auto* b=realm.create_entity("Opponent");
        REQUIRE(a!=nullptr);
        REQUIRE(b!=nullptr);
        CHECK(a!=b);
        CHECK(a->get_id()!=b->get_id());
        CHECK(realm.get_entity_by_id(a->get_id())==a);
        CHECK(realm.get_entity_by_id(b->get_id())==b);
    }

    TEST_CASE("realm extended entity lookup remains consistent after renaming another entity") {
        Salix::Realm realm;
        auto* first=realm.create_entity("Before");
        auto* second=realm.create_entity("Other");
        REQUIRE(first!=nullptr);
        REQUIRE(second!=nullptr);
        first->set_name("After");
        CHECK(realm.get_entity_by_name("Before")==nullptr);
        CHECK(realm.get_entity_by_name("After")==first);
        CHECK(realm.get_entity_by_name("Other")==second);
    }

    TEST_CASE("realm extended lookup of a missing GUID does not mutate the entity collection") {
        Salix::Realm realm;
        REQUIRE(realm.create_entity("Resident")!=nullptr);
        const auto before=realm.get_entities().size();
        CHECK(realm.get_entity_by_id(Salix::SimpleGuid::invalid())==nullptr);
        CHECK(realm.get_entity_by_id(Salix::SimpleGuid::from_value(999999999ULL))==nullptr);
        CHECK(realm.get_entities().size()==before);
    }

    TEST_CASE("realm extended an explicitly assigned entity GUID survives unrelated insertions") {
        Salix::Realm realm;
        const auto id=Salix::SimpleGuid::from_value(1234567ULL);
        auto* entity=realm.create_entity(id,"Tracked");
        REQUIRE(entity!=nullptr);
        REQUIRE(realm.create_entity("Other")!=nullptr);
        CHECK(entity->get_id()==id);
        CHECK(realm.get_entity_by_id(id)==entity);
    }

    TEST_CASE("realm extended clearing entities is idempotent and leaves realm metadata alone") {
        Salix::Realm realm("Map","Scenes/Map.json");
        REQUIRE(realm.create_entity("First")!=nullptr);
        realm.clear_all_entities();
        realm.clear_all_entities();
        CHECK(realm.get_entities().empty());
        CHECK(realm.get_name()=="Map");
        CHECK(realm.get_path()=="Scenes/Map.json");
    }

    TEST_CASE("realm extended new entities can be created after a complete clear") {
        Salix::Realm realm;
        REQUIRE(realm.create_entity("Before")!=nullptr);
        realm.clear_all_entities();
        auto* after=realm.create_entity("After");
        REQUIRE(after!=nullptr);
        CHECK(realm.get_entity_by_name("Before")==nullptr);
        CHECK(realm.get_entity_by_id(after->get_id())==after);
        CHECK(realm.get_entities().size()==1);
    }

    TEST_CASE("realm extended maintain on an empty realm does not create a hidden entity") {
        Salix::Realm realm;
        realm.maintain();
        realm.maintain();
        CHECK(realm.get_entities().empty());
        CHECK(realm.get_entity_by_name("Anything")==nullptr);
    }

    TEST_CASE("realm extended maintenance removes one purged entity in the middle and preserves order") {
        Salix::Realm realm;
        auto* a=realm.create_entity("A");
        auto* b=realm.create_entity("B");
        auto* c=realm.create_entity("C");
        REQUIRE(a!=nullptr);
        REQUIRE(b!=nullptr);
        REQUIRE(c!=nullptr);
        const auto doomed=b->get_id();
        b->simple_purge();
        realm.maintain();
        const auto remaining=realm.get_entities();
        REQUIRE(remaining.size()==2);
        CHECK(remaining[0]==a);
        CHECK(remaining[1]==c);
        CHECK(realm.get_entity_by_id(doomed)==nullptr);
    }

    TEST_CASE("realm extended maintain can process a second purge on a later frame") {
        Salix::Realm realm;
        auto* first=realm.create_entity("First");
        auto* second=realm.create_entity("Second");
        REQUIRE(first!=nullptr);
        REQUIRE(second!=nullptr);
        const auto first_id=first->get_id();
        const auto second_id=second->get_id();
        first->simple_purge();
        realm.maintain();
        CHECK(realm.get_entity_by_id(second_id)==second);
        second->simple_purge();
        realm.maintain();
        CHECK(realm.get_entity_by_id(first_id)==nullptr);
        CHECK(realm.get_entity_by_id(second_id)==nullptr);
        CHECK(realm.get_entities().empty());
    }

    TEST_CASE("realm extended updating an empty and later populated realm preserves entity identity") {
        Salix::Realm realm;
        realm.update(0.f);
        auto* player=realm.create_entity("Player");
        REQUIRE(player!=nullptr);
        const auto id=player->get_id();
        realm.update(0.016f);
        realm.update(0.f);
        CHECK(realm.get_entity_by_id(id)==player);
        CHECK(realm.get_entities().size()==1);
    }
}

TEST_SUITE("Salix::ecs::RealmBatch2347") {
    TEST_CASE("batch2347 repeated name and path assignments preserve latest nonempty values") {
        Salix::Realm realm; realm.set_name("First"); realm.set_name("Second"); realm.set_path("A.realm"); realm.set_path("B.realm");
        CHECK(realm.get_name()=="Second"); CHECK(realm.get_path()=="B.realm");
    }
    TEST_CASE("batch2347 duplicate entity names remain distinct owned entities") {
        Salix::Realm realm; auto* a=realm.create_entity("Duplicate"); auto* b=realm.create_entity("Duplicate");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); CHECK(a!=b); CHECK(a->get_id()!=b->get_id()); REQUIRE(realm.get_entities().size()==2);
    }
    TEST_CASE("batch2347 first duplicate name remains the name lookup result") {
        Salix::Realm realm; auto* first=realm.create_entity("Same"); auto* second=realm.create_entity("Same"); REQUIRE(first!=nullptr); REQUIRE(second!=nullptr);
        CHECK(realm.get_entity_by_name("Same")==first);
    }
    TEST_CASE("batch2347 clear all entities resets active and main camera selection") {
        Salix::Realm realm; auto* e=realm.create_entity("Camera2347"); REQUIRE(e!=nullptr); auto* camera=e->add_element<Salix::Camera>(); REQUIRE(camera!=nullptr);
        realm.set_active_camera_entity(e->get_id()); REQUIRE(realm.get_active_camera()==camera); REQUIRE(realm.get_main_camera_entity_id()==e->get_id());
        realm.clear_all_entities(); CHECK(realm.get_entities().empty()); CHECK(realm.get_active_camera()==nullptr); CHECK(realm.get_main_camera_entity_id()==Salix::SimpleGuid::invalid());
    }
    TEST_CASE("batch2347 invalid active camera entity clears previous selection and deactivates old camera") {
        Salix::Realm realm; auto* e=realm.create_entity("Camera2347"); REQUIRE(e!=nullptr); auto* camera=e->add_element<Salix::Camera>(); REQUIRE(camera!=nullptr);
        realm.set_active_camera_entity(e->get_id()); REQUIRE(camera->get_is_active()); realm.set_active_camera_entity(Salix::SimpleGuid::invalid());
        CHECK(realm.get_active_camera()==nullptr); CHECK_FALSE(camera->get_is_active()); CHECK(realm.get_main_camera_entity_id()==Salix::SimpleGuid::invalid());
    }
    TEST_CASE("batch2347 direct main camera ID assignment does not activate a component") {
        Salix::Realm realm; auto* e=realm.create_entity("Camera2347"); REQUIRE(e!=nullptr); auto* camera=e->add_element<Salix::Camera>(); REQUIRE(camera!=nullptr);
        realm.set_main_camera_entity(e->get_id()); CHECK(realm.get_main_camera_entity_id()==e->get_id()); CHECK(realm.get_active_camera()==nullptr); CHECK_FALSE(camera->get_is_active());
    }
    TEST_CASE("batch2347 maintenance removes multiple purged entities in one pass") {
        Salix::Realm realm; auto* a=realm.create_entity("A"); auto* b=realm.create_entity("B"); auto* c=realm.create_entity("C"); REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); REQUIRE(c!=nullptr);
        const auto aid=a->get_id(),cid=c->get_id(); a->simple_purge(); c->simple_purge(); realm.maintain();
        const auto entities=realm.get_entities(); REQUIRE(entities.size()==1); CHECK(entities[0]==b); CHECK(realm.get_entity_by_id(aid)==nullptr); CHECK(realm.get_entity_by_id(cid)==nullptr);
    }
    TEST_CASE("batch2347 clear followed by recreation keeps realm metadata intact") {
        Salix::Realm realm("KeepName","KeepPath.realm"); REQUIRE(realm.create_entity("Before")!=nullptr); realm.clear_all_entities();
        auto* after=realm.create_entity("After"); REQUIRE(after!=nullptr); CHECK(realm.get_name()=="KeepName"); CHECK(realm.get_path()=="KeepPath.realm"); CHECK(realm.get_entity_by_name("After")==after);
    }
    TEST_CASE("batch2347 direct null camera assignment cannot clear an existing direct camera pointer") {
        Salix::Realm realm; auto* e=realm.create_entity("Camera"); REQUIRE(e!=nullptr); auto* camera=e->add_element<Salix::Camera>(); REQUIRE(camera!=nullptr);
        realm.set_active_camera(camera); REQUIRE(realm.get_active_camera()==camera); realm.set_active_camera(nullptr); CHECK(realm.get_active_camera()==camera);
    }
    TEST_CASE("batch2347 explicit GUID entity creation preserves insertion order beside generated entity") {
        Salix::Realm realm; const auto fixed=Salix::SimpleGuid::from_value(2347001ULL); auto* a=realm.create_entity(fixed,"Fixed"); auto* b=realm.create_entity("Generated");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); const auto entities=realm.get_entities(); REQUIRE(entities.size()==2); CHECK(entities[0]==a); CHECK(entities[1]==b); CHECK(a->get_id()==fixed);
    }
}

TEST_SUITE("Salix::ecs::RealmBatch2647") {
    TEST_CASE("batch2647 named constructor preserves exact supplied name") {
        Salix::Realm realm("Realm 2647");
        CHECK(realm.get_name()=="Realm 2647");
    }
    TEST_CASE("batch2647 name and path may both be cleared explicitly") {
        Salix::Realm realm("Name","Path.realm");
        realm.set_name(""); realm.set_path("");
        CHECK(realm.get_name().empty()); CHECK(realm.get_path().empty());
    }
    TEST_CASE("batch2647 entity snapshot mutation cannot alter realm ownership") {
        Salix::Realm realm; auto* a=realm.create_entity("A"); auto* b=realm.create_entity("B");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr);
        auto snapshot=realm.get_entities(); snapshot.erase(snapshot.begin());
        const auto current=realm.get_entities(); REQUIRE(current.size()==2); CHECK(current[0]==a); CHECK(current[1]==b);
    }
    TEST_CASE("batch2647 maintain removes adjacent purged entities while preserving later survivor") {
        Salix::Realm realm; auto* a=realm.create_entity("A"); auto* b=realm.create_entity("B"); auto* c=realm.create_entity("C");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); REQUIRE(c!=nullptr); a->simple_purge(); b->simple_purge();
        realm.maintain(); const auto remaining=realm.get_entities(); REQUIRE(remaining.size()==1); CHECK(remaining[0]==c);
    }
    TEST_CASE("batch2647 maintain preserves unpurged duplicate-name entities independently") {
        Salix::Realm realm; auto* first=realm.create_entity("Same"); auto* second=realm.create_entity("Same");
        REQUIRE(first!=nullptr); REQUIRE(second!=nullptr); first->simple_purge(); realm.maintain();
        const auto remaining=realm.get_entities(); REQUIRE(remaining.size()==1); CHECK(remaining[0]==second); CHECK(realm.get_entity_by_name("Same")==second);
    }
    TEST_CASE("batch2647 explicit invalid GUID entity remains retrievable by invalid GUID until cleared") {
        Salix::Realm realm; auto* entity=realm.create_entity(Salix::SimpleGuid::invalid(),"InvalidId");
        REQUIRE(entity!=nullptr); CHECK_FALSE(entity->get_id().is_valid()); CHECK(realm.get_entity_by_id(Salix::SimpleGuid::invalid())==entity);
    }
    TEST_CASE("batch2647 empty realm lifecycle remains safe across load update render and maintain") {
        Salix::Realm realm; Salix::InitContext context{};
        CHECK_NOTHROW(realm.on_load(context)); CHECK_NOTHROW(realm.update(-1.f)); CHECK_NOTHROW(realm.render(nullptr)); CHECK_NOTHROW(realm.maintain());
        CHECK(realm.get_entities().empty());
    }
    TEST_CASE("batch2647 clear all entities is idempotent after populated realm") {
        Salix::Realm realm; REQUIRE(realm.create_entity("One")!=nullptr); REQUIRE(realm.create_entity("Two")!=nullptr);
        realm.clear_all_entities(); realm.clear_all_entities(); CHECK(realm.get_entities().empty());
    }
    TEST_CASE("batch2647 direct main camera identifier assignment roundtrips arbitrary valid ID") {
        Salix::Realm realm; const auto id=Salix::SimpleGuid::from_value(2647999ULL);
        realm.set_main_camera_entity(id); CHECK(realm.get_main_camera_entity_id()==id); CHECK(realm.get_active_camera()==nullptr);
    }
    TEST_CASE("batch2647 nonexistent active camera entity clears current camera and records requested identifier") {
        Salix::Realm realm; auto* e=realm.create_entity("Camera"); REQUIRE(e!=nullptr); auto* camera=e->add_element<Salix::Camera>(); REQUIRE(camera!=nullptr);
        realm.set_active_camera_entity(e->get_id()); REQUIRE(realm.get_active_camera()==camera);
        const auto missing=Salix::SimpleGuid::from_value(987654321ULL); realm.set_active_camera_entity(missing);
        CHECK(realm.get_active_camera()==nullptr); CHECK_FALSE(camera->get_is_active()); CHECK(realm.get_main_camera_entity_id()==missing);
    }
}

