// CRI relay 002: ProjectSettingsPanel; off-screen and observable behavior.
#include "CriRelay002Fixtures.h"
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
