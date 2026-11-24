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

#include <Salix/gui/DialogBox.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
namespace {
struct ActiveThemeFixture {
    PanelContext c; ImGuiThemeManager themes; ThemeEditorPanel panel;
    ActiveThemeFixture() {
        c.ctx.theme_manager=&themes;
        auto t=std::make_unique<ImGuiTheme>("CRI006", "fixture.yaml");
        REQUIRE(themes.register_theme(std::move(t)));
        REQUIRE(themes.set_active_theme("CRI006"));
        panel.initialize(&c.ctx);
        panel.set_visibility(true);
    }
    ImGuiThemeData* data(){auto* t=dynamic_cast<ImGuiTheme*>(themes.get_active_theme());REQUIRE(t);return t->get_data();}
    void draw(GuiFrame& g){for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowSize(ImVec2(700,500),ImGuiCond_Always);panel.on_gui_update();});}
};
}
TEST_SUITE("EditorCRI006_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI006 ThemeEditorPanel: initialized theme editor registers image dialog with filter") {
        ActiveThemeFixture f; auto* d=f.c.gui.get_dialog("SelectIconImage"); REQUIRE(d); CHECK(d->get_title()=="Select An Icon Image"); CHECK(d->get_filters().find("*.png")!=std::string::npos);
    }

    TEST_CASE("CRI006 ThemeEditorPanel: reinitialization reuses existing image dialog") {
        ActiveThemeFixture f; auto* d=f.c.gui.get_dialog("SelectIconImage"); REQUIRE(d); f.panel.initialize(&f.c.ctx); CHECK(f.c.gui.get_dialog("SelectIconImage")==d);
    }

    TEST_CASE("CRI006 ThemeEditorPanel: uninitialized visible panel returns without GUI context") {
        ThemeEditorPanel p; p.set_visibility(true); p.on_gui_update(); CHECK(p.get_visibility());
    }

    TEST_CASE("CRI006 ThemeEditorPanel: missing GUI on initialization leaves active theme untouched") {
        ThemeFixture f; f.c.ctx.gui=nullptr; f.panel.initialize(&f.c.ctx); CHECK_FALSE(f.panel.get_visibility()); CHECK(f.themes.get_active_theme()==nullptr);
    }

    TEST_CASE("CRI006 ThemeEditorPanel: missing theme manager returns without GUI context") {
        ThemeFixture f; f.c.ctx.theme_manager=nullptr; f.panel.set_visibility(true); f.panel.on_gui_update(); CHECK(f.panel.get_visibility());
    }

    TEST_CASE("CRI006 ThemeEditorPanel: active theme without icon service displays safe fallback") {
        GuiFrame g; ThemeFixture f; REQUIRE(f.themes.register_theme(std::make_unique<ImGuiTheme>("NoIcons"))); REQUIRE(f.themes.set_active_theme("NoIcons")); f.panel.set_visibility(true); CHECK(g.draw([&]{f.panel.on_gui_update();})>0); CHECK(f.themes.get_active_theme()->get_name()=="NoIcons");
    }

    TEST_CASE("CRI006 ThemeEditorPanel: active theme draws without changing name or file path") {
        GuiFrame g; ActiveThemeFixture f; REQUIRE(f.data()); auto name=f.data()->name; f.draw(g); CHECK(f.data()->name==name); CHECK(f.themes.get_active_theme()->get_file_path()=="fixture.yaml"); CHECK(f.panel.get_visibility());
    }

    TEST_CASE("CRI006 ThemeEditorPanel: locked active theme keeps disabled stack balanced") {
        GuiFrame g; ActiveThemeFixture f; f.panel.lock(); f.draw(g); CHECK(g.current->DisabledStackSize==0); CHECK(f.panel.is_locked()); REQUIRE(f.data());
    }

    TEST_CASE("CRI006 ThemeEditorPanel: collapsed active theme keeps frame balanced") {
        GuiFrame g; ActiveThemeFixture f; f.draw(g); g.frame([&]{ImGui::SetNextWindowCollapsed(true,ImGuiCond_Always); f.panel.on_gui_update();}); CHECK(g.current->DisabledStackSize==0); CHECK(f.panel.get_visibility());
    }

    TEST_CASE("CRI006 ThemeEditorPanel: hidden active theme needs no frame and retains data") {
        ActiveThemeFixture f; auto* d=f.data(); REQUIRE(d); f.panel.set_visibility(false); f.panel.on_gui_update(); f.panel.on_render(); f.panel.on_gui_render(); CHECK(f.data()==d); CHECK_FALSE(f.panel.get_visibility());
    }
}
