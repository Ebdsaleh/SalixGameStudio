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
