// Tests for Editor/management/RealmSnapshot.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/management/RealmSnapshot.h>
#include "../fixtures/RealmFixtures.h"

namespace {
namespace coverage_batch_001 {
namespace {
using namespace EditorCri;
auto sample() { register_types(); auto a=entity(); a.elements.push_back(element()); return std::vector<EntityArchetype>{a}; }
auto snap(const std::vector<EntityArchetype>& a) { return RealmSnapshot::load_from_entity_archetype_vector(a); }
}
TEST_SUITE("EditorCRI001_RealmSnapshot") {
    TEST_CASE("CRI001 RealmSnapshot: empty snapshot has no entries") {
        RealmSnapshot s; CHECK(s.get_entity_map().empty()); CHECK(s.get_entity_by_id(id(1))==nullptr); CHECK(s.get_element_by_id(id(2))==nullptr); CHECK(s.validate_snapshot({}));
    }

    TEST_CASE("CRI001 RealmSnapshot: source path survives move construction and assignment") {
        RealmSnapshot a; a.set_source_file_path("first.realm"); RealmSnapshot b(std::move(a)); CHECK(b.get_source_file_path()=="first.realm"); RealmSnapshot c; c=std::move(b); CHECK(c.get_source_file_path()=="first.realm");
    }

    TEST_CASE("CRI001 RealmSnapshot: file factory keeps source path and entities") {
        TempRealm f; auto p=f.write("Entities: [{name: Root, id: 9}]"); auto s=RealmSnapshot::load_from_file(p); CHECK(s.get_source_file_path()==p); REQUIRE(s.get_entity_by_id(id(9))); CHECK(s.get_entity_by_id(id(9))->name=="Root");
    }

    TEST_CASE("CRI001 RealmSnapshot: vector factory indexes entities and elements") {
        auto a=sample(); auto s=snap(a); REQUIRE(s.get_entity_by_id(a[0].id)); REQUIRE(s.get_element_by_id(a[0].elements[0].id)); CHECK(s.get_entity_by_id(a[0].id)->name=="Player"); CHECK(s.get_element_by_id(a[0].elements[0].id)->owner_id==a[0].id); CHECK(s.validate_snapshot(a));
    }

    TEST_CASE("CRI001 RealmSnapshot: snapshot YAML remains independent of subsequent live edits") {
        auto a=sample(); auto s=snap(a); auto eid=a[0].elements[0].id; a[0].elements[0].data["name"]="Changed"; REQUIRE(s.get_element_by_id(eid)); CHECK(s.get_element_by_id(eid)->data["name"].as<std::string>()=="Transform"); REQUIRE(s.get_initial_element_data_as_string(eid)); CHECK(YAML::Load(*s.get_initial_element_data_as_string(eid))["name"].as<std::string>()=="Transform");
    }

    TEST_CASE("CRI001 RealmSnapshot: validation rejects missing and renamed entities") {
        auto a=sample(); auto s=snap(a); CHECK_FALSE(s.validate_snapshot({})); a[0].name="Renamed"; CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI001 RealmSnapshot: validation rejects hierarchy and element data changes") {
        auto a=sample(); auto s=snap(a); a[0].parent_id=id(9); CHECK_FALSE(s.validate_snapshot(a)); a[0].parent_id=SimpleGuid::invalid(); a[0].elements[0].data["name"]="Other"; CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI001 RealmSnapshot: fossilized property comparisons detect edits and missing keys") {
        auto a=sample(); auto s=snap(a); auto eid=a[0].elements[0].id; REQUIRE(s.get_fossilized_data_for_element(eid)); CHECK_FALSE(s.has_element_property_changed(eid,"name",PropertyValue{std::string("Transform")})); CHECK(s.has_element_property_changed(eid,"name",PropertyValue{std::string("Other")})); CHECK(s.has_element_property_changed(eid,"absent",PropertyValue{1})); CHECK(s.has_element_property_changed(id(9),"name",PropertyValue{1}));
    }

    TEST_CASE("CRI001 RealmSnapshot: missing snapshot entries are reported as modified") {
        RealmSnapshot s; auto a=entity(); auto e=element(); CHECK(s.is_entity_modified(a)); CHECK(s.is_element_modified(e)); CHECK(s.get_initial_element_data_as_string(e.id)==nullptr); CHECK(s.get_initial_data_property_value(e.id,"name").empty());
    }

    TEST_CASE("CRI001 RealmSnapshot: entity dirty detection tracks reflected value edits") {
        auto a=sample(); auto s=snap(a); CHECK_FALSE(s.is_entity_modified(a[0])); a[0].elements[0].data["name"]="Changed"; CHECK(s.is_entity_modified(a[0]));
    }
}
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
namespace {
using namespace EditorCri;
std::vector<EntityArchetype> model(){register_types();auto a=entity(11,"Root");a.elements.push_back(element("Transform",201,"{name: Transform, visible: true}"));a.elements[0].owner_id=a.id;return {a};}
}
TEST_SUITE("EditorCRI003_RealmSnapshot") {
    TEST_CASE("CRI003 RealmSnapshot: same size with replaced entity ID rejected") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].id=id(99);CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI003 RealmSnapshot: child list mismatch rejected") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].child_ids.push_back(id(12));CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI003 RealmSnapshot: element count mismatch rejected") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].elements.clear();CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI003 RealmSnapshot: element ID mismatch rejected") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].elements[0].id=id(202);CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI003 RealmSnapshot: element type mismatch rejected") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].elements[0].type_name="Other";CHECK_FALSE(s.validate_snapshot(a));
    }

    TEST_CASE("CRI003 RealmSnapshot: new state marks otherwise identical entity and element dirty") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);a[0].state=ArchetypeState::New;CHECK(s.is_entity_modified(a[0]));a[0].state=ArchetypeState::UnModified;a[0].elements[0].state=ArchetypeState::New;CHECK(s.is_element_modified(a[0].elements[0]));
    }

    TEST_CASE("CRI003 RealmSnapshot: owner and duplication changes independently marked dirty") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);auto& e=a[0].elements[0];e.owner_id=id(99);CHECK(s.is_element_modified(e));e.owner_id=a[0].id;e.allows_duplication=false;CHECK(s.is_element_modified(e));
    }

    TEST_CASE("CRI003 RealmSnapshot: same count with deleted property still detects change") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);auto& e=a[0].elements[0];e.data.remove("visible");e.data["unreflected"]=true;CHECK(s.is_element_modified(e));
    }

    TEST_CASE("CRI003 RealmSnapshot: fossilized property string retained after live edit") {
        auto a=model();auto s=RealmSnapshot::load_from_entity_archetype_vector(a);auto& e=a[0].elements[0];e.data["name"]="Changed";CHECK(s.get_initial_data_property_value(e.id,"name")=="Transform");CHECK(s.get_initial_data_property_value(e.id,"missing").empty());
    }

    TEST_CASE("CRI003 RealmSnapshot: unreflected element cannot be declared unchanged from fossilized data") {
        auto a=model();a[0].elements[0].type_name="CRI003Unknown";auto s=RealmSnapshot::load_from_entity_archetype_vector(a);CHECK(s.get_fossilized_data_for_element(a[0].elements[0].id)==nullptr);CHECK(s.is_element_modified(a[0].elements[0]));
    }
}
} // namespace coverage_batch_003
} // namespace

