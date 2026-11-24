// CRI relay 003: EditorRealmManager; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Salix/ecs/Camera.h>
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI003_EditorRealmManager") {
    TEST_CASE("CRI003 EditorRealmManager: child addition remains valid through vector growth") {
        FamilyFixture f;f.root(11,"Root");for(uint64_t n=20;n<40;++n)f.child(n,11,"Child");REQUIRE(f.get(11));CHECK(f.get(11)->child_ids.size()==20);CHECK(f.manager.get_realm_size()==21);REQUIRE(f.manager.get_hierarchy().size()==1);CHECK(f.manager.get_hierarchy()[0]->children.size()==20);
    }

    TEST_CASE("CRI003 EditorRealmManager: missing or invalid parent rejected without inserting orphan") {
        FamilyFixture f;f.root(11,"Root");f.child(12,99,"Missing parent");f.child(13,0,"Invalid parent");CHECK(f.manager.get_realm_size()==1);CHECK(f.get(12)==nullptr);CHECK(f.get(13)==nullptr);
    }

    TEST_CASE("CRI003 EditorRealmManager: reparent updates both parent child lists") {
        FamilyFixture f;f.family();f.manager.reparent_entity(id(12),id(14));REQUIRE(f.get(11));REQUIRE(f.get(14));CHECK(f.get(11)->child_ids.empty());REQUIRE(f.get(14)->child_ids.size()==1);CHECK(f.get(14)->child_ids[0]==id(12));CHECK(f.get(12)->parent_id==id(14));
    }

    TEST_CASE("CRI003 EditorRealmManager: release detaches child and preserves descendants") {
        FamilyFixture f;f.family();f.manager.release_from_parent(id(12));CHECK_FALSE(f.get(12)->parent_id.is_valid());CHECK(f.get(11)->child_ids.empty());CHECK(f.get(13)->parent_id==id(12));CHECK(f.manager.get_hierarchy().size()==3);
    }

    TEST_CASE("CRI003 EditorRealmManager: purge single parent orphans children without deleting them") {
        FamilyFixture f;f.family();f.manager.purge_entity(id(12));CHECK(f.get(12)==nullptr);REQUIRE(f.get(13));CHECK_FALSE(f.get(13)->parent_id.is_valid());CHECK(f.get(11)->child_ids.empty());CHECK(f.manager.get_realm_size()==3);
    }

    TEST_CASE("CRI003 EditorRealmManager: purge descendants preserves parent stored after child") {
        FamilyFixture f;auto child=entity(12,"Child");child.parent_id=id(11);f.manager.add_entity(child);f.root(11,"Root");f.root(14,"Other");REQUIRE(f.get(11));f.get(11)->child_ids={id(12)};f.manager.take_snapshot();f.manager.purge_entity_descendants(id(11));REQUIRE(f.get(11));CHECK(f.get(11)->child_ids.empty());CHECK(f.get(12)==nullptr);CHECK(f.get(14)!=nullptr);CHECK(f.manager.get_realm_size()==2);
    }

    TEST_CASE("CRI003 EditorRealmManager: family purge removes only selected subtree") {
        FamilyFixture f;f.family();f.manager.purge_entity_and_family(id(12));CHECK(f.get(12)==nullptr);CHECK(f.get(13)==nullptr);REQUIRE(f.get(11));CHECK(f.get(11)->child_ids.empty());CHECK(f.get(14)!=nullptr);
    }

    TEST_CASE("CRI003 EditorRealmManager: bloodline purge removes ancestors and siblings but unrelated root survives") {
        FamilyFixture f;f.family();f.manager.purge_entity_bloodline(id(13));CHECK(f.get(11)==nullptr);CHECK(f.get(12)==nullptr);CHECK(f.get(13)==nullptr);REQUIRE(f.get(14));CHECK(f.manager.get_realm_size()==1);
    }

    TEST_CASE("CRI003 EditorRealmManager: element add duplicate and purge update actual archetype membership") {
        FamilyFixture f;f.root(11,"Root");f.manager.add_element_to_entity(id(11),element("Transform",201,"{name: Transform}"));REQUIRE(f.get(11)->elements.size()==1);f.manager.duplicate_element(id(11),id(201));REQUIRE(f.get(11)->elements.size()==2);auto copied=f.get(11)->elements[1].id;CHECK(copied!=id(201));f.manager.purge_element(id(11),copied);REQUIRE(f.get(11)->elements.size()==1);CHECK(f.get(11)->elements[0].id==id(201));
    }

    TEST_CASE("CRI003 EditorRealmManager: preview rebuild selects authored camera and clears stale selection") {
        FamilyFixture f;f.root(11,"Root");f.manager.add_element_to_entity(id(11),element("Camera",201,"{name: Camera, active: true}"));f.manager.sync_preview_realm();auto& ctx=f.services.ctx;REQUIRE(ctx.main_camera);CHECK(ctx.preview_realm->get_main_camera_entity_id()==id(11));CHECK(ctx.main_camera->get_is_active());f.get(11)->elements[0].data["active"]=false;f.manager.sync_preview_realm();CHECK(ctx.main_camera==nullptr);CHECK_FALSE(ctx.preview_realm->get_main_camera_entity_id().is_valid());
    }
}
