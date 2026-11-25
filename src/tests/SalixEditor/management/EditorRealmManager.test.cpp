// Tests for Editor/management/EditorRealmManager.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/management/EditorRealmManager.h>
#include <Editor/EditorContext.h>
#include <Salix/events/EventManager.h>
#include "../fixtures/RealmFixtures.h"
#include <Salix/ecs/Camera.h>
#include <iterator>

namespace {
namespace coverage_batch_001 {
namespace {
using namespace EditorCri;
void load_pair(EditorRealmManager& m, TempRealm& f) {
    m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}]"));
}
}
TEST_SUITE("EditorCRI001_EditorRealmManager") {
    TEST_CASE("CRI001 EditorRealmManager: default manager starts dirty with no snapshot") {
        EditorRealmManager m; CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_snapshot()==nullptr); CHECK(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: context constructor has empty clean snapshot") {
        EditorContext c; EditorRealmManager m(&c); REQUIRE(m.get_snapshot()); CHECK_FALSE(m.is_dirty()); CHECK(m.get_realm_size()==0);
    }

    TEST_CASE("CRI001 EditorRealmManager: load establishes hierarchy and clean snapshot") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); REQUIRE(m.get_realm_size()==2); REQUIRE(m.get_hierarchy().size()==1); auto root=m.get_hierarchy()[0]; CHECK(root->entity_id==id(11)); REQUIRE(root->children.size()==1); CHECK(root->children[0]->entity_id==id(12)); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: lookup works for const and mutable manager") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); const auto& cm=m; REQUIRE(m.get_archetype(id(12))); CHECK(cm.get_archetype(id(12))==m.get_archetype(id(12))); CHECK(cm.get_archetype(id(99))==nullptr); CHECK(m.get_archetype(id(99))==nullptr);
    }

    TEST_CASE("CRI001 EditorRealmManager: all archetypes and name search reflect loaded data") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); auto all=m.get_all_archetypes(); REQUIRE(all.size()==2); CHECK(all[0]->name=="Root"); CHECK(all[1]->name=="Child"); CHECK(m.does_entity_name_exist("Root")); CHECK_FALSE(m.does_entity_name_exist("root"));
    }

    TEST_CASE("CRI001 EditorRealmManager: rename makes dirty until snapshot refreshed") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); REQUIRE(m.get_archetype(id(11))); m.get_archetype(id(11))->name="Renamed"; CHECK(m.is_dirty()); m.take_snapshot(); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: clear resets realm map hierarchy and snapshot") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); m.clear_realm(); CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_hierarchy().empty()); REQUIRE(m.get_snapshot()); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: missing entity purge does not alter existing realm") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); m.purge_entity(id(99)); m.purge_entity_descendants(id(99)); CHECK(m.get_realm_size()==2); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: root addition queues notification and updates lookup") {
        EventManager events; EditorContext c; c.event_manager=&events; EditorRealmManager m(&c); m.add_entity(entity(11,"Root")); REQUIRE(m.get_archetype(id(11))); CHECK(m.get_realm_size()==1); CHECK(m.is_dirty()); CHECK_FALSE(events.is_queue_empty());
    }

    TEST_CASE("CRI001 EditorRealmManager: root purge updates lookup and queues notification") {
        EventManager events; EditorContext c; c.event_manager=&events; EditorRealmManager m(&c); m.add_entity(entity(11,"Root")); events.process_queue(); m.purge_entity(id(11)); CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_archetype(id(11))==nullptr); CHECK_FALSE(events.is_queue_empty());
    }
}
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
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
} // namespace coverage_batch_003
} // namespace



