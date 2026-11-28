// Tests for Editor/Archetypes.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <doctest.h>
#include <Editor/Archetypes.h>
#include <utility>
#include "fixtures/ArchetypeFixtures.h"

namespace {
namespace coverage_batch_000 {
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
} // namespace coverage_batch_000
} // namespace

namespace {
namespace coverage_batch_001 {
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI001_Archetypes") {
    TEST_CASE("CRI001 Archetypes: empty entities compare equal") {
        auto a=entity(); auto b=entity(); CHECK_FALSE(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: child order is significant") {
        auto a=entity(); auto b=entity(); a.child_ids={id(7),id(8)}; b.child_ids={id(8),id(7)}; CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: lookup distinguishes IDs sharing names") {
        auto a=entity(); a.elements={element(),element("Transform",20002)}; CHECK(a.get_element_by_id(id(20002))==&a.elements[1]);
    }

    TEST_CASE("CRI001 Archetypes: primary transform ignores colliders") {
        auto a=entity(); a.elements={element("BoxCollider"),element("Transform",20002)}; a.elements[0].allows_duplication=false; CHECK(a.get_primary_transform_id()==id(20002));
    }

    TEST_CASE("CRI001 Archetypes: empty property maps compare equal") {
        auto a=element("Transform",1,"{}"); auto b=element("Transform",1,"{}"); CHECK_FALSE(a.data_is_different(b));
    }

    TEST_CASE("CRI001 Archetypes: empty versus populated properties compare different") {
        auto a=element("Transform",1,"{}"); auto b=element(); CHECK(a.data_is_different(b)); CHECK(b.data_is_different(a));
    }

    TEST_CASE("CRI001 Archetypes: nested map size changes are detected") {
        auto a=element("Transform",1,"{p: {x: 1}}"); auto b=element("Transform",1,"{p: {x: 1, y: 2}}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: nested sequence length changes are detected") {
        auto a=element("Transform",1,"{p: [1,2]}"); auto b=element("Transform",1,"{p: [1]}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: nested YAML node type changes are detected") {
        auto a=element("Transform",1,"{p: [1]}"); auto b=element("Transform",1,"{p: 1}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: owner changes propagate through entity comparison") {
        auto a=entity(); auto b=entity(); a.elements.push_back(element()); b.elements.push_back(element()); b.elements[0].owner_id=id(900); CHECK(a.is_different_from(b));
    }
}
} // namespace coverage_batch_001
} // namespace

TEST_SUITE("EditorCRI024_Archetypes") {
    using namespace EditorCri;
    TEST_CASE("CRI024 Archetypes: nested YAML maps compare equal independently of key ordering") {
        auto a=element("Transform",21,"{position: {x: 1, y: 2, z: 3}}");
        auto b=element("Transform",21,"{position: {z: 3, x: 1, y: 2}}");
        CHECK_FALSE(a.data_is_different(b));
        CHECK_FALSE(b.data_is_different(a));
    }
    TEST_CASE("CRI024 Archetypes: nested YAML scalar change is detected in both directions") {
        auto a=element("Transform",21,"{position: {x: 1, y: 2, z: 3}}");
        auto b=element("Transform",21,"{position: {x: 9, y: 2, z: 3}}");
        CHECK(a.data_is_different(b));
        CHECK(b.data_is_different(a));
    }
    TEST_CASE("CRI024 Archetypes: sequences preserve order and nested values") {
        auto a=element("Transform",21,"{points: [{x: 1}, {x: 2}]}");
        auto b=element("Transform",21,"{points: [{x: 1}, {x: 2}]}");
        CHECK_FALSE(a.data_is_different(b));
        b.data=YAML::Load("{points: [{x: 2}, {x: 1}]}");
        CHECK(a.data_is_different(b));
        CHECK(b.data_is_different(a));
    }
    TEST_CASE("CRI024 Archetypes: explicit null compares equal but differs from scalar or missing key") {
        auto a=element("Transform",21,"{target: null}");
        auto b=element("Transform",21,"{target: null}");
        CHECK_FALSE(a.data_is_different(b));
        b.data=YAML::Load("{target: 'null'}");
        CHECK(a.data_is_different(b));
        b.data=YAML::Load("{different: null}");
        CHECK(a.data_is_different(b));
        CHECK(b.data_is_different(a));
    }
    TEST_CASE("CRI024 Archetypes: nested comparison never mutates either authored document") {
        auto a=element("Transform",21,"{position: {x: 1}, tags: [a,b]}");
        auto b=element("Transform",21,"{position: {y: 1}, tags: [a,b]}");
        const auto before_a=YAML::Dump(a.data), before_b=YAML::Dump(b.data);
        CHECK(a.data_is_different(b));
        CHECK(YAML::Dump(a.data)==before_a);
        CHECK(YAML::Dump(b.data)==before_b);
    }
}
