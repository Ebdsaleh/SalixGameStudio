// Tests for Editor/panels/ProjectSettingsPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/ProjectSettingsPanel.h>
namespace {
using namespace EditorCri;
struct SettingsProbe:ProjectSettingsPanel{void content(){on_panel_gui_update();} ImGuiWindowFlags flags()const{return get_window_flags();}};
struct SettingsFixture {PanelContext c; SettingsProbe panel; SettingsFixture(){panel.initialize(&c.ctx);}};
void unchanged(const GridSettings& a,const GridSettings& b){CHECK(a.size==b.size);CHECK(a.major_division==b.major_division);CHECK(a.minor_division==b.minor_division);CHECK(a.snap_size==b.snap_size);CHECK(a.snap_enabled==b.snap_enabled);CHECK(a.color.r==b.color.r);CHECK(a.color.a==b.color.a);}
}
TEST_SUITE("EditorCRI002_ProjectSettingsPanel") {
    TEST_CASE("CRI002 ProjectSettingsPanel: constructor configures panel title and icon geometry") {
        SettingsProbe p; CHECK(p.get_name()=="Project Settings Panel"); CHECK(p.get_title()=="Project Settings"); CHECK(p.get_lock_icon_size().x==16); CHECK(p.flags()==ImGuiWindowFlags_None); CHECK(p.is_panel_derived());
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: null initialize preserves preexisting visibility") {
        SettingsProbe p; p.set_visibility(true); p.initialize(nullptr); CHECK(p.get_visibility());
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: initialization hides panel until requested") {
        SettingsFixture f; CHECK_FALSE(f.panel.get_visibility()); CHECK(f.panel.get_current_lock_icon_path()=="unlocked.png");
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: opening and closing retains current grid model") {
        SettingsFixture f; auto before=f.c.ctx.grid_settings; f.panel.set_visibility(true); CHECK(f.panel.get_visibility()); f.panel.set_visibility(false); f.panel.on_gui_update(); unchanged(before,f.c.ctx.grid_settings);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: draw default grid controls with no input") {
        GuiFrame g; SettingsFixture f; auto before=f.c.ctx.grid_settings; CHECK(g.draw([&]{f.panel.content();})>0); unchanged(before,f.c.ctx.grid_settings);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: minimum major division branch leaves minor setting intact") {
        GuiFrame g; SettingsFixture f; f.c.ctx.grid_settings.major_division=.1f; f.c.ctx.grid_settings.minor_division=.02f; auto before=f.c.ctx.grid_settings; CHECK(g.draw([&]{f.panel.content();})>0); unchanged(before,f.c.ctx.grid_settings);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: nondefault grid values displayed without writes") {
        GuiFrame g; SettingsFixture f; auto& grid=f.c.ctx.grid_settings; grid.size=42; grid.major_division=5; grid.minor_division=.5f; grid.snap_size=.25f; grid.snap_enabled=false; auto before=grid; CHECK(g.draw([&]{f.panel.content();})>0); unchanged(before,grid);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: independent editor contexts retain separate settings") {
        GuiFrame g; SettingsFixture a; SettingsFixture b; a.c.ctx.grid_settings.size=25; b.c.ctx.grid_settings.size=75; CHECK(g.draw([&]{a.panel.content();})>0); CHECK(g.draw([&]{b.panel.content();})>0); CHECK(a.c.ctx.grid_settings.size==25); CHECK(b.c.ctx.grid_settings.size==75);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: locked state retains grid data during offscreen update") {
        GuiFrame g; SettingsFixture f; f.panel.lock(); f.panel.set_visibility(true); auto before=f.c.ctx.grid_settings; g.frame([&]{f.panel.on_gui_update();}); CHECK(f.panel.is_locked()); unchanged(before,f.c.ctx.grid_settings);
    }

    TEST_CASE("CRI002 ProjectSettingsPanel: grid section can be collapsed without mutating configuration") {
        GuiFrame g; SettingsFixture f; auto before=f.c.ctx.grid_settings; g.draw([&]{ImGui::SetNextItemOpen(false,ImGuiCond_Always);f.panel.content();}); unchanged(before,f.c.ctx.grid_settings); CHECK(f.c.ctx.deferred_commands.empty());
    }
}