TEST_SUITE("EditorCRI006_EditorRealmManager") {
    using namespace EditorCri;
    TEST_CASE("CRI006 EditorRealmManager: validate clean realm and dump hierarchy") {
        FamilyFixture f; f.family(); f.manager.add_element_to_entity(id(13),element("Transform",201)); f.manager.take_snapshot(); CoutCapture out; f.manager.validate_realm(); f.manager.print_hierarchy(); CHECK(out.str().find("Realm Snapshot is valid")!=std::string::npos); CHECK(out.str().find("13 - Grandchild")!=std::string::npos); CHECK(out.str().find("Transform")!=std::string::npos);
    }

    TEST_CASE("CRI006 EditorRealmManager: validate modified realm retains authored data") {
        FamilyFixture f; f.family(); f.get(11)->name="Edited"; f.manager.validate_realm(); CHECK(f.manager.is_dirty()); CHECK(f.get(11)->name=="Edited");
    }

    TEST_CASE("CRI006 EditorRealmManager: single copy inserts detached root") {
        FamilyFixture f; f.family(); f.manager.duplicate_entity(id(12)); REQUIRE(f.manager.get_realm_size()==5); const auto& copy=f.manager.get_realm().back(); CHECK(copy.name=="Child (Copy)"); CHECK_FALSE(copy.parent_id.is_valid()); CHECK(copy.child_ids.empty()); CHECK(copy.id!=id(12));
    }

    TEST_CASE("CRI006 EditorRealmManager: sibling copy attaches to original parent") {
        FamilyFixture f; f.family(); f.manager.duplicate_entity_as_sibling(id(12)); REQUIRE(f.manager.get_realm_size()==5); const auto& copy=f.manager.get_realm().back(); CHECK(copy.parent_id==id(11)); REQUIRE(f.get(11)->child_ids.size()==2); CHECK(f.get(11)->child_ids.back()==copy.id);
    }

    TEST_CASE("CRI006 EditorRealmManager: root sibling copy stays root") {
        FamilyFixture f; f.family(); f.manager.duplicate_entity_as_sibling(id(14)); REQUIRE(f.manager.get_realm_size()==5); CHECK_FALSE(f.manager.get_realm().back().parent_id.is_valid()); CHECK(f.manager.get_hierarchy().size()==3);
    }

    TEST_CASE("CRI006 EditorRealmManager: family copy inserts all descendants") {
        FamilyFixture f; f.family(); f.manager.duplicate_entity_with_children(id(11)); const auto& all=f.manager.get_realm(); REQUIRE(all.size()==7); CHECK(all[5].parent_id==all[4].id); CHECK(all[6].parent_id==all[5].id); CHECK_FALSE(all[4].parent_id.is_valid());
    }

    TEST_CASE("CRI006 EditorRealmManager: sibling family copy links external parent") {
        FamilyFixture f; f.family(); f.manager.duplicate_family_as_sibling(id(12)); const auto& all=f.manager.get_realm(); REQUIRE(all.size()==6); CHECK(all[4].parent_id==id(11)); CHECK(all[5].parent_id==all[4].id); REQUIRE(f.get(11)->child_ids.size()==2); CHECK(f.get(11)->child_ids.back()==all[4].id);
    }

    TEST_CASE("CRI006 EditorRealmManager: missing source IDs do not append copies") {
        FamilyFixture f; f.family(); f.manager.duplicate_entity(id(99)); f.manager.duplicate_entity_as_sibling(id(99)); f.manager.duplicate_entity_with_children(id(99)); f.manager.duplicate_family_as_sibling(id(99)); CHECK(f.manager.get_realm_size()==4);
    }

    TEST_CASE("CRI006 EditorRealmManager: purging newly added element restores snapshot state") {
        FamilyFixture f; f.family(); auto e=element("Transform",201); e.owner_id=id(11); f.manager.add_element_to_entity(id(11),e); CHECK(f.manager.is_dirty()); f.manager.purge_element(id(11),e.id); CHECK_FALSE(f.manager.is_dirty()); CHECK(f.get(11)->state==ArchetypeState::UnModified);
    }

    TEST_CASE("CRI006 EditorRealmManager: preview sync without preview realm is a no-op") {
        FamilyFixture f; f.family(); f.services.ctx.preview_realm.reset(); f.manager.sync_preview_realm(); CHECK(f.services.ctx.preview_realm==nullptr); CHECK(f.manager.get_realm_size()==4);
    }
}



