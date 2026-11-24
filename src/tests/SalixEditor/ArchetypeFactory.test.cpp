// Tests for Editor/ArchetypeFactory.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "fixtures/ArchetypeFixtures.h"
#include <Editor/ArchetypeFactory.h>
#include <Editor/management/EditorRealmManager.h>
#include "fixtures/RealmFixtures.h"
#include <Salix/ecs/Camera.h>

namespace {
namespace coverage_batch_001 {
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
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI003_ArchetypeFactory") {
    TEST_CASE("CRI003 ArchetypeFactory: sprite archetype captures typed defaults") {
        register_types();auto e=ArchetypeFactory::create_element_archetype("Sprite2D");REQUIRE(e.id.is_valid());CHECK(e.data["sorting_layer"].as<int>()==0);CHECK(e.data["offset"].IsMap());CHECK(e.data["color"].IsMap());CHECK(e.data["flip_h"].as<bool>()==false);
    }

    TEST_CASE("CRI003 ArchetypeFactory: camera archetype captures named enum and clip properties") {
        register_types();EnumRegistry::register_all_enums();auto e=ArchetypeFactory::create_element_archetype("Camera");REQUIRE(e.id.is_valid());CHECK(e.data["projection_mode"].as<std::string>()=="Perspective");CHECK(e.data["near_clip"].as<float>()>0);CHECK(e.data["far_clip"].as<float>()>e.data["near_clip"].as<float>());
    }

    TEST_CASE("CRI003 ArchetypeFactory: single duplicate receives fresh identity and no parent") {
        FamilyFixture f;f.family();auto* a=f.get(12);REQUIRE(a);auto b=ArchetypeFactory::duplicate_entity_archetype(*a,&f.manager,&f.services.ctx);CHECK(b.name=="Child (Copy)");CHECK(b.id!=a->id);CHECK_FALSE(b.parent_id.is_valid());CHECK(b.child_ids.empty());CHECK(b.state==ArchetypeState::New);
    }

    TEST_CASE("CRI003 ArchetypeFactory: duplicate elements are deep copies with new owner") {
        FamilyFixture f;f.root(11,"Root");auto* a=f.get(11);REQUIRE(a);a->elements.push_back(element("Transform",201,"{name: Local, position: {x: 1, y: 2, z: 3}}"));a->elements[0].allows_duplication=false;a->elements[0].is_visible=false;auto b=ArchetypeFactory::duplicate_entity_archetype(*a,&f.manager,&f.services.ctx);REQUIRE(b.elements.size()==1);auto& e=b.elements[0];CHECK(e.id!=a->elements[0].id);CHECK(e.owner_id==b.id);CHECK_FALSE(e.is_visible);CHECK_FALSE(e.allows_duplication);e.data["position"]["x"]=9;CHECK(a->elements[0].data["position"]["x"].as<int>()==1);
    }

    TEST_CASE("CRI003 ArchetypeFactory: live transform is preserved in detached duplicate") {
        FamilyFixture f;f.root(11,"Root");auto* a=f.get(11);REQUIRE(a);a->elements.push_back(element());auto* live=f.services.ctx.preview_realm->create_entity(id(11),"Root");live->get_transform()->set_position(3,4,5);auto b=ArchetypeFactory::duplicate_entity_archetype(*a,&f.manager,&f.services.ctx);REQUIRE(b.elements.size()==1);auto p=b.elements[0].data["position"].as<Vector3>();CHECK(p.x==doctest::Approx(3));CHECK(p.y==doctest::Approx(4));CHECK(p.z==doctest::Approx(5));
    }

    TEST_CASE("CRI003 ArchetypeFactory: unknown family ID returns no duplicates") {
        FamilyFixture f;f.family();CHECK(ArchetypeFactory::duplicate_entity_archetype_and_children(id(99),&f.services.ctx).empty());auto absent=entity(99,"Missing");CHECK(ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(absent,&f.services.ctx).empty());
    }

    TEST_CASE("CRI003 ArchetypeFactory: recursive copy rewires three generations") {
        FamilyFixture f;f.family();auto family=ArchetypeFactory::duplicate_entity_archetype_and_children(id(11),&f.services.ctx);REQUIRE(family.size()==3);CHECK_FALSE(family[0].parent_id.is_valid());CHECK(family[1].parent_id==family[0].id);CHECK(family[2].parent_id==family[1].id);REQUIRE(family[0].child_ids.size()==1);CHECK(family[0].child_ids[0]==family[1].id);CHECK(family[0].name=="Root (Copy)");CHECK(f.manager.get_realm_size()==4);
    }

    TEST_CASE("CRI003 ArchetypeFactory: subfamily copied as detached root") {
        FamilyFixture f;f.family();auto family=ArchetypeFactory::duplicate_entity_archetype_and_children(id(12),&f.services.ctx);REQUIRE(family.size()==2);CHECK_FALSE(family[0].parent_id.is_valid());CHECK(family[1].parent_id==family[0].id);CHECK(family[1].name=="Grandchild (Copy)");
    }

