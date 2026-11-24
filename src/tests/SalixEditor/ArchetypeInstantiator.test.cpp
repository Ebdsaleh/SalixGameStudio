// Tests for Editor/ArchetypeInstantiator.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "fixtures/ArchetypeFixtures.h"
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include "fixtures/RealmFixtures.h"
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/BoxCollider.h>

namespace {
namespace coverage_batch_001 {
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
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
namespace {
using namespace EditorCri;
void instantiate_one(Realm& r,EntityArchetype a){register_types();EnumRegistry::register_all_enums();InitContext c{};ArchetypeInstantiator::instantiate(a,&r,c);}
Sprite2D* sprite(Realm& r,const char* data){auto a=entity();a.elements.push_back(element("Sprite2D",20001,data));instantiate_one(r,a);auto* live=r.get_entity_by_id(a.id);REQUIRE(live);auto* s=live->get_element<Sprite2D>();REQUIRE(s);return s;}
}
TEST_SUITE("EditorCRI003_ArchetypeInstantiator") {
    TEST_CASE("CRI003 ArchetypeInstantiator: single transform applies all three vector properties") {
        Realm r;auto a=entity();a.elements.push_back(element("Transform",20001,"{position: {x: 1, y: 2, z: 3}, scale: {x: 2, y: 3, z: 4}, rotation: {x: 0, y: 0, z: 0}}"));instantiate_one(r,a);auto* t=r.get_entity_by_id(a.id)->get_transform();REQUIRE(t);CHECK(t->get_position().y==2);CHECK(t->get_scale().z==4);CHECK(t->get_rotation().x==0);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: new sprite receives authored ID and owner") {
        Realm r;auto* s=sprite(r,"{name: Sprite, texture_path: ''}");CHECK(s->get_id()==id(20001));CHECK(s->get_owner()==r.get_entity_by_id(id(10001)));CHECK(s->get_texture()==nullptr);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: sprite offset and pivot applied as Vector2") {
        Realm r;auto* s=sprite(r,"{offset: {x: 3, y: -2}, pivot: {x: 0.25, y: 0.75}}");CHECK(s->offset.x==3);CHECK(s->offset.y==-2);CHECK(s->pivot.x==doctest::Approx(.25));CHECK(s->pivot.y==doctest::Approx(.75));
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: sprite color applied as four independent channels") {
        Realm r;auto* s=sprite(r,"{color: {r: 0.2, g: 0.4, b: 0.6, a: 0.8}}");CHECK(s->color.r==doctest::Approx(.2));CHECK(s->color.g==doctest::Approx(.4));CHECK(s->color.b==doctest::Approx(.6));CHECK(s->color.a==doctest::Approx(.8));
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: sprite float integer and boolean properties applied") {
        Realm r;auto* s=sprite(r,"{local_rotation: 35.5, sorting_layer: -2, flip_h: true, flip_v: true, use_entity_rotation: true}");CHECK(s->local_rotation==doctest::Approx(35.5));CHECK(s->sorting_layer==-2);CHECK(s->flip_h);CHECK(s->flip_v);CHECK(s->use_entity_rotation);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: camera enum and active flag applied through reflection") {
        Realm r;auto a=entity();a.elements.push_back(element("Camera",20002,"{projection_mode: Orthographic, active: true, orthographic_size: 12}"));instantiate_one(r,a);auto* cam=r.get_entity_by_id(a.id)->get_element<Camera>();REQUIRE(cam);CHECK(cam->get_projection_mode()==ProjectionMode::Orthographic);CHECK(cam->get_is_active());CHECK(cam->get_orthographic_size()==12);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: second collider receives independent authored ID") {
        Realm r;auto a=entity();a.elements={element("BoxCollider",20001,"{}"),element("BoxCollider",20002,"{}")};instantiate_one(r,a);auto* live=r.get_entity_by_id(a.id);REQUIRE(live);CHECK(live->get_element_by_id(id(20001))!=nullptr);CHECK(live->get_element_by_id(id(20002))!=nullptr);CHECK(live->get_element_by_id(id(20001))!=live->get_element_by_id(id(20002)));
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: multi pass creates collider and sprite in same entity") {
        register_types();InitContext c{};Realm r;auto a=entity();a.elements={element("BoxCollider",20001,"{}"),element("Sprite2D",20002,"{sorting_layer: 7}")};ArchetypeInstantiator::instantiate_realm({a},&r,c);auto* live=r.get_entity_by_id(a.id);REQUIRE(live);CHECK(live->get_element_by_id(id(20001))!=nullptr);auto* s=live->get_element<Sprite2D>();REQUIRE(s);CHECK(s->get_id()==id(20002));CHECK(s->sorting_layer==7);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: debug output identifies null realm without dereference") {
        CoutCapture out;ArchetypeInstantiator::print_all_entity_ids(nullptr,"missing");CHECK(out.str().find("missing")!=std::string::npos);CHECK(out.str().find("nullptr")!=std::string::npos);
    }

    TEST_CASE("CRI003 ArchetypeInstantiator: debug output lists real realm IDs and names") {
        Realm r;r.create_entity(id(11),"Alpha");r.create_entity(id(12),"Beta");CoutCapture out;ArchetypeInstantiator::print_all_entity_ids(&r,"fixture");auto s=out.str();CHECK(s.find("contains 2 entities")!=std::string::npos);CHECK(s.find("Alpha")!=std::string::npos);CHECK(s.find("ID: 12")!=std::string::npos);
    }
}
} // namespace coverage_batch_003
} // namespace
