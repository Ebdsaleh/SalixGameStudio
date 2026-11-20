#include <doctest.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <string>
#include <Salix/core/InitContext.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <algorithm>

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


TEST_SUITE("Salix::management::ProjectManagerBatch3047") {
    TEST_CASE("batch3047 repeated initialization preserves pending file and directory selections") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected3047.salixproj");
        manager.set_loaded_project_directory("Root3047");
        Salix::InitContext first{}, second{};
        manager.initialize(first);
        manager.initialize(second);
        CHECK(manager.get_project_to_load() == "Selected3047.salixproj");
        CHECK(manager.get_loaded_project_directory() == "Root3047");
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch3047 missing project load leaves an existing placeholder project installed") {
        Salix::ProjectManager manager;
        manager.set_active_project("Placeholder");
        auto* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK_FALSE(manager.load_project("__batch3047_missing_project__.salixproj"));
        CHECK(manager.get_active_project() == before);
        CHECK(manager.get_active_project()->get_name() == "TestProject");
    }

    TEST_CASE("batch3047 missing pointer load leaves an existing placeholder project installed") {
        Salix::ProjectManager manager;
        manager.set_active_project("Placeholder");
        auto* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file("__batch3047_missing_pointer_project__.salixproj") == nullptr);
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch3047 shutdown after placeholder ownership preserves path selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending3047.salixproj");
        manager.set_loaded_project_directory("PendingRoot3047");
        manager.set_active_project("Placeholder");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Pending3047.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot3047");
    }

    TEST_CASE("batch3047 placeholder project can be recreated after shutdown with canonical metadata") {
        Salix::ProjectManager manager;
        manager.set_active_project("First");
        manager.shutdown();
        manager.set_active_project("Second");
        auto* project = manager.get_active_project();
        REQUIRE(project != nullptr);
        CHECK(project->get_name() == "TestProject");
        CHECK(project->get_path() == "Sandbox/TestProject");
        CHECK(project->get_project_file_name() == "TestProject.salixproj");
    }

    TEST_CASE("batch3047 simple project creation preserves spaces in directory and file names") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Project 3047"));
        const auto project = root.path / "Project 3047";
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_regular_file(project / "Project 3047.salixproj"));
    }

    TEST_CASE("batch3047 two simple projects created under one root remain independent") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Alpha3047"));
        REQUIRE(manager.create_new_project(root.path.string(), "Beta3047"));
        CHECK(std::filesystem::is_regular_file(root.path / "Alpha3047" / "Alpha3047.salixproj"));
        CHECK(std::filesystem::is_regular_file(root.path / "Beta3047" / "Beta3047.salixproj"));
    }

    TEST_CASE("batch3047 simple project creation works beneath an already existing nested root") {
        Batch1947ProjectRoot root;
        const auto nested = root.path / "Existing" / "Nested";
        std::filesystem::create_directories(nested);
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(nested.string(), "Child3047"));
        CHECK(std::filesystem::is_directory(nested / "Child3047" / "Assets" / "Models"));
        CHECK(std::filesystem::is_regular_file(nested / "Child3047" / "Child3047.salixproj"));
    }

    TEST_CASE("batch3047 simple project punctuation remains exact in generated starter metadata") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Project-3047_v1";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        std::ifstream in(root.path / name / (name + ".salixproj"));
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text.find("ProjectName: " + name) != std::string::npos);
        CHECK(text.find("StartingRealm: MainLevel") != std::string::npos);
    }

    TEST_CASE("batch3047 update and render remain safe across placeholder shutdown and recreation") {
        Salix::ProjectManager manager;
        manager.set_active_project("One");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK_NOTHROW(manager.update(0.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        manager.shutdown();
        CHECK_NOTHROW(manager.update(-1.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        manager.set_active_project("Two");
        CHECK_NOTHROW(manager.update(1.0f));
        CHECK_NOTHROW(manager.render(nullptr));
    }
}
TEST_SUITE("Salix::management::ProjectManagerBatch3847") {
    TEST_CASE("batch3847 project to load preserves embedded null bytes as owned metadata") {
        Salix::ProjectManager manager;
        const std::string value("Project\0Load3847.salixproj", 26);
        manager.set_project_to_load(value);
        CHECK(manager.get_project_to_load() == value);
        CHECK(manager.get_project_to_load().size() == value.size());
    }

    TEST_CASE("batch3847 loaded project directory preserves embedded null bytes") {
        Salix::ProjectManager manager;
        const std::string value("Root\0Directory3847", 18);
        manager.set_loaded_project_directory(value);
        CHECK(manager.get_loaded_project_directory() == value);
        CHECK(manager.get_loaded_project_directory().size() == value.size());
    }

    TEST_CASE("batch3847 empty metadata setters clear their fields independently") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending3847.salixproj");
        manager.set_loaded_project_directory("Root3847");
        manager.set_project_to_load("");
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory() == "Root3847");
        manager.set_loaded_project_directory("");
        CHECK(manager.get_loaded_project_directory().empty());
    }

    TEST_CASE("batch3847 initialize preserves an existing placeholder active project") {
        Salix::ProjectManager manager;
        manager.set_active_project("Placeholder3847");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        Salix::InitContext context{};
        manager.initialize(context);
        CHECK(manager.get_active_project() == active);
        CHECK(manager.get_active_project()->get_name() == "TestProject");
    }

    TEST_CASE("batch3847 placeholder setter ignores requested name and exposes canonical placeholder metadata") {
        Salix::ProjectManager manager;
        manager.set_active_project("Requested Name 3847!");
        auto* project = manager.get_active_project();
        REQUIRE(project != nullptr);
        CHECK(project->get_name() == "TestProject");
        CHECK(project->get_path() == "Sandbox/TestProject");
        CHECK(project->get_project_file_name() == "TestProject.salixproj");
    }

    TEST_CASE("batch3847 replacing placeholder discards prior placeholder persistent mutations") {
        Salix::ProjectManager manager;
        manager.set_active_project("First3847");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.get_active_project()->set_starting_realm("MutatedRealm3847");
        manager.get_active_project()->add_realm_path(
            "Realm3847", "Assets/Realms/Realm3847.realm");
        REQUIRE(manager.get_active_project()->get_starting_realm() == "MutatedRealm3847");
        REQUIRE(manager.get_active_project()->get_realm_paths().size() == 1);

        manager.set_active_project("Second3847");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_name() == "TestProject");
        CHECK(manager.get_active_project()->get_starting_realm().empty());
        CHECK(manager.get_active_project()->get_realm_paths().empty());
    }

    TEST_CASE("batch3847 blocked project creation preserves selection metadata and active placeholder") {
        Batch1947ProjectRoot root;
        const auto blocker = root.path / "Blocked3847";
        { std::ofstream out(blocker); out << "block"; }
        Salix::ProjectManager manager;
        manager.set_project_to_load("Keep3847.salixproj");
        manager.set_loaded_project_directory("KeepRoot3847");
        manager.set_active_project("KeepActive3847");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        CHECK_FALSE(manager.create_new_project(root.path.string(), "Blocked3847"));
        CHECK(manager.get_project_to_load() == "Keep3847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "KeepRoot3847");
        CHECK(manager.get_active_project() == active);
    }

    TEST_CASE("batch3847 simple project creation does not implicitly install an active project") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.get_active_project() == nullptr);
        REQUIRE(manager.create_new_project(root.path.string(), "NoActivate3847"));
        CHECK(manager.get_active_project() == nullptr);
        CHECK(std::filesystem::is_regular_file(
            root.path / "NoActivate3847" / "NoActivate3847.salixproj"));
    }

    TEST_CASE("batch3847 separate managers keep project selection metadata independent") {
        Salix::ProjectManager first;
        Salix::ProjectManager second;
        first.set_project_to_load("First3847.salixproj");
        first.set_loaded_project_directory("FirstRoot3847");
        second.set_project_to_load("Second3847.salixproj");
        second.set_loaded_project_directory("SecondRoot3847");
        CHECK(first.get_project_to_load() == "First3847.salixproj");
        CHECK(first.get_loaded_project_directory() == "FirstRoot3847");
        CHECK(second.get_project_to_load() == "Second3847.salixproj");
        CHECK(second.get_loaded_project_directory() == "SecondRoot3847");
    }

    TEST_CASE("batch3847 shutdown with no active project preserves metadata and lifecycle no ops remain safe") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending3847.salixproj");
        manager.set_loaded_project_directory("PendingRoot3847");
        REQUIRE(manager.get_active_project() == nullptr);
        manager.shutdown();
        CHECK(manager.get_project_to_load() == "Pending3847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot3847");
        CHECK_NOTHROW(manager.update(-3847.0f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_project() == nullptr);
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch4447") {
    TEST_CASE("batch4447 project to load setter copies caller storage") {
        Salix::ProjectManager manager;
        std::string value = "Root/Project4447.salixproj";
        manager.set_project_to_load(value);
        value = "MutatedCaller";
        CHECK(manager.get_project_to_load() == "Root/Project4447.salixproj");
    }

    TEST_CASE("batch4447 loaded project directory setter copies caller storage") {
        Salix::ProjectManager manager;
        std::string value = "Root/Directory4447";
        manager.set_loaded_project_directory(value);
        value.clear();
        CHECK(manager.get_loaded_project_directory() == "Root/Directory4447");
    }

    TEST_CASE("batch4447 selection metadata preserves whitespace and control characters") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("\t Project 4447.salixproj \n");
        manager.set_loaded_project_directory(" Root 4447 ");
        CHECK(manager.get_project_to_load() == "\t Project 4447.salixproj \n");
        CHECK(manager.get_loaded_project_directory() == " Root 4447 ");
    }

    TEST_CASE("batch4447 placeholder active pointer remains stable through update and render") {
        Salix::ProjectManager manager;
        manager.set_active_project("Placeholder4447");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        manager.update(0.0f);
        manager.update(-4447.0f);
        manager.render(nullptr);
        CHECK(manager.get_active_project() == active);
    }

    TEST_CASE("batch4447 repeated initialize preserves selection metadata and placeholder ownership") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected4447.salixproj");
        manager.set_loaded_project_directory("SelectedRoot4447");
        manager.set_active_project("Active4447");
        auto* active = manager.get_active_project();
        Salix::InitContext context{};
        manager.initialize(context);
        manager.initialize(context);
        CHECK(manager.get_project_to_load() == "Selected4447.salixproj");
        CHECK(manager.get_loaded_project_directory() == "SelectedRoot4447");
        CHECK(manager.get_active_project() == active);
    }

    TEST_CASE("batch4447 shutdown after active placeholder preserves selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending4447.salixproj");
        manager.set_loaded_project_directory("PendingRoot4447");
        manager.set_active_project("Active4447");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Pending4447.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot4447");
    }

    TEST_CASE("batch4447 replacing placeholder resets prior persistent project mutations") {
        Salix::ProjectManager manager;
        manager.set_active_project("First4447");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.get_active_project()->set_starting_realm("Mutated4447");
        manager.get_active_project()->add_realm_path("Mutated4447", "Mutated4447.realm");
        manager.set_active_project("Second4447");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_starting_realm().empty());
        CHECK(manager.get_active_project()->get_realm_paths().empty());
    }

    TEST_CASE("batch4447 separate managers own independent placeholder projects") {
        Salix::ProjectManager first;
        Salix::ProjectManager second;
        first.set_active_project("First4447");
        second.set_active_project("Second4447");
        REQUIRE(first.get_active_project() != nullptr);
        REQUIRE(second.get_active_project() != nullptr);
        CHECK(first.get_active_project() != second.get_active_project());
        first.get_active_project()->set_starting_realm("OnlyFirst4447");
        CHECK(second.get_active_project()->get_starting_realm().empty());
    }

    TEST_CASE("batch4447 project creation accepts dot hyphen and underscore name") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Game-4447_v2.1";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        const auto project = root.path / name;
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_regular_file(project / (name + ".salixproj")));
    }

    TEST_CASE("batch4447 recreating project preserves unrelated user file under project tree") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Preserve4447"));
        const auto custom = root.path / "Preserve4447" / "Assets" / "custom-user-file.txt";
        { std::ofstream out(custom); out << "user-content-4447"; }
        REQUIRE(manager.create_new_project(root.path.string(), "Preserve4447"));
        REQUIRE(std::filesystem::is_regular_file(custom));
        std::ifstream in(custom);
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text == "user-content-4447");
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch4947") {
    TEST_CASE("batch4947 selection metadata preserves embedded null bytes in independent strings") {
        Salix::ProjectManager manager;
        const std::string project("Project\0File4947", 16);
        const std::string directory("Root\0Dir4947", 12);
        manager.set_project_to_load(project);
        manager.set_loaded_project_directory(directory);
        CHECK(manager.get_project_to_load() == project);
        CHECK(manager.get_loaded_project_directory() == directory);
        CHECK(manager.get_project_to_load().size() == project.size());
        CHECK(manager.get_loaded_project_directory().size() == directory.size());
    }

    TEST_CASE("batch4947 placeholder update accepts infinite delta without changing ownership") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored4947");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        CHECK_NOTHROW(manager.update(std::numeric_limits<float>::infinity()));
        CHECK(manager.get_active_project() == active);
    }

    TEST_CASE("batch4947 successful project creation preserves manager selection metadata and active placeholder") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        manager.set_project_to_load("Keep4947.salixproj");
        manager.set_loaded_project_directory("KeepRoot4947");
        manager.set_active_project("KeepActive4947");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        REQUIRE(manager.create_new_project(root.path.string(), "Created4947"));
        CHECK(manager.get_project_to_load() == "Keep4947.salixproj");
        CHECK(manager.get_loaded_project_directory() == "KeepRoot4947");
        CHECK(manager.get_active_project() == active);
    }

    TEST_CASE("batch4947 project name containing spaces creates matching directory and project filename") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Project Name 4947";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        CHECK(std::filesystem::is_directory(root.path / name / "Assets" / "Realms"));
        CHECK(std::filesystem::is_regular_file(root.path / name / (name + ".salixproj")));
    }

    TEST_CASE("batch4947 generated starter project file has exact canonical three line content") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Exact4947"));
        std::ifstream in(root.path / "Exact4947" / "Exact4947.salixproj");
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text ==
            "// Salix Game Studio Project File\n"
            "ProjectName: Exact4947\n"
            "StartingRealm: MainLevel\n");
    }

    TEST_CASE("batch4947 recreating project restores deleted asset subdirectory without rewriting project file") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_project(root.path.string(), "Restore4947"));
        const auto project = root.path / "Restore4947";
        const auto file = project / "Restore4947.salixproj";
        { std::ofstream out(file, std::ios::trunc); out << "custom4947"; }
        std::filesystem::remove_all(project / "Assets" / "Audio");
        REQUIRE_FALSE(std::filesystem::exists(project / "Assets" / "Audio"));
        REQUIRE(manager.create_new_project(root.path.string(), "Restore4947"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Audio"));
        std::ifstream in(file);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text == "custom4947");
    }

    TEST_CASE("batch4947 file obstruction at first Realms directory stops project creation before later asset directories") {
        Batch1947ProjectRoot root;
        const auto project = root.path / "Blocked4947";
        std::filesystem::create_directories(project / "Assets");
        { std::ofstream out(project / "Assets" / "Realms"); out << "block"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_project(root.path.string(), "Blocked4947"));
        CHECK(std::filesystem::is_regular_file(project / "Assets" / "Realms"));
        CHECK_FALSE(std::filesystem::exists(project / "Assets" / "Scripts"));
        CHECK_FALSE(std::filesystem::exists(project / "Blocked4947.salixproj"));
    }

    TEST_CASE("batch4947 repeated placeholder replacement leaves selection metadata untouched") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected4947.salixproj");
        manager.set_loaded_project_directory("SelectedRoot4947");
        manager.set_active_project("First4947");
        manager.set_active_project("Second4947");
        manager.set_active_project("Third4947");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_project_to_load() == "Selected4947.salixproj");
        CHECK(manager.get_loaded_project_directory() == "SelectedRoot4947");
        CHECK(manager.get_active_project()->get_name() == "TestProject");
    }

    TEST_CASE("batch4947 shutdown after repeated placeholder replacement remains idempotent and preserves metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending4947.salixproj");
        manager.set_loaded_project_directory("PendingRoot4947");
        manager.set_active_project("One4947");
        manager.set_active_project("Two4947");
        manager.shutdown();
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Pending4947.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot4947");
    }

    TEST_CASE("batch4947 initialization after placeholder creation preserves exact active project pointer") {
        Salix::ProjectManager manager;
        manager.set_active_project("BeforeInit4947");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        Salix::InitContext context{};
        manager.initialize(context);
        CHECK(manager.get_active_project() == active);
        CHECK(manager.get_active_project()->get_name() == "TestProject");
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch5347") {
    TEST_CASE("batch5347 fresh manager exposes no active project or path metadata") {
        Salix::ProjectManager manager;
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory().empty());
    }

    TEST_CASE("batch5347 project selection strings replace independently") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("First5347.salixproj");
        manager.set_loaded_project_directory("RootA5347");
        manager.set_project_to_load("Second5347.salixproj");
        CHECK(manager.get_project_to_load() == "Second5347.salixproj");
        CHECK(manager.get_loaded_project_directory() == "RootA5347");
        manager.set_loaded_project_directory("RootB5347");
        CHECK(manager.get_project_to_load() == "Second5347.salixproj");
        CHECK(manager.get_loaded_project_directory() == "RootB5347");
    }

    TEST_CASE("batch5347 initialization preserves path selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected5347.salixproj");
        manager.set_loaded_project_directory("SelectedRoot5347");
        Salix::InitContext context{};
        CHECK_NOTHROW(manager.initialize(context));
        CHECK(manager.get_project_to_load() == "Selected5347.salixproj");
        CHECK(manager.get_loaded_project_directory() == "SelectedRoot5347");
    }

    TEST_CASE("batch5347 update and render remain harmless without active project") {
        Salix::ProjectManager manager;
        CHECK_NOTHROW(manager.update(-5.347f));
        CHECK_NOTHROW(manager.render(nullptr));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch5347 placeholder active project exposes documented default metadata") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored5347");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        CHECK(active->get_name() == "TestProject");
        CHECK(active->get_path() == "Sandbox/TestProject");
        CHECK(active->get_project_file_name() == "TestProject.salixproj");
    }

    TEST_CASE("batch5347 placeholder ownership does not modify selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Keep5347.salixproj");
        manager.set_loaded_project_directory("KeepRoot5347");
        manager.set_active_project("Ignored");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_project_to_load() == "Keep5347.salixproj");
        CHECK(manager.get_loaded_project_directory() == "KeepRoot5347");
    }

    TEST_CASE("batch5347 shutdown is repeatable and preserves selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending5347.salixproj");
        manager.set_loaded_project_directory("PendingRoot5347");
        manager.set_active_project("Owned5347");
        manager.shutdown();
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Pending5347.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot5347");
    }

    TEST_CASE("batch5347 missing project file boolean load fails without active ownership") {
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.load_project("__batch5347_missing__/Missing5347.salixproj"));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch5347 missing project file pointer load returns null without active ownership") {
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file("__batch5347_missing__/PointerMissing5347.salixproj") == nullptr);
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch5347 simple project creation with punctuation safe name builds canonical file") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Demo_5347";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        const auto project = root.path / name;
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Scripts"));
        CHECK(std::filesystem::is_regular_file(project / (name + ".salixproj")));
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch5847") {
    TEST_CASE("batch5847 fresh manager starts without ownership or selection metadata") {
        Salix::ProjectManager manager;
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory().empty());
    }

    TEST_CASE("batch5847 project and directory selections replace independently") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("First5847.salixproj");
        manager.set_loaded_project_directory("RootA5847");
        manager.set_project_to_load("Second5847.salixproj");
        CHECK(manager.get_project_to_load() == "Second5847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "RootA5847");
        manager.set_loaded_project_directory("RootB5847");
        CHECK(manager.get_loaded_project_directory() == "RootB5847");
    }

    TEST_CASE("batch5847 selection metadata preserves embedded null bytes") {
        Salix::ProjectManager manager;
        const std::string project("Project\0File5847", 16);
        const std::string directory("Root\0Dir5847", 12);
        manager.set_project_to_load(project);
        manager.set_loaded_project_directory(directory);
        CHECK(manager.get_project_to_load() == project);
        CHECK(manager.get_loaded_project_directory() == directory);
    }

    TEST_CASE("batch5847 initialize preserves existing selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected5847.salixproj");
        manager.set_loaded_project_directory("SelectedRoot5847");
        Salix::InitContext context{};
        CHECK_NOTHROW(manager.initialize(context));
        CHECK(manager.get_project_to_load() == "Selected5847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "SelectedRoot5847");
    }

    TEST_CASE("batch5847 placeholder active project retains documented defaults") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored5847");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        CHECK(active->get_name() == "TestProject");
        CHECK(active->get_path() == "Sandbox/TestProject");
        CHECK(active->get_project_file_name() == "TestProject.salixproj");
    }

    TEST_CASE("batch5847 replacing placeholder does not alter selection metadata") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Keep5847.salixproj");
        manager.set_loaded_project_directory("KeepRoot5847");
        manager.set_active_project("First5847");
        manager.set_active_project("Second5847");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_project_to_load() == "Keep5847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "KeepRoot5847");
    }

    TEST_CASE("batch5847 shutdown remains idempotent and preserves selections") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending5847.salixproj");
        manager.set_loaded_project_directory("PendingRoot5847");
        manager.set_active_project("Owned5847");
        manager.shutdown();
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Pending5847.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot5847");
    }

    TEST_CASE("batch5847 missing boolean project load fails without ownership") {
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.load_project("__batch5847_missing__/Missing5847.salixproj"));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch5847 missing pointer project load returns null without ownership") {
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file("__batch5847_missing__/Pointer5847.salixproj") == nullptr);
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch5847 temp project creation produces canonical core layout") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Demo_5847";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        const auto project = root.path / name;
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Scripts"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Audio"));
        CHECK(std::filesystem::is_regular_file(project / (name + ".salixproj")));
    }
}

