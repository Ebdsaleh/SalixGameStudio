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
