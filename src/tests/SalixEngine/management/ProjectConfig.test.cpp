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