TEST_SUITE("EditorCRI009_EditorRealmManager") {
    using namespace EditorCri;
    TEST_CASE("CRI009 EditorRealmManager: invalid starting ancestor ID does not dirty model") {
        FamilyFixture f; f.family(); f.manager.update_ancestor_states(SimpleGuid::invalid()); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: unknown starting ancestor ID leaves hierarchy intact") {
        FamilyFixture f; f.family(); f.manager.update_ancestor_states(id(99)); CHECK(f.manager.get_realm_size()==4); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: missing ancestor reference terminates traversal safely") {
        FamilyFixture f; f.family(); f.get(12)->parent_id=id(99); f.manager.update_ancestor_states(id(12)); CHECK(f.get(12)->parent_id==id(99)); CHECK(f.get(11)!=nullptr);
    }

    TEST_CASE("CRI009 EditorRealmManager: same parent reparent cannot duplicate child entries") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(12),id(11)); REQUIRE(f.get(11)->child_ids.size()==1); CHECK(f.get(11)->child_ids[0]==id(12)); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: missing child reparent does not alter root count") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(99),id(11)); CHECK(f.manager.get_hierarchy().size()==2); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: missing target rejected by model boundary") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(12),id(99)); CHECK(f.get(12)->parent_id==id(11)); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: self parenting rejected by model boundary") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(11),id(11)); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: direct cycle rejected by model boundary") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(11),id(12)); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK(f.get(12)->parent_id==id(11));
    }

    TEST_CASE("CRI009 EditorRealmManager: transitive cycle rejected by model boundary") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(11),id(13)); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK(f.get(13)->parent_id==id(12)); CHECK_FALSE(f.manager.is_dirty());
    }

    TEST_CASE("CRI009 EditorRealmManager: valid reparent moves subtree without changing descendant parent") {
        FamilyFixture f; f.family(); f.manager.reparent_entity(id(12),id(14)); CHECK(f.get(12)->parent_id==id(14)); CHECK(f.get(13)->parent_id==id(12)); CHECK(f.get(11)->child_ids.empty()); REQUIRE(f.get(14)->child_ids.size()==1); CHECK(f.get(14)->child_ids[0]==id(12));
    }
}

TEST_SUITE("EditorCRI015_EditorRealmManager") {
    using namespace EditorCri;

    TEST_CASE("CRI015 EditorRealmManager: loaded parent and child links agree before snapshot") {
        TempRealm f;
        EditorRealmManager m;
        m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}, {name: Grandchild, id: 13, parent: 12}]"));
        const auto* root = m.get_archetype(id(11));
        const auto* child = m.get_archetype(id(12));
        const auto* grandchild = m.get_archetype(id(13));
        REQUIRE(root);
        REQUIRE(child);
        REQUIRE(grandchild);
        REQUIRE(root->child_ids.size() == 1);
        CHECK(root->child_ids[0] == child->id);
        CHECK(child->parent_id == root->id);
        REQUIRE(child->child_ids.size() == 1);
        CHECK(child->child_ids[0] == grandchild->id);
        CHECK(grandchild->parent_id == child->id);
        CHECK(grandchild->child_ids.empty());
        REQUIRE(m.get_snapshot());
        CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI015 EditorRealmManager: out of order YAML preserves sibling membership and order") {
        TempRealm f;
        EditorRealmManager m;
        m.load_realm_from_file(f.write("Entities: [{name: First, id: 14, parent: 11}, {name: Second, id: 12, parent: 11}, {name: Root, id: 11}, {name: Other, id: 15}]"));
        const auto* root = m.get_archetype(id(11));
        REQUIRE(root);
        REQUIRE(root->child_ids.size() == 2);
        CHECK(root->child_ids[0] == id(14));
        CHECK(root->child_ids[1] == id(12));
        REQUIRE(m.get_archetype(id(15)));
        CHECK(m.get_archetype(id(15))->child_ids.empty());
        REQUIRE(m.get_hierarchy().size() == 2);
        REQUIRE(m.get_hierarchy()[0]->children.size() == 2);
        CHECK(m.get_hierarchy()[0]->children[0]->entity_id == root->child_ids[0]);
        CHECK(m.get_hierarchy()[0]->children[1]->entity_id == root->child_ids[1]);
    }

    TEST_CASE("CRI015 EditorRealmManager: reloading replaces child links without accumulation") {
        TempRealm f;
        EditorRealmManager m;
        const auto file = f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}]");
        m.load_realm_from_file(file);
        m.load_realm_from_file(file);
        REQUIRE(m.get_archetype(id(11)));
        REQUIRE(m.get_archetype(id(11))->child_ids.size() == 1);
        CHECK(m.get_archetype(id(11))->child_ids[0] == id(12));
        m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Replacement, id: 13, parent: 11}]"));
        REQUIRE(m.get_archetype(id(11)));
        REQUIRE(m.get_archetype(id(11))->child_ids.size() == 1);
        CHECK(m.get_archetype(id(11))->child_ids[0] == id(13));
        CHECK(m.get_archetype(id(12)) == nullptr);
        CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI015 EditorRealmManager: rejected duplicate and missing parent do not create child links") {
        TempRealm f;
        EditorRealmManager m;
        m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}, {name: Duplicate, id: 12, parent: 11}, {name: Orphan, id: 13, parent: 99}, {name: Self, id: 14, parent: 14}]"));
        REQUIRE(m.get_realm_size() == 4);
        REQUIRE(m.get_archetype(id(11)));
        REQUIRE(m.get_archetype(id(11))->child_ids.size() == 1);
        CHECK(m.get_archetype(id(11))->child_ids[0] == id(12));
        REQUIRE(m.get_archetype(id(13)));
        CHECK(m.get_archetype(id(13))->parent_id == id(99));
        CHECK(m.get_archetype(id(13))->child_ids.empty());
        REQUIRE(m.get_archetype(id(14)));
        CHECK_FALSE(m.get_archetype(id(14))->parent_id.is_valid());
        CHECK(m.get_archetype(id(14))->child_ids.empty());
        CHECK(m.get_archetype(id(99)) == nullptr);
        CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI015 EditorRealmManager: loaded descendants can be purged without manual link repair") {
        TempRealm f;
        EventManager events;
        EditorContext c;
        c.event_manager = &events;
        EditorRealmManager m(&c);
        m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}, {name: Grandchild, id: 13, parent: 12}, {name: Other, id: 14}]"));
        m.purge_entity_descendants(id(11));
        CHECK(m.get_realm_size() == 2);
        REQUIRE(m.get_archetype(id(11)));
        CHECK(m.get_archetype(id(11))->child_ids.empty());
        CHECK(m.get_archetype(id(12)) == nullptr);
        CHECK(m.get_archetype(id(13)) == nullptr);
        CHECK(m.get_archetype(id(14)) != nullptr);
        CHECK(m.is_dirty());
        CHECK_FALSE(events.is_queue_empty());
    }
}