TEST_SUITE("Salix::management::ProjectManagerBatch6247") {
    TEST_CASE("batch6247 project selection can be cleared without clearing loaded directory") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected6247.salixproj");
        manager.set_loaded_project_directory("Root6247");
        manager.set_project_to_load("");
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory() == "Root6247");
    }

    TEST_CASE("batch6247 loaded directory can be cleared without clearing project selection") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Selected6247.salixproj");
        manager.set_loaded_project_directory("Root6247");
        manager.set_loaded_project_directory("");
        CHECK(manager.get_project_to_load() == "Selected6247.salixproj");
        CHECK(manager.get_loaded_project_directory().empty());
    }

    TEST_CASE("batch6247 initialize with empty selections leaves them empty") {
        Salix::ProjectManager manager;
        Salix::InitContext context{};
        manager.initialize(context);
        CHECK(manager.get_project_to_load().empty());
        CHECK(manager.get_loaded_project_directory().empty());
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch6247 no project update accepts zero and negative deltas safely") {
        Salix::ProjectManager manager;
        CHECK_NOTHROW(manager.update(0.0f));
        CHECK_NOTHROW(manager.update(-62.47f));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch6247 placeholder project supports repeated update and null render dispatch") {
        Salix::ProjectManager manager;
        manager.set_active_project("Ignored6247");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK_NOTHROW(manager.update(0.0f));
        CHECK_NOTHROW(manager.update(0.25f));
        CHECK_NOTHROW(manager.render(nullptr));
    }

    TEST_CASE("batch6247 shutdown clears placeholder ownership but preserves both selections") {
        Salix::ProjectManager manager;
        manager.set_project_to_load("Keep6247.salixproj");
        manager.set_loaded_project_directory("KeepRoot6247");
        manager.set_active_project("Ignored6247");
        manager.shutdown();
        CHECK(manager.get_active_project() == nullptr);
        CHECK(manager.get_project_to_load() == "Keep6247.salixproj");
        CHECK(manager.get_loaded_project_directory() == "KeepRoot6247");
    }

    TEST_CASE("batch6247 placeholder can be reacquired after shutdown") {
        Salix::ProjectManager manager;
        manager.set_active_project("First6247");
        REQUIRE(manager.get_active_project() != nullptr);
        manager.shutdown();
        REQUIRE(manager.get_active_project() == nullptr);
        manager.set_active_project("Second6247");
        auto* active = manager.get_active_project();
        REQUIRE(active != nullptr);
        CHECK(active->get_name() == "TestProject");
    }

    TEST_CASE("batch6247 missing boolean load preserves existing placeholder ownership") {
        Salix::ProjectManager manager;
        manager.set_active_project("Keep6247");
        auto* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK_FALSE(manager.load_project("__batch6247_missing__/Missing6247.salixproj"));
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch6247 missing pointer load preserves existing placeholder ownership") {
        Salix::ProjectManager manager;
        manager.set_active_project("Keep6247");
        auto* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file("__batch6247_missing__/Pointer6247.salixproj") == nullptr);
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch6247 temp project creation produces all canonical asset directories") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        const std::string name = "Demo_6247";
        REQUIRE(manager.create_new_project(root.path.string(), name));
        const auto project = root.path / name;
        CHECK(std::filesystem::is_directory(project / "Assets" / "Realms"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Scripts"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Images"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Audio"));
        CHECK(std::filesystem::is_directory(project / "Assets" / "Models"));
        CHECK(std::filesystem::is_regular_file(project / (name + ".salixproj")));
    }
}

