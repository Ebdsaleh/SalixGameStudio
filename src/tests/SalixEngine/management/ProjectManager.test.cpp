#include <doctest.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <string>

TEST_SUITE("Salix::management::ProjectManagerBatch1147") {
    TEST_CASE("fresh manager has no active project or pending project paths") {
        Salix::ProjectManager manager;
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory().empty());
    }

    TEST_CASE("project to load path can be assigned replaced and cleared") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("D:/Games/First/First.salixproj");
        CHECK(manager.get_project_to_load() == "D:/Games/First/First.salixproj");
        manager.set_project_to_load("D:/Games/Second/Second.salixproj");
        CHECK(manager.get_project_to_load() == "D:/Games/Second/Second.salixproj");
        manager.set_project_to_load("");
        CHECK(manager.get_project_to_load().empty());
    }

    TEST_CASE("loaded project directory can be assigned independently from project file selection") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Root/Game.salixproj");
        manager.set_loaded_project_directory("Root");
        CHECK(manager.get_project_to_load() == "Root/Game.salixproj");
        CHECK(manager.get_loaded_project_directory() == "Root");
        manager.set_loaded_project_directory("OtherRoot");
        CHECK(manager.get_project_to_load() == "Root/Game.salixproj");
        CHECK(manager.get_loaded_project_directory() == "OtherRoot");
    }

    TEST_CASE("update and render are safe while no project is active") {
        Salix::ProjectManager manager;
        CHECK_NOTHROW(manager.update(0.016f));
        CHECK_NOTHROW(manager.update(-2.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("shutdown without an active project leaves path selections intact") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("pending.salixproj");
        manager.set_loaded_project_directory("pending-root");
        CHECK_NOTHROW(manager.shutdown());
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "pending.salixproj");
        CHECK(manager.get_loaded_project_directory() == "pending-root");
    }

    TEST_CASE("placeholder set active project creates the documented default Project object") {
        Salix::ProjectManager manager;
        manager.set_active_project("IgnoredForNow");
        Salix::Project* project = manager.get_active_project();
        REQUIRE(project != nullptr);
        CHECK(project->get_name() == "TestProject");
        CHECK(project->get_path() == "Sandbox/TestProject");
        CHECK(project->get_project_file_name() == "TestProject.salixproj");
    }

    TEST_CASE("shutdown releases a placeholder active project") {
        Salix::ProjectManager manager;
        manager.set_active_project("Project");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("missing project file makes load project fail without creating an active project") {
        Salix::ProjectManager manager;
        const std::string missing = "Batch1147_missing_project_file.salixproj";
        CHECK_FALSE(manager.load_project(missing));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("missing project file makes pointer loading fail with null result") {
        Salix::ProjectManager manager;
        const std::string missing = "Batch1147_missing_project_from_file.salixproj";
        CHECK(manager.load_project_from_file(missing) == nullptr);
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("failed load does not erase manager path-selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("selected.salixproj");
        manager.set_loaded_project_directory("selected-root");
        CHECK_FALSE(manager.load_project("Batch1147_missing_selected_project.salixproj"));
        CHECK(manager.get_project_to_load() == "selected.salixproj");
        CHECK(manager.get_loaded_project_directory() == "selected-root");
        CHECK(manager.get_active_project() == nullptr);
    }
}
