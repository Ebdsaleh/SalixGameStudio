// First SalixEditor CRI fixture: deterministic data-model tests.
// Exercise the DLL's exported Archetype methods; no native GUI or renderer.
#include <doctest.h>
#include <Editor/Archetypes.h>
#include <utility>

namespace {
    using Salix::ElementArchetype;
    using Salix::EntityArchetype;
    using Salix::SimpleGuid;

    ElementArchetype make_element(const char* type = "Transform", uint64_t id = 101,
                                  bool duplicable = true) {
        ElementArchetype element;
        element.id = SimpleGuid::from_value(id);
        element.owner_id = SimpleGuid::from_value(10);
        element.type_name = type;
        element.name = type;
        element.allows_duplication = duplicable;
        // Each call constructs independent YAML storage.
        element.data = YAML::Load("{enabled: true, speed: 5}");
        return element;
    }

    EntityArchetype make_entity() {
        EntityArchetype entity;
        entity.id = SimpleGuid::from_value(10);
        entity.name = "Player";
        entity.parent_id = SimpleGuid::from_value(20);
        entity.child_ids = {SimpleGuid::from_value(30)};
        entity.elements = {make_element("Transform", 101, false),
                           make_element("Sprite2D", 102)};
        return entity;
    }
}

TEST_SUITE("EditorArchetypes") {
    TEST_CASE("EditorArchetypes: identical element base properties compare equal") {
        const auto first = make_element();
        const auto second = make_element();
        CHECK_FALSE(first.base_properties_are_different(second));
        CHECK_FALSE(second.base_properties_are_different(first));
    }

    TEST_CASE("EditorArchetypes: each tracked element base property detects changes") {
        const auto original = make_element();
        auto changed = make_element();
        SUBCASE("identity") { changed.id = SimpleGuid::from_value(999); }
        SUBCASE("type") { changed.type_name = "Sprite2D"; }
        SUBCASE("name") { changed.name = "Custom Transform"; }
        SUBCASE("owner") { changed.owner_id = SimpleGuid::from_value(99); }
        SUBCASE("duplication policy") { changed.allows_duplication = false; }
        CHECK(original.base_properties_are_different(changed));
        CHECK(changed.base_properties_are_different(original));
    }

    TEST_CASE("EditorArchetypes: scalar YAML comparison detects additions and changed keys or values") {
        const auto original = make_element();
        auto candidate = make_element();
        CHECK_FALSE(original.data_is_different(candidate));
        SUBCASE("map order is immaterial") {
            candidate.data = YAML::Load("{speed: 5, enabled: true}");
            CHECK_FALSE(original.data_is_different(candidate));
        }
        SUBCASE("extra property") {
            candidate.data["extra"] = 7;
            CHECK(original.data_is_different(candidate));
        }
        SUBCASE("different key at same size") {
            candidate.data = YAML::Load("{enabled: true, acceleration: 5}");
            CHECK(original.data_is_different(candidate));
        }
        SUBCASE("different scalar value") {
            candidate.data["speed"] = 8;
            CHECK(original.data_is_different(candidate));
        }
    }

    TEST_CASE("EditorArchetypes: complete element comparison detects metadata and nested YAML changes") {
        auto original = make_element();
        auto candidate = make_element();
        original.data = YAML::Load("{position: {x: 1, y: 2}, tags: [a, b]}");
        candidate.data = YAML::Load("{tags: [a, b], position: {y: 2, x: 1}}");
        CHECK_FALSE(original.is_different_from(candidate));
        SUBCASE("name") { candidate.name = "Renamed"; }
        SUBCASE("type") { candidate.type_name = "Sprite2D"; }
        SUBCASE("duplication policy") { candidate.allows_duplication = false; }
        SUBCASE("nested property") { candidate.data["position"]["x"] = 9; }
        SUBCASE("sequence order") {
            candidate.data = YAML::Load("{position: {x: 1, y: 2}, tags: [b, a]}");
        }
        CHECK(original.is_different_from(candidate));
    }

    TEST_CASE("EditorArchetypes: entity comparison accepts matching elements by ID even if reordered") {
        const auto original = make_entity();
        auto candidate = make_entity();
        CHECK_FALSE(original.is_different_from(candidate));
        std::swap(candidate.elements[0], candidate.elements[1]);
        CHECK_FALSE(original.is_different_from(candidate));
    }

    TEST_CASE("EditorArchetypes: entity comparison detects name hierarchy and element membership changes") {
        const auto original = make_entity();
        auto candidate = make_entity();
        SUBCASE("name") { candidate.name = "NPC"; }
        SUBCASE("parent") { candidate.parent_id = SimpleGuid::invalid(); }
        SUBCASE("children") { candidate.child_ids.clear(); }
        SUBCASE("element count") { candidate.elements.pop_back(); }
        SUBCASE("element ID") { candidate.elements[0].id = SimpleGuid::from_value(999); }
        SUBCASE("element base property") { candidate.elements[0].name = "Renamed"; }
        CHECK(original.is_different_from(candidate));
    }

    TEST_CASE("EditorArchetypes: element lookup returns the stored element or nullptr") {
        auto entity = make_entity();
        auto* found = entity.get_element_by_id(SimpleGuid::from_value(102));
        REQUIRE(found == &entity.elements[1]);
        found->name = "Player Sprite";
        CHECK(entity.elements[1].name == "Player Sprite");
        CHECK(entity.get_element_by_id(SimpleGuid::from_value(999)) == nullptr);
        EntityArchetype empty;
        CHECK(empty.get_element_by_id(SimpleGuid::from_value(101)) == nullptr);
    }

    TEST_CASE("EditorArchetypes: type lookup returns all exact matches in storage order") {
        auto entity = make_entity();
        entity.elements.push_back(make_element("Transform", 103));
        const auto transforms = entity.get_elements_by_type_name("Transform");
        REQUIRE(transforms.size() == 2);
        CHECK(transforms[0] == &entity.elements[0]);
        CHECK(transforms[1] == &entity.elements[2]);
        CHECK(entity.get_elements_by_type_name("transform").empty());
        CHECK(entity.get_elements_by_type_name("Missing").empty());
        CHECK(entity.get_elements_by_type_name("").empty());
    }

    TEST_CASE("EditorArchetypes: primary transform prefers first nonduplicable transform then falls back") {
        auto entity = make_entity();
        entity.elements[0].allows_duplication = true;
        entity.elements.push_back(make_element("Transform", 103, false));
        entity.elements.push_back(make_element("Transform", 104, false));
        entity.elements[1].allows_duplication = false; // Not a Transform.
        CHECK(entity.get_primary_transform_id().get_value() == 103);
        entity.elements[2].allows_duplication = true;
        entity.elements[3].allows_duplication = true;
        CHECK(entity.get_primary_transform_id().get_value() == 101);
        entity.elements = {make_element("Sprite2D", 102, false)};
        CHECK_FALSE(entity.get_primary_transform_id().is_valid());
        entity.elements.clear();
        CHECK_FALSE(entity.get_primary_transform_id().is_valid());
    }

    TEST_CASE("EditorArchetypes: type presence tracks exact membership and removal") {
        auto entity = make_entity();
        CHECK(entity.has_element_of_type("Transform"));
        CHECK(entity.has_element_of_type("Sprite2D"));
        CHECK_FALSE(entity.has_element_of_type("transform"));
        CHECK_FALSE(entity.has_element_of_type("Missing"));
        CHECK_FALSE(entity.has_element_of_type(""));
        entity.elements.erase(entity.elements.begin());
        CHECK_FALSE(entity.has_element_of_type("Transform"));
        CHECK(entity.has_element_of_type("Sprite2D"));
        entity.elements.clear();
        CHECK_FALSE(entity.has_element_of_type("Sprite2D"));
    }
}
