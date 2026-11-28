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

#include <Editor/ArchetypeFactory.h>
namespace {
namespace cri007_instantiator {
using namespace EditorCri;

TEST_SUITE("EditorCRI007_ArchetypeInstantiator") {
    TEST_CASE("CRI007 ArchetypeInstantiator: null realm accepts single and multiple authored requests safely") {
        register_types(); InitContext c{}; auto a=entity(); a.elements.push_back(element()); CHECK_NOTHROW(ArchetypeInstantiator::instantiate(a,nullptr,c)); CHECK_NOTHROW(ArchetypeInstantiator::instantiate_realm({a},nullptr,c));
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single entity visibility matches multi pass") {
        register_types(); InitContext c{}; Realm one,many; auto a=entity(); a.is_visible=false; ArchetypeInstantiator::instantiate(a,&one,c); ArchetypeInstantiator::instantiate_realm({a},&many,c); REQUIRE(one.get_entity_by_id(a.id)); REQUIRE(many.get_entity_by_id(a.id)); CHECK_FALSE(one.get_entity_by_id(a.id)->is_visible()); CHECK(one.get_entity_by_id(a.id)->is_visible()==many.get_entity_by_id(a.id)->is_visible());
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single element visibility applied independently of entity") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements.push_back(element("Transform",201,"{}")); a.elements[0].is_visible=false; ArchetypeInstantiator::instantiate(a,&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK(live->is_visible()); CHECK_FALSE(live->get_transform()->is_visible());
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single collider authored visibility retained") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements.push_back(element("BoxCollider",201,"{}")); a.elements[0].is_visible=false; ArchetypeInstantiator::instantiate(a,&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); REQUIRE(live->get_element_by_id(id(201))); CHECK_FALSE(live->get_element_by_id(id(201))->is_visible());
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single sprite authored visibility retained") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements.push_back(element("Sprite2D",201,"{}")); a.elements[0].is_visible=false; ArchetypeInstantiator::instantiate(a,&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); REQUIRE(live->get_element_by_id(id(201))); CHECK_FALSE(live->get_element_by_id(id(201))->is_visible());
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: debug logs report actual created element ID") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements.push_back(element("Transform",201,"{}")); CoutCapture out; ArchetypeInstantiator::instantiate(a,&r,c); CHECK(out.str().find("Live Element 'Transform' created with ID: 201")!=std::string::npos);
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single factory archetype round trips default collider and transform IDs") {
        register_types(); EnumRegistry::register_all_enums(); InitContext c{}; Realm r; auto a=ArchetypeFactory::create_entity_archetype("Roundtrip"); ArchetypeInstantiator::instantiate(a,&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); REQUIRE(a.elements.size()==2); CHECK(live->get_element_by_id(a.elements[0].id)==live->get_transform()); CHECK(live->get_element_by_id(a.elements[1].id)!=nullptr);
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: multi pass applies authored sprite pivot and tint") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements.push_back(element("Sprite2D",201,"{pivot: {x: 0.2, y: 0.8}, color: {r: 0.1, g: 0.2, b: 0.3, a: 0.4}}")); ArchetypeInstantiator::instantiate_realm({a},&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); auto* s=live->get_element<Sprite2D>(); REQUIRE(s); CHECK(s->pivot.x==doctest::Approx(.2)); CHECK(s->color.a==doctest::Approx(.4));
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: multi pass skips unknown components but applies later known element") {
        register_types(); InitContext c{}; Realm r; auto a=entity(); a.elements={element("CRI007Unknown",201,"{}"),element("Transform",202,"{position: {x: 2, y: 3, z: 4}}")}; ArchetypeInstantiator::instantiate_realm({a},&r,c); auto* live=r.get_entity_by_id(a.id); REQUIRE(live); CHECK(live->get_element_by_id(id(201))==nullptr); CHECK(live->get_transform()->get_position().z==4);
    }

    TEST_CASE("CRI007 ArchetypeInstantiator: single path preserves entity ID after reflected component ID") {
        register_types(); InitContext c{}; Realm r; auto a=entity(11); a.elements.push_back(element("Transform",201,"{name: Authored}")); ArchetypeInstantiator::instantiate(a,&r,c); auto* live=r.get_entity_by_id(id(11)); REQUIRE(live); CHECK(live->get_transform()->get_id()==id(201)); CHECK(live->get_transform()->get_name()=="Authored");
    }
}
}
}