namespace {
using namespace EditorCri;
struct SettingsInteraction {
    GuiFrame gui; SettingsFixture f; ImVec2 checkbox{},save{};
    void frame(bool disabled=false,bool collapse=false) {
        gui.draw([&]{
            ImGui::SetNextItemOpen(!collapse,ImGuiCond_Always);
            auto top=ImGui::GetCursorScreenPos();float h=ImGui::GetFrameHeight();
            checkbox=ImVec2(top.x+h*.5f,top.y+h+ImGui::GetStyle().ItemSpacing.y+h*.5f);
            if(disabled)ImGui::BeginDisabled();
            f.panel.content();
            if(disabled)ImGui::EndDisabled();
            if(f.panel.has_pending_changes_for_testing()){
                auto t=ImGui::GetItemRectMin();
                save=ImVec2(t.x-ImGui::GetStyle().ItemSpacing.x-60,t.y+ImGui::GetTextLineHeight()*.5f);
            }
        });
    }
    void click(ImVec2 pos,bool disabled=false) {
        ImGui::GetIO().AddMousePosEvent(pos.x,pos.y);frame(disabled);
        ImGui::GetIO().AddMouseButtonEvent(0,true);frame(disabled);
        ImGui::GetIO().AddMouseButtonEvent(0,false);frame(disabled);
    }
    void toggle(){frame();frame();click(checkbox);}
};
}
TEST_SUITE("EditorCRI009_ProjectSettingsPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 ProjectSettingsPanel: uninitialized content does not require grid context") {
        GuiFrame g;SettingsProbe p;CHECK(g.draw([&]{p.content();})==0);CHECK_FALSE(p.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: initial drawing keeps pending flag clear") {
        SettingsInteraction s;s.frame();s.frame();CHECK_FALSE(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: grid enable checkbox updates model and pending flag") {
        SettingsInteraction s;bool old=s.f.c.ctx.grid_settings.snap_enabled;s.toggle();CHECK(s.f.c.ctx.grid_settings.snap_enabled!=old);CHECK(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: pending changes survive frames without input") {
        SettingsInteraction s;s.toggle();REQUIRE(s.f.panel.has_pending_changes_for_testing());s.frame();s.frame();CHECK(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: second checkbox edit restores value but remains pending") {
        SettingsInteraction s;bool old=s.f.c.ctx.grid_settings.snap_enabled;s.toggle();s.click(s.checkbox);CHECK(s.f.c.ctx.grid_settings.snap_enabled==old);CHECK(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: apply validates grid and clears pending changes") {
        SettingsInteraction s;s.toggle();REQUIRE(s.f.panel.has_pending_changes_for_testing());s.f.c.ctx.grid_settings.size=5000;s.frame();s.click(s.save);CHECK(s.f.c.ctx.grid_settings.size==1000);CHECK_FALSE(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: collapsed grid section retains pending edits") {
        SettingsInteraction s;s.toggle();REQUIRE(s.f.panel.has_pending_changes_for_testing());s.frame(false,true);CHECK(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: disabled widgets cannot change grid or pending flag") {
        SettingsInteraction s;bool old=s.f.c.ctx.grid_settings.snap_enabled;s.frame(true);s.frame(true);s.click(s.checkbox,true);CHECK(s.f.c.ctx.grid_settings.snap_enabled==old);CHECK_FALSE(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: panel hide and show retains pending changes") {
        SettingsInteraction s;s.toggle();REQUIRE(s.f.panel.has_pending_changes_for_testing());s.f.panel.set_visibility(false);s.f.panel.set_visibility(true);s.frame();CHECK(s.f.panel.has_pending_changes_for_testing());
    }

    TEST_CASE("CRI009 ProjectSettingsPanel: pending settings isolated between contexts") {
        SettingsInteraction s;s.toggle();REQUIRE(s.f.panel.has_pending_changes_for_testing());SettingsFixture other;CHECK_FALSE(other.panel.has_pending_changes_for_testing());CHECK(other.c.ctx.grid_settings.snap_enabled!=s.f.c.ctx.grid_settings.snap_enabled);
    }
}
