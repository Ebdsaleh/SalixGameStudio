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
