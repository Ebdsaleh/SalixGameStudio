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

// Additional behaviour and lifecycle tests; existing cases preserved.
TEST_SUITE("Salix::management::RealmManagerBatch647") {
    TEST_CASE("creating a realm does not implicitly make it active") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("New","Scenes/New.json")!=nullptr);
        CHECK(manager.get_active_realm()==nullptr);
        manager.update(0.016f);
    }

    TEST_CASE("switching between named realms keeps each realm object stable") {
        Salix::RealmManager manager;
        auto* first=manager.create_realm("First","First.json");
        auto* second=manager.create_realm("Second","Second.json");
        REQUIRE(first!=nullptr); REQUIRE(second!=nullptr);
        REQUIRE(manager.set_active_realm("First"));
        CHECK(manager.get_active_realm()==first);
        REQUIRE(manager.set_active_realm("Second"));
        CHECK(manager.get_active_realm()==second);
        REQUIRE(manager.set_active_realm("First"));
        CHECK(manager.get_active_realm()==first);
    }

    TEST_CASE("an invalid selection does not disrupt the active realm's metadata") {
        Salix::RealmManager manager;
        auto* first=manager.create_realm("First","First.json"); REQUIRE(first!=nullptr);
        REQUIRE(manager.set_active_realm("First"));
        CHECK_FALSE(manager.set_active_realm(""));
        CHECK_FALSE(manager.set_active_realm("Missing"));
        CHECK(manager.get_active_realm()==first);
        CHECK(first->get_path()=="First.json");
    }

    TEST_CASE("replacing another named realm preserves both unrelated selection and path") {
        Salix::RealmManager manager;
        auto* active=manager.create_realm("Active","Active.json"); REQUIRE(active!=nullptr);
        REQUIRE(manager.create_realm("Replace","One.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Active"));
        auto* updated=manager.create_realm("Replace","Two.json");
        REQUIRE(updated!=nullptr);
        CHECK(updated->get_path()=="Two.json");
        CHECK(manager.get_active_realm()==active);
    }

    TEST_CASE("replacing an active realm leaves selection valid and updates the path") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Live","Before.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Live"));
        auto* replacement=manager.create_realm("Live","After.json");
        REQUIRE(replacement!=nullptr);
        CHECK(manager.get_active_realm()==replacement);
        CHECK(manager.get_active_realm()->get_path()=="After.json");
    }

    TEST_CASE("empty name and path are rejected without changing selection") {
        Salix::RealmManager manager;
        auto* active=manager.create_realm("Good","Good.json"); REQUIRE(active!=nullptr);
        REQUIRE(manager.set_active_realm("Good"));
        CHECK(manager.create_realm("","missing.json")==nullptr);
        CHECK(manager.create_realm("missing","")==nullptr);
        CHECK(manager.get_active_realm()==active);
    }

    TEST_CASE("setting an empty project root preserves the last nonempty root") {
        Salix::RealmManager manager;
        manager.set_project_root_path("Projects/One");
        manager.set_project_root_path("");
        CHECK(manager.get_project_root_path()=="Projects/One");
        manager.set_project_root_path("Projects/Two");
        CHECK(manager.get_project_root_path()=="Projects/Two");
    }

    TEST_CASE("an uninitialized active realm fails asset loading without dereferencing pointers") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Pending","Pending.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Pending"));
        CHECK_FALSE(manager.load_active_realm());
        CHECK(manager.get_active_realm()!=nullptr);
    }

    TEST_CASE("after shutdown previously registered realm names cannot be selected") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Previous","Previous.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Previous"));
        manager.shutdown();
        CHECK(manager.get_active_realm()==nullptr);
        CHECK_FALSE(manager.set_active_realm("Previous"));
        REQUIRE(manager.create_realm("Fresh","Fresh.json")!=nullptr);
        CHECK(manager.set_active_realm("Fresh"));
    }

    TEST_CASE("a failed load of a nonexistent realm file preserves active selection") {
        Salix::RealmManager manager;
        auto* active=manager.create_realm("Stable","Stable.json"); REQUIRE(active!=nullptr);
        REQUIRE(manager.set_active_realm("Stable"));
        CHECK_FALSE(manager.load_realm("nonexistent_sg_unlikely_20261019.json"));
        CHECK(manager.get_active_realm()==active);
    }
}


