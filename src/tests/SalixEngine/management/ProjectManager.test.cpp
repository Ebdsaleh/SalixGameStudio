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

// ---- Relocated from src/tests/SalixEngine/management/ProjectConfig.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/management/ProjectConfig.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <string>

TEST_SUITE("Salix::management::ProjectConfigBatch1047") {
    TEST_CASE("realm info constructor preserves name and relative path") {
        Salix::RealmInfo info("Battle", "Assets/Scenes/Battle.realm");
        CHECK(info.name == "Battle");
        CHECK(info.path == "Assets/Scenes/Battle.realm");
    }

    TEST_CASE("build settings defaults expose the engine version and an empty game DLL") {
        Salix::BuildSettings settings;
        CHECK(settings.engine_version == "0.1.0-Alpha");
        CHECK(settings.game_dll_name.empty());
    }

    TEST_CASE("project data defaults are empty and contain no realms") {
        Salix::ProjectData data;
        CHECK(data.project_path.empty());
        CHECK(data.project_file_name.empty());
        CHECK(data.project_name.empty());
        CHECK(data.realms.empty());
        CHECK(data.starting_realm.empty());
    }

    TEST_CASE("project config composes independent project and build defaults") {
        Salix::ProjectConfig config;
        CHECK(config.project_data.realms.empty());
        CHECK(config.project_data.project_name.empty());
        CHECK(config.build_settings.engine_version == "0.1.0-Alpha");
        CHECK(config.build_settings.game_dll_name.empty());
    }

    TEST_CASE("JSON serialization uses project_file alias rather than the C++ member name") {
        Salix::ProjectConfig config;
        config.project_data.project_file_name = "game.salixproject";
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(config); }
        const std::string json = stream.str();
        CHECK(json.find("\"project_file\"") != std::string::npos);
        CHECK(json.find("game.salixproject") != std::string::npos);
        CHECK(json.find("project_file_name") == std::string::npos);
    }

    TEST_CASE("JSON roundtrip preserves project metadata and realm ordering") {
        Salix::ProjectConfig source;
        source.project_data.project_path = "C:/Games/MyGame";
        source.project_data.project_file_name = "MyGame.project";
        source.project_data.project_name = "My Game";
        source.project_data.realms = {
            Salix::RealmInfo("Menu", "Assets/Scenes/Menu.realm"),
            Salix::RealmInfo("Battle", "Assets/Scenes/Battle.realm")
        };
        source.project_data.starting_realm = "Menu";
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        REQUIRE(restored.project_data.realms.size() == 2);
        CHECK(restored.project_data.project_path == source.project_data.project_path);
        CHECK(restored.project_data.project_file_name == source.project_data.project_file_name);
        CHECK(restored.project_data.project_name == source.project_data.project_name);
        CHECK(restored.project_data.realms[0].name == "Menu");
        CHECK(restored.project_data.realms[1].name == "Battle");
        CHECK(restored.project_data.starting_realm == "Menu");
    }

    TEST_CASE("binary roundtrip preserves custom build settings independently of project data") {
        Salix::ProjectConfig source;
        source.project_data.project_name = "Binary Project";
        source.build_settings.engine_version = "9.8.7-Test";
        source.build_settings.game_dll_name = "BinaryGame.dll";
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.project_data.project_name == "Binary Project");
        CHECK(restored.build_settings.engine_version == "9.8.7-Test");
        CHECK(restored.build_settings.game_dll_name == "BinaryGame.dll");
    }

    TEST_CASE("empty realm collection survives JSON persistence without inventing entries") {
        Salix::ProjectConfig source;
        source.project_data.project_name = "No Realms Yet";
        source.project_data.starting_realm.clear();
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        restored.project_data.realms.emplace_back("Old", "old.realm");
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.project_data.project_name == "No Realms Yet");
        CHECK(restored.project_data.realms.empty());
        CHECK(restored.project_data.starting_realm.empty());
    }

    TEST_CASE("copying project config owns independent nested strings and realm vectors") {
        Salix::ProjectConfig original;
        original.project_data.project_name = "Original";
        original.project_data.realms.emplace_back("World", "World.realm");
        Salix::ProjectConfig copy = original;
        copy.project_data.project_name = "Copy";
        copy.project_data.realms[0].name = "Changed";
        copy.project_data.realms.emplace_back("Extra", "Extra.realm");
        CHECK(original.project_data.project_name == "Original");
        REQUIRE(original.project_data.realms.size() == 1);
        CHECK(original.project_data.realms[0].name == "World");
        CHECK(copy.project_data.realms.size() == 2);
    }

    TEST_CASE("deserialization replaces preexisting destination project and build state") {
        Salix::ProjectConfig source;
        source.project_data.project_name = "Loaded";
        source.project_data.realms.emplace_back("LoadedRealm", "loaded.realm");
        source.build_settings.game_dll_name = "Loaded.dll";
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }

        Salix::ProjectConfig destination;
        destination.project_data.project_name = "Stale";
        destination.project_data.realms.emplace_back("StaleRealm", "stale.realm");
        destination.build_settings.game_dll_name = "Stale.dll";
        { cereal::BinaryInputArchive in(stream); in(destination); }

        CHECK(destination.project_data.project_name == "Loaded");
        REQUIRE(destination.project_data.realms.size() == 1);
        CHECK(destination.project_data.realms[0].name == "LoadedRealm");
        CHECK(destination.build_settings.game_dll_name == "Loaded.dll");
    }
}