#include <Salix/management/ProjectConfig.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace {
std::filesystem::path batch6947_write_valid_project(
    Batch1947ProjectRoot& root,
    const std::string& file_stem,
    const std::string& project_name,
    const std::string& starting_realm = "",
    const std::vector<Salix::RealmInfo>& realms = {}) {
    Salix::ProjectConfig config;
    config.project_data.project_path = root.path.string();
    config.project_data.project_file_name = file_stem + ".salixproj";
    config.project_data.project_name = project_name;
    config.project_data.starting_realm = starting_realm;
    config.project_data.realms = realms;
    config.build_settings.game_dll_name = "__Batch6947_missing_game__.dll";
    const auto path = root.path / config.project_data.project_file_name;
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    cereal::JSONOutputArchive archive(out);
    archive(config);
    return path;
}
}

TEST_SUITE("Salix::management::ProjectManagerBatch6947") {
    TEST_CASE("batch6947 valid project file exercises successful load_project path") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947a", "Batch6947A");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(file.string()));
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_name() == "Batch6947A");
    }

    TEST_CASE("batch6947 successful load preserves configured project root path") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947b", "Batch6947B");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(file.string()));
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_path() == root.path.string());
    }

    TEST_CASE("batch6947 successful load applies configured starting realm text") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947c", "Batch6947C", "Start6947");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(file.string()));
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_starting_realm() == "Start6947");
    }

    TEST_CASE("batch6947 successful load copies configured realm path entries") {
        Batch1947ProjectRoot root;
        const std::vector<Salix::RealmInfo> realms{
            {"RealmA6947", "Assets/Realms/A6947.realm"},
            {"RealmB6947", "Assets/Realms/B6947.realm"}};
        const auto file = batch6947_write_valid_project(root, "valid6947d", "Batch6947D", "", realms);
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(file.string()));
        REQUIRE(manager.get_active_project() != nullptr);
        const auto& paths = manager.get_active_project()->get_realm_paths();
        CHECK(paths.at("RealmA6947") == "Assets/Realms/A6947.realm");
        CHECK(paths.at("RealmB6947") == "Assets/Realms/B6947.realm");
    }

    TEST_CASE("batch6947 successful load replaces placeholder project metadata") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947e", "Batch6947E");
        Salix::ProjectManager manager;
        manager.set_active_project("placeholder");
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_name() == "TestProject");
        REQUIRE(manager.load_project(file.string()));
        REQUIRE(manager.get_active_project() != nullptr);
        CHECK(manager.get_active_project()->get_name() == "Batch6947E");
    }

    TEST_CASE("batch6947 valid pointer loader returns active project identity") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947f", "Batch6947F");
        Salix::ProjectManager manager;
        Salix::Project* loaded = manager.load_project_from_file(file.string());
        REQUIRE(loaded != nullptr);
        CHECK(loaded == manager.get_active_project());
        CHECK(loaded->get_name() == "Batch6947F");
    }

    TEST_CASE("batch6947 pointer loader preserves starting realm and root metadata") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947g", "Batch6947G", "PointerStart6947");
        Salix::ProjectManager manager;
        Salix::Project* loaded = manager.load_project_from_file(file.string());
        REQUIRE(loaded != nullptr);
        CHECK(loaded->get_starting_realm() == "PointerStart6947");
        CHECK(loaded->get_path() == root.path.string());
    }

    TEST_CASE("batch6947 pointer loader copies multiple realm paths") {
        Batch1947ProjectRoot root;
        const std::vector<Salix::RealmInfo> realms{
            {"One6947", "One6947.realm"}, {"Two6947", "Two6947.realm"}};
        const auto file = batch6947_write_valid_project(root, "valid6947h", "Batch6947H", "", realms);
        Salix::ProjectManager manager;
        Salix::Project* loaded = manager.load_project_from_file(file.string());
        REQUIRE(loaded != nullptr);
        CHECK(loaded->get_realm_paths().size() == 2);
        CHECK(loaded->get_realm_paths().at("Two6947") == "Two6947.realm");
    }

    TEST_CASE("batch6947 repeated successful loads replace active project metadata") {
        Batch1947ProjectRoot root;
        const auto first = batch6947_write_valid_project(root, "valid6947i1", "Batch6947I1");
        const auto second = batch6947_write_valid_project(root, "valid6947i2", "Batch6947I2");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(first.string()));
        CHECK(manager.get_active_project()->get_name() == "Batch6947I1");
        REQUIRE(manager.load_project(second.string()));
        CHECK(manager.get_active_project()->get_name() == "Batch6947I2");
    }

    TEST_CASE("batch6947 successful load does not rewrite pending manager selection metadata") {
        Batch1947ProjectRoot root;
        const auto file = batch6947_write_valid_project(root, "valid6947j", "Batch6947J");
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending6947.salixproj");
        manager.set_loaded_project_directory("PendingRoot6947");
        REQUIRE(manager.load_project(file.string()));
        CHECK(manager.get_project_to_load() == "Pending6947.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot6947");
    }
}