// Runtime-facing isolated contracts for the 847+ coverage batch.
TEST_SUITE("Salix::management::RealmManagerBatch847") {
    TEST_CASE("realm manager repeated initialization does not select a realm implicitly") {
        Salix::RealmManager manager; Salix::InitContext a{},b{}; manager.initialize(a); manager.initialize(b); CHECK(manager.get_active_realm()==nullptr);
    }

    TEST_CASE("realm manager shutdown is idempotent on an empty manager") {
        Salix::RealmManager manager; manager.shutdown(); CHECK(manager.get_active_realm()==nullptr); manager.shutdown(); CHECK(manager.get_active_realm()==nullptr);
    }

    TEST_CASE("realm manager selecting the same realm repeatedly preserves pointer identity") {
        Salix::RealmManager manager; auto* realm=manager.create_realm("Stable","Stable.json"); REQUIRE(realm!=nullptr); REQUIRE(manager.set_active_realm("Stable")); auto* first=manager.get_active_realm(); REQUIRE(manager.set_active_realm("Stable")); CHECK(manager.get_active_realm()==first); CHECK(first==realm);
    }

    TEST_CASE("realm manager replacing an inactive realm does not steal active selection") {
        Salix::RealmManager manager; auto* active=manager.create_realm("Active","A.json"); REQUIRE(active!=nullptr); REQUIRE(manager.create_realm("Other","One.json")!=nullptr); REQUIRE(manager.set_active_realm("Active")); auto* replacement=manager.create_realm("Other","Two.json"); REQUIRE(replacement!=nullptr); CHECK(manager.get_active_realm()==active); CHECK(replacement->get_path()=="Two.json");
    }

    TEST_CASE("realm manager project root survives realm shutdown because it is manager configuration") {
        Salix::RealmManager manager; manager.set_project_root_path("C:/Projects/Salix"); REQUIRE(manager.create_realm("Temp","Temp.json")!=nullptr); REQUIRE(manager.set_active_realm("Temp")); manager.shutdown(); CHECK(manager.get_active_realm()==nullptr); CHECK(manager.get_project_root_path()=="C:/Projects/Salix");
    }

    TEST_CASE("realm manager can create and select fresh realms after shutdown") {
        Salix::RealmManager manager; REQUIRE(manager.create_realm("Old","Old.json")!=nullptr); REQUIRE(manager.set_active_realm("Old")); manager.shutdown(); auto* fresh=manager.create_realm("Fresh","Fresh.json"); REQUIRE(fresh!=nullptr); REQUIRE(manager.set_active_realm("Fresh")); CHECK(manager.get_active_realm()==fresh);
    }

    TEST_CASE("realm manager failed empty selection is harmless before any realm exists") {
        Salix::RealmManager manager; CHECK_FALSE(manager.set_active_realm("")); CHECK(manager.get_active_realm()==nullptr); CHECK_NOTHROW(manager.update(-1.f)); CHECK_NOTHROW(manager.render(nullptr));
    }

    TEST_CASE("realm manager active realm metadata may change without invalidating selection") {
        Salix::RealmManager manager; auto* realm=manager.create_realm("Mutable","Before.json"); REQUIRE(realm!=nullptr); REQUIRE(manager.set_active_realm("Mutable")); realm->set_name("Display Name"); realm->set_path("After.json"); CHECK(manager.get_active_realm()==realm); CHECK(manager.get_active_realm()->get_name()=="Display Name"); CHECK(manager.get_active_realm()->get_path()=="After.json");
    }

    TEST_CASE("realm manager replacing active realm resets its entity collection with the new object") {
        Salix::RealmManager manager; auto* old=manager.create_realm("World","Old.json"); REQUIRE(old!=nullptr); REQUIRE(old->create_entity("OldEntity")!=nullptr); REQUIRE(manager.set_active_realm("World")); auto* replacement=manager.create_realm("World","New.json"); REQUIRE(replacement!=nullptr); CHECK(manager.get_active_realm()==replacement); CHECK(replacement->get_entities().empty()); CHECK(replacement->get_path()=="New.json");
    }

    TEST_CASE("realm manager failed missing file load leaves an empty manager empty") {
        Salix::RealmManager manager; CHECK_FALSE(manager.load_realm("realm_manager_batch847_file_that_does_not_exist.json")); CHECK(manager.get_active_realm()==nullptr); CHECK_FALSE(manager.set_active_realm("realm_manager_batch847_file_that_does_not_exist"));
    }
}

