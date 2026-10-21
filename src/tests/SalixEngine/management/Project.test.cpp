#include <doctest.h>
#include <Salix/management/Project.h>
#include <Salix/rendering/IRenderer.h>
#include <map>
#include <string>

TEST_SUITE("Salix::management::ProjectBatch1147") {
    TEST_CASE("default project derives stable development defaults") {
        Salix::Project project;
        CHECK(project.get_name() == "TestProject");
        CHECK(project.get_path() == "Sandbox/TestProject");
        CHECK(project.get_project_file_name() == "TestProject.salixproj");
        CHECK(project.get_engine_version() == "0.1.0-Alpha");
        CHECK(project.get_game_dll_name() == "TestProject.dll");
        CHECK(project.get_starting_realm().empty());
        CHECK(project.get_realm_paths().empty());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("named project derives project and DLL filenames without rewriting root path") {
        Salix::Project project("My Game", "D:/Projects/My Game");
        CHECK(project.get_name() == "My Game");
        CHECK(project.get_path() == "D:/Projects/My Game");
        CHECK(project.get_project_file_name() == "My Game.salixproj");
        CHECK(project.get_game_dll_name() == "My Game.dll");
        CHECK(project.get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("starting realm assignment replaces only starting realm metadata") {
        Salix::Project project("Game", "Root");
        project.set_starting_realm("Menu");
        CHECK(project.get_starting_realm() == "Menu");
        CHECK(project.get_name() == "Game");
        CHECK(project.get_path() == "Root");
        project.set_starting_realm("World");
        CHECK(project.get_starting_realm() == "World");
    }

    TEST_CASE("realm paths are indexed by realm name and preserve independent values") {
        Salix::Project project;
        project.add_realm_path("Menu", "Assets/Realms/Menu.realm");
        project.add_realm_path("World", "Assets/Realms/World.realm");
        const auto& paths = project.get_realm_paths();
        REQUIRE(paths.size() == 2);
        CHECK(paths.at("Menu") == "Assets/Realms/Menu.realm");
        CHECK(paths.at("World") == "Assets/Realms/World.realm");
    }

    TEST_CASE("adding a realm with an existing name replaces its path rather than duplicating the realm") {
        Salix::Project project;
        project.add_realm_path("World", "old.realm");
        project.add_realm_path("World", "new.realm");
        const auto& paths = project.get_realm_paths();
        REQUIRE(paths.size() == 1);
        CHECK(paths.at("World") == "new.realm");
    }

    TEST_CASE("remove realm path removes the matching map value even when file is absent on disk") {
        Salix::Project project;
        const std::string missing = "Batch1147/definitely_missing/World.realm";
        project.add_realm_path("World", missing);
        CHECK(project.remove_realm_path(missing));
        CHECK(project.get_realm_paths().empty());
    }

    TEST_CASE("remove realm path returns false without disturbing unrelated mappings") {
        Salix::Project project;
        project.add_realm_path("Menu", "menu.realm");
        project.add_realm_path("World", "world.realm");
        CHECK_FALSE(project.remove_realm_path("missing.realm"));
        REQUIRE(project.get_realm_paths().size() == 2);
        CHECK(project.get_realm_paths().at("Menu") == "menu.realm");
        CHECK(project.get_realm_paths().at("World") == "world.realm");
    }

    TEST_CASE("load starting realm fails cleanly when no starting realm was configured") {
        Salix::Project project;
        CHECK(project.get_realm_manager() == nullptr);
        CHECK_FALSE(project.load_starting_realm());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("lifecycle update render and shutdown are no-ops before project initialization") {
        Salix::Project project;
        CHECK_NOTHROW(project.update(0.016f));
        CHECK_NOTHROW(project.update(-1.0f));
        CHECK_NOTHROW(project.render(nullptr));
        CHECK_NOTHROW(project.shutdown());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("default realm creation is safely ignored until a realm manager exists") {
        Salix::Project project;
        project.add_realm_path("Default", "Default.realm");
        project.set_starting_realm("Default");
        CHECK_NOTHROW(project.create_and_save_default_realm());
        CHECK(project.get_realm_manager() == nullptr);
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_starting_realm() == "Default");
    }
}
