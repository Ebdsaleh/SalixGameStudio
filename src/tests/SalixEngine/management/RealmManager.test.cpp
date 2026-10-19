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
