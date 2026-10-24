#include <doctest.h>
#include <Salix/management/Project.h>
#include <Salix/rendering/IRenderer.h>
#include <map>
#include <string>
#include <Salix/management/RealmManager.h>
#include <Salix/core/InitContext.h>
#include <filesystem>
#include <fstream>

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

TEST_SUITE("Salix::management::ProjectBatch1947") {
    TEST_CASE("empty project name still derives filename suffixes deterministically") {
        Salix::Project project("", "Root");
        CHECK(project.get_name().empty());
        CHECK(project.get_path() == "Root");
        CHECK(project.get_project_file_name() == ".salixproj");
        CHECK(project.get_game_dll_name() == ".dll");
    }

    TEST_CASE("realm map accepts empty key and path as explicit metadata") {
        Salix::Project project;
        project.add_realm_path("", "");
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_realm_paths().at("").empty());
    }

    TEST_CASE("removing one duplicated path removes only the first matching realm mapping") {
        Salix::Project project;
        const std::string path = "__batch1947_shared_missing.realm";
        project.add_realm_path("A", path);
        project.add_realm_path("B", path);
        REQUIRE(project.remove_realm_path(path));
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_realm_paths().begin()->second == path);
    }

    TEST_CASE("removing a mapped path succeeds when the physical file exists") {
        const auto path = std::filesystem::temp_directory_path() / "SalixGameStudio_Batch1947_existing.realm";
        { std::ofstream out(path); out << "{}"; }
        Salix::Project project;
        project.add_realm_path("Existing", path.string());
        CHECK(project.remove_realm_path(path.string()));
        CHECK(project.get_realm_paths().empty());
        std::error_code ec; std::filesystem::remove(path, ec);
    }

    TEST_CASE("initialize constructs a realm manager even when optional services and game DLL are absent") {
        Salix::Project project("Batch1947MissingDllA", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK(project.get_realm_manager() != nullptr);
        project.shutdown();
    }

    TEST_CASE("initialize materializes configured realm shells before missing DLL return") {
        Salix::Project project("Batch1947MissingDllB", "Root");
        project.add_realm_path("Menu", "Assets/Realms/Menu.realm");
        project.set_starting_realm("Menu");
        Salix::InitContext context{};
        project.initialize(context);
        REQUIRE(project.get_realm_manager() != nullptr);
        CHECK(project.load_starting_realm());
        project.shutdown();
    }

    TEST_CASE("shutdown after initialization releases owned realm manager") {
        Salix::Project project("Batch1947MissingDllC", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        REQUIRE(project.get_realm_manager() != nullptr);
        project.shutdown();
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("update and render remain safe after initialization with no active realm") {
        Salix::Project project("Batch1947MissingDllD", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK_NOTHROW(project.update(0.5f));
        CHECK_NOTHROW(project.render(nullptr));
        project.shutdown();
    }

    TEST_CASE("project can be initialized again after a completed shutdown") {
        Salix::Project project("Batch1947MissingDllE", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        project.shutdown();
        REQUIRE(project.get_realm_manager() == nullptr);
        project.initialize(context);
        CHECK(project.get_realm_manager() != nullptr);
        project.shutdown();
    }

    TEST_CASE("latest starting realm selection is used after initialization") {
        Salix::Project project("Batch1947MissingDllF", "Root");
        project.add_realm_path("First", "First.realm");
        project.add_realm_path("Second", "Second.realm");
        project.set_starting_realm("First");
        project.set_starting_realm("Second");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK(project.get_starting_realm() == "Second");
        CHECK(project.load_starting_realm());
        project.shutdown();
    }
}