    TEST_CASE("CRI003 ArchetypeFactory: sibling family retains original external parent") {
        FamilyFixture f;f.family();auto* child=f.get(12);REQUIRE(child);auto family=ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(*child,&f.services.ctx);REQUIRE(family.size()==2);CHECK(family[0].parent_id==id(11));CHECK(family[1].parent_id==family[0].id);CHECK(family[0].id!=id(12));
    }

    TEST_CASE("CRI003 ArchetypeFactory: same named descendants receive distinct copy names") {
        FamilyFixture f;f.root(11,"Same");f.child(12,11,"Same");auto family=ArchetypeFactory::duplicate_entity_archetype_and_children(id(11),&f.services.ctx);REQUIRE(family.size()==2);CHECK(family[0].name=="Same (Copy)");CHECK(family[1].name=="Same (Copy 2)");
    }
}
} // namespace coverage_batch_003
} // namespace

#include <Salix/ecs/Element.h>
namespace {
namespace cri007_factory {
using namespace EditorCri;
struct UnreflectedProbe : Element {
    std::shared_ptr<int> destroyed;
    explicit UnreflectedProbe(std::shared_ptr<int> count):destroyed(std::move(count)){}
    ~UnreflectedProbe() override {++*destroyed;}
    const char* get_class_name()const override{return "CRI007Unreflected";}
};

TEST_SUITE("EditorCRI007_ArchetypeFactory") {
    TEST_CASE("CRI007 ArchetypeFactory: constructor without reflection is destroyed and yields invalid archetype") {
        register_types(); auto destroyed=std::make_shared<int>(0); ByteMirror::register_constructor("CRI007Unreflected",[destroyed]()->Element*{return new UnreflectedProbe(destroyed);}); auto a=ArchetypeFactory::create_element_archetype("CRI007Unreflected"); CHECK_FALSE(a.id.is_valid()); CHECK(*destroyed==1);
    }

    TEST_CASE("CRI007 ArchetypeFactory: constructor returning null produces no authored component") {
        ByteMirror::register_constructor("CRI007Null",[]()->Element*{return nullptr;}); auto a=ArchetypeFactory::create_element_archetype("CRI007Null"); CHECK_FALSE(a.id.is_valid()); CHECK(a.type_name.empty());
    }

    TEST_CASE("CRI007 ArchetypeFactory: duplication without preview realm retains authored transform") {
        FamilyFixture f; f.services.ctx.preview_realm.reset(); auto a=entity(); a.elements.push_back(element("Transform",201,"{position: {x: 1, y: 2, z: 3}}")); auto b=ArchetypeFactory::duplicate_entity_archetype(a,&f.manager,&f.services.ctx); REQUIRE(b.elements.size()==1); CHECK(b.elements[0].data["position"]["z"].as<int>()==3);
    }

    TEST_CASE("CRI007 ArchetypeFactory: hidden entity duplicate retains authored visibility") {
        FamilyFixture f; auto a=entity(); a.is_visible=false; auto b=ArchetypeFactory::duplicate_entity_archetype(a,&f.manager,&f.services.ctx); CHECK_FALSE(b.is_visible); CHECK(b.id!=a.id);
    }

    TEST_CASE("CRI007 ArchetypeFactory: standalone component copy belongs to supplied parent") {
        auto a=entity(11); auto e=element("Sprite2D",21); auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK(b.owner_id==a.id); CHECK(b.id!=e.id);
    }

    TEST_CASE("CRI007 ArchetypeFactory: standalone component copy retains visibility") {
        auto a=entity(); auto e=element(); e.is_visible=false; auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK_FALSE(b.is_visible);
    }

    TEST_CASE("CRI007 ArchetypeFactory: standalone copy preserves source display order") {
        auto a=entity(); auto e=element(); e.display_order=7; auto b=ArchetypeFactory::duplicate_element_archetype(e,a); CHECK(b.display_order==7);
    }

    TEST_CASE("CRI007 ArchetypeFactory: duplicate family skips missing child references") {
        FamilyFixture f; f.root(11,"Root"); f.get(11)->child_ids={id(99)}; auto copy=ArchetypeFactory::duplicate_entity_archetype_and_children(id(11),&f.services.ctx); REQUIRE(copy.size()==1); CHECK(copy[0].child_ids.empty());
    }

    TEST_CASE("CRI007 ArchetypeFactory: duplicate family handles absent preview for all descendants") {
        FamilyFixture f; f.family(); f.services.ctx.preview_realm.reset(); auto copy=ArchetypeFactory::duplicate_entity_archetype_and_children(id(11),&f.services.ctx); REQUIRE(copy.size()==3); CHECK(copy[1].parent_id==copy[0].id); CHECK(copy[2].parent_id==copy[1].id);
    }

    TEST_CASE("CRI007 ArchetypeFactory: recursive copy ignores repeated child reference") {
        FamilyFixture f; f.root(11,"Root"); f.child(12,11,"Child"); f.get(11)->child_ids.push_back(id(12)); auto copy=ArchetypeFactory::duplicate_entity_archetype_and_children(id(11),&f.services.ctx); REQUIRE(copy.size()==2); REQUIRE(copy[0].child_ids.size()==1); CHECK(copy[0].child_ids[0]==copy[1].id);
    }
}
}
}