#include <Editor/management/RealmLoader.h>
namespace {
namespace cri007_snapshot {
using namespace EditorCri;

TEST_SUITE("EditorCRI007_RealmSnapshot") {
    TEST_CASE("CRI007 RealmSnapshot: file snapshot indexes each element by authored ID") {
        register_types(); TempRealm f; auto p=f.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 21, name: Local}}}]"); auto s=RealmSnapshot::load_from_file(p); REQUIRE(s.get_element_by_id(id(21))); CHECK(s.get_element_by_id(id(21))->owner_id==id(11)); CHECK(s.get_source_file_path()==p);
    }

    TEST_CASE("CRI007 RealmSnapshot: file snapshot fossilizes reflected values") {
        register_types(); TempRealm f; auto s=RealmSnapshot::load_from_file(f.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 21, name: Local}}}]")); REQUIRE(s.get_fossilized_data_for_element(id(21))); CHECK_FALSE(s.has_element_property_changed(id(21),"name",std::string("Local"))); CHECK(s.has_element_property_changed(id(21),"name",std::string("Edited")));
    }

    TEST_CASE("CRI007 RealmSnapshot: file snapshot stores original component YAML") {
        register_types(); TempRealm f; auto s=RealmSnapshot::load_from_file(f.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 21, name: Local}}}]")); auto* original=s.get_initial_element_data_as_string(id(21)); REQUIRE(original); CHECK(YAML::Load(*original)["name"].as<std::string>()=="Local");
    }

    TEST_CASE("CRI007 RealmSnapshot: file snapshot agrees with independently parsed model") {
        register_types(); TempRealm f; auto p=f.write("Entities: [{name: Parent, id: 11}, {name: Child, id: 12, parent: 11, elements: {Transform: {id: 21, name: Local}}}]"); auto s=RealmSnapshot::load_from_file(p); auto model=load_archetypes_from_file(p); CHECK(s.validate_snapshot(model)); CHECK_FALSE(s.is_entity_modified(model[1]));
    }

    TEST_CASE("CRI007 RealmSnapshot: file snapshot remains immutable after file is rewritten") {
        register_types(); TempRealm f; auto p=f.write("Entities: [{name: Old, id: 11}]"); auto s=RealmSnapshot::load_from_file(p); f.write("Entities: [{name: New, id: 11}]"); REQUIRE(s.get_entity_by_id(id(11))); CHECK(s.get_entity_by_id(id(11))->name=="Old"); CHECK_FALSE(s.validate_snapshot(load_archetypes_from_file(p)));
    }

    TEST_CASE("CRI007 RealmSnapshot: file snapshot keeps unknown component original YAML") {
        register_types(); TempRealm f; auto s=RealmSnapshot::load_from_file(f.write("Entities: [{name: E, id: 11, elements: {Future: {id: 21, payload: [1, 2]}}}]")); REQUIRE(s.get_element_by_id(id(21))); REQUIRE(s.get_initial_element_data_as_string(id(21))); CHECK(YAML::Load(*s.get_initial_element_data_as_string(id(21)))["payload"][1].as<int>()==2); CHECK(s.get_fossilized_data_for_element(id(21))==nullptr);
    }

    TEST_CASE("CRI007 RealmSnapshot: moving file snapshot preserves indexed element identity") {
        register_types(); TempRealm f; auto s=RealmSnapshot::load_from_file(f.write("Entities: [{name: E, id: 11, elements: {Transform: {id: 21}}}]")); auto* e=s.get_element_by_id(id(21)); REQUIRE(e); RealmSnapshot moved(std::move(s)); CHECK(moved.get_element_by_id(id(21))==e);
    }

    TEST_CASE("CRI007 RealmSnapshot: missing file retains source path but has no indexes") {
        TempRealm f; auto s=RealmSnapshot::load_from_file(f.path()); CHECK(s.get_source_file_path()==f.path()); CHECK(s.get_entity_map().empty()); CHECK(s.get_element_by_id(id(21))==nullptr);
    }

    TEST_CASE("CRI007 RealmSnapshot: multiple elements indexed separately across owners") {
        register_types(); TempRealm f; auto s=RealmSnapshot::load_from_file(f.write("Entities: [{name: A, id: 11, elements: {Transform: {id: 21}}}, {name: B, id: 12, elements: {Transform: {id: 22}}}]")); REQUIRE(s.get_element_by_id(id(21))); REQUIRE(s.get_element_by_id(id(22))); CHECK(s.get_element_by_id(id(21))->owner_id==id(11)); CHECK(s.get_element_by_id(id(22))->owner_id==id(12));
    }

    TEST_CASE("CRI007 RealmSnapshot: duplicate source IDs cannot masquerade as identical snapshot") {
        auto a=entity(11),b=entity(12); auto s=RealmSnapshot::load_from_entity_archetype_vector({a,b}); CHECK_FALSE(s.validate_snapshot({a,a}));
    }
}
}
}

