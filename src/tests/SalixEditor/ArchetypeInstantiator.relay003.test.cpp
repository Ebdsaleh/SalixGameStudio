// CRI relay 003: ArchetypeInstantiator; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/BoxCollider.h>
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