namespace {
struct Batch7047ExternalProjectEnv {
    std::filesystem::path old_cwd = std::filesystem::current_path();
    std::filesystem::path root;
    std::filesystem::path projects;

    Batch7047ExternalProjectEnv() {
        static std::atomic<unsigned long> counter{0};
        root = std::filesystem::temp_directory_path() /
            ("SalixGameStudio_Batch7047_Project_" + std::to_string(++counter));
        projects = root / "Projects";
        std::filesystem::create_directories(projects);
        write_template(false);
        write_asset_templates();
        std::filesystem::current_path(root);
    }

    ~Batch7047ExternalProjectEnv() {
        std::error_code ec;
        std::filesystem::current_path(old_cwd, ec);
        std::filesystem::remove_all(root, ec);
    }

    std::filesystem::path template_root() const {
        return root / "src" / "Salix" / "resources" / "templates" / "default";
    }

    void write_template(bool include_default) {
        const auto dir = template_root();
        std::filesystem::create_directories(dir);
        Salix::ProjectConfig config;
        config.project_data.project_path = "TemplateRoot";
        config.project_data.project_file_name = "Template.salixproj";
        config.project_data.project_name = "TemplateProject";
        config.project_data.starting_realm = "TemplateStart";
        if (include_default)
            config.project_data.realms.emplace_back("Default", "TemplateDefault.realm");
        else
            config.project_data.realms.emplace_back("Existing7047", "Existing7047.realm");
        config.build_settings.game_dll_name = "Template.dll";
        std::ofstream out(dir / "project.json", std::ios::out | std::ios::trunc);
        cereal::JSONOutputArchive archive(out);
        archive(
            cereal::make_nvp("project_data", config.project_data),
            cereal::make_nvp("build_settings", config.build_settings));
    }

