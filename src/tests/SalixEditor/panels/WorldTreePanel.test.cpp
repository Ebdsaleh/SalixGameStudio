// Tests for Editor/panels/WorldTreePanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct TreeProbe : WorldTreePanel { void content(){on_panel_gui_update();} ImGuiWindowFlags flags()const{return get_window_flags();} };
struct TreeFixture { PanelContext c; TreeProbe panel; TempRealm file;
    TreeFixture(){c.ctx.editor_realm_manager->load_realm_from_file(file.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 12, name: Original}}}]")); panel.initialize(&c.ctx);}
    EntityArchetype* root(){return c.ctx.editor_realm_manager->get_archetype(id(11));}
    void change(uint64_t entity_id,uint64_t element_id,const char* name,PropertyValue value){PropertyValueChangedEvent e(id(entity_id),id(element_id),"Transform",name,value);panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI002_WorldTreePanel") {
    TEST_CASE("CRI002 WorldTreePanel: constructor sets window identity and flags") {
        TreeProbe p; CHECK(p.get_name()=="World Tree Panel"); CHECK(p.get_title()=="World Tree"); CHECK(p.is_panel_derived()); CHECK(p.flags()==ImGuiWindowFlags_None);
    }

    TEST_CASE("CRI002 WorldTreePanel: null init preserves existing visibility") {
        TreeProbe p; p.set_visibility(false); p.initialize(nullptr); CHECK_FALSE(p.get_visibility());
    }

    TEST_CASE("CRI002 WorldTreePanel: initialization restores visibility and unlocked icon") {
        PanelContext c; TreeProbe p; p.set_visibility(false); p.initialize(&c.ctx); CHECK(p.get_visibility()); CHECK(p.get_current_lock_icon_path()=="unlocked.png");
    }

    TEST_CASE("CRI002 WorldTreePanel: entity selection updates shared context and clears element") {
        TreeFixture f; f.c.ctx.selected_element_id=id(77); EntitySelectedEvent e(id(11)); f.panel.on_event(e); CHECK(f.c.ctx.selected_entity_id==id(11)); CHECK_FALSE(f.c.ctx.selected_element_id.is_valid());
    }

    TEST_CASE("CRI002 WorldTreePanel: element event delivered through subscription") {
        TreeFixture f; ElementSelectedEvent e(id(12),id(11)); f.c.events.dispatch(e); f.c.events.process_queue(); CHECK(f.c.ctx.selected_entity_id==id(11)); CHECK(f.c.ctx.selected_element_id==id(12));
    }

    TEST_CASE("CRI002 WorldTreePanel: deselection clears stable IDs") {
        TreeFixture f; f.c.ctx.selected_entity_id=id(11); f.c.ctx.selected_element_id=id(12); EntitySelectedEvent e(SimpleGuid::invalid()); f.panel.on_event(e); CHECK_FALSE(f.c.ctx.selected_entity_id.is_valid()); CHECK_FALSE(f.c.ctx.selected_element_id.is_valid());
    }

    TEST_CASE("CRI002 WorldTreePanel: name change updates YAML and dirty states") {
        TreeFixture f; f.change(11,12,"name",std::string("Renamed")); REQUIRE(f.root()); REQUIRE(f.root()->elements.size()==1); CHECK(f.root()->elements[0].name=="Renamed"); CHECK(f.root()->elements[0].data["name"].as<std::string>()=="Renamed"); CHECK(f.root()->state==ArchetypeState::Modified);
    }

    TEST_CASE("CRI002 WorldTreePanel: restoring original name clears modified states") {
        TreeFixture f; f.change(11,12,"name",std::string("Renamed")); f.change(11,12,"name",std::string("Original")); REQUIRE(f.root()); CHECK(f.root()->state==ArchetypeState::UnModified); CHECK(f.root()->elements[0].state==ArchetypeState::UnModified);
    }

    TEST_CASE("CRI002 WorldTreePanel: missing IDs and unrelated event do not mutate realm") {
        TreeFixture f; f.change(99,12,"name",std::string("Bad")); f.change(11,99,"name",std::string("Bad")); WindowResizeEvent e(5,5); f.panel.on_event(e); REQUIRE(f.root()); CHECK(f.root()->elements[0].name=="Original"); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI002 WorldTreePanel: offscreen hierarchy drawing preserves the model") {
        GuiFrame g; TreeFixture f; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.editor_realm_manager->get_realm_size()==1); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }
}