TEST_SUITE("Salix::management::RealmManagerBatch2247") {
    TEST_CASE("batch2247 realm creation preserves exact name and path metadata") {
        Salix::RealmManager manager; auto* realm=manager.create_realm("Realm 2247","Scenes/Realm 2247.json");
        REQUIRE(realm!=nullptr); CHECK(realm->get_name()=="Realm 2247"); CHECK(realm->get_path()=="Scenes/Realm 2247.json"); CHECK(manager.get_active_realm()==nullptr);
    }
    TEST_CASE("batch2247 empty realm name remains rejected without changing manager state") {
        Salix::RealmManager manager; CHECK(manager.create_realm("","Valid.json")==nullptr); CHECK(manager.get_active_realm()==nullptr);
    }
    TEST_CASE("batch2247 empty realm path remains rejected without changing manager state") {
        Salix::RealmManager manager; CHECK(manager.create_realm("Valid","")==nullptr); CHECK(manager.get_active_realm()==nullptr);
    }
    TEST_CASE("batch2247 failed selection preserves previously active realm pointer") {
        Salix::RealmManager manager; auto* realm=manager.create_realm("Stable2247","Stable2247.json"); REQUIRE(realm!=nullptr); REQUIRE(manager.set_active_realm("Stable2247"));
        CHECK_FALSE(manager.set_active_realm("Missing2247")); CHECK(manager.get_active_realm()==realm);
    }
    TEST_CASE("batch2247 replacing active realm retargets selection to replacement object") {
        Salix::RealmManager manager; auto* first=manager.create_realm("Replace2247","Old.json"); REQUIRE(first!=nullptr); REQUIRE(manager.set_active_realm("Replace2247"));
        auto* replacement=manager.create_realm("Replace2247","New.json"); REQUIRE(replacement!=nullptr); CHECK(manager.get_active_realm()==replacement); CHECK(replacement->get_path()=="New.json");
    }
    TEST_CASE("batch2247 replacing inactive realm never steals active selection") {
        Salix::RealmManager manager; auto* active=manager.create_realm("Active2247","A.json"); REQUIRE(active!=nullptr); REQUIRE(manager.create_realm("Other2247","B.json")!=nullptr); REQUIRE(manager.set_active_realm("Active2247"));
        auto* replacement=manager.create_realm("Other2247","C.json"); REQUIRE(replacement!=nullptr); CHECK(manager.get_active_realm()==active); CHECK(replacement->get_path()=="C.json");
    }
    TEST_CASE("batch2247 shutdown clears realms but preserves configured project root") {
        Salix::RealmManager manager; manager.set_project_root_path("C:/Projects/Batch2247"); REQUIRE(manager.create_realm("Temp2247","Temp.json")!=nullptr); REQUIRE(manager.set_active_realm("Temp2247"));
        manager.shutdown(); CHECK(manager.get_active_realm()==nullptr); CHECK(manager.get_project_root_path()=="C:/Projects/Batch2247"); CHECK_FALSE(manager.set_active_realm("Temp2247"));
    }
    TEST_CASE("batch2247 empty project root assignment never erases existing configured root") {
        Salix::RealmManager manager; manager.set_project_root_path("Root2247"); manager.set_project_root_path(""); CHECK(manager.get_project_root_path()=="Root2247");
    }
    TEST_CASE("batch2247 active realm load fails without an asset manager in initialization context") {
        Salix::RealmManager manager; Salix::InitContext context{}; manager.initialize(context); REQUIRE(manager.create_realm("Load2247","Load.json")!=nullptr); REQUIRE(manager.set_active_realm("Load2247"));
        CHECK_FALSE(manager.load_active_realm());
    }
    TEST_CASE("batch2247 empty active realm can load when initialization context supplies an asset manager") {
        Salix::AssetManager assets; Salix::InitContext context{}; context.asset_manager=&assets; Salix::RealmManager manager; manager.initialize(context);
        auto* realm=manager.create_realm("Loadable2247","Loadable.json"); REQUIRE(realm!=nullptr); REQUIRE(manager.set_active_realm("Loadable2247"));
        CHECK(manager.load_active_realm()); CHECK(manager.get_active_realm()==realm);
    }
}