TEST_SUITE("Salix::management::ProjectConfigBatch2047") {
    TEST_CASE("realm info default construction owns two empty strings") {
        Salix::RealmInfo info;
        CHECK(info.name.empty());
        CHECK(info.path.empty());
    }

    TEST_CASE("realm info copies keep name and path storage independent") {
        Salix::RealmInfo original("Hub", "Assets/Realms/Hub.realm");
        Salix::RealmInfo copy = original;
        copy.name = "Arena";
        copy.path = "Assets/Realms/Arena.realm";
        CHECK(original.name == "Hub");
        CHECK(original.path == "Assets/Realms/Hub.realm");
        CHECK(copy.name == "Arena");
        CHECK(copy.path == "Assets/Realms/Arena.realm");
    }

    TEST_CASE("fresh build settings instances do not share mutable strings") {
        Salix::BuildSettings first;
        Salix::BuildSettings second;
        first.engine_version = "Custom-A";
        first.game_dll_name = "First.dll";
        CHECK(second.engine_version == "0.1.0-Alpha");
        CHECK(second.game_dll_name.empty());
    }

    TEST_CASE("project data permits duplicate realm names with distinct paths and stable order") {
        Salix::ProjectData data;
        data.realms.emplace_back("Level", "A/Level.realm");
        data.realms.emplace_back("Level", "B/Level.realm");
        REQUIRE(data.realms.size() == 2);
        CHECK(data.realms[0].name == "Level");
        CHECK(data.realms[0].path == "A/Level.realm");
        CHECK(data.realms[1].name == "Level");
        CHECK(data.realms[1].path == "B/Level.realm");
    }

    TEST_CASE("starting realm remains independent of whether a matching realm entry exists") {
        Salix::ProjectData data;
        data.starting_realm = "MissingRealm";
        CHECK(data.realms.empty());
        CHECK(data.starting_realm == "MissingRealm");
    }

    TEST_CASE("project config copy assignment deep copies nested realm storage") {
        Salix::ProjectConfig source;
        source.project_data.project_name = "Source";
        source.project_data.realms.emplace_back("World", "World.realm");
        source.build_settings.game_dll_name = "Source.dll";
        Salix::ProjectConfig destination;
        destination = source;
        source.project_data.project_name = "Mutated";
        source.project_data.realms[0].path = "Changed.realm";
        source.build_settings.game_dll_name = "Changed.dll";
        CHECK(destination.project_data.project_name == "Source");
        REQUIRE(destination.project_data.realms.size() == 1);
        CHECK(destination.project_data.realms[0].path == "World.realm");
        CHECK(destination.build_settings.game_dll_name == "Source.dll");
    }

    TEST_CASE("JSON roundtrip preserves an explicitly empty realm path and empty starting realm") {
        Salix::ProjectConfig source;
        source.project_data.project_name = "Sparse";
        source.project_data.realms.emplace_back("Placeholder", "");
        source.project_data.starting_realm.clear();
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        REQUIRE(restored.project_data.realms.size() == 1);
        CHECK(restored.project_data.realms[0].name == "Placeholder");
        CHECK(restored.project_data.realms[0].path.empty());
        CHECK(restored.project_data.starting_realm.empty());
    }

    TEST_CASE("JSON roundtrip preserves spaces punctuation and nested project metadata") {
        Salix::ProjectConfig source;
        source.project_data.project_path = "C:/Projects/Salix Test #1";
        source.project_data.project_file_name = "Salix Test.project";
        source.project_data.project_name = "Salix: Test #1";
        source.build_settings.game_dll_name = "Game Build.dll";
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.project_data.project_path == source.project_data.project_path);
        CHECK(restored.project_data.project_file_name == source.project_data.project_file_name);
        CHECK(restored.project_data.project_name == source.project_data.project_name);
        CHECK(restored.build_settings.game_dll_name == source.build_settings.game_dll_name);
    }

    TEST_CASE("binary deserialization replaces stale engine version and realm collection together") {
        Salix::ProjectConfig source;
        source.build_settings.engine_version = "2.0.0-Test";
        source.project_data.realms.emplace_back("Fresh", "Fresh.realm");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::ProjectConfig destination;
        destination.build_settings.engine_version = "stale";
        destination.project_data.realms.emplace_back("OldA", "OldA.realm");
        destination.project_data.realms.emplace_back("OldB", "OldB.realm");
        { cereal::BinaryInputArchive in(stream); in(destination); }
        CHECK(destination.build_settings.engine_version == "2.0.0-Test");
        REQUIRE(destination.project_data.realms.size() == 1);
        CHECK(destination.project_data.realms[0].name == "Fresh");
    }

    TEST_CASE("binary roundtrip preserves deliberately empty build setting strings") {
        Salix::ProjectConfig source;
        source.build_settings.engine_version.clear();
        source.build_settings.game_dll_name.clear();
        source.project_data.project_name = "EmptyBuildStrings";
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::ProjectConfig restored;
        restored.build_settings.engine_version = "stale";
        restored.build_settings.game_dll_name = "stale.dll";
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.build_settings.engine_version.empty());
        CHECK(restored.build_settings.game_dll_name.empty());
        CHECK(restored.project_data.project_name == "EmptyBuildStrings");
    }
}


