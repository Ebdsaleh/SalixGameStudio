// CRI relay 002: ScryingMirrorPanel; off-screen and observable behavior.
#include "CriRelay002Fixtures.h"
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