namespace {
using namespace EditorCri;
struct LoadedHierarchyFixture {
    TempRealm file;
    EventManager events;
    EditorContext context;
    EditorRealmManager manager;
    LoadedHierarchyFixture(): manager(&context) { context.event_manager = &events; }
    void load(const char* yaml) { manager.load_realm_from_file(file.write(yaml)); }
    EntityArchetype* get(uint64_t n) {
        auto* record = manager.get_archetype(id(n));
        REQUIRE(record);
        return record;
    }
    void root(uint64_t n) { CHECK_FALSE(get(n)->parent_id.is_valid()); }
    void child(uint64_t n, uint64_t parent) { CHECK(get(n)->parent_id == id(parent)); }
};
}
TEST_SUITE("EditorCRI016_EditorRealmManager") {
    using namespace EditorCri;
    TEST_CASE("CRI016 EditorRealmManager: two entity cycle preserves both records as roots") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}]");
        REQUIRE(f.manager.get_realm_size() == 2);
        f.root(11); f.root(12);
        CHECK(f.get(11)->child_ids.empty());
        CHECK(f.get(12)->child_ids.empty());
        REQUIRE(f.manager.get_hierarchy().size() == 2);
        CHECK(f.manager.get_hierarchy()[0]->entity_id == id(11));
        CHECK(f.manager.get_hierarchy()[1]->entity_id == id(12));
    }

    TEST_CASE("CRI016 EditorRealmManager: longer cycle preserves entity and component metadata") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12, elements: {Transform: {id: 21}}}, {name: B, id: 12, parent: 13}, {name: C, id: 13, parent: 11}]");
        f.root(11); f.root(12); f.root(13);
        CHECK(f.get(11)->name == "A");
        REQUIRE(f.get(11)->elements.size() == 1);
        CHECK(f.get(11)->elements[0].id == id(21));
        CHECK(f.manager.get_hierarchy().size() == 3);
    }

    TEST_CASE("CRI016 EditorRealmManager: tail leading into cycle retains its valid parent") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: Tail, id: 13, parent: 11}, {name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}]");
        f.child(13, 11); f.root(11); f.root(12);
        REQUIRE(f.get(11)->child_ids.size() == 1);
        CHECK(f.get(11)->child_ids[0] == id(13));
        CHECK(f.get(12)->child_ids.empty());
        CHECK(f.manager.get_hierarchy().size() == 2);
    }

    TEST_CASE("CRI016 EditorRealmManager: independent descendant chain survives cycle repair") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}, {name: Root, id: 14}, {name: Child, id: 15, parent: 14}, {name: Grandchild, id: 16, parent: 15}]");
        f.root(11); f.root(12); f.root(14);
        f.child(15, 14); f.child(16, 15);
        REQUIRE(f.get(14)->child_ids.size() == 1);
        CHECK(f.get(14)->child_ids[0] == id(15));
        REQUIRE(f.get(15)->child_ids.size() == 1);
        CHECK(f.get(15)->child_ids[0] == id(16));
        CHECK(f.manager.get_hierarchy().size() == 3);
    }

    TEST_CASE("CRI016 EditorRealmManager: disjoint cycles normalize independently") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: C, id: 13, parent: 14}, {name: B, id: 12, parent: 11}, {name: D, id: 14, parent: 13}]");
        for (uint64_t n : {11, 12, 13, 14}) {
            f.root(n);
            CHECK(f.get(n)->child_ids.empty());
        }
        CHECK(f.manager.get_hierarchy().size() == 4);
    }

    TEST_CASE("CRI016 EditorRealmManager: reversing YAML order does not choose a different cycle root policy") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: B, id: 12, parent: 11}, {name: A, id: 11, parent: 12}, {name: Child, id: 13, parent: 12}]");
        f.root(11); f.root(12); f.child(13, 12);
        REQUIRE(f.get(12)->child_ids.size() == 1);
        CHECK(f.get(12)->child_ids[0] == id(13));
        CHECK(f.get(11)->child_ids.empty());
    }

    TEST_CASE("CRI016 EditorRealmManager: orphan reference remains unchanged beside a malformed cycle") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}, {name: Orphan, id: 13, parent: 99}, {name: Child, id: 14, parent: 13}]");
        f.root(11); f.root(12); f.child(13, 99); f.child(14, 13);
        REQUIRE(f.get(13)->child_ids.size() == 1);
        CHECK(f.get(13)->child_ids[0] == id(14));
        CHECK(f.manager.get_archetype(id(99)) == nullptr);
    }

    TEST_CASE("CRI016 EditorRealmManager: clean snapshot describes normalized in-memory hierarchy") {
        LoadedHierarchyFixture f;
        const char* yaml = "Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}]";
        f.load(yaml);
        REQUIRE(f.manager.get_snapshot());
        CHECK_FALSE(f.manager.is_dirty());
        std::ifstream original(f.file.path());
        std::string contents((std::istreambuf_iterator<char>(original)), std::istreambuf_iterator<char>());
        CHECK(contents == yaml);
    }

    TEST_CASE("CRI016 EditorRealmManager: normalized cycle permits descendant purge without deleting former peer") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}, {name: Child, id: 13, parent: 11}]");
        f.manager.purge_entity_descendants(id(11));
        CHECK(f.manager.get_realm_size() == 2);
        CHECK(f.manager.get_archetype(id(13)) == nullptr);
        f.root(11); f.root(12);
        CHECK(f.get(11)->child_ids.empty());
        CHECK(f.manager.is_dirty());
    }

    TEST_CASE("CRI016 EditorRealmManager: subsequent valid reload restores intended hierarchy") {
        LoadedHierarchyFixture f;
        f.load("Entities: [{name: A, id: 11, parent: 12}, {name: B, id: 12, parent: 11}]");
        f.load("Entities: [{name: A, id: 11}, {name: B, id: 12, parent: 11}]");
        f.root(11); f.child(12, 11);
        REQUIRE(f.get(11)->child_ids.size() == 1);
        CHECK(f.get(11)->child_ids[0] == id(12));
        CHECK(f.manager.get_hierarchy().size() == 1);
        CHECK_FALSE(f.manager.is_dirty());
    }
}
