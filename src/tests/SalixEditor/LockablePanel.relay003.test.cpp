// CRI relay 003: LockablePanel; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/panels/LockablePanel.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct PanelProbe:LockablePanel{
    bool derived=true;int calls=0;bool disabled=false;
    PanelProbe(){set_title("CRI003 panel");}
    bool is_panel_derived()const override{return derived;}
    void on_panel_gui_update()override{++calls;disabled=ImGui::GetCurrentContext()->DisabledStackSize>0;ImGui::TextUnformatted("Panel contents");}
};
void draw_panel(GuiFrame& gui,PanelProbe& p){for(int i=0;i<2;++i)gui.frame([&]{p.on_gui_update();});}
}
TEST_SUITE("EditorCRI003_LockablePanel") {
    TEST_CASE("CRI003 LockablePanel: null init context does not prevent safe lock transitions") {
        PanelProbe p;p.initialize(nullptr);p.lock();CHECK(p.is_locked());p.unlock();CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI003 LockablePanel: missing icon service remains safe for selection and lock") {
        PanelContext c;c.init.icon_manager=nullptr;PanelProbe p;p.initialize(&c.ctx);p.set_lock_icon("Missing");p.lock();CHECK(p.is_locked());p.unlock();CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI003 LockablePanel: derived panel draws content and unlocked icon") {
        GuiFrame g;PanelContext c;PanelProbe p;p.initialize(&c.ctx);draw_panel(g,p);CHECK(p.calls>0);CHECK_FALSE(p.disabled);CHECK(p.get_current_lock_icon_path()=="unlocked.png");
    }

    TEST_CASE("CRI003 LockablePanel: derived locked panel renders content disabled") {
        GuiFrame g;PanelContext c;PanelProbe p;p.initialize(&c.ctx);p.lock();draw_panel(g,p);CHECK(p.calls>0);CHECK(p.disabled);CHECK(g.current->DisabledStackSize==0);CHECK(p.get_current_lock_icon_path()=="locked.png");
    }

    TEST_CASE("CRI003 LockablePanel: standalone locked panel suppresses content") {
        GuiFrame g;PanelContext c;PanelProbe p;p.derived=false;p.initialize(&c.ctx);p.lock();draw_panel(g,p);CHECK(p.calls==0);p.unlock();draw_panel(g,p);CHECK(p.calls>0);
    }

    TEST_CASE("CRI003 LockablePanel: missing icon fallback still renders content") {
        GuiFrame g;PanelContext c;c.init.icon_manager=nullptr;PanelProbe p;p.initialize(&c.ctx);draw_panel(g,p);CHECK(p.calls>0);CHECK_FALSE(p.disabled);
    }

    TEST_CASE("CRI003 LockablePanel: unregistered icon lookup does not create texture") {
        GuiFrame g;PanelContext c;PanelProbe p;p.initialize(&c.ctx);p.set_lock_icon("Unknown");CHECK(p.get_current_lock_icon_path().empty());draw_panel(g,p);CHECK(p.calls>0);
    }

    TEST_CASE("CRI003 LockablePanel: collapsed window keeps frame balanced") {
        GuiFrame g;PanelContext c;PanelProbe p;p.initialize(&c.ctx);for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowCollapsed(true,ImGuiCond_Always);p.on_gui_update();});CHECK(p.calls==0);CHECK(p.get_visibility());
    }

    TEST_CASE("CRI003 LockablePanel: hidden panel does not draw even with valid context") {
        GuiFrame g;PanelContext c;PanelProbe p;p.initialize(&c.ctx);p.set_visibility(false);draw_panel(g,p);CHECK(p.calls==0);CHECK_FALSE(p.get_visibility());
    }

    TEST_CASE("CRI003 LockablePanel: render and event hooks preserve configuration") {
        PanelContext c;PanelProbe p;p.initialize(&c.ctx);p.set_locked_state_tint_color(ImVec4(.2f,.3f,.4f,1));p.set_unlocked_state_tint_color(ImVec4(1,1,1,1));p.lock();WindowResizeEvent e(50,60);p.on_event(e);p.on_render();p.on_gui_render();CHECK(p.is_locked());CHECK(p.get_title()=="CRI003 panel");CHECK(p.calls==0);
    }
}