    void write_asset_templates() {
        const auto realm = template_root() / "Assets" / "Realms";
        const auto sprite = template_root() / "Assets" / "Images" / "Sprites";
        std::filesystem::create_directories(realm);
        std::filesystem::create_directories(sprite);
        { std::ofstream out(realm / "Default.realm"); out << "Batch7047 realm template"; }
        { std::ofstream out(sprite / "test.png", std::ios::binary); out << "Batch7047 sprite template"; }
    }

    Salix::ProjectConfig read_generated(const std::string& name, const std::filesystem::path& base = {}) const {
        const auto root_path = base.empty() ? projects : base;
        std::ifstream in(root_path / name / (name + ".salixproj"));
        Salix::ProjectConfig config;
        cereal::JSONInputArchive archive(in);
        archive(config);
        return config;
    }
};
}

TEST_SUITE("Salix::management::ProjectManagerBatch7047") {
    TEST_CASE("batch7047 external project creation succeeds with synthetic templates") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        CHECK(manager.create_new_external_project(env.projects.string(), "External7047A"));
    }

    TEST_CASE("batch7047 external project creation emits canonical project file") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047B"));
        CHECK(std::filesystem::is_regular_file(env.projects / "External7047B" / "External7047B.salixproj"));
    }

    TEST_CASE("batch7047 external project creation copies realm template") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047C"));
        const auto copied = env.projects / "External7047C" / "Assets" / "Realms" / "Default.realm";
        REQUIRE(std::filesystem::is_regular_file(copied));
        CHECK(Salix::FileManager::read_file_content(copied.string()) == "Batch7047 realm template");
    }

    TEST_CASE("batch7047 external project creation copies sprite template") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047D"));
        const auto copied = env.projects / "External7047D" / "Assets" / "Images" / "test.png";
        REQUIRE(std::filesystem::is_regular_file(copied));
        CHECK(Salix::FileManager::read_file_content(copied.string()) == "Batch7047 sprite template");
    }

    TEST_CASE("batch7047 generated project metadata uses requested name and dll") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047E"));
        const auto config = env.read_generated("External7047E");
        CHECK(config.project_data.project_name == "External7047E");
        CHECK(config.build_settings.game_dll_name == "External7047E.dll");
    }

    TEST_CASE("batch7047 generated project metadata stores absolute project root") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047F"));
        const auto config = env.read_generated("External7047F");
        CHECK(std::filesystem::path(config.project_data.project_path) ==
              std::filesystem::absolute(env.projects / "External7047F"));
    }

    TEST_CASE("batch7047 generated project always selects Default starting realm") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047G"));
        const auto config = env.read_generated("External7047G");
        CHECK(config.project_data.starting_realm == "Default");
    }

    TEST_CASE("batch7047 missing Default realm is inserted exactly once") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047H"));
        const auto config = env.read_generated("External7047H");
        const auto count = std::count_if(
            config.project_data.realms.begin(), config.project_data.realms.end(),
            [](const Salix::RealmInfo& info) { return info.name == "Default"; });
        CHECK(count == 1);
        CHECK(config.project_data.realms.size() == 2);
    }

    TEST_CASE("batch7047 existing Default realm is not duplicated") {
        Batch7047ExternalProjectEnv env;
        env.write_template(true);
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), "External7047I"));
        const auto config = env.read_generated("External7047I");
        const auto count = std::count_if(
            config.project_data.realms.begin(), config.project_data.realms.end(),
            [](const Salix::RealmInfo& info) { return info.name == "Default"; });
        CHECK(count == 1);
        CHECK(config.project_data.realms.size() == 1);
    }

    TEST_CASE("batch7047 existing project directory returns true before template parsing") {
        Batch7047ExternalProjectEnv env;
        const auto existing = env.projects / "Existing7047J";
        std::filesystem::create_directories(existing);
        std::filesystem::remove(env.template_root() / "project.json");
        Salix::ProjectManager manager;
        CHECK(manager.create_new_external_project(env.projects.string(), "Existing7047J"));
    }

    TEST_CASE("batch7047 malformed project template is rejected after directory creation") {
        Batch7047ExternalProjectEnv env;
        { std::ofstream out(env.template_root() / "project.json", std::ios::trunc); out << "{ malformed"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "Malformed7047K"));
        CHECK(std::filesystem::is_directory(env.projects / "Malformed7047K" / "Assets" / "Realms"));
    }

    TEST_CASE("batch7047 missing realm template fails after project file generation") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::remove(env.template_root() / "Assets" / "Realms" / "Default.realm");
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "MissingRealm7047L"));
        CHECK(std::filesystem::is_regular_file(
            env.projects / "MissingRealm7047L" / "MissingRealm7047L.salixproj"));
    }

    TEST_CASE("batch7047 missing sprite template fails after realm template copy") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::remove(env.template_root() / "Assets" / "Images" / "Sprites" / "test.png");
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "MissingSprite7047M"));
        CHECK(std::filesystem::is_regular_file(
            env.projects / "MissingSprite7047M" / "Assets" / "Realms" / "Default.realm"));
    }

    TEST_CASE("batch7047 internal project wrapper creates project beneath synthetic src Sandbox") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::create_directories(env.root / "src" / "Sandbox");
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_internal_project("Internal7047N"));
        CHECK(std::filesystem::is_regular_file(
            env.root / "src" / "Sandbox" / "Internal7047N" / "Internal7047N.salixproj"));
    }

    TEST_CASE("batch7047 project names containing spaces survive generated metadata") {
        Batch7047ExternalProjectEnv env;
        Salix::ProjectManager manager;
        const std::string name = "External 7047 O";
        REQUIRE(manager.create_new_external_project(env.projects.string(), name));
        const auto config = env.read_generated(name);
        CHECK(config.project_data.project_name == name);
        CHECK(config.project_data.project_file_name == name + ".salixproj");
    }
}