TEST_SUITE("Salix::management::RealmManagerBatch2847") {
    TEST_CASE("batch2847 creating several realms never implicitly selects one") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Menu2847","Menu2847.json")!=nullptr);
        REQUIRE(manager.create_realm("Game2847","Game2847.json")!=nullptr);
        REQUIRE(manager.create_realm("Credits2847","Credits2847.json")!=nullptr);
        CHECK(manager.get_active_realm()==nullptr);
    }
    TEST_CASE("batch2847 explicit selection can move between existing realms") {
        Salix::RealmManager manager;
        auto* first=manager.create_realm("First2847","First.json");
        auto* second=manager.create_realm("Second2847","Second.json");
        REQUIRE(first!=nullptr); REQUIRE(second!=nullptr);
        REQUIRE(manager.set_active_realm("First2847")); CHECK(manager.get_active_realm()==first);
        REQUIRE(manager.set_active_realm("Second2847")); CHECK(manager.get_active_realm()==second);
    }
    TEST_CASE("batch2847 replacing active realm keeps same logical selection on replacement") {
        Salix::RealmManager manager;
        auto* old=manager.create_realm("Replace2847","Old2847.json"); REQUIRE(old!=nullptr);
        REQUIRE(manager.set_active_realm("Replace2847"));
        auto* replacement=manager.create_realm("Replace2847","New2847.json"); REQUIRE(replacement!=nullptr);
        CHECK(manager.get_active_realm()==replacement);
        CHECK(replacement->get_path()=="New2847.json");
    }
    TEST_CASE("batch2847 replacing inactive realm preserves different active selection") {
        Salix::RealmManager manager;
        auto* active=manager.create_realm("Active2847","Active.json"); REQUIRE(active!=nullptr);
        REQUIRE(manager.create_realm("Other2847","Old.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Active2847"));
        REQUIRE(manager.create_realm("Other2847","New.json")!=nullptr);
        CHECK(manager.get_active_realm()==active);
    }
    TEST_CASE("batch2847 invalid empty name and path creations leave existing active realm unchanged") {
        Salix::RealmManager manager;
        auto* active=manager.create_realm("Stable2847","Stable.json"); REQUIRE(active!=nullptr); REQUIRE(manager.set_active_realm("Stable2847"));
        CHECK(manager.create_realm("","Valid.json")==nullptr);
        CHECK(manager.create_realm("Valid","")==nullptr);
        CHECK(manager.get_active_realm()==active);
    }
    TEST_CASE("batch2847 empty project-root update cannot erase configured root") {
        Salix::RealmManager manager;
        manager.set_project_root_path("C:/Project2847");
        manager.set_project_root_path("");
        CHECK(manager.get_project_root_path()=="C:/Project2847");
    }
    TEST_CASE("batch2847 shutdown clears active and stored realms but preserves project root") {
        Salix::RealmManager manager;
        manager.set_project_root_path("Root2847");
        REQUIRE(manager.create_realm("Temp2847","Temp.json")!=nullptr);
        REQUIRE(manager.set_active_realm("Temp2847"));
        manager.shutdown();
        CHECK(manager.get_active_realm()==nullptr);
        CHECK(manager.get_project_root_path()=="Root2847");
        CHECK_FALSE(manager.set_active_realm("Temp2847"));
    }
    TEST_CASE("batch2847 load active realm fails when no realm is selected") {
        Salix::RealmManager manager; Salix::InitContext context{}; manager.initialize(context);
        CHECK_FALSE(manager.load_active_realm());
    }
    TEST_CASE("batch2847 empty manager update and render remain safe after initialization") {
        Salix::RealmManager manager; Salix::InitContext context{}; manager.initialize(context);
        CHECK_NOTHROW(manager.update(0.016f));
        CHECK_NOTHROW(manager.update(-1.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_realm()==nullptr);
    }
    TEST_CASE("batch2847 active empty realm loads when context supplies asset manager") {
        Salix::AssetManager assets; Salix::InitContext context{}; context.asset_manager=&assets;
        Salix::RealmManager manager; manager.initialize(context);
        auto* realm=manager.create_realm("Load2847","Load2847.json"); REQUIRE(realm!=nullptr);
        REQUIRE(manager.set_active_realm("Load2847"));
        CHECK(manager.load_active_realm());
        CHECK(manager.get_active_realm()==realm);
    }
}


