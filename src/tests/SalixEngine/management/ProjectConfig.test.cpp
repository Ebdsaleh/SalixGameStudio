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
