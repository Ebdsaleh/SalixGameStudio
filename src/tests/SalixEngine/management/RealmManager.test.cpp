// RealmManager.test.cpp - Memory-only manager lifecycle and selection tests.
// These tests intentionally do not invoke save_active_realm(): current source
// hard-codes a sandbox path, which must not be used for unattended tests.
#include <doctest.h>
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Realm.h>
#include <Salix/management/RealmManager.h>

TEST_SUITE("Salix::management::RealmManager") {

    TEST_CASE("a new manager has no active realm and can shut down empty") {
        Salix::RealmManager manager;
        const Salix::InitContext context{};
        manager.initialize(context);
        CHECK(manager.get_active_realm() == nullptr);
        CHECK_FALSE(manager.save_active_realm());
        CHECK_FALSE(manager.load_active_realm());
        manager.update(0.016f);
        manager.render(nullptr);
        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("invalid realm creation does not install an active realm") {
        Salix::RealmManager manager;
        CHECK(manager.create_realm("", "Scenes/Empty.json") == nullptr);
        CHECK(manager.create_realm("Named", "") == nullptr);
        CHECK(manager.get_active_realm() == nullptr);
        CHECK_FALSE(manager.set_active_realm("Missing"));
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("realm creation and selection retain the selected realm") {
        Salix::RealmManager manager;
        const Salix::InitContext context{};
        manager.initialize(context);
        Salix::Realm* menu = manager.create_realm("Menu", "Scenes/Menu.json");
        Salix::Realm* game = manager.create_realm("Game", "Scenes/Game.json");
        REQUIRE(menu != nullptr);
        REQUIRE(game != nullptr);
        CHECK(menu != game);
        CHECK(menu->get_name() == "Menu");
        CHECK(game->get_path() == "Scenes/Game.json");
        CHECK(manager.get_active_realm() == nullptr);

        CHECK(manager.set_active_realm("Menu"));
        CHECK(manager.get_active_realm() == menu);
        CHECK_FALSE(manager.set_active_realm("Missing"));
        CHECK(manager.get_active_realm() == menu);
        CHECK(manager.set_active_realm("Game"));
        CHECK(manager.get_active_realm() == game);
        manager.update(0.016f);  // Empty realm: safe, no renderer or assets needed.
        manager.render(nullptr);
        CHECK_FALSE(manager.load_active_realm()); // No asset manager in context.

        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("project root ignores empty path updates") {
        Salix::RealmManager manager;
        CHECK(manager.get_project_root_path().empty());
        manager.set_project_root_path("C:/GameProject");
        manager.set_project_root_path("");
        CHECK(manager.get_project_root_path() == "C:/GameProject");
    }

    TEST_CASE("replacing a selected realm retains a valid active realm pointer") {
        Salix::RealmManager manager;
        Salix::Realm* initial = manager.create_realm("Battle", "Scenes/Battle-old.json");
        REQUIRE(initial != nullptr);
        REQUIRE(manager.set_active_realm("Battle"));
        REQUIRE(manager.get_active_realm() == initial);

        Salix::Realm* replacement = manager.create_realm("Battle", "Scenes/Battle-new.json");
        REQUIRE(replacement != nullptr);
        // Do not dereference the old realm if it was destroyed.
        REQUIRE(manager.get_active_realm() == replacement);
        CHECK(manager.get_active_realm()->get_name() == "Battle");
        CHECK(manager.get_active_realm()->get_path() == "Scenes/Battle-new.json");
        manager.update(0.016f);
        manager.render(nullptr);
    }

    TEST_CASE("replacing a non-active realm preserves the active selection") {
        Salix::RealmManager manager;
        Salix::Realm* menu = manager.create_realm("Menu", "Scenes/Menu.json");
        REQUIRE(menu != nullptr);
        REQUIRE(manager.create_realm("Battle", "Scenes/Battle-old.json") != nullptr);
        REQUIRE(manager.set_active_realm("Menu"));

        Salix::Realm* replacement = manager.create_realm("Battle", "Scenes/Battle-new.json");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == menu);
        CHECK(menu->get_path() == "Scenes/Menu.json");
        REQUIRE(manager.set_active_realm("Battle"));
        CHECK(manager.get_active_realm() == replacement);
    }

}

#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/core/InitContext.h>

namespace {
    class AdditionalRealmProbe final : public Salix::Element {
    public:
        int updates = 0;
        int loads = 0;
        float last_delta = 0.f;
        const char* get_class_name() const override { return "AdditionalRealmProbe"; }
        void update(float delta) override { ++updates; last_delta = delta; }
        void on_load(const Salix::InitContext&) override { ++loads; }
    };
}

TEST_SUITE("Salix::management::RealmManagerAdditional") {
    TEST_CASE("realm manager fresh manager never selects a realm without an explicit request") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Menu", "Scenes/Menu.json") != nullptr);
        REQUIRE(manager.create_realm("Game", "Scenes/Game.json") != nullptr);
        CHECK(manager.get_active_realm() == nullptr);
        CHECK_FALSE(manager.load_active_realm());
    }

    TEST_CASE("realm manager creating distinct realm names keeps independent metadata") {
        Salix::RealmManager manager;
        auto* first = manager.create_realm("Menu", "Scenes/Menu.json");
        auto* second = manager.create_realm("Game", "Scenes/Game.json");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(first->get_name() == "Menu");
        CHECK(second->get_path() == "Scenes/Game.json");
    }

    TEST_CASE("realm manager missing selection on an empty manager returns false and remains empty") {
        Salix::RealmManager manager;
        CHECK_FALSE(manager.set_active_realm("Unknown"));
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("realm manager failed selection preserves the previously selected realm object") {
        Salix::RealmManager manager;
        auto* selected = manager.create_realm("Menu", "Scenes/Menu.json");
        REQUIRE(selected != nullptr);
        REQUIRE(manager.set_active_realm("Menu"));
        CHECK_FALSE(manager.set_active_realm("Nonexistent"));
        CHECK(manager.get_active_realm() == selected);
    }

    TEST_CASE("realm manager update forwards delta time only to the currently selected realm") {
        Salix::RealmManager manager;
        auto* a = manager.create_realm("A", "A.json");
        auto* b = manager.create_realm("B", "B.json");
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        auto* ea = a->create_entity("a");
        auto* eb = b->create_entity("b");
        REQUIRE(ea != nullptr);
        REQUIRE(eb != nullptr);
        auto* pa = ea->add_element<AdditionalRealmProbe>();
        auto* pb = eb->add_element<AdditionalRealmProbe>();
        REQUIRE(pa != nullptr);
        REQUIRE(pb != nullptr);
        REQUIRE(manager.set_active_realm("B"));
        manager.update(0.125f);
        CHECK(pa->updates == 0);
        CHECK(pb->updates == 1);
        CHECK(pb->last_delta == doctest::Approx(0.125f));
    }

    TEST_CASE("realm manager switching active realms changes where subsequent updates are delivered") {
        Salix::RealmManager manager;
        auto* a = manager.create_realm("A", "A.json");
        auto* b = manager.create_realm("B", "B.json");
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        auto* ea = a->create_entity("a");
        auto* eb = b->create_entity("b");
        REQUIRE(ea != nullptr);
        REQUIRE(eb != nullptr);
        auto* pa = ea->add_element<AdditionalRealmProbe>();
        auto* pb = eb->add_element<AdditionalRealmProbe>();
        REQUIRE(pa != nullptr);
        REQUIRE(pb != nullptr);
        REQUIRE(manager.set_active_realm("A"));
        manager.update(1.f);
        REQUIRE(manager.set_active_realm("B"));
        manager.update(2.f);
        CHECK(pa->updates == 1);
        CHECK(pb->updates == 1);
        CHECK(pa->last_delta == 1.f);
        CHECK(pb->last_delta == 2.f);
    }

    TEST_CASE("realm manager shutdown discards all realms and accepts subsequent idle calls") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Old", "Old.json") != nullptr);
        REQUIRE(manager.set_active_realm("Old"));
        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
        CHECK_FALSE(manager.set_active_realm("Old"));
        CHECK_NOTHROW(manager.update(0.1f));
        CHECK_NOTHROW(manager.render(nullptr));
    }

    TEST_CASE("realm manager manager may be initialized again after shutdown and create a new realm") {
        Salix::RealmManager manager;
        Salix::InitContext context{};
        manager.initialize(context);
        REQUIRE(manager.create_realm("One", "One.json") != nullptr);
        REQUIRE(manager.set_active_realm("One"));
        manager.shutdown();
        manager.initialize(context);
        REQUIRE(manager.create_realm("Two", "Two.json") != nullptr);
        REQUIRE(manager.set_active_realm("Two"));
        CHECK(manager.get_active_realm()->get_name() == "Two");
    }

    TEST_CASE("realm manager load active realm rejects missing asset manager without invoking components") {
        Salix::RealmManager manager;
        Salix::InitContext context{};
        manager.initialize(context);
        auto* realm = manager.create_realm("Unloaded", "Unloaded.json");
        REQUIRE(realm != nullptr);
        auto* entity = realm->create_entity("probe");
        REQUIRE(entity != nullptr);
        auto* probe = entity->add_element<AdditionalRealmProbe>();
        REQUIRE(probe != nullptr);
        REQUIRE(manager.set_active_realm("Unloaded"));
        CHECK_FALSE(manager.load_active_realm());
        CHECK(probe->loads == 0);
    }

    TEST_CASE("realm manager load active realm calls entity on_load for each explicit request") {
        Salix::RealmManager manager;
        Salix::AssetManager assets;
        Salix::InitContext context{};
        context.asset_manager = &assets;
        manager.initialize(context);
        auto* realm = manager.create_realm("Ready", "Ready.json");
        REQUIRE(realm != nullptr);
        auto* entity = realm->create_entity("probe");
        REQUIRE(entity != nullptr);
        auto* probe = entity->add_element<AdditionalRealmProbe>();
        REQUIRE(probe != nullptr);
        REQUIRE(manager.set_active_realm("Ready"));
        REQUIRE(manager.load_active_realm());
        CHECK(probe->loads == 1);
        REQUIRE(manager.load_active_realm());
        CHECK(probe->loads == 2);
    }
}
