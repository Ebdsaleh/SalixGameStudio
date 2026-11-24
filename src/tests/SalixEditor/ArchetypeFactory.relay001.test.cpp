// CRI relay 001: ArchetypeFactory; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/ArchetypeFactory.h>
#include <Editor/management/EditorRealmManager.h>
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI001_ArchetypeFactory") {
    TEST_CASE("CRI001 ArchetypeFactory: unregistered component yields invalid archetype") {
        auto e=ArchetypeFactory::create_element_archetype("CRI001_UnknownType"); CHECK_FALSE(e.id.is_valid()); CHECK(e.type_name.empty());
    }

    TEST_CASE("CRI001 ArchetypeFactory: registered transform includes reflected defaults") {
        register_types(); auto e=ArchetypeFactory::create_element_archetype("Transform"); REQUIRE(e.id.is_valid()); CHECK(e.type_name=="Transform"); CHECK(e.state==ArchetypeState::New); CHECK(e.data["position"].IsMap()); CHECK(e.data["scale"]["x"].as<float>()==doctest::Approx(1));
    }

    TEST_CASE("CRI001 ArchetypeFactory: new entities have distinct identities") {
        register_types(); auto a=ArchetypeFactory::create_entity_archetype("A"); auto b=ArchetypeFactory::create_entity_archetype("B"); CHECK(a.id.is_valid()); CHECK(b.id.is_valid()); CHECK(a.id!=b.id); CHECK(a.name=="A"); CHECK(b.name=="B");
    }

    TEST_CASE("CRI001 ArchetypeFactory: default elements linked to their entity") {
        register_types(); auto a=ArchetypeFactory::create_entity_archetype(); REQUIRE(a.elements.size()==2); CHECK(a.name=="Entity"); CHECK(a.state==ArchetypeState::New); CHECK(a.has_element_of_type("Transform")); CHECK(a.has_element_of_type("BoxCollider")); for(const auto& e:a.elements){ CHECK(e.id.is_valid()); CHECK(e.owner_id==a.id); CHECK_FALSE(e.allows_duplication); }
    }

    TEST_CASE("CRI001 ArchetypeFactory: duplicate element owns independent YAML") {
        auto a=entity(); auto e=element("Custom",20001,"{name: Custom, p: {x: 3}}"); auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK(b.id.is_valid()); CHECK(b.id!=e.id); CHECK(b.state==ArchetypeState::New); CHECK(b.name=="Custom (Copy)"); b.data["p"]["x"]=9; CHECK(e.data["p"]["x"].as<int>()==3);
    }

    TEST_CASE("CRI001 ArchetypeFactory: duplicate element skips occupied sibling names") {
        auto a=entity(); auto e=element("Custom"); a.elements={element("Custom (Copy)"),element("Custom (Copy 2)",20002)}; auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK(b.name=="Custom (Copy 3)"); CHECK(b.data["name"].as<std::string>()==b.name);
    }

    TEST_CASE("CRI001 ArchetypeFactory: duplicate normalizes prior copy suffix and preserves policy") {
        auto a=entity(); auto e=element("Custom"); e.name="Custom (Copy 8)"; e.allows_duplication=false; auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK(b.name=="Custom (Copy)"); CHECK_FALSE(b.allows_duplication); CHECK(b.type_name==e.type_name);
    }

    TEST_CASE("CRI001 ArchetypeFactory: realm existence is based on ID not name") {
        auto a=entity(); auto b=entity(10002,"Player"); auto c=entity(10001,"Renamed"); CHECK_FALSE(ArchetypeFactory::entity_exists_in_realm({a},b)); CHECK(ArchetypeFactory::entity_exists_in_realm({a},c));
    }

    TEST_CASE("CRI001 ArchetypeFactory: unique name considers both realm and pending batch") {
        TempRealm f; EditorRealmManager m; m.load_realm_from_file(f.write("Entities: [{name: 'Player (Copy)', id: 9}]")); auto pending=entity(10003,"Player (Copy 2)"); CHECK(ArchetypeFactory::generate_unique_entity_name("Player (Copy 8)",&m,{pending})=="Player (Copy 3)");
    }

    TEST_CASE("CRI001 ArchetypeFactory: family duplication rejects absent context and manager") {
        EditorContext c; auto a=entity(); CHECK(ArchetypeFactory::duplicate_entity_archetype_and_children(a.id,nullptr).empty()); CHECK(ArchetypeFactory::duplicate_entity_archetype_and_children(a.id,&c).empty()); CHECK(ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(a,nullptr).empty()); CHECK(ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(a,&c).empty());
    }
}