// ---- Relocated from src/tests/SalixEngine/management/RealmData.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/management/RealmData.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <limits>
#include <sstream>
#include <string>

namespace {
Salix::ReferencedAsset batch1047_asset(const std::string& path) {
    Salix::ReferencedAsset asset;
    asset.path = path;
    return asset;
}
}

TEST_SUITE("Salix::management::RealmDataBatch1047") {
    TEST_CASE("referenced asset defaults to an empty portable path") {
        Salix::ReferencedAsset asset;
        CHECK(asset.path.empty());
    }

    TEST_CASE("referenced asset JSON roundtrip preserves spaces and relative separators") {
        Salix::ReferencedAsset source;
        source.path = "Assets/My Textures/Hero Idle.png";
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ReferencedAsset restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.path == source.path);
    }

    TEST_CASE("referenced asset binary roundtrip preserves UTF-8 path bytes") {
        Salix::ReferencedAsset source;
        source.path = "Assets/special-%23/hero.png";
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::ReferencedAsset restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.path == source.path);
    }

    TEST_CASE("realm metadata defaults are empty with zero entities") {
        Salix::RealmData data;
        CHECK(data.realm_name.empty());
        CHECK(data.realm_file_path_relative.empty());
        CHECK(data.entity_count == 0);
        CHECK(data.engine_version.empty());
        CHECK(data.unique_referenced_assets.empty());
    }

    TEST_CASE("JSON roundtrip preserves complete realm manifest metadata") {
        Salix::RealmData source;
        source.realm_name = "Dungeon";
        source.realm_file_path_relative = "Assets/Scenes/Dungeon.realm";
        source.entity_count = 42;
        source.engine_version = "0.1.0-Alpha";
        source.unique_referenced_assets = {batch1047_asset("Assets/Textures/wall.png"), batch1047_asset("Assets/Audio/theme.ogg")};
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.realm_name == source.realm_name);
        CHECK(restored.realm_file_path_relative == source.realm_file_path_relative);
        CHECK(restored.entity_count == 42);
        CHECK(restored.engine_version == "0.1.0-Alpha");
        REQUIRE(restored.unique_referenced_assets.size() == 2);
        CHECK(restored.unique_referenced_assets[0].path == "Assets/Textures/wall.png");
        CHECK(restored.unique_referenced_assets[1].path == "Assets/Audio/theme.ogg");
    }

    TEST_CASE("asset manifest preserves ordering and deliberate duplicate references") {
        Salix::RealmData source;
        source.unique_referenced_assets = {batch1047_asset("A.png"), batch1047_asset("B.png"), batch1047_asset("A.png")};
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        REQUIRE(restored.unique_referenced_assets.size() == 3);
        CHECK(restored.unique_referenced_assets[0].path == "A.png");
        CHECK(restored.unique_referenced_assets[1].path == "B.png");
        CHECK(restored.unique_referenced_assets[2].path == "A.png");
    }

    TEST_CASE("binary persistence supports a large representable entity count") {
        Salix::RealmData source;
        source.entity_count = (std::numeric_limits<std::size_t>::max)() / 2;
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.entity_count == source.entity_count);
    }

    TEST_CASE("empty asset list remains empty after JSON roundtrip") {
        Salix::RealmData source;
        source.realm_name = "Empty";
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        restored.unique_referenced_assets.push_back(batch1047_asset("stale.asset"));
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.realm_name == "Empty");
        CHECK(restored.unique_referenced_assets.empty());
    }

    TEST_CASE("copying realm metadata keeps asset vectors independent") {
        Salix::RealmData original;
        original.realm_name = "Original";
        original.unique_referenced_assets.push_back(batch1047_asset("one.asset"));
        Salix::RealmData copy = original;
        copy.realm_name = "Copy";
        copy.unique_referenced_assets[0].path = "changed.asset";
        copy.unique_referenced_assets.push_back(batch1047_asset("two.asset"));
        CHECK(original.realm_name == "Original");
        REQUIRE(original.unique_referenced_assets.size() == 1);
        CHECK(original.unique_referenced_assets[0].path == "one.asset");
        CHECK(copy.unique_referenced_assets.size() == 2);
    }

    TEST_CASE("binary deserialization replaces stale destination manifest values") {
        Salix::RealmData source;
        source.realm_name = "Fresh";
        source.realm_file_path_relative = "Fresh.realm";
        source.entity_count = 3;
        source.engine_version = "FreshEngine";
        source.unique_referenced_assets = {batch1047_asset("fresh.asset")};
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }

        Salix::RealmData destination;
        destination.realm_name = "Stale";
        destination.entity_count = 999;
        destination.unique_referenced_assets = {batch1047_asset("stale.asset"), batch1047_asset("old.asset")};
        { cereal::BinaryInputArchive in(stream); in(destination); }

        CHECK(destination.realm_name == "Fresh");
        CHECK(destination.realm_file_path_relative == "Fresh.realm");
        CHECK(destination.entity_count == 3);
        CHECK(destination.engine_version == "FreshEngine");
        REQUIRE(destination.unique_referenced_assets.size() == 1);
        CHECK(destination.unique_referenced_assets[0].path == "fresh.asset");
    }
}


