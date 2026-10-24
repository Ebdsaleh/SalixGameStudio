#include <doctest.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/core/InitContext.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
class Batch1947ProjectRoot {
public:
    Batch1947ProjectRoot() {
        static std::atomic<unsigned long> counter{0};
        path = std::filesystem::temp_directory_path() /
            ("SalixGameStudio_Batch1947_ProjectRoot_" + std::to_string(++counter));
        std::filesystem::create_directories(path);
    }
    ~Batch1947ProjectRoot() { std::error_code ec; std::filesystem::remove_all(path, ec); }
    std::filesystem::path path;
};
}

TEST_SUITE("Salix::management::ProjectManagerCoverage1947") {
    TEST_CASE("initialize is safe with a value initialized context and preserves selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("selected.salixproj");
        manager.set_loaded_project_directory("Root");
        Salix::InitContext context{};
        CHECK_NOTHROW(manager.initialize(context));
        CHECK(manager.get_project_to_load() == "selected.salixproj");
        CHECK(manager.get_loaded_project_directory() == "Root");
    }

    TEST_CASE("placeholder active project supports update and render dispatch safely") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK_NOTHROW(manager.update(0.016f));
        CHECK_NOTHROW(manager.render(nullptr));
    }

    TEST_CASE("shutdown is idempotent after placeholder project ownership") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored");
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK_NOTHROW(manager.shutdown());
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("simple project creation builds expected directory tree and project file") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Demo"));
        const auto project = root.path / "Demo";
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Scripts"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Images"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Audio"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Models"));
        CHECK(std::filesystem::is_regular_file(project / "Demo.salixproj"));
    }

    TEST_CASE("simple project file contains canonical starter metadata") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "ContentProbe"));
        std::ifstream in(root.path / "ContentProbe" / "ContentProbe.salixproj");
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text.find("ProjectName: ContentProbe") != std::string::npos);
        CHECK(text.find("StartingRealm: MainLevel") != std::string::npos);
    }

    TEST_CASE("creating same simple project twice preserves existing project file content") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Existing"));
        const auto file = root.path / "Existing" / "Existing.salixproj";
        { std::ofstream out(file, std::ios::trunc); out << "custom-content"; }
        REQUIRE(manager.create_new_project(root.path.string(), "Existing"));
        std::ifstream in(file);
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text == "custom-content");
    }

    TEST_CASE("simple project creation accepts empty project name inside isolated root") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), ""));
        CHECK(std::filesystem::is_directory(root.path / "Assets" / "Realms"));
        CHECK(std::filesystem::is_regular_file(root.path / ".salixproj"));
    }

    TEST_CASE("simple project creation fails when requested project root is blocked by a file") {
        Batch1947ProjectRoot root;
        const auto blocker = root.path / "Blocked";
        { std::ofstream out(blocker); out << "block"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_project(root.path.string(), "Blocked"));
        CHECK(std::filesystem::is_regular_file(blocker));
    }

    TEST_CASE("project-to-load getter returns a value copy independent from manager storage") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Original.salixproj");
        std::string copy = manager.get_project_to_load();
        copy = "Changed.salixproj";
        CHECK(manager.get_project_to_load() == "Original.salixproj");
    }

    TEST_CASE("loaded-directory getter returns a value copy independent from manager storage") {
        Salix::ProjectManager manager;
        manager.set_loaded_project_directory("OriginalRoot");
        std::string copy = manager.get_loaded_project_directory();
        copy = "ChangedRoot";
        CHECK(manager.get_loaded_project_directory() == "OriginalRoot");
    }
}
