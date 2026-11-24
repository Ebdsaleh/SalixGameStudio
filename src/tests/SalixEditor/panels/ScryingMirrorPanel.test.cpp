// Tests for Editor/panels/ScryingMirrorPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct MirrorProbe:ScryingMirrorPanel {void content(){on_panel_gui_update();} ImGuiWindowFlags flags()const{return get_window_flags();}};
struct MirrorFixture {PanelContext c; MirrorProbe panel; TempRealm file;
    MirrorFixture(){c.ctx.editor_realm_manager->load_realm_from_file(file.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 12, name: Local}}}]"));panel.initialize(&c.ctx);}
    void select(uint64_t n){EntitySelectedEvent e(id(n));panel.on_event(e);}
    void select_element(uint64_t n,uint64_t owner){ElementSelectedEvent e(id(n),id(owner));panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI002_ScryingMirrorPanel") {
    TEST_CASE("CRI002 ScryingMirrorPanel: constructor identity and derived flags") {
        MirrorProbe p; CHECK(p.get_name()=="Scrying Mirror Panel"); CHECK(p.get_title()=="Scrying Mirror"); CHECK(p.is_panel_derived()); CHECK(p.flags()==ImGuiWindowFlags_None);
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: null context leaves hidden state unchanged") {
        MirrorProbe p; p.set_visibility(false); p.initialize(nullptr); CHECK_FALSE(p.get_visibility());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: initialize subscribes and restores visibility") {
        MirrorFixture f; CHECK(f.panel.get_visibility()); EntitySelectedEvent e(id(11)); f.c.events.dispatch(e); f.c.events.process_queue(); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: entity selection clears prior element") {
        MirrorFixture f; f.select_element(12,11); f.select(11); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: element selection keeps owner and element") {
        MirrorFixture f; f.select_element(12,11); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(f.panel.get_selected_element_id_for_testing()==id(12));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: entity deselection clears both IDs") {
        MirrorFixture f; f.select_element(12,11); f.select(0); CHECK_FALSE(f.panel.get_selected_entity_id_for_testing().is_valid()); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: element deselection clears both IDs") {
        MirrorFixture f; f.select_element(12,11); f.select_element(0,11); CHECK_FALSE(f.panel.get_selected_entity_id_for_testing().is_valid()); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: unrelated event leaves selection intact") {
        MirrorFixture f; f.select_element(12,11); WindowResizeEvent e(9,9); f.panel.on_event(e); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(f.panel.get_selected_element_id_for_testing()==id(12));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: unselected inspector draws without changing realm") {
        GuiFrame g; MirrorFixture f; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: selected transform inspector renders without input mutation") {
        GuiFrame g; MirrorFixture f; f.select_element(12,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(g.draw([&]{f.panel.content();})>0); auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); CHECK(a->elements[0].data["name"].as<std::string>()=="Local"); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }
}

TEST_SUITE("EditorCRI005_ScryingMirrorPanel") {
    TEST_CASE("CRI005 ScryingMirrorPanel: unregistered component can be inspected without changing data") {
        GuiFrame gui; MirrorFixture f;
        auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a);
        a->elements.push_back(element("CRI005_Unknown",13,"{name: Unsupported}"));
        f.select_element(13,11); auto before=YAML::Dump(a->elements.back().data);
        CHECK(gui.draw([&]{f.panel.content();})>0);
        CHECK(YAML::Dump(a->elements.back().data)==before);
        CHECK(f.c.ctx.deferred_commands.empty());
    }
    TEST_CASE("CRI005 ScryingMirrorPanel: absent realm manager suppresses inspector safely") {
        GuiFrame gui; MirrorFixture f; f.c.ctx.editor_realm_manager.reset();
        f.select(11); CHECK(gui.draw([&]{f.panel.content();})==0);
        CHECK(f.c.ctx.deferred_commands.empty());
    }
}



TEST_SUITE("EditorCRI006_ScryingMirrorPanel") {
    TEST_CASE("CRI006 ScryingMirrorPanel: whole entity inspector preserves selection and source") {
        GuiFrame g; MirrorFixture f; f.select(11); auto before=YAML::Dump(f.c.ctx.editor_realm_manager->get_archetype(id(11))->elements[0].data); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(YAML::Dump(f.c.ctx.editor_realm_manager->get_archetype(id(11))->elements[0].data)==before);
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: stale entity selection renders fallback") {
        GuiFrame g; MirrorFixture f; f.select(99); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: stale element selection does not edit other element") {
        GuiFrame g; MirrorFixture f; f.select_element(99,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: live mode avoids YAML inspector mutations") {
        GuiFrame g; MirrorFixture f; f.select(11); f.c.ctx.data_mode=EditorDataMode::Live; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: hidden content is suppressed") {
        GuiFrame g; MirrorFixture f; f.select(11); f.panel.set_visibility(false); CHECK(g.draw([&]{f.panel.content();})==0); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11));
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: box collider inspector without sprite remains unchanged") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.push_back(element("BoxCollider",13,"{name: Bounds}")); a->elements.back().owner_id=a->id; f.c.ctx.editor_realm_manager->take_snapshot(); f.select_element(13,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: collider with authored sprite dimensions draws resize controls") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.push_back(element("BoxCollider",13,"{name: Bounds}")); a->elements.back().owner_id=a->id; a->elements.push_back(element("Sprite2D",14,"{name: Image, width: 64, height: 32}")); a->elements.back().owner_id=a->id; f.select_element(13,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK(a->elements.back().data["width"].as<int>()==64);
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: whole inspector skips unknown components and continues") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.insert(a->elements.begin(),element("CRI006Unknown",13)); f.select(11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(a->elements.size()==2); CHECK(f.c.ctx.deferred_commands.empty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: locked inspector renders without writing") {
        GuiFrame g; MirrorFixture f; f.select(11); f.panel.lock(); for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowSize(ImVec2(600,400),ImGuiCond_Always); f.panel.on_gui_update();}); CHECK(f.panel.is_locked()); CHECK(g.current->DisabledStackSize==0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: switching selection between entities preserves both archetypes") {
        GuiFrame g; MirrorFixture f; auto a=entity(20,"Second"); a.elements.push_back(element("Transform",21,"{name: Other}")); f.c.ctx.editor_realm_manager->add_entity(a); f.select(11); CHECK(g.draw([&]{f.panel.content();})>0); f.select_element(21,20); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(20))->elements[0].name=="Transform"); CHECK(f.panel.get_selected_element_id_for_testing()==id(21));
    }
}
