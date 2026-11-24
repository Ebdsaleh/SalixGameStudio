// CRI relay 003: RealmSnapshot; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/management/RealmSnapshot.h>
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