namespace {
std::filesystem::path batch7147_write_raw_project(
    Batch1947ProjectRoot& root,
    const std::string& name,
    const std::string& text) {
    const auto path = root.path / name;
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << text;
    return path;
}
}

TEST_SUITE("Salix::management::ProjectManagerBatch7147") {
    TEST_CASE("batch7147 load_project existing directory reaches input stream open failure") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.load_project(root.path.string()));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch7147 pointer loader existing directory reaches input stream open failure") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file(root.path.string()) == nullptr);
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch7147 load_project directory path containing spaces remains controlled failure") {
        Batch1947ProjectRoot root;
        const auto dir = root.path / "Directory With Spaces 7147";
        std::filesystem::create_directories(dir);
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.load_project(dir.string()));
    }

    TEST_CASE("batch7147 pointer loader directory path containing spaces remains controlled failure") {
        Batch1947ProjectRoot root;
        const auto dir = root.path / "Pointer Directory With Spaces 7147";
        std::filesystem::create_directories(dir);
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file(dir.string()) == nullptr);
    }

    TEST_CASE("batch7147 pointer loader open failure after placeholder active project preserves object identity") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        manager.set_active_project("Placeholder7147");
        Salix::Project* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file(root.path.string()) == nullptr);
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch7147 repeated pointer loads shut down prior active project before replacement") {
        Batch1947ProjectRoot root;
        const auto first = batch6947_write_valid_project(root, "pointer7147a", "Pointer7147A");
        const auto second = batch6947_write_valid_project(root, "pointer7147b", "Pointer7147B");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project_from_file(first.string()) != nullptr);
        REQUIRE(manager.load_project_from_file(second.string()) != nullptr);
        CHECK(manager.get_active_project()->get_name() == "Pointer7147B");
    }

    TEST_CASE("batch7147 pointer directory failure after valid load retains prior project object") {
        Batch1947ProjectRoot root;
        const auto valid = batch6947_write_valid_project(root, "pointer7147c", "Pointer7147C");
        Salix::ProjectManager manager;
        Salix::Project* before = manager.load_project_from_file(valid.string());
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file(root.path.string()) == nullptr);
        CHECK(manager.get_active_project() == before);
        CHECK(before->get_name() == "Pointer7147C");
    }

    TEST_CASE("batch7147 pointer malformed file after valid load retains prior project object") {
        Batch1947ProjectRoot root;
        const auto valid = batch6947_write_valid_project(root, "pointer7147d", "Pointer7147D");
        const auto malformed = batch7147_write_raw_project(root, "malformed7147d.salixproj", "{ malformed");
        Salix::ProjectManager manager;
        Salix::Project* before = manager.load_project_from_file(valid.string());
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file(malformed.string()) == nullptr);
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch7147 pointer missing file after valid load does not discard active project") {
        Batch1947ProjectRoot root;
        const auto valid = batch6947_write_valid_project(root, "pointer7147e", "Pointer7147E");
        Salix::ProjectManager manager;
        Salix::Project* before = manager.load_project_from_file(valid.string());
        REQUIRE(before != nullptr);
        CHECK(manager.load_project_from_file((root.path / "missing7147e.salixproj").string()) == nullptr);
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch7147 load_project directory failure after valid load retains prior project object") {
        Batch1947ProjectRoot root;
        const auto valid = batch6947_write_valid_project(root, "load7147f", "Load7147F");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project(valid.string()));
        Salix::Project* before = manager.get_active_project();
        REQUIRE(before != nullptr);
        CHECK_FALSE(manager.load_project(root.path.string()));
        CHECK(manager.get_active_project() == before);
    }

    TEST_CASE("batch7147 missing external project template reaches missing regular file diagnostic branch") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::remove(env.template_root() / "project.json");
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "MissingTemplate7147A"));
    }

    TEST_CASE("batch7147 missing external template still creates destination asset directories before parse failure") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::remove(env.template_root() / "project.json");
        Salix::ProjectManager manager;
        REQUIRE_FALSE(manager.create_new_external_project(env.projects.string(), "MissingTemplate7147B"));
        CHECK(std::filesystem::is_directory(env.projects / "MissingTemplate7147B" / "Assets" / "Realms"));
        CHECK(std::filesystem::is_directory(env.projects / "MissingTemplate7147B" / "Assets" / "Scripts"));
    }

    TEST_CASE("batch7147 template path replaced by directory is rejected as non regular file") {
        Batch7047ExternalProjectEnv env;
        const auto template_path = env.template_root() / "project.json";
        std::filesystem::remove(template_path);
        std::filesystem::create_directories(template_path);
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "TemplateDirectory7147C"));
    }

    TEST_CASE("batch7147 empty project template is rejected by cereal parse") {
        Batch7047ExternalProjectEnv env;
        { std::ofstream out(env.template_root() / "project.json", std::ios::trunc); }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "EmptyTemplate7147D"));
    }

    TEST_CASE("batch7147 whitespace only project template is rejected by cereal parse") {
        Batch7047ExternalProjectEnv env;
        { std::ofstream out(env.template_root() / "project.json", std::ios::trunc); out << "   \n\t  "; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "WhitespaceTemplate7147E"));
    }

    TEST_CASE("batch7147 scalar project template is rejected by cereal parse") {
        Batch7047ExternalProjectEnv env;
        { std::ofstream out(env.template_root() / "project.json", std::ios::trunc); out << "7147"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "ScalarTemplate7147F"));
    }

    TEST_CASE("batch7147 array project template is rejected by expected named cereal fields") {
        Batch7047ExternalProjectEnv env;
        { std::ofstream out(env.template_root() / "project.json", std::ios::trunc); out << "[1,2,3]"; }
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.create_new_external_project(env.projects.string(), "ArrayTemplate7147G"));
    }

    TEST_CASE("batch7147 missing template failure does not emit generated project file") {
        Batch7047ExternalProjectEnv env;
        std::filesystem::remove(env.template_root() / "project.json");
        Salix::ProjectManager manager;
        const std::string name = "MissingTemplate7147H";
        REQUIRE_FALSE(manager.create_new_external_project(env.projects.string(), name));
        CHECK_FALSE(std::filesystem::exists(env.projects / name / (name + ".salixproj")));
    }

    TEST_CASE("batch7147 existing destination project still returns true when template is absent") {
        Batch7047ExternalProjectEnv env;
        const std::string name = "Existing7147I";
        std::filesystem::create_directories(env.projects / name);
        std::filesystem::remove(env.template_root() / "project.json");
        Salix::ProjectManager manager;
        CHECK(manager.create_new_external_project(env.projects.string(), name));
    }

    TEST_CASE("batch7147 existing destination project preserves existing marker file") {
        Batch7047ExternalProjectEnv env;
        const std::string name = "Existing7147J";
        const auto project = env.projects / name;
        std::filesystem::create_directories(project);
        { std::ofstream out(project / "marker.txt"); out << "7147"; }
        Salix::ProjectManager manager;
        REQUIRE(manager.create_new_external_project(env.projects.string(), name));
        CHECK(Salix::FileManager::read_file_content((project / "marker.txt").string()) == "7147");
    }

    TEST_CASE("batch7147 three sequential pointer loads leave third project active") {
        Batch1947ProjectRoot root;
        const auto a = batch6947_write_valid_project(root, "seq7147a", "Seq7147A");
        const auto b = batch6947_write_valid_project(root, "seq7147b", "Seq7147B");
        const auto c = batch6947_write_valid_project(root, "seq7147c", "Seq7147C");
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project_from_file(a.string()) != nullptr);
        REQUIRE(manager.load_project_from_file(b.string()) != nullptr);
        REQUIRE(manager.load_project_from_file(c.string()) != nullptr);
        CHECK(manager.get_active_project()->get_name() == "Seq7147C");
    }

    TEST_CASE("batch7147 pointer replacement copies realm metadata from latest project") {
        Batch1947ProjectRoot root;
        const std::vector<Salix::RealmInfo> first_realms{{"Old7147", "Old7147.realm"}};
        const std::vector<Salix::RealmInfo> second_realms{{"New7147", "New7147.realm"}};
        const auto first = batch6947_write_valid_project(root, "realm7147a", "Realm7147A", "Old7147", first_realms);
        const auto second = batch6947_write_valid_project(root, "realm7147b", "Realm7147B", "New7147", second_realms);
        Salix::ProjectManager manager;
        REQUIRE(manager.load_project_from_file(first.string()) != nullptr);
        REQUIRE(manager.load_project_from_file(second.string()) != nullptr);
        CHECK(manager.get_active_project()->get_starting_realm() == "New7147");
        CHECK(manager.get_active_project()->get_realm_paths().count("New7147") == 1);
    }

    TEST_CASE("batch7147 malformed load_project file fails without creating active project") {
        Batch1947ProjectRoot root;
        const auto malformed = batch7147_write_raw_project(root, "malformed7147k.salixproj", "{ invalid");
        Salix::ProjectManager manager;
        CHECK_FALSE(manager.load_project(malformed.string()));
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch7147 malformed pointer project file fails without creating active project") {
        Batch1947ProjectRoot root;
        const auto malformed = batch7147_write_raw_project(root, "malformed7147l.salixproj", "{ invalid");
        Salix::ProjectManager manager;
        CHECK(manager.load_project_from_file(malformed.string()) == nullptr);
        CHECK(manager.get_active_project() == nullptr);
    }

    TEST_CASE("batch7147 directory load failure preserves pending selection metadata") {
        Batch1947ProjectRoot root;
        Salix::ProjectManager manager;
        manager.set_project_to_load("Pending7147.salixproj");
        manager.set_loaded_project_directory("PendingRoot7147");
        REQUIRE_FALSE(manager.load_project(root.path.string()));
        CHECK(manager.get_project_to_load() == "Pending7147.salixproj");
        CHECK(manager.get_loaded_project_directory() == "PendingRoot7147");
    }
}