TEST_SUITE("Salix::management::RealmManagerBatch3047") {
    TEST_CASE("batch3047 load realm returns true immediately when matching stem is already registered") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Already3047", "Original.json") != nullptr);
        CHECK(manager.load_realm("Already3047.realm"));
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch3047 duplicate load by stem does not replace existing realm metadata") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Stem3047", "OriginalPath.json");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.load_realm("nested/Stem3047.realm"));
        CHECK(realm->get_path() == "OriginalPath.json");
        CHECK(realm->get_name() == "Stem3047");
    }

    TEST_CASE("batch3047 active realm selection remains exact and case sensitive") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Case3047", "Case.json");
        REQUIRE(realm != nullptr);
        CHECK_FALSE(manager.set_active_realm("case3047"));
        CHECK(manager.get_active_realm() == nullptr);
        REQUIRE(manager.set_active_realm("Case3047"));
        CHECK(manager.get_active_realm() == realm);
    }

    TEST_CASE("batch3047 empty selection attempt preserves an existing active realm") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Active3047", "Active.json");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.set_active_realm("Active3047"));
        CHECK_FALSE(manager.set_active_realm(""));
        CHECK(manager.get_active_realm() == realm);
    }

    TEST_CASE("batch3047 repeated project root assignments retain the latest nonempty value") {
        Salix::RealmManager manager;
        manager.set_project_root_path("Root/One");
        manager.set_project_root_path("Root/Two");
        manager.set_project_root_path("");
        CHECK(manager.get_project_root_path() == "Root/Two");
    }

    TEST_CASE("batch3047 reinitializing context can make an existing active realm loadable") {
        Salix::RealmManager manager;
        Salix::InitContext empty{};
        manager.initialize(empty);
        auto* realm = manager.create_realm("Load3047", "Load3047.json");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.set_active_realm("Load3047"));
        CHECK_FALSE(manager.load_active_realm());

        Salix::AssetManager assets;
        Salix::InitContext ready{};
        ready.asset_manager = &assets;
        manager.initialize(ready);
        CHECK(manager.load_active_realm());
        CHECK(manager.get_active_realm() == realm);
    }

    TEST_CASE("batch3047 replacing an inactive realm keeps manager with no implicit selection") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Replace3047", "Before.json") != nullptr);
        auto* replacement = manager.create_realm("Replace3047", "After.json");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == nullptr);
        CHECK(replacement->get_path() == "After.json");
    }

    TEST_CASE("batch3047 replacing active realm preserves configured project root") {
        Salix::RealmManager manager;
        manager.set_project_root_path("ProjectRoot3047");
        REQUIRE(manager.create_realm("World3047", "Before.json") != nullptr);
        REQUIRE(manager.set_active_realm("World3047"));
        auto* replacement = manager.create_realm("World3047", "After.json");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == replacement);
        CHECK(manager.get_project_root_path() == "ProjectRoot3047");
    }

    TEST_CASE("batch3047 shutdown removes duplicate load shortcut for previously registered realm name") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Gone3047", "Original.json") != nullptr);
        REQUIRE(manager.load_realm("Gone3047.realm"));
        manager.shutdown();
        CHECK_FALSE(manager.load_realm("__batch3047_missing__/Gone3047.realm"));
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch3047 failed missing file load never disrupts a selected realm") {
        Salix::RealmManager manager;
        auto* active = manager.create_realm("Stable3047", "Stable.json");
        REQUIRE(active != nullptr);
        REQUIRE(manager.set_active_realm("Stable3047"));
        CHECK_FALSE(manager.load_realm("__batch3047_missing__/Other3047.realm"));
        CHECK(manager.get_active_realm() == active);
        CHECK(active->get_path() == "Stable.json");
    }
}