#include <Salix/assets/AssetManager.h>
#include <Salix/ecs/Sprite2D.h>
namespace {
using namespace EditorCri;
struct TexturedInstantiation {
    RenderProbe renderer; AssetManager assets; InitContext init{}; Realm realm;
    TexturedInstantiation(){register_types();assets.initialize(&renderer);init.asset_manager=&assets;}
    Sprite2D* run(const char* yaml,bool multi){
        auto a=entity(11);auto s=element("Sprite2D",21,yaml);s.owner_id=a.id;a.elements.push_back(s);
        if(multi)ArchetypeInstantiator::instantiate_realm({a},&realm,init);
        else ArchetypeInstantiator::instantiate(a,&realm,init);
        auto* live=realm.get_entity_by_id(id(11));REQUIRE(live);auto* sprite=live->get_element<Sprite2D>();REQUIRE(sprite);return sprite;
    }
};
}
TEST_SUITE("EditorCRI009_ArchetypeInstantiator") {
    using namespace EditorCri;
    TEST_CASE("CRI009 ArchetypeInstantiator: single authored ID property cannot overwrite metadata identity") {
        TexturedInstantiation f;auto* s=f.run("{id: 999, name: Sprite, texture_path: 'a.png'}",false);CHECK(s->get_id()==id(21));CHECK(s->get_name()=="Sprite");
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: multi authored ID property cannot overwrite metadata identity") {
        TexturedInstantiation f;auto* s=f.run("{id: 999, name: Sprite, texture_path: 'a.png'}",true);CHECK(s->get_id()==id(21));CHECK(s->get_name()=="Sprite");
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: single sprite dimensions derive from actual loaded texture") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', width: 5, height: 9}",false);REQUIRE(s->get_texture());CHECK(s->get_texture_width()==80);CHECK(s->get_texture_height()==40);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: multi sprite dimensions derive from actual loaded texture") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', width: 5, height: 9}",true);REQUIRE(s->get_texture());CHECK(s->get_texture_width()==80);CHECK(s->get_texture_height()==40);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: single sprite ignores malformed read-only dimensions") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', width: [bad], height: bad}",false);CHECK(s->get_texture_width()==80);CHECK(s->get_texture_height()==40);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: multi sprite ignores malformed read-only dimensions") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', width: [bad], height: bad}",true);CHECK(s->get_texture_width()==80);CHECK(s->get_texture_height()==40);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: single sprite applies texture offset and sorting alongside load") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', offset: {x: 2,y: 3}, sorting_layer: -5}",false);CHECK(s->get_texture()!=nullptr);CHECK(s->offset.x==2);CHECK(s->sorting_layer==-5);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: multi sprite applies texture pivot and flip alongside load") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: 'a.png', pivot: {x: 0.2,y: 0.8}, flip_h: true}",true);CHECK(s->get_texture()!=nullptr);CHECK(s->pivot.y==doctest::Approx(.8));CHECK(s->flip_h);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: same texture path shared across independently authored entities") {
        TexturedInstantiation f;auto a=entity(11),b=entity(12);a.elements.push_back(element("Sprite2D",21,"{texture_path: 'shared.png'}"));b.elements.push_back(element("Sprite2D",22,"{texture_path: 'shared.png'}"));ArchetypeInstantiator::instantiate_realm({a,b},&f.realm,f.init);auto* sa=f.realm.get_entity_by_id(id(11))->get_element<Sprite2D>();auto* sb=f.realm.get_entity_by_id(id(12))->get_element<Sprite2D>();REQUIRE(sa);REQUIRE(sb);REQUIRE(sa->get_texture());CHECK(sa->get_texture()==sb->get_texture());CHECK(f.renderer.texture_loads==1);
    }

    TEST_CASE("CRI009 ArchetypeInstantiator: empty path cannot create mock texture or fake dimensions") {
        TexturedInstantiation f;auto* s=f.run("{texture_path: '', width: 80, height: 40}",true);CHECK(s->get_texture()==nullptr);CHECK(s->get_texture_width()==0);CHECK(s->get_texture_height()==0);CHECK(f.renderer.texture_loads==0);
    }
}

