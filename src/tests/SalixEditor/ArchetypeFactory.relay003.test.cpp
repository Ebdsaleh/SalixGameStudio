// CRI relay 003: ArchetypeFactory; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/ArchetypeFactory.h>
#include <Salix/ecs/Camera.h>
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