TEST_SUITE("Salix::management::RealmManagerBatch4347") {
    TEST_CASE("batch4347 fresh project root is empty until first nonempty assignment") {
        Salix::RealmManager manager;
        CHECK(manager.get_project_root_path().empty());
        manager.set_project_root_path("Root4347");
        CHECK(manager.get_project_root_path() == "Root4347");
    }

    TEST_CASE("batch4347 project root storage reference remains stable across assignments") {
        Salix::RealmManager manager;
        const auto* storage = &manager.get_project_root_path();
        manager.set_project_root_path("First4347");
        manager.set_project_root_path("Second4347");
        CHECK(&manager.get_project_root_path() == storage);
        CHECK(*storage == "Second4347");
    }

    TEST_CASE("batch4347 realm creation preserves spaces and punctuation in metadata") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Realm 4347!", "Scenes/Realm 4347!.realm");
        REQUIRE(realm != nullptr);
        CHECK(realm->get_name() == "Realm 4347!");
        CHECK(realm->get_path() == "Scenes/Realm 4347!.realm");
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch4347 replacing inactive realm never creates implicit selection") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Replace4347", "Old.realm") != nullptr);
        auto* replacement = manager.create_realm("Replace4347", "New.realm");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == nullptr);
        CHECK(replacement->get_path() == "New.realm");
    }

    TEST_CASE("batch4347 replacing active realm retargets selection to latest object") {
        Salix::RealmManager manager;
        auto* first = manager.create_realm("Active4347", "Old.realm");
        REQUIRE(first != nullptr);
        REQUIRE(manager.set_active_realm("Active4347"));
        auto* replacement = manager.create_realm("Active4347", "New.realm");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == replacement);
        CHECK(replacement->get_path() == "New.realm");
    }

    TEST_CASE("batch4347 selection remains case sensitive and preserves active pointer on miss") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Case4347", "Case.realm");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.set_active_realm("Case4347"));
        CHECK_FALSE(manager.set_active_realm("case4347"));
        CHECK(manager.get_active_realm() == realm);
    }

    TEST_CASE("batch4347 shutdown permits fresh realm with same logical name afterward") {
        Salix::RealmManager manager;
        auto* first = manager.create_realm("Fresh4347", "Before.realm");
        REQUIRE(first != nullptr);
        REQUIRE(manager.set_active_realm("Fresh4347"));
        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
        auto* second = manager.create_realm("Fresh4347", "After.realm");
        REQUIRE(second != nullptr);
        CHECK(second->get_path() == "After.realm");
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch4347 duplicate load by registered stem returns true without replacing metadata") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Stem4347", "Original4347.realm");
        REQUIRE(realm != nullptr);
        CHECK(manager.load_realm("nested/path/Stem4347.realm"));
        CHECK(realm->get_name() == "Stem4347");
        CHECK(realm->get_path() == "Original4347.realm");
    }

    TEST_CASE("batch4347 active empty realm update and render are safe without initialization context") {
        Salix::RealmManager manager;
        auto* realm = manager.create_realm("Runtime4347", "Runtime4347.realm");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.set_active_realm("Runtime4347"));
        CHECK_NOTHROW(manager.update(0.0f));
        CHECK_NOTHROW(manager.update(-4347.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_realm() == realm);
    }

    TEST_CASE("batch4347 reinitializing with asset manager makes existing active realm loadable") {
        Salix::RealmManager manager;
        Salix::InitContext empty{};
        manager.initialize(empty);
        auto* realm = manager.create_realm("Load4347", "Load4347.realm");
        REQUIRE(realm != nullptr);
        REQUIRE(manager.set_active_realm("Load4347"));
        CHECK_FALSE(manager.load_active_realm());
        Salix::AssetManager assets;
        Salix::InitContext ready{};
        ready.asset_manager = &assets;
        manager.initialize(ready);
        CHECK(manager.load_active_realm());
        CHECK(manager.get_active_realm() == realm);
    }
}