TEST_SUITE("EditorCRI006_WorldTreePanel") {
    TEST_CASE("CRI006 WorldTreePanel: position event preserves vector components") {
        TreeFixture f; f.change(11,12,"position",Vector3(2,3,4)); auto n=f.root()->elements[0].data["position"]; CHECK(n["x"].as<float>()==2); CHECK(n["y"].as<float>()==3); CHECK(n["z"].as<float>()==4);
    }

    TEST_CASE("CRI006 WorldTreePanel: visibility event changes YAML without replacing name") {
        TreeFixture f; f.change(11,12,"visible",false); CHECK_FALSE(f.root()->elements[0].data["visible"].as<bool>()); CHECK(f.root()->elements[0].name=="Original");
    }

    TEST_CASE("CRI006 WorldTreePanel: projection event does not overwrite stored enum string") {
        TreeFixture f; f.root()->elements[0].data["projection_mode"]="Orthographic"; f.change(11,12,"projection_mode",0); CHECK(f.root()->elements[0].data["projection_mode"].as<std::string>()=="Orthographic");
    }

    TEST_CASE("CRI006 WorldTreePanel: nonstring name event preserves mirrored name") {
        TreeFixture f; f.change(11,12,"name",42); CHECK(f.root()->elements[0].name=="Original");
    }

    TEST_CASE("CRI006 WorldTreePanel: empty hierarchy renders without new entities") {
        GuiFrame g; TreeFixture f; f.c.ctx.editor_realm_manager->clear_realm(); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.editor_realm_manager->realm_is_empty());
    }

    TEST_CASE("CRI006 WorldTreePanel: selected root rendered without changing selection") {
        GuiFrame g; TreeFixture f; EntitySelectedEvent e(id(11)); f.panel.on_event(e); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.selected_entity_id==id(11)); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 WorldTreePanel: missing manager suppresses content") {
        GuiFrame g; TreeFixture f; f.c.ctx.editor_realm_manager.reset(); CHECK(g.draw([&]{f.panel.content();})==0);
    }

    TEST_CASE("CRI006 WorldTreePanel: new root rendered with its unsaved state") {
        GuiFrame g; TreeFixture f; auto a=entity(20,"Unsaved"); a.state=ArchetypeState::New; f.c.ctx.editor_realm_manager->add_entity(a); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(20))->state==ArchetypeState::New);
    }

    TEST_CASE("CRI006 WorldTreePanel: modified root rendered without clearing dirty state") {
        GuiFrame g; TreeFixture f; f.root()->name="Edited"; f.root()->state=ArchetypeState::Modified; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.root()->state==ArchetypeState::Modified); CHECK(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 WorldTreePanel: multiple root entries stay in model order after rendering") {
        GuiFrame g; TreeFixture f; f.c.ctx.editor_realm_manager->add_entity(entity(20,"Second")); f.c.ctx.editor_realm_manager->add_entity(entity(21,"Third")); CHECK(g.draw([&]{f.panel.content();})>0); const auto& r=f.c.ctx.editor_realm_manager->get_realm(); REQUIRE(r.size()==3); CHECK(r[0].id==id(11)); CHECK(r[1].id==id(20)); CHECK(r[2].id==id(21));
    }
}