namespace {
void batch7478_expect_nested_project_filename_failure(const std::string& name) {
    Batch1947ProjectRoot root;
    Salix::ProjectManager manager;
    CHECK_FALSE(manager.create_new_project(root.path.string(), name));
    const std::filesystem::path project_root = root.path / std::filesystem::path(name);
    CHECK(std::filesystem::is_directory(project_root / "Assets" / "Realms"));
    CHECK(std::filesystem::is_directory(project_root / "Assets" / "Scripts"));
}
}

TEST_SUITE("Salix::management::ProjectManagerBatch7478") {
    TEST_CASE("batch7478 nested project name A reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478A/NestedA"); }
    TEST_CASE("batch7478 nested project name B reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478B/NestedB"); }
    TEST_CASE("batch7478 nested project name C reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478C/NestedC"); }
    TEST_CASE("batch7478 nested project name D reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478D/NestedD"); }
    TEST_CASE("batch7478 nested project name E reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478E/NestedE"); }
    TEST_CASE("batch7478 nested project name F reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478F/NestedF"); }
    TEST_CASE("batch7478 nested project name G reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478G/NestedG"); }
    TEST_CASE("batch7478 nested project name H reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478H/NestedH"); }
    TEST_CASE("batch7478 nested project name I reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478I/NestedI"); }
    TEST_CASE("batch7478 nested project name J reaches project file open failure") { batch7478_expect_nested_project_filename_failure("Batch7478J/NestedJ"); }
}

namespace {
void batch7578_expect_external_nested_project_file_failure(const std::string& name) {
    Batch7047ExternalProjectEnv env;
    Salix::ProjectManager manager;
    CHECK_FALSE(manager.create_new_external_project(env.projects.string(), name));
    CHECK(std::filesystem::is_directory(env.projects / std::filesystem::path(name) / "Assets" / "Realms"));
}
}

