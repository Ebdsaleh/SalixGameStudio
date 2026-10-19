// RealmPersistence.test.cpp - isolated realm persistence and manager loading regressions.
// All disk operations occur inside a newly created, uniquely named temporary
// directory. This suite never reads, overwrites, or deletes project/scene files.
#include <doctest.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Transform.h>
#include <Salix/management/RealmManager.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>

namespace {
    // Deletes only a directory it created itself, never a pre-existing path.
    struct OwnedTemporaryDirectory {
        std::filesystem::path path;
        bool owned = false;

        OwnedTemporaryDirectory() {
            std::error_code ec;
            const auto base = std::filesystem::temp_directory_path(ec);
            if (ec) return;

            const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
            for (int attempt = 0; attempt < 16; ++attempt) {
                const std::string name = "salix-realm-test-"
                    + std::to_string(tick) + "-" + std::to_string(attempt);
                path = base / name;
                ec.clear();
                if (std::filesystem::create_directory(path, ec)) {
                    owned = true;
                    return;
                }
                if (ec) break;
            }
            path.clear();
        }

        ~OwnedTemporaryDirectory() {
            if (!owned) return;
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }

        OwnedTemporaryDirectory(const OwnedTemporaryDirectory&) = delete;
        OwnedTemporaryDirectory& operator=(const OwnedTemporaryDirectory&) = delete;

        std::filesystem::path file(const char* name) const {
            return path / name;
        }
    };
}

