// Tests for Editor/management/RealmLoader.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/management/RealmLoader.h>

namespace {
using namespace EditorCri;
auto load(TempRealm& f, const std::string& yaml) { return load_archetypes_from_file(f.write(yaml)); }
}
TEST_SUITE("EditorCRI001_RealmLoader") {
    TEST_CASE("CRI001 RealmLoader: missing file returns empty") {
        TempRealm f; CHECK(load_archetypes_from_file(f.path()).empty());
    }

    TEST_CASE("CRI001 RealmLoader: invalid YAML returns empty") {
        TempRealm f; CHECK(load(f,"Entities: [").empty());
    }

    TEST_CASE("CRI001 RealmLoader: missing or nonsequence Entities rejected") {
        TempRealm f; CHECK(load(f,"Other: []").empty()); CHECK(load(f,"Entities: {name: Bad}").empty());
    }

    TEST_CASE("CRI001 RealmLoader: empty entity sequence accepted") {
        TempRealm f; CHECK(load(f,"Entities: []").empty());
    }

    TEST_CASE("CRI001 RealmLoader: identity and root defaults preserved") {
        TempRealm f; auto r=load(f,"Entities: [{name: Root, id: 10001}]"); REQUIRE(r.size()==1); CHECK(r[0].name=="Root"); CHECK(r[0].id==id(10001)); CHECK_FALSE(r[0].parent_id.is_valid());
    }

    TEST_CASE("CRI001 RealmLoader: invalid entities skipped without dropping valid siblings") {
        TempRealm f; auto r=load(f,"Entities: [bad, {name: NoId}, {id: 3}, {name: BadId, id: nope}, {name: Good, id: 9}]"); REQUIRE(r.size()==1); CHECK(r[0].id==id(9));
    }

    TEST_CASE("CRI001 RealmLoader: parent references and self parenting") {
        TempRealm f; auto r=load(f,"Entities: [{name: Child, id: 2, parent: 1}, {name: Self, id: 3, parent: 3}, {name: Root, id: 1, parent: null}]"); REQUIRE(r.size()==3); CHECK(r[0].parent_id==id(1)); CHECK_FALSE(r[1].parent_id.is_valid()); CHECK_FALSE(r[2].parent_id.is_valid());
    }

    TEST_CASE("CRI001 RealmLoader: element metadata separated from property YAML") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 1, elements: {Transform: {id: 2, name: Local, allows_duplication: false, position: {x: 1,y: 2,z: 3}}}}]"); REQUIRE(r.size()==1); REQUIRE(r[0].elements.size()==1); const auto& e=r[0].elements[0]; CHECK(e.id==id(2)); CHECK(e.owner_id==id(1)); CHECK(e.name=="Local"); CHECK_FALSE(e.allows_duplication); CHECK_FALSE(e.data["id"]); CHECK_FALSE(e.data["allows_duplication"]); CHECK(e.data["position"]["x"].as<int>()==1);
    }

    TEST_CASE("CRI001 RealmLoader: unknown component data and legacy name preserved") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 1, elements: {FutureType: {id: 2, payload: [4,5]}}}]"); REQUIRE(r.size()==1); REQUIRE(r[0].elements.size()==1); const auto& e=r[0].elements[0]; CHECK(e.type_name=="FutureType"); CHECK(e.name=="FutureType"); CHECK(e.data["name"].as<std::string>()=="FutureType"); CHECK(e.allows_duplication); CHECK(e.data["payload"][1].as<int>()==5);
    }

    TEST_CASE("CRI001 RealmLoader: malformed elements skipped independently") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 1, elements: {Bad: 3, Missing: {name: X}, Invalid: {id: nope}, BadFlag: {id: 4, allows_duplication: nope}, Good: {id: 5}}}]"); REQUIRE(r.size()==1); REQUIRE(r[0].elements.size()==1); CHECK(r[0].elements[0].id==id(5));
    }
}


