// CRI relay 001: LockablePanel; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/panels/LockablePanel.h>
#include <Salix/core/InitContext.h>
#include <Salix/gui/IIconManager.h>
#include <Salix/gui/IconInfo.h>
namespace {
using namespace EditorCri;
struct Icons : IIconManager {
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
