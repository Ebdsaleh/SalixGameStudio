// CRI relay 001: RealmSnapshot; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/management/RealmSnapshot.h>
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