namespace {
using namespace EditorCri;
}
TEST_SUITE("EditorCRI007_RealmLoader") {
    using namespace EditorCri;
    TEST_CASE("CRI007 RealmLoader: non scalar root yields empty model") {
        TempRealm f; CHECK(load(f,"[1, 2, 3]").empty());
    }

    TEST_CASE("CRI007 RealmLoader: malformed parent field falls back to root") {
        TempRealm f; auto r=load(f,"Entities: [{name: Root, id: 11, parent: invalid}]"); REQUIRE(r.size()==1); CHECK_FALSE(r[0].parent_id.is_valid());
    }

    TEST_CASE("CRI007 RealmLoader: zero parent is treated as root") {
        TempRealm f; auto r=load(f,"Entities: [{name: Root, id: 11, parent: 0}]"); REQUIRE(r.size()==1); CHECK_FALSE(r[0].parent_id.is_valid());
    }

    TEST_CASE("CRI007 RealmLoader: uint64 identity beyond 32 bits survives loading") {
        TempRealm f; auto r=load(f,"Entities: [{name: Large, id: 4294967297, parent: 4294967296}]"); REQUIRE(r.size()==1); CHECK(r[0].id==id(4294967297ULL)); CHECK(r[0].parent_id==id(4294967296ULL));
    }

    TEST_CASE("CRI007 RealmLoader: empty elements mapping preserves entity") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 11, elements: {}}]"); REQUIRE(r.size()==1); CHECK(r[0].elements.empty());
    }

    TEST_CASE("CRI007 RealmLoader: nonmapping elements are ignored without dropping entity") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 11, elements: [one,two]}]"); REQUIRE(r.size()==1); CHECK(r[0].elements.empty());
    }

    TEST_CASE("CRI007 RealmLoader: nested component data remains structured") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 11, elements: {Future: {id: 21, settings: {enabled: true, list: [a,b]}}}}]"); REQUIRE(r.size()==1); REQUIRE(r[0].elements.size()==1); auto n=r[0].elements[0].data["settings"]; CHECK(n["enabled"].as<bool>()); CHECK(n["list"][1].as<std::string>()=="b");
    }

    TEST_CASE("CRI007 RealmLoader: invalid element name skips only malformed component") {
        TempRealm f; auto r=load(f,"Entities: [{name: E, id: 11, elements: {Bad: {id: 21, name: [a,b]}, Good: {id: 22}}}]"); REQUIRE(r.size()==1); REQUIRE(r[0].elements.size()==1); CHECK(r[0].elements[0].id==id(22));
    }

    TEST_CASE("CRI007 RealmLoader: owner IDs assigned to components in separate entities") {
        TempRealm f; auto r=load(f,"Entities: [{name: A, id: 11, elements: {Transform: {id: 21}}}, {name: B, id: 12, elements: {Transform: {id: 22}}}]"); REQUIRE(r.size()==2); REQUIRE(r[0].elements.size()==1); REQUIRE(r[1].elements.size()==1); CHECK(r[0].elements[0].owner_id==id(11)); CHECK(r[1].elements[0].owner_id==id(12));
    }

    TEST_CASE("CRI007 RealmLoader: repeated loads do not share mutable YAML nodes") {
        TempRealm f; auto path=f.write("Entities: [{name: E, id: 11, elements: {Transform: {id: 21, name: Original}}}]"); auto a=load_archetypes_from_file(path),b=load_archetypes_from_file(path); REQUIRE(a.size()==1); REQUIRE(b.size()==1); a[0].elements[0].data["name"]="Edited"; CHECK(b[0].elements[0].data["name"].as<std::string>()=="Original");
    }
}