TEST_SUITE("Salix::management::RealmDataBatch2047") {
    TEST_CASE("referenced asset copy owns independent path storage") {
        Salix::ReferencedAsset source = batch1047_asset("Assets/Textures/original.png");
        Salix::ReferencedAsset copy = source;
        copy.path = "Assets/Textures/copy.png";
        CHECK(source.path == "Assets/Textures/original.png");
        CHECK(copy.path == "Assets/Textures/copy.png");
    }

    TEST_CASE("fresh realm data instances do not share manifest strings or asset vectors") {
        Salix::RealmData first;
        Salix::RealmData second;
        first.realm_name = "First";
        first.unique_referenced_assets.push_back(batch1047_asset("first.asset"));
        CHECK(second.realm_name.empty());
        CHECK(second.unique_referenced_assets.empty());
    }

    TEST_CASE("empty referenced asset path survives JSON persistence exactly") {
        Salix::ReferencedAsset source;
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ReferencedAsset restored = batch1047_asset("stale.asset");
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.path.empty());
    }

    TEST_CASE("binary persistence preserves maximum representable entity count") {
        Salix::RealmData source;
        source.entity_count = (std::numeric_limits<std::size_t>::max)();
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.entity_count == source.entity_count);
    }

    TEST_CASE("JSON manifest preserves duplicate and empty asset paths in original order") {
        Salix::RealmData source;
        source.unique_referenced_assets = {
            batch1047_asset("same.asset"), batch1047_asset(""), batch1047_asset("same.asset")
        };
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        REQUIRE(restored.unique_referenced_assets.size() == 3);
        CHECK(restored.unique_referenced_assets[0].path == "same.asset");
        CHECK(restored.unique_referenced_assets[1].path.empty());
        CHECK(restored.unique_referenced_assets[2].path == "same.asset");
    }

    TEST_CASE("binary manifest preserves spaces punctuation and relative traversal text verbatim") {
        Salix::RealmData source;
        source.realm_file_path_relative = "../Scenes/My Realm #1.realm";
        source.unique_referenced_assets = {batch1047_asset("../Textures/My Hero #1.png")};
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.realm_file_path_relative == "../Scenes/My Realm #1.realm");
        REQUIRE(restored.unique_referenced_assets.size() == 1);
        CHECK(restored.unique_referenced_assets[0].path == "../Textures/My Hero #1.png");
    }

    TEST_CASE("realm data copy assignment deep copies referenced asset vector") {
        Salix::RealmData source;
        source.realm_name = "Source";
        source.unique_referenced_assets = {batch1047_asset("one.asset"), batch1047_asset("two.asset")};
        Salix::RealmData destination;
        destination = source;
        source.unique_referenced_assets[0].path = "changed.asset";
        source.unique_referenced_assets.clear();
        CHECK(destination.realm_name == "Source");
        REQUIRE(destination.unique_referenced_assets.size() == 2);
        CHECK(destination.unique_referenced_assets[0].path == "one.asset");
        CHECK(destination.unique_referenced_assets[1].path == "two.asset");
    }

    TEST_CASE("zero entity count from binary input replaces stale nonzero destination count") {
        Salix::RealmData source;
        source.entity_count = 0;
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData destination;
        destination.entity_count = 9999;
        { cereal::BinaryInputArchive in(stream); in(destination); }
        CHECK(destination.entity_count == 0);
    }

    TEST_CASE("engine version realm name and file path remain independently mutable") {
        Salix::RealmData data;
        data.realm_name = "Realm A";
        data.realm_file_path_relative = "A.realm";
        data.engine_version = "Engine A";
        data.engine_version = "Engine B";
        CHECK(data.realm_name == "Realm A");
        CHECK(data.realm_file_path_relative == "A.realm");
        CHECK(data.engine_version == "Engine B");
    }

    TEST_CASE("binary roundtrip preserves populated assets with otherwise empty realm metadata") {
        Salix::RealmData source;
        source.unique_referenced_assets = {batch1047_asset("only.asset")};
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RealmData restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.realm_name.empty());
        CHECK(restored.realm_file_path_relative.empty());
        CHECK(restored.engine_version.empty());
        CHECK(restored.entity_count == 0);
        REQUIRE(restored.unique_referenced_assets.size() == 1);
        CHECK(restored.unique_referenced_assets[0].path == "only.asset");
    }
}