TEST_SUITE("Salix::management::ProjectManagerBatch7578") {
    TEST_CASE("batch7578 external nested project A reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578A/NestedA"); }
    TEST_CASE("batch7578 external nested project B reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578B/NestedB"); }
    TEST_CASE("batch7578 external nested project C reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578C/NestedC"); }
    TEST_CASE("batch7578 external nested project D reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578D/NestedD"); }
    TEST_CASE("batch7578 external nested project E reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578E/NestedE"); }
    TEST_CASE("batch7578 external nested project F reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578F/NestedF"); }
    TEST_CASE("batch7578 external nested project G reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578G/NestedG"); }
    TEST_CASE("batch7578 external nested project H reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578H/NestedH"); }
    TEST_CASE("batch7578 external nested project I reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578I/NestedI"); }
    TEST_CASE("batch7578 external nested project J reaches generated file open failure") { batch7578_expect_external_nested_project_file_failure("Batch7578J/NestedJ"); }
}

namespace {
void batch7978_expect_malformed_project_load(const std::string& suffix, const std::string& bytes, bool pointer_api) {
    Batch1947ProjectRoot root;
    const auto path=batch7147_write_raw_project(root,"batch7978_"+suffix+".salixproj",bytes);
    Salix::ProjectManager manager;
    if(pointer_api) CHECK(manager.load_project_from_file(path.string())==nullptr);
    else CHECK_FALSE(manager.load_project(path.string()));
    CHECK(manager.get_active_project()==nullptr);
}
}

TEST_SUITE("Salix::management::ProjectManagerBatch7978") {
    TEST_CASE("batch7978 truncated object fails bool project loader") { batch7978_expect_malformed_project_load("a","{",false); }
    TEST_CASE("batch7978 truncated array fails pointer project loader") { batch7978_expect_malformed_project_load("b","[",true); }
    TEST_CASE("batch7978 unterminated string fails bool project loader") { batch7978_expect_malformed_project_load("c","\"unterminated",false); }
    TEST_CASE("batch7978 invalid object token fails pointer project loader") { batch7978_expect_malformed_project_load("d","{ invalid",true); }
    TEST_CASE("batch7978 truncated project data array fails bool loader") { batch7978_expect_malformed_project_load("e","{\"project_data\":[",false); }
    TEST_CASE("batch7978 truncated build settings object fails pointer loader") { batch7978_expect_malformed_project_load("f","{\"build_settings\":",true); }
    TEST_CASE("batch7978 plain text fails bool project loader") { batch7978_expect_malformed_project_load("g","not-json-7978",false); }
    TEST_CASE("batch7978 malformed null token fails pointer project loader") { batch7978_expect_malformed_project_load("h","null trailing",true); }
    TEST_CASE("batch7978 missing member value fails bool project loader") { batch7978_expect_malformed_project_load("i","{\"x\":",false); }
    TEST_CASE("batch7978 truncated numeric array fails pointer project loader") { batch7978_expect_malformed_project_load("j","[1,",true); }
}

TEST_SUITE("Salix::management::ProjectManagerBatch8178") {
    TEST_CASE("batch8178 missing bool project load remains false") { Salix::ProjectManager m; CHECK_FALSE(m.load_project("__missing8178a__.salixproj")); }
    TEST_CASE("batch8178 missing pointer project load remains null") { Salix::ProjectManager m; CHECK(m.load_project_from_file("__missing8178b__.salixproj")==nullptr); }
    TEST_CASE("batch8178 malformed empty object bool load remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"a8178.salixproj","{}");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8178 malformed empty object pointer load remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"b8178.salixproj","{}");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8178 malformed empty array bool load remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"c8178.salixproj","[]");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8178 malformed empty array pointer load remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"d8178.salixproj","[]");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8178 malformed boolean bool load remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"e8178.salixproj","true");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8178 malformed boolean pointer load remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"f8178.salixproj","false");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8178 metadata setters roundtrip without project") { Salix::ProjectManager m;m.set_project_to_load("P8178");m.set_loaded_project_directory("D8178");CHECK(m.get_project_to_load()=="P8178");CHECK(m.get_loaded_project_directory()=="D8178"); }
    TEST_CASE("batch8178 empty shutdown leaves active project null") { Salix::ProjectManager m;m.shutdown();CHECK(m.get_active_project()==nullptr); }
}

TEST_SUITE("Salix::management::ProjectManagerBatch8278") {
    TEST_CASE("batch8278 missing bool project remains false") { Salix::ProjectManager m;CHECK_FALSE(m.load_project("__missing8278a__.salixproj")); }
    TEST_CASE("batch8278 missing pointer project remains null") { Salix::ProjectManager m;CHECK(m.load_project_from_file("__missing8278b__.salixproj")==nullptr); }
    TEST_CASE("batch8278 scalar JSON bool project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"a8278.salixproj","8278");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8278 scalar JSON pointer project remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"b8278.salixproj","-8278");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8278 quoted JSON bool project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"c8278.salixproj","\"8278\"");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8278 quoted JSON pointer project remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"d8278.salixproj","\"project\"");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8278 null JSON bool project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"e8278.salixproj","null");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8278 null JSON pointer project remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"f8278.salixproj","null");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8278 pending metadata remains independent") { Salix::ProjectManager m;m.set_project_to_load("P8278");m.set_loaded_project_directory("D8278");CHECK(m.get_project_to_load()=="P8278");CHECK(m.get_loaded_project_directory()=="D8278"); }
    TEST_CASE("batch8278 empty manager update render shutdown remain controlled") { Salix::ProjectManager m;CHECK_NOTHROW(m.update(8.278f));CHECK_NOTHROW(m.render(nullptr));CHECK_NOTHROW(m.shutdown());CHECK(m.get_active_project()==nullptr); }
}

TEST_SUITE("Salix::management::ProjectManagerBatch8378") {
    TEST_CASE("batch8378 missing bool project remains false") { Salix::ProjectManager m;CHECK_FALSE(m.load_project("__missing8378a__.salixproj")); }
    TEST_CASE("batch8378 missing pointer project remains null") { Salix::ProjectManager m;CHECK(m.load_project_from_file("__missing8378b__.salixproj")==nullptr); }
    TEST_CASE("batch8378 numeric project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"a8378.salixproj","8378");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8378 numeric pointer project remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"b8378.salixproj","8378");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8378 string project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"c8378.salixproj","\"8378\"");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8378 null project remains false") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"d8378.salixproj","null");Salix::ProjectManager m;CHECK_FALSE(m.load_project(p.string())); }
    TEST_CASE("batch8378 null pointer project remains null") { Batch1947ProjectRoot r;auto p=batch7147_write_raw_project(r,"e8378.salixproj","null");Salix::ProjectManager m;CHECK(m.load_project_from_file(p.string())==nullptr); }
    TEST_CASE("batch8378 metadata setters preserve independent values") { Salix::ProjectManager m;m.set_project_to_load("P8378");m.set_loaded_project_directory("D8378");CHECK(m.get_project_to_load()=="P8378");CHECK(m.get_loaded_project_directory()=="D8378"); }
    TEST_CASE("batch8378 empty update and render remain safe") { Salix::ProjectManager m;CHECK_NOTHROW(m.update(.8378f));CHECK_NOTHROW(m.render(nullptr)); }
    TEST_CASE("batch8378 empty shutdown remains idempotent") { Salix::ProjectManager m;m.shutdown();m.shutdown();CHECK(m.get_active_project()==nullptr); }
}