TEST_SUITE("Salix::management::RealmManagerBatch5347") {
    TEST_CASE("batch5347 fresh manager starts without active realm or project root") {
        Salix::RealmManager manager;
        CHECK(manager.get_active_realm() == nullptr);
        CHECK(manager.get_project_root_path().empty());
    }

    TEST_CASE("batch5347 empty name or path realm creation is rejected") {
        Salix::RealmManager manager;
        CHECK(manager.create_realm("", "Scenes/Realm5347.realm") == nullptr);
        CHECK(manager.create_realm("Realm5347", "") == nullptr);
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch5347 distinct realm creation preserves independent metadata") {
        Salix::RealmManager manager;
        auto* first = manager.create_realm("First5347", "Scenes/First5347.realm");
        auto* second = manager.create_realm("Second5347", "Scenes/Second5347.realm");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(first->get_name() == "First5347");
        CHECK(second->get_path() == "Scenes/Second5347.realm");
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch5347 failed selection preserves existing active realm") {
        Salix::RealmManager manager;
        auto* first = manager.create_realm("First5347", "Scenes/First5347.realm");
        REQUIRE(first != nullptr);
        REQUIRE(manager.set_active_realm("First5347"));
        CHECK_FALSE(manager.set_active_realm("Missing5347"));
        CHECK(manager.get_active_realm() == first);
    }

    TEST_CASE("batch5347 replacing active realm retargets active pointer to replacement") {
        Salix::RealmManager manager;
        auto* original = manager.create_realm("Replace5347", "Scenes/Old5347.realm");
        REQUIRE(original != nullptr);
        REQUIRE(manager.set_active_realm("Replace5347"));
        auto* replacement = manager.create_realm("Replace5347", "Scenes/New5347.realm");
        REQUIRE(replacement != nullptr);
        CHECK(manager.get_active_realm() == replacement);
        CHECK(replacement->get_path() == "Scenes/New5347.realm");
    }

    TEST_CASE("batch5347 replacing inactive realm never changes selected realm") {
        Salix::RealmManager manager;
        auto* selected = manager.create_realm("Selected5347", "Scenes/Selected5347.realm");
        REQUIRE(selected != nullptr);
        REQUIRE(manager.create_realm("Other5347", "Scenes/OldOther5347.realm") != nullptr);
        REQUIRE(manager.set_active_realm("Selected5347"));
        REQUIRE(manager.create_realm("Other5347", "Scenes/NewOther5347.realm") != nullptr);
        CHECK(manager.get_active_realm() == selected);
    }

    TEST_CASE("batch5347 project root rejects empty replacement") {
        Salix::RealmManager manager;
        manager.set_project_root_path("C:/Batch5347/Root");
        manager.set_project_root_path("");
        CHECK(manager.get_project_root_path() == "C:/Batch5347/Root");
    }

    TEST_CASE("batch5347 update and render are no ops with no active realm") {
        Salix::RealmManager manager;
        CHECK_NOTHROW(manager.update(-5.347f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("batch5347 active realm cannot load assets without configured asset manager") {
        Salix::RealmManager manager;
        Salix::InitContext context{};
        manager.initialize(context);
        REQUIRE(manager.create_realm("NoAssets5347", "Scenes/NoAssets5347.realm") != nullptr);
        REQUIRE(manager.set_active_realm("NoAssets5347"));
        CHECK_FALSE(manager.load_active_realm());
    }

    TEST_CASE("batch5347 shutdown clears active selection and previously created realm lookup") {
        Salix::RealmManager manager;
        REQUIRE(manager.create_realm("Shutdown5347", "Scenes/Shutdown5347.realm") != nullptr);
        REQUIRE(manager.set_active_realm("Shutdown5347"));
        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
        CHECK_FALSE(manager.set_active_realm("Shutdown5347"));
    }
}

