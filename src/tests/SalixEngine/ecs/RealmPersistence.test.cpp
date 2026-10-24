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


// Integration boundaries for the 847+ coverage batch.
TEST_SUITE("Salix::ecs::RealmPersistenceBatch847") {
    TEST_CASE("saving the same realm twice replaces persisted metadata with the latest values") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("overwrite.json"); Salix::Realm realm("First","Scenes/First.json"); REQUIRE(realm.save_to_file(file.string())); realm.set_name("Second"); realm.set_path("Scenes/Second.json"); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); CHECK(loaded->get_name()=="Second"); CHECK(loaded->get_path()=="Scenes/Second.json");
    }

    TEST_CASE("realm main camera entity identifier survives JSON even without a camera component") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("camera-id.json"); Salix::Realm realm("CameraId"); auto* e=realm.create_entity("Marker"); REQUIRE(e!=nullptr); realm.set_main_camera_entity(e->get_id()); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); CHECK(loaded->get_main_camera_entity_id()==e->get_id());
    }

    TEST_CASE("invalid main camera identifier remains invalid after a populated realm roundtrip") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("invalid-camera.json"); Salix::Realm realm("NoCamera"); REQUIRE(realm.create_entity("Player")!=nullptr); realm.set_main_camera_entity(Salix::SimpleGuid::invalid()); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); CHECK(loaded->get_main_camera_entity_id()==Salix::SimpleGuid::invalid());
    }

    TEST_CASE("realm entity insertion order survives persistence") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("order.json"); Salix::Realm realm("Order"); REQUIRE(realm.create_entity("A")!=nullptr); REQUIRE(realm.create_entity("B")!=nullptr); REQUIRE(realm.create_entity("C")!=nullptr); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); const auto entities=loaded->get_entities(); REQUIRE(entities.size()==3); CHECK(entities[0]->get_name()=="A"); CHECK(entities[1]->get_name()=="B"); CHECK(entities[2]->get_name()=="C");
    }

    TEST_CASE("duplicate entity names remain separate identities after persistence") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("duplicates.json"); Salix::Realm realm("Dupes"); auto* a=realm.create_entity("Same"); auto* b=realm.create_entity("Same"); REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); const auto aid=a->get_id(),bid=b->get_id(); REQUIRE(aid!=bid); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); CHECK(loaded->get_entity_by_id(aid)!=nullptr); CHECK(loaded->get_entity_by_id(bid)!=nullptr); CHECK(loaded->get_entity_by_name("Same")->get_id()==aid);
    }

    TEST_CASE("empty metadata setter calls cannot erase a persisted realm name or path") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("metadata.json"); Salix::Realm realm("KeepName","Scenes/Keep.json"); realm.set_name(""); realm.set_path(""); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); CHECK(loaded->get_name()=="KeepName"); CHECK(loaded->get_path()=="Scenes/Keep.json");
    }

    TEST_CASE("explicit high bit entity GUID survives realm persistence and lookup") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("high-guid.json"); Salix::Realm realm("HighGuid"); const auto id=Salix::SimpleGuid::from_value((uint64_t{1}<<50)+77ULL); REQUIRE(realm.create_entity(id,"High")!=nullptr); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); auto* e=loaded->get_entity_by_id(id); REQUIRE(e!=nullptr); CHECK(e->get_name()=="High");
    }

    TEST_CASE("multiple transformed entities retain independent signed positions") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("positions.json"); Salix::Realm realm("Positions"); auto* a=realm.create_entity("A"); auto* b=realm.create_entity("B"); REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); a->get_transform()->set_position(-1.f,2.f,-3.f); b->get_transform()->set_position(10.f,-20.f,30.f); const auto aid=a->get_id(),bid=b->get_id(); REQUIRE(realm.save_to_file(file.string())); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); auto* la=loaded->get_entity_by_id(aid); auto* lb=loaded->get_entity_by_id(bid); REQUIRE(la!=nullptr); REQUIRE(lb!=nullptr); CHECK(la->get_transform()->get_position()==Salix::Vector3(-1.f,2.f,-3.f)); CHECK(lb->get_transform()->get_position()==Salix::Vector3(10.f,-20.f,30.f));
    }

    TEST_CASE("loading a valid realm does not modify its source file bytes") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto file=temp.file("immutable-source.json"); Salix::Realm realm("Immutable"); REQUIRE(realm.create_entity("One")!=nullptr); REQUIRE(realm.save_to_file(file.string())); std::ifstream before_stream(file,std::ios::binary); const std::string before((std::istreambuf_iterator<char>(before_stream)),std::istreambuf_iterator<char>()); const Salix::InitContext context{}; auto loaded=Salix::Realm::load_from_file(file.string(),context); REQUIRE(loaded!=nullptr); std::ifstream after_stream(file,std::ios::binary); const std::string after((std::istreambuf_iterator<char>(after_stream)),std::istreambuf_iterator<char>()); CHECK(after==before);
    }

    TEST_CASE("realm manager loads two files with different stems without replacing the first") {
        OwnedTemporaryDirectory temp; REQUIRE(temp.owned); const auto one=temp.file("One.json"),two=temp.file("Two.json"); Salix::Realm a("Realm A"),b("Realm B"); REQUIRE(a.save_to_file(one.string())); REQUIRE(b.save_to_file(two.string())); Salix::RealmManager manager; const Salix::InitContext context{}; manager.initialize(context); REQUIRE(manager.load_realm(one.string())); REQUIRE(manager.load_realm(two.string())); REQUIRE(manager.set_active_realm("One")); CHECK(manager.get_active_realm()->get_name()=="Realm A"); REQUIRE(manager.set_active_realm("Two")); CHECK(manager.get_active_realm()->get_name()=="Realm B");
    }
}
