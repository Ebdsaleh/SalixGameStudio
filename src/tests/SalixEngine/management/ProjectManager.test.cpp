#include <doctest.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <string>
#include <Salix/core/InitContext.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>

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

TEST_SUITE("Salix::management::ProjectManagerBatch1947") {
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

TEST_SUITE("Salix::management::ProjectManagerBatch2247") {
    TEST_CASE("batch2247 initialization leaves fresh manager without an active project") {
        Salix::ProjectManager manager; Salix::InitContext context{}; manager.initialize(context);
        CHECK(manager.get_active_project()==nullptr);
    }
    TEST_CASE("batch2247 project selection metadata supports spaces and punctuation exactly") {
        Salix::ProjectManager manager; manager.set_project_to_load("D:/My Games/Project One/Game.salixproj"); manager.set_loaded_project_directory("D:/My Games/Project One");
        CHECK(manager.get_project_to_load()=="D:/My Games/Project One/Game.salixproj"); CHECK(manager.get_loaded_project_directory()=="D:/My Games/Project One");
    }
    TEST_CASE("batch2247 placeholder active project remains safe for repeated update and null render calls") {
        Salix::ProjectManager manager; manager.set_active_project("placeholder"); REQUIRE(manager.get_active_project()!=nullptr);
        CHECK_NOTHROW(manager.update(0.f)); CHECK_NOTHROW(manager.update(-100.f)); CHECK_NOTHROW(manager.render(nullptr)); CHECK(manager.get_active_project()!=nullptr);
    }
    TEST_CASE("batch2247 shutdown clears active project while retaining pending path metadata") {
        Salix::ProjectManager manager; manager.set_project_to_load("pending2247.salixproj"); manager.set_loaded_project_directory("pending2247"); manager.set_active_project("placeholder");
        manager.shutdown(); CHECK(manager.get_active_project()==nullptr); CHECK(manager.get_project_to_load()=="pending2247.salixproj"); CHECK(manager.get_loaded_project_directory()=="pending2247");
    }
    TEST_CASE("batch2247 simple project creation supports spaces in project name and creates canonical file") {
        Batch1947ProjectRoot root; Salix::ProjectManager manager; REQUIRE(manager.create_new_project(root.path.string(),"Project 2247"));
        const auto project=root.path/"Project 2247"; CHECK(std::filesystem::is_directory(project/"Assets"/"Realms")); CHECK(std::filesystem::is_regular_file(project/"Project 2247.salixproj"));
    }
    TEST_CASE("batch2247 repeating project creation preserves an existing custom project file") {
        Batch1947ProjectRoot root; Salix::ProjectManager manager; REQUIRE(manager.create_new_project(root.path.string(),"Repeat2247"));
        const auto file=root.path/"Repeat2247"/"Repeat2247.salixproj"; {std::ofstream out(file,std::ios::trunc); out<<"custom-2247";}
        REQUIRE(manager.create_new_project(root.path.string(),"Repeat2247")); std::ifstream in(file); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>()); CHECK(text=="custom-2247");
    }
    TEST_CASE("batch2247 project creation fails when project directory path is occupied by a file") {
        Batch1947ProjectRoot root; const auto blocker=root.path/"Blocked2247"; {std::ofstream out(blocker); out<<"block";}
        Salix::ProjectManager manager; CHECK_FALSE(manager.create_new_project(root.path.string(),"Blocked2247")); CHECK(std::filesystem::is_regular_file(blocker));
    }
    TEST_CASE("batch2247 malformed existing project file makes load_project return false") {
        Batch1947ProjectRoot root; const auto file=root.path/"malformed2247.salixproj"; {std::ofstream out(file); out<<"not valid cereal json";}
        Salix::ProjectManager manager; CHECK_FALSE(manager.load_project(file.string())); CHECK(manager.get_active_project()==nullptr);
    }
    TEST_CASE("batch2247 malformed existing project file makes pointer loader return null") {
        Batch1947ProjectRoot root; const auto file=root.path/"malformed_ptr2247.salixproj"; {std::ofstream out(file); out<<"{";}
        Salix::ProjectManager manager; CHECK(manager.load_project_from_file(file.string())==nullptr); CHECK(manager.get_active_project()==nullptr);
    }
    TEST_CASE("batch2247 missing project load fails before disturbing an existing placeholder project") {
        Salix::ProjectManager manager; manager.set_active_project("placeholder"); auto* active=manager.get_active_project(); REQUIRE(active!=nullptr);
        CHECK_FALSE(manager.load_project("Batch2247_definitely_missing.salixproj")); CHECK(manager.get_active_project()==active);
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch2847") {
    TEST_CASE("batch2847 initialized fresh manager keeps empty selection metadata") {
        Salix::ProjectManager manager; Salix::InitContext context{};
        manager.initialize(context);
        CHECK(manager.get_active_project()==nullptr);
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory().empty());
    }
    TEST_CASE("batch2847 project file selection supports spaces and punctuation exactly") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("D:/Projects/My Game-2847/My Game.salixproj");
        CHECK(manager.get_project_to_load()=="D:/Projects/My Game-2847/My Game.salixproj");
    }
    TEST_CASE("batch2847 loaded directory replacement never alters selected project file") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Root/Game.salixproj");
        manager.set_loaded_project_directory("Root");
        manager.set_loaded_project_directory("Other Root");
        CHECK(manager.get_project_to_load()=="Root/Game.salixproj");
        CHECK(manager.get_loaded_project_directory()=="Other Root");
    }
    TEST_CASE("batch2847 simple project creation builds every canonical asset directory") {
        Batch1947ProjectRoot root; Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(),"Tree2847"));
        const auto p=root.path/"Tree2847"/"Assets";
        CHECK(std::filesystem::is_directory(p/"Realms"));
        CHECK(std::filesystem::is_directory(p/"Scripts"));
        CHECK(std::filesystem::is_directory(p/"Images"));
        CHECK(std::filesystem::is_directory(p/"Audio"));
        CHECK(std::filesystem::is_directory(p/"Models"));
    }
    TEST_CASE("batch2847 generated project file contains project name and starting realm metadata") {
        Batch1947ProjectRoot root; Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(),"Metadata2847"));
        std::ifstream in(root.path/"Metadata2847"/"Metadata2847.salixproj");
        REQUIRE(in.is_open());
        const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("ProjectName: Metadata2847")!=std::string::npos);
        CHECK(text.find("StartingRealm: MainLevel")!=std::string::npos);
    }
    TEST_CASE("batch2847 recreating an existing project never overwrites its project file") {
        Batch1947ProjectRoot root; Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(),"Existing2847"));
        const auto file=root.path/"Existing2847"/"Existing2847.salixproj";
        { std::ofstream out(file,std::ios::trunc); out<<"custom2847"; }
        REQUIRE(manager.create_new_project(root.path.string(),"Existing2847"));
        std::ifstream in(file); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text=="custom2847");
    }
    TEST_CASE("batch2847 project creation works inside an already existing nested root") {
        Batch1947ProjectRoot root; const auto nested=root.path/"Nested"/"Workspace";
        std::filesystem::create_directories(nested);
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(nested.string(),"Nested2847"));
        CHECK(std::filesystem::is_regular_file(nested/"Nested2847"/"Nested2847.salixproj"));
    }
    TEST_CASE("batch2847 project creation fails when project directory is blocked by a regular file") {
        Batch1947ProjectRoot root; const auto blocker=root.path/"Blocked2847";
        { std::ofstream out(blocker); out<<"block"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_project(root.path.string(),"Blocked2847"));
        CHECK(std::filesystem::is_regular_file(blocker));
    }
    TEST_CASE("batch2847 malformed project load preserves pending path metadata") {
        Batch1947ProjectRoot root; const auto file=root.path/"malformed2847.salixproj";
        { std::ofstream out(file); out<<"not-json"; }
        Salix::ProjectManager manager; manager.set_project_to_load("pending2847.salixproj"); manager.set_loaded_project_directory("pending2847");
        CHECK_FALSE(manager.load_project(file.string()));
        CHECK(manager.get_project_to_load()=="pending2847.salixproj");
        CHECK(manager.get_loaded_project_directory()=="pending2847");
    }
    TEST_CASE("batch2847 malformed pointer load returns null without installing an active project") {
        Batch1947ProjectRoot root; const auto file=root.path/"malformed-pointer2847.salixproj";
        { std::ofstream out(file); out<<"{"; }
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file(file.string())==nullptr);
        CHECK(manager.get_active_project()==nullptr);
    }
}