namespace {
using namespace EditorCri;
void open_root(GuiFrame& g,TreeFixture& f) {
    g.draw([&]{f.panel.content();}); g.draw([&]{f.panel.content();});
    ImGuiWindow* child=nullptr;
    for(auto* w:g.current->Windows) if(std::string(w->Name).find("WorldTreeContent")!=std::string::npos)child=w;
    REQUIRE(child);
    const ImGuiID seed=child->GetID(11);
    void* ptr=reinterpret_cast<void*>(static_cast<uintptr_t>(11));
    const ImGuiID root_id=ImHashData(&ptr,sizeof(ptr),seed);
    child->StateStorage.SetInt(root_id,1);
    g.draw([&]{f.panel.content();});
    CHECK(child->StateStorage.GetInt(root_id)==1);
}
} // namespace
TEST_SUITE("EditorCRI007_WorldTreePanel") {
    using namespace EditorCri;
    TEST_CASE("CRI007 WorldTreePanel: expanded root renders original component without dirtying snapshot") {
        GuiFrame g; TreeFixture f; open_root(g,f); CHECK(f.root()->elements[0].name=="Original"); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded root renders selected element without replacing owner selection") {
        GuiFrame g; TreeFixture f; ElementSelectedEvent e(id(12),id(11)); f.panel.on_event(e); open_root(g,f); CHECK(f.c.ctx.selected_entity_id==id(11)); CHECK(f.c.ctx.selected_element_id==id(12));
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded modified element retains dirty state") {
        GuiFrame g; TreeFixture f; f.change(11,12,"name",std::string("Edited")); open_root(g,f); CHECK(f.root()->elements[0].state==ArchetypeState::Modified); CHECK(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded new component retains unsaved state") {
        GuiFrame g; TreeFixture f; auto a=element("Sprite2D",20); a.state=ArchetypeState::New; f.root()->elements.push_back(a); open_root(g,f); CHECK(f.root()->elements.back().state==ArchetypeState::New); CHECK(f.root()->elements.size()==2);
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded root traverses child with stable parent reference") {
        GuiFrame g; TreeFixture f; auto a=entity(20,"Child"); a.parent_id=id(11); f.c.ctx.editor_realm_manager->add_child_entity(a); open_root(g,f); REQUIRE(f.c.ctx.editor_realm_manager->get_archetype(id(20))); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(20))->parent_id==id(11)); CHECK(f.root()->child_ids.size()==1);
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded hierarchy preserves child and component ordering") {
        GuiFrame g; TreeFixture f; auto a=entity(20,"Child"); a.parent_id=id(11); f.c.ctx.editor_realm_manager->add_child_entity(a); f.root()->elements.push_back(element("BoxCollider",21)); open_root(g,f); REQUIRE(f.root()->elements.size()==2); CHECK(f.root()->elements[0].id==id(12)); CHECK(f.root()->elements[1].id==id(21)); CHECK(f.root()->child_ids[0]==id(20));
    }

    TEST_CASE("CRI007 WorldTreePanel: expanded tree tolerates missing child reference") {
        GuiFrame g; TreeFixture f; f.root()->child_ids.push_back(id(99)); open_root(g,f); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(99))==nullptr); CHECK(f.root()->child_ids[0]==id(99));
    }

    TEST_CASE("CRI007 WorldTreePanel: multiple edits on same property restore snapshot and state") {
        TreeFixture f; f.change(11,12,"name",std::string("A")); f.change(11,12,"name",std::string("B")); CHECK(f.root()->elements[0].state==ArchetypeState::Modified); f.change(11,12,"name",std::string("Original")); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI007 WorldTreePanel: editing new component does not lose new-state indicator") {
        TreeFixture f; f.root()->elements[0].state=ArchetypeState::New; f.root()->state=ArchetypeState::New; f.change(11,12,"name",std::string("Renamed")); CHECK(f.root()->elements[0].state==ArchetypeState::New); CHECK(f.root()->state==ArchetypeState::New);
    }

    TEST_CASE("CRI007 WorldTreePanel: entity and property events before initialization are harmless") {
        TreeProbe p; EntitySelectedEvent e(id(11)); p.on_event(e); PropertyValueChangedEvent change(id(11),id(12),"Transform","name",std::string("A")); p.on_event(change); CHECK(p.get_name()=="World Tree Panel");
    }
}