namespace {
using namespace EditorCri;
EntityArchetype metadata_model(){
    register_types();auto a=entity(11,"Root");a.is_visible=false;a.display_order=3;
    auto e=element("Transform",21,"{name: Local}");e.name="Local";e.owner_id=a.id;e.is_visible=false;e.display_order=5;a.elements.push_back(e);return a;
}
}
TEST_SUITE("EditorCRI011_RealmSnapshot") {
    using namespace EditorCri;
    TEST_CASE("CRI011 RealmSnapshot: snapshot preserves entity visibility and display order") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});auto* stored=s.get_entity_by_id(a.id);REQUIRE(stored);CHECK_FALSE(stored->is_visible);CHECK(stored->display_order==3);CHECK_FALSE(s.is_entity_modified(a));
    }

    TEST_CASE("CRI011 RealmSnapshot: snapshot preserves component visibility and display order") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});auto* stored=s.get_element_by_id(id(21));REQUIRE(stored);CHECK_FALSE(stored->is_visible);CHECK(stored->display_order==5);CHECK_FALSE(s.is_element_modified(a.elements[0]));
    }

    TEST_CASE("CRI011 RealmSnapshot: changing entity visibility detects dirty state and restoration clears it") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.is_visible=true;CHECK(s.is_entity_modified(a));CHECK_FALSE(s.validate_snapshot({a}));a.is_visible=false;CHECK_FALSE(s.is_entity_modified(a));CHECK(s.validate_snapshot({a}));
    }

    TEST_CASE("CRI011 RealmSnapshot: changing entity display order is detected independently") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.display_order=9;CHECK(s.is_entity_modified(a));CHECK_FALSE(s.validate_snapshot({a}));CHECK_FALSE(s.is_element_modified(a.elements[0]));
    }

    TEST_CASE("CRI011 RealmSnapshot: changing component visibility marks component and owner dirty") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.elements[0].is_visible=true;CHECK(s.is_element_modified(a.elements[0]));CHECK(s.is_entity_modified(a));CHECK_FALSE(s.validate_snapshot({a}));
    }

    TEST_CASE("CRI011 RealmSnapshot: changing component display order is detected independently") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.elements[0].display_order=6;CHECK(s.is_element_modified(a.elements[0]));CHECK(s.is_entity_modified(a));CHECK_FALSE(s.validate_snapshot({a}));
    }

    TEST_CASE("CRI011 RealmSnapshot: validation checks metadata name independently of YAML") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.elements[0].name="Changed metadata";CHECK_FALSE(s.validate_snapshot({a}));CHECK(s.is_element_modified(a.elements[0]));CHECK(s.get_initial_data_property_value(id(21),"name")=="Local");
    }

    TEST_CASE("CRI011 RealmSnapshot: validation checks component owner independently of YAML") {
        auto a=metadata_model();auto s=RealmSnapshot::load_from_entity_archetype_vector({a});a.elements[0].owner_id=id(99);CHECK_FALSE(s.validate_snapshot({a}));CHECK(s.is_element_modified(a.elements[0]));
    }

    TEST_CASE("CRI011 RealmSnapshot: duplicate entity cannot overwrite first fossilized property") {
        auto a=metadata_model();auto duplicate=metadata_model();duplicate.name="Duplicate";duplicate.elements[0].data["name"]="Different";auto s=RealmSnapshot::load_from_entity_archetype_vector({a,duplicate});REQUIRE(s.get_entity_by_id(id(11)));CHECK(s.get_entity_by_id(id(11))->name=="Root");CHECK(s.get_initial_data_property_value(id(21),"name")=="Local");CHECK_FALSE(s.is_entity_modified(a));CHECK_FALSE(s.validate_snapshot({a,duplicate}));
    }

    TEST_CASE("CRI011 RealmSnapshot: duplicate entity cannot leave orphan fossilized component entries") {
        auto a=metadata_model();auto duplicate=entity(11,"Duplicate");duplicate.elements.push_back(element("Transform",99,"{name: Orphan}"));auto s=RealmSnapshot::load_from_entity_archetype_vector({a,duplicate});CHECK(s.get_element_by_id(id(99))==nullptr);CHECK(s.get_initial_element_data_as_string(id(99))==nullptr);CHECK(s.get_fossilized_data_for_element(id(99))==nullptr);CHECK(s.validate_snapshot({a}));
    }
}
