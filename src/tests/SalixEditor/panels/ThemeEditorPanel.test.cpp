// Tests for Editor/panels/ThemeEditorPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/ThemeEditorPanel.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct ThemeFixture {PanelContext c; ImGuiThemeManager themes; ThemeEditorPanel panel;
    ThemeFixture(){c.gui.icons=nullptr; c.ctx.theme_manager=&themes; panel.initialize(&c.ctx);}
    void empty_data(){auto theme=std::make_unique<ImGuiTheme>("CRI002"); theme->clear_data_for_testing(); REQUIRE(themes.register_theme(std::move(theme))); REQUIRE(themes.set_active_theme("CRI002"));}
};
}
TEST_SUITE("EditorCRI002_ThemeEditorPanel") {
    TEST_CASE("CRI002 ThemeEditorPanel: constructor defaults to hidden unlocked theme editor") {
        ThemeEditorPanel p; CHECK(p.get_name()=="Theme Editor"); CHECK_FALSE(p.get_visibility()); CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI002 ThemeEditorPanel: null init retains visibility and custom name") {
        ThemeEditorPanel p; p.set_name("Custom"); p.set_visibility(true); p.initialize(nullptr); CHECK(p.get_visibility()); CHECK(p.get_name()=="Custom");
    }

    TEST_CASE("CRI002 ThemeEditorPanel: initialization without icon service does not create visible panel") {
        ThemeFixture f; CHECK_FALSE(f.panel.get_visibility()); CHECK(f.themes.get_active_theme()==nullptr);
    }

    TEST_CASE("CRI002 ThemeEditorPanel: hidden panel needs no ImGui context") {
        ThemeEditorPanel p; p.on_gui_update(); CHECK_FALSE(p.get_visibility()); CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI002 ThemeEditorPanel: locking independent from visibility") {
        ThemeEditorPanel p; p.lock(); p.set_visibility(true); CHECK(p.is_locked()); CHECK(p.get_visibility()); p.set_visibility(false); CHECK(p.is_locked()); p.unlock(); CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI002 ThemeEditorPanel: unrelated events preserve name and state") {
        ThemeEditorPanel p; p.set_name("Other"); p.lock(); WindowResizeEvent e(10,10); p.on_event(e); CHECK(p.get_name()=="Other"); CHECK(p.is_locked()); CHECK_FALSE(p.get_visibility());
    }

    TEST_CASE("CRI002 ThemeEditorPanel: no active theme displays fallback text") {
        GuiFrame g; ThemeFixture f; f.panel.set_visibility(true); CHECK(g.draw([&]{f.panel.on_gui_update();})>0); CHECK(f.themes.get_active_theme()==nullptr);
    }

    TEST_CASE("CRI002 ThemeEditorPanel: locked missing theme balances disabled stack") {
        GuiFrame g; ThemeFixture f; f.panel.set_visibility(true); f.panel.lock(); const int depth=g.current->DisabledStackSize; CHECK(g.draw([&]{f.panel.on_gui_update();})>0); CHECK(g.current->DisabledStackSize==depth); CHECK(f.panel.is_locked());
    }

    TEST_CASE("CRI002 ThemeEditorPanel: active theme with missing data displays fallback") {
        GuiFrame g; ThemeFixture f; f.empty_data(); f.panel.set_visibility(true); CHECK(g.draw([&]{f.panel.on_gui_update();})>0); auto* t=dynamic_cast<ImGuiTheme*>(f.themes.get_active_theme()); REQUIRE(t); CHECK(t->get_data()==nullptr);
    }

    TEST_CASE("CRI002 ThemeEditorPanel: locked missing data balances disabled stack and preserves theme") {
        GuiFrame g; ThemeFixture f; f.empty_data(); f.panel.set_visibility(true); f.panel.lock(); const int depth=g.current->DisabledStackSize; CHECK(g.draw([&]{f.panel.on_gui_update();})>0); CHECK(g.current->DisabledStackSize==depth); CHECK(f.themes.get_active_theme()!=nullptr);
    }
}