namespace {
using namespace EditorCri;
struct CameraInstantiationParity {
    InitContext init{};
    Realm single, multiple;
    EntityArchetype authored=entity(11,"Camera owner");
    CameraInstantiationParity(const char* yaml) {
        register_types();
        EnumRegistry::register_all_enums();
        auto camera=element("Camera",21,yaml);
        camera.owner_id=authored.id;
        authored.elements.push_back(camera);
    }
    void instantiate_both() {
        ArchetypeInstantiator::instantiate(authored,&single,init);
        ArchetypeInstantiator::instantiate_realm({authored},&multiple,init);
    }
    Camera* get(Realm& realm) {
        auto* owner=realm.get_entity_by_id(id(11));
        REQUIRE(owner);
        auto* camera=dynamic_cast<Camera*>(owner->get_element_by_id(id(21)));
        REQUIRE(camera);
        return camera;
    }
};
}
TEST_SUITE("EditorCRI029_ArchetypeInstantiator") {
    using namespace EditorCri;
    TEST_CASE("CRI029 ArchetypeInstantiator: both creation pathways accept registered orthographic projection") {
        CameraInstantiationParity f("{projection_mode: Orthographic, orthographic_size: 12, active: true}");
        f.instantiate_both();
        for(auto* realm:{&f.single,&f.multiple}) {
            auto* camera=f.get(*realm);
            CHECK(camera->get_projection_mode()==ProjectionMode::Orthographic);
            CHECK(camera->get_orthographic_size()==12);
            CHECK(camera->get_is_active());
        }
    }
    TEST_CASE("CRI029 ArchetypeInstantiator: both creation pathways accept registered perspective projection") {
        CameraInstantiationParity f("{projection_mode: Perspective, field_of_view: 75}");
        f.instantiate_both();
        for(auto* realm:{&f.single,&f.multiple}) {
            auto* camera=f.get(*realm);
            CHECK(camera->get_projection_mode()==ProjectionMode::Perspective);
            CHECK(camera->get_field_of_view()==75);
        }
    }
    TEST_CASE("CRI029 ArchetypeInstantiator: unknown enum name preserves default while valid sibling fields apply") {
        CameraInstantiationParity f("{projection_mode: NotAProjection, orthographic_size: 14, field_of_view: 65}");
        const auto before=YAML::Dump(f.authored.elements[0].data);
        f.instantiate_both();
        for(auto* realm:{&f.single,&f.multiple}) {
            auto* camera=f.get(*realm);
            CHECK(camera->get_projection_mode()==ProjectionMode::Perspective);
            CHECK(camera->get_orthographic_size()==14);
            CHECK(camera->get_field_of_view()==65);
        }
        CHECK(YAML::Dump(f.authored.elements[0].data)==before);
    }
    TEST_CASE("CRI029 ArchetypeInstantiator: numeric text cannot inject unregistered scoped enum values") {
        CameraInstantiationParity f("{projection_mode: 99, field_of_view: 55}");
        f.instantiate_both();
        for(auto* realm:{&f.single,&f.multiple}) {
            auto* camera=f.get(*realm);
            CHECK(camera->get_projection_mode()==ProjectionMode::Perspective);
            CHECK(camera->get_field_of_view()==55);
        }
    }
    TEST_CASE("CRI029 ArchetypeInstantiator: malformed read-only matrices are ignored by both creation pathways") {
        CameraInstantiationParity f("{projection_mode: Orthographic, view_matrix: [bad], projection_matrix: invalid}");
        CHECK_NOTHROW(f.instantiate_both());
        for(auto* realm:{&f.single,&f.multiple}) {
            auto* camera=f.get(*realm);
            CHECK(camera->get_projection_mode()==ProjectionMode::Orthographic);
            CHECK(camera->get_view_matrix()[3][3]==doctest::Approx(1.f));
        }
    }
}
