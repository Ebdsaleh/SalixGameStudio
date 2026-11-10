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
