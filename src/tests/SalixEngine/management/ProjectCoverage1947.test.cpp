#include <doctest.h>
#include <Salix/management/Project.h>
#include <Salix/management/RealmManager.h>
#include <Salix/core/InitContext.h>
#include <filesystem>
#include <fstream>
#include <string>

TEST_SUITE("Salix::management::ProjectCoverage1947") {
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
