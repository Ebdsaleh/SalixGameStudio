// Tests for Editor/panels/LockablePanel.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/panels/LockablePanel.h>
#include <Salix/core/InitContext.h>
#include <Salix/gui/IIconManager.h>
#include <Salix/gui/IconInfo.h>
#include "../fixtures/RealmFixtures.h"
#include <Salix/events/sdl/SDLEvent.h>

namespace {
namespace coverage_batch_001 {
namespace {
using namespace EditorCri;
struct Icons : Salix::IIconManager {
    std::map<std::string,IconInfo> registry;
    IconInfo absent;
    Icons() { registry["Panel Locked"].path="locked.png"; registry["Panel Unlocked"].path="unlocked.png"; registry["Custom"].path="custom.png"; registry["Custom"].texture_id=ImTextureID(1); }
    void initialize(AssetManager*) override {}
    void register_default_icons() override {}
    const IconInfo& get_icon_for_entity(Entity*) override { return absent; }
    const IconInfo& get_icon_for_element(Element*) override { return absent; }
    const IconInfo& get_icon_by_name(const std::string& n) const override { auto it=registry.find(n); return it==registry.end()?absent:it->second; }
    void update_icon(const std::string&,const std::string&) override {}
    const std::map<std::string,IconInfo>& get_icon_registry() override { return registry; }
};
struct Probe : LockablePanel { int updates=0; void on_panel_gui_update() override {++updates;} void draw_for_test(){draw_panel_contents();} };
struct PanelFixture { Icons icons; InitContext init{}; EditorContext context; Probe panel;
    PanelFixture(){init.icon_manager=&icons; context.init_context=&init; panel.initialize(&context);}
};
}
TEST_SUITE("EditorCRI001_LockablePanel") {
    TEST_CASE("CRI001 LockablePanel: default state visible unlocked and named") {
        Probe p; CHECK(p.get_visibility()); CHECK_FALSE(p.is_locked()); CHECK(p.get_name()=="New Lockable Panel"); CHECK(p.get_lock_icon()=="Panel Unlocked");
    }

    TEST_CASE("CRI001 LockablePanel: name and title are independent") {
        Probe p; p.set_name("id"); p.set_title("Visible title"); CHECK(p.get_name()=="id"); CHECK(p.get_title()=="Visible title");
    }

    TEST_CASE("CRI001 LockablePanel: hidden update does not invoke content") {
        Probe p; p.set_visibility(false); p.on_gui_update(); CHECK_FALSE(p.get_visibility()); CHECK(p.updates==0); p.set_visibility(true); CHECK(p.get_visibility());
    }

    TEST_CASE("CRI001 LockablePanel: icon geometry round trips") {
        Probe p; p.set_lock_icon_size(ImVec2(20,30)); p.set_top_left(ImVec2(.1f,.2f)); p.set_bottom_right(ImVec2(.8f,.9f)); CHECK(p.get_lock_icon_size().x==20); CHECK(p.get_lock_icon_size().y==30); CHECK(p.get_top_left().x==doctest::Approx(.1)); CHECK(p.get_bottom_right().y==doctest::Approx(.9));
    }

    TEST_CASE("CRI001 LockablePanel: initialization selects unlocked icon") {
        PanelFixture f; CHECK(f.panel.get_current_lock_icon_path()=="unlocked.png"); CHECK_FALSE(f.panel.is_locked());
    }

    TEST_CASE("CRI001 LockablePanel: lock switches state and icon") {
        PanelFixture f; f.panel.lock(); CHECK(f.panel.is_locked()); CHECK(f.panel.get_lock_icon()=="Panel Locked"); CHECK(f.panel.get_current_lock_icon_path()=="locked.png");
    }

    TEST_CASE("CRI001 LockablePanel: unlock restores state and icon") {
        PanelFixture f; f.panel.lock(); f.panel.unlock(); CHECK_FALSE(f.panel.is_locked()); CHECK(f.panel.get_current_lock_icon_path()=="unlocked.png");
    }

    TEST_CASE("CRI001 LockablePanel: standalone content suppressed while locked") {
        PanelFixture f; f.panel.draw_for_test(); CHECK(f.panel.updates==1); f.panel.lock(); f.panel.draw_for_test(); CHECK(f.panel.updates==1); f.panel.unlock(); f.panel.draw_for_test(); CHECK(f.panel.updates==2);
    }

    TEST_CASE("CRI001 LockablePanel: empty icon selection preserves existing icon") {
        PanelFixture f; f.panel.set_lock_icon(""); CHECK(f.panel.get_current_lock_icon_path()=="unlocked.png");
    }

    TEST_CASE("CRI001 LockablePanel: custom icon uses registry path") {
        PanelFixture f; f.panel.set_lock_icon("Custom"); CHECK(f.panel.get_current_lock_icon_path()=="custom.png"); f.panel.unlock(); CHECK(f.panel.get_current_lock_icon_path()=="unlocked.png");
    }
}
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
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
} // namespace coverage_batch_003
} // namespace