TEST_SUITE("EditorCRI011_RealmLoader") {
    using namespace EditorCri;
    TEST_CASE("CRI011 RealmLoader: zero entity identity rejected while valid siblings load") {
        TempRealm f;auto r=load(f,"Entities: [{name: Invalid, id: 0}, {name: Valid, id: 11}]");REQUIRE(r.size()==1);CHECK(r[0].id==id(11));
    }

    TEST_CASE("CRI011 RealmLoader: duplicate entity identity preserves first complete record") {
        TempRealm f;auto r=load(f,"Entities: [{name: First, id: 11}, {name: Duplicate, id: 11}, {name: Next, id: 12}]");REQUIRE(r.size()==2);CHECK(r[0].name=="First");CHECK(r[1].id==id(12));
    }

    TEST_CASE("CRI011 RealmLoader: duplicate entity cannot reserve component IDs from later valid entities") {
        TempRealm f;auto r=load(f,"Entities: [{name: First, id: 11}, {name: Duplicate, id: 11, elements: {Transform: {id: 21}}}, {name: Valid, id: 12, elements: {Transform: {id: 21}}}]");REQUIRE(r.size()==2);REQUIRE(r[1].elements.size()==1);CHECK(r[1].elements[0].owner_id==id(12));
    }

    TEST_CASE("CRI011 RealmLoader: invalid zero component does not discard valid owner") {
        TempRealm f;auto r=load(f,"Entities: [{name: Root, id: 11, elements: {Bad: {id: 0}, Good: {id: 21}}}]");REQUIRE(r.size()==1);REQUIRE(r[0].elements.size()==1);CHECK(r[0].elements[0].id==id(21));
    }

    TEST_CASE("CRI011 RealmLoader: duplicate component IDs within one entity preserve first") {
        TempRealm f;auto r=load(f,"Entities: [{name: Root, id: 11, elements: {First: {id: 21, name: Original}, Second: {id: 21, name: Duplicate}}}]");REQUIRE(r.size()==1);REQUIRE(r[0].elements.size()==1);CHECK(r[0].elements[0].type_name=="First");CHECK(r[0].elements[0].name=="Original");
    }

    TEST_CASE("CRI011 RealmLoader: duplicate component IDs across owners preserve unique lookup") {
        TempRealm f;auto r=load(f,"Entities: [{name: A, id: 11, elements: {Transform: {id: 21}}}, {name: B, id: 12, elements: {Sprite2D: {id: 21}, Camera: {id: 22}}}]");REQUIRE(r.size()==2);REQUIRE(r[0].elements.size()==1);REQUIRE(r[1].elements.size()==1);CHECK(r[0].elements[0].owner_id==id(11));CHECK(r[1].elements[0].id==id(22));
    }

    TEST_CASE("CRI011 RealmLoader: malformed component does not reserve its ID") {
        TempRealm f;auto r=load(f,"Entities: [{name: Root, id: 11, elements: {Bad: {id: 21, name: [bad]}, Good: {id: 21, name: Valid}}}]");REQUIRE(r.size()==1);REQUIRE(r[0].elements.size()==1);CHECK(r[0].elements[0].name=="Valid");
    }

    TEST_CASE("CRI011 RealmLoader: malformed entity name does not reserve its ID") {
        TempRealm f;auto r=load(f,"Entities: [{name: [bad], id: 11}, {name: Valid, id: 11}]");REQUIRE(r.size()==1);CHECK(r[0].name=="Valid");
    }

    TEST_CASE("CRI011 RealmLoader: sequence component key skips only invalid component") {
        TempRealm f;auto r=load(f,"Entities:\n - name: Root\n   id: 11\n   elements:\n     ? [Bad, Key]\n     : {id: 21}\n     Good: {id: 22}\n");REQUIRE(r.size()==1);REQUIRE(r[0].elements.size()==1);CHECK(r[0].elements[0].id==id(22));
    }

    TEST_CASE("CRI011 RealmLoader: null component key skips only invalid component") {
        TempRealm f;auto r=load(f,"Entities: [{name: Root, id: 11, elements: {null: {id: 21}, Good: {id: 22}}}]");REQUIRE(r.size()==1);REQUIRE(r[0].elements.size()==1);CHECK(r[0].elements[0].type_name=="Good");
    }
}
