// CRI relay 001: ArchetypeInstantiator; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
namespace {
using namespace EditorCri;
void many(Realm& r, const std::vector<EntityArchetype>& a) { register_types(); InitContext c{}; ArchetypeInstantiator::instantiate_realm(a,&r,c); }
void one(Realm& r, const EntityArchetype& a) { register_types(); InitContext c{}; ArchetypeInstantiator::instantiate(a,&r,c); }
}
TEST_SUITE("EditorCRI001_ArchetypeInstantiator") {
    TEST_CASE("CRI001 ArchetypeInstantiator: empty input does not delete existing world") {
        Realm r; auto* existing=r.create_entity(id(9),"Existing"); many(r,{}); CHECK(r.get_entity_by_id(id(9))==existing); CHECK(r.get_entities().size()==1);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: single entity carries stable ID and name") {
        Realm r; one(r,entity()); REQUIRE(r.get_entity_by_id(id(10001))); CHECK(r.get_entity_by_id(id(10001))->get_name()=="Player"); CHECK(r.get_entities().size()==1);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: single transform reuses default component with authored ID") {
        Realm r; auto a=entity(); a.elements.push_back(element()); one(r,a); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); REQUIRE(live->get_transform()); CHECK(live->get_element_by_id(id(20001))==live->get_transform()); CHECK(live->get_elements_by_type_name("Transform").size()==1);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: single collider reuses default component") {
        Realm r; auto a=entity(); a.elements.push_back(element("BoxCollider",20002,"{}")); one(r,a); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK(live->get_element_by_id(id(20002))!=nullptr); CHECK(live->get_elements_by_type_name("BoxCollider").size()==1);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: unknown component skipped without removing entity") {
        Realm r; auto a=entity(); a.elements.push_back(element("CRI001_Unknown",20009,"{}")); one(r,a); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK(live->get_element_by_id(id(20009))==nullptr); CHECK(live->get_transform()!=nullptr);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: multi pass assigns all entity identities") {
        Realm r; many(r,{entity(11,"One"),entity(12,"Two")}); REQUIRE(r.get_entities().size()==2); REQUIRE(r.get_entity_by_id(id(11))); REQUIRE(r.get_entity_by_id(id(12))); CHECK(r.get_entity_by_id(id(11))->get_name()=="One"); CHECK(r.get_entity_by_id(id(12))->get_name()=="Two");
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: multi pass visibility applied to entity and transform") {
        Realm r; auto a=entity(); a.is_visible=false; a.elements.push_back(element("Transform",20001,"{}")); a.elements[0].is_visible=false; many(r,{a}); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK_FALSE(live->is_visible()); REQUIRE(live->get_transform()); CHECK_FALSE(live->get_transform()->is_visible());
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: multi pass position applied through reflection") {
        Realm r; auto a=entity(); a.elements.push_back(element("Transform",20001,"{position: {x: 3, y: 4, z: 5}}")); many(r,{a}); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); REQUIRE(live->get_transform()); auto p=live->get_transform()->get_position(); CHECK(p.x==doctest::Approx(3)); CHECK(p.y==doctest::Approx(4)); CHECK(p.z==doctest::Approx(5));
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: hierarchy resolved even when child precedes parent") {
        Realm r; auto a=entity(11,"Parent"); auto b=entity(12,"Child"); b.parent_id=a.id; many(r,{b,a}); auto* parent=r.get_entity_by_id(a.id); auto* child=r.get_entity_by_id(b.id); REQUIRE(parent); REQUIRE(child); CHECK(child->get_parent()==parent); REQUIRE(parent->get_children().size()==1); CHECK(parent->get_children()[0]==child);
    }

    TEST_CASE("CRI001 ArchetypeInstantiator: missing parent leaves entity root") {
        Realm r; auto a=entity(); a.parent_id=id(99); many(r,{a}); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK(live->get_parent()==nullptr); CHECK(live->is_root());
    }
}