namespace {
using namespace EditorCri;
struct DropFixture {
    PanelContext c; TreeProbe panel; TempRealm file;
    DropFixture(){c.ctx.editor_realm_manager->load_realm_from_file(file.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}, {name: Grandchild, id: 13, parent: 12}, {name: Other, id: 14}]"));panel.initialize(&c.ctx);}
    auto* get(uint64_t n){return c.ctx.editor_realm_manager->get_archetype(id(n));}
    void drop(uint64_t a,uint64_t b){panel.process_entity_drop_for_testing(id(a),id(b));}
};
}
TEST_SUITE("EditorCRI009_WorldTreePanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 WorldTreePanel: missing dragged ID queues no action") {
        DropFixture f; f.drop(99,14); CHECK(f.c.events.is_queue_empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI009 WorldTreePanel: same parent drop preserves clean model") {
        DropFixture f; f.drop(12,11); CHECK(f.get(12)->parent_id==id(11)); CHECK(f.c.events.is_queue_empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI009 WorldTreePanel: self drop does not create a cycle") {
        DropFixture f; f.drop(11,11); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI009 WorldTreePanel: direct descendant drop rejected") {
        DropFixture f; f.drop(11,12); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK(f.get(12)->parent_id==id(11)); CHECK(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI009 WorldTreePanel: deep descendant drop rejected") {
        DropFixture f; f.drop(11,13); CHECK_FALSE(f.get(11)->parent_id.is_valid()); CHECK(f.get(13)->parent_id==id(12)); CHECK(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI009 WorldTreePanel: missing target leaves original hierarchy intact") {
        DropFixture f; f.drop(12,99); CHECK(f.get(12)->parent_id==id(11)); CHECK(f.c.events.is_queue_empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI009 WorldTreePanel: valid drop marks old parent new parent and child modified") {
        DropFixture f; f.drop(12,14); CHECK(f.get(12)->parent_id==id(14)); CHECK(f.get(11)->state==ArchetypeState::Modified); CHECK(f.get(14)->state==ArchetypeState::Modified); CHECK(f.get(12)->state==ArchetypeState::Modified); CHECK_FALSE(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI009 WorldTreePanel: drop to root keeps child subtree intact") {
        DropFixture f; f.drop(12,0); CHECK_FALSE(f.get(12)->parent_id.is_valid()); CHECK(f.get(13)->parent_id==id(12)); CHECK(f.get(11)->child_ids.empty()); CHECK(f.c.ctx.editor_realm_manager->get_hierarchy().size()==3);
    }

    TEST_CASE("CRI009 WorldTreePanel: drop preserves new unsaved entity and parent states") {
        DropFixture f; f.get(12)->state=ArchetypeState::New; f.get(14)->state=ArchetypeState::New; f.drop(12,14); CHECK(f.get(12)->state==ArchetypeState::New); CHECK(f.get(14)->state==ArchetypeState::New); CHECK(f.get(12)->parent_id==id(14));
    }

    TEST_CASE("CRI009 WorldTreePanel: drop before initialization performs no work") {
        TreeProbe p; CHECK_NOTHROW(p.process_entity_drop_for_testing(id(11),id(14))); CHECK(p.get_name()=="World Tree Panel");
    }
}

namespace {
using namespace EditorCri;
void run_tree_commands(TreeFixture& f) {
    auto pending=std::move(f.c.ctx.deferred_commands);
    f.c.ctx.deferred_commands.clear();
    for(auto& command:pending) command();
}
void add_tree_component(const char* type) {
    TreeFixture f;
    SimpleGuid::update_next_id(100000);
    auto count=f.root()->elements.size();
    f.panel.queue_add_element_for_testing(id(11),type);
    REQUIRE(f.c.ctx.deferred_commands.size()==1);
    CHECK(f.root()->elements.size()==count);
    run_tree_commands(f);
    REQUIRE(f.root()->elements.size()==count+1);
    const auto& component=f.root()->elements.back();
    CHECK(component.type_name==type);
    CHECK(component.owner_id==id(11));
    CHECK(component.id.is_valid());
    CHECK(f.c.ctx.selected_entity_id==id(11));
    CHECK_FALSE(f.c.events.is_queue_empty());
}
}
TEST_SUITE("EditorCRI011_WorldTreePanel") {
    using namespace EditorCri;
    TEST_CASE("CRI011 WorldTreePanel: root menu command defers actual creation and selects new root") {
        TreeFixture f; SimpleGuid::update_next_id(100000); f.c.ctx.selected_element_id=id(12); f.panel.queue_add_root_for_testing(); CHECK(f.c.ctx.editor_realm_manager->get_realm_size()==1); run_tree_commands(f); REQUIRE(f.c.ctx.editor_realm_manager->get_realm_size()==2); auto selected=f.c.ctx.selected_entity_id; CHECK(selected!=id(11)); CHECK_FALSE(f.c.ctx.selected_element_id.is_valid()); REQUIRE(f.c.ctx.editor_realm_manager->get_archetype(selected)); CHECK_FALSE(f.c.ctx.editor_realm_manager->get_archetype(selected)->parent_id.is_valid());
    }

    TEST_CASE("CRI011 WorldTreePanel: transform menu adds component to selected owner") {
        add_tree_component("Transform");
    }

    TEST_CASE("CRI011 WorldTreePanel: collider menu adds actual reflected BoxCollider") {
        add_tree_component("BoxCollider");
    }

    TEST_CASE("CRI011 WorldTreePanel: sprite menu adds actual reflected Sprite2D") {
        add_tree_component("Sprite2D");
    }

    TEST_CASE("CRI011 WorldTreePanel: camera menu adds actual reflected Camera") {
        add_tree_component("Camera");
    }

    TEST_CASE("CRI011 WorldTreePanel: unknown component type does not change selection or model") {
        TreeFixture f; f.c.ctx.selected_entity_id=id(99); f.panel.queue_add_element_for_testing(id(11),"CRI011Unknown"); run_tree_commands(f); CHECK(f.root()->elements.size()==1); CHECK(f.c.ctx.selected_entity_id==id(99)); CHECK(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI011 WorldTreePanel: removed owner invalidates deferred command safely") {
        TreeFixture f; f.panel.queue_add_element_for_testing(id(11),"Sprite2D"); f.c.ctx.editor_realm_manager->purge_entity(id(11)); run_tree_commands(f); CHECK(f.c.ctx.editor_realm_manager->get_realm_size()==0);
    }

    TEST_CASE("CRI011 WorldTreePanel: queued component commands retain original owners") {
        TreeFixture f; SimpleGuid::update_next_id(100000); f.c.ctx.editor_realm_manager->add_entity(entity(21,"Other")); f.panel.queue_add_element_for_testing(id(11),"Camera"); f.panel.queue_add_element_for_testing(id(21),"Sprite2D"); run_tree_commands(f); CHECK(f.root()->has_element_of_type("Camera")); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(21))->has_element_of_type("Sprite2D"));
    }

    TEST_CASE("CRI011 WorldTreePanel: two queued root commands create distinct identities") {
        TreeFixture f; SimpleGuid::update_next_id(100000); f.panel.queue_add_root_for_testing(); f.panel.queue_add_root_for_testing(); run_tree_commands(f); const auto& r=f.c.ctx.editor_realm_manager->get_realm(); REQUIRE(r.size()==3); CHECK(r[1].id!=r[2].id); CHECK(f.c.ctx.selected_entity_id==r[2].id);
    }

    TEST_CASE("CRI011 WorldTreePanel: uninitialized command handlers leave queue untouched") {
        TreeProbe p; CHECK_NOTHROW(p.queue_add_root_for_testing()); CHECK_NOTHROW(p.queue_add_element_for_testing(id(11),"Camera"));
    }
}