TEST_SUITE("Salix::ecs::RealmPersistence") {

    TEST_CASE("realm JSON roundtrip preserves metadata entity identity and transform") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto file = temp.file("roundtrip.json");

        Salix::Realm source("Saved World", "Scenes/SavedWorld.json");
        Salix::Entity* player = source.create_entity("Player");
        Salix::Entity* camera_marker = source.create_entity("CameraMarker");
        REQUIRE(player != nullptr);
        REQUIRE(camera_marker != nullptr);
        REQUIRE(player->get_transform() != nullptr);
        player->get_transform()->set_position(12.0f, -4.0f, 9.5f);
        const Salix::SimpleGuid player_id = player->get_id();
        const Salix::SimpleGuid marker_id = camera_marker->get_id();

        REQUIRE(source.save_to_file(file.string()));
        REQUIRE(std::filesystem::is_regular_file(file));

        const Salix::InitContext context{};
        std::unique_ptr<Salix::Realm> loaded =
            Salix::Realm::load_from_file(file.string(), context);
        REQUIRE(loaded != nullptr);
        CHECK(loaded->get_name() == "Saved World");
        CHECK(loaded->get_path() == "Scenes/SavedWorld.json");
        CHECK(loaded->get_entities().size() == 2);

        Salix::Entity* restored_player = loaded->get_entity_by_id(player_id);
        Salix::Entity* restored_marker = loaded->get_entity_by_id(marker_id);
        REQUIRE(restored_player != nullptr);
        REQUIRE(restored_marker != nullptr);
        CHECK(restored_player->get_name() == "Player");
        CHECK(restored_marker->get_name() == "CameraMarker");
        CHECK(loaded->get_entity_by_name("Player") == restored_player);
        REQUIRE(restored_player->get_transform() != nullptr);
        CHECK(restored_player->get_transform()->get_position().x == doctest::Approx(12.0f));
        CHECK(restored_player->get_transform()->get_position().y == doctest::Approx(-4.0f));
        CHECK(restored_player->get_transform()->get_position().z == doctest::Approx(9.5f));
    }

    TEST_CASE("empty realm JSON roundtrip preserves name and empty entity list") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto file = temp.file("empty.json");
        Salix::Realm source("Empty World", "Scenes/Empty.json");
        REQUIRE(source.save_to_file(file.string()));

        const Salix::InitContext context{};
        auto loaded = Salix::Realm::load_from_file(file.string(), context);
        REQUIRE(loaded != nullptr);
        CHECK(loaded->get_name() == "Empty World");
        CHECK(loaded->get_path() == "Scenes/Empty.json");
        CHECK(loaded->get_entities().empty());
        CHECK(loaded->get_main_camera_entity_id() == Salix::SimpleGuid::invalid());
    }

    TEST_CASE("save rejects an existing directory and never removes it") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        Salix::Realm realm("No overwrite");
        CHECK_FALSE(realm.save_to_file(temp.path.string()));
        CHECK(std::filesystem::is_directory(temp.path));
    }

    TEST_CASE("save rejects missing parent directory without creating it") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto missing_parent = temp.path / "not-created";
        const auto file = missing_parent / "realm.json";
        Salix::Realm realm("No accidental mkdir");
        CHECK_FALSE(realm.save_to_file(file.string()));
        CHECK_FALSE(std::filesystem::exists(missing_parent));
    }

    TEST_CASE("load rejects missing realm file without creating it") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto file = temp.file("never-created.json");
        const Salix::InitContext context{};
        CHECK(Salix::Realm::load_from_file(file.string(), context) == nullptr);
        CHECK_FALSE(std::filesystem::exists(file));
    }

    TEST_CASE("load rejects malformed JSON and preserves the source file") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto file = temp.file("malformed.json");
        const std::string invalid_json = "{ this is not valid Cereal JSON";
        {
            std::ofstream output(file, std::ios::binary);
            REQUIRE(output.is_open());
            output << invalid_json;
            REQUIRE(output.good());
        }

        const Salix::InitContext context{};
        CHECK(Salix::Realm::load_from_file(file.string(), context) == nullptr);
        std::ifstream input(file, std::ios::binary);
        REQUIRE(input.is_open());
        const std::string unchanged(
            (std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        CHECK(unchanged == invalid_json);
    }

    TEST_CASE("realm manager loads and selects saved realm by filename stem") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        const auto file = temp.file("SavedBattle.json");

        Salix::Realm source("Battle Scene", "Scenes/Battle.json");
        Salix::Entity* player = source.create_entity("Hero");
        REQUIRE(player != nullptr);
        const Salix::SimpleGuid player_id = player->get_id();
        REQUIRE(source.save_to_file(file.string()));

        Salix::RealmManager manager;
        const Salix::InitContext context{};
        manager.initialize(context);
        REQUIRE(manager.load_realm(file.string()));
        CHECK(manager.get_active_realm() == nullptr);
        REQUIRE(manager.set_active_realm("SavedBattle"));
        Salix::Realm* loaded = manager.get_active_realm();
        REQUIRE(loaded != nullptr);
        CHECK(loaded->get_name() == "Battle Scene");
        CHECK(loaded->get_path() == "Scenes/Battle.json");
        CHECK(loaded->get_entity_by_id(player_id) != nullptr);

        // The already-loaded realm must not be discarded and recreated.
        CHECK(manager.load_realm(file.string()));
        CHECK(manager.get_active_realm() == loaded);
        manager.shutdown();
        CHECK(manager.get_active_realm() == nullptr);
    }

    TEST_CASE("realm manager missing load does not change the active realm") {
        OwnedTemporaryDirectory temp;
        REQUIRE(temp.owned);
        Salix::RealmManager manager;
        Salix::Realm* active = manager.create_realm("Menu", "Scenes/Menu.json");
        REQUIRE(active != nullptr);
        REQUIRE(manager.set_active_realm("Menu"));
        CHECK_FALSE(manager.load_realm(temp.file("missing.json").string()));
        CHECK(manager.get_active_realm() == active);
        CHECK(active->get_name() == "Menu");
    }

    TEST_CASE("load_active_realm accepts an asset-manager context with an empty realm") {
        Salix::AssetManager asset_manager;
        Salix::InitContext context{};
        context.asset_manager = &asset_manager;
        Salix::RealmManager manager;
        manager.initialize(context);
        REQUIRE(manager.create_realm("Empty", "Scenes/Empty.json") != nullptr);
        REQUIRE(manager.set_active_realm("Empty"));
        CHECK(manager.load_active_realm());
    }
}
