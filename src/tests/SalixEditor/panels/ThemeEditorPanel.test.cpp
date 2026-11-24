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

#include <Salix/gui/imgui/ImGuiFontManager.h>
namespace {
using namespace EditorCri;
void open_theme_section(GuiFrame& g,ActiveThemeFixture& f,const char* section) {
    f.draw(g);
    auto* window=ImGui::FindWindowByName(f.panel.get_name().c_str());
    REQUIRE(window);
    window->StateStorage.SetInt(window->GetID(section),1);
    f.draw(g);
    CHECK(window->StateStorage.GetInt(window->GetID(section))==1);
    CHECK(g.current->DisabledStackSize==0);
}
} // namespace
TEST_SUITE("EditorCRI007_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI007 ThemeEditorPanel: expanded colors preserve theme and current style without edits") {
        GuiFrame g; ActiveThemeFixture f; auto before=f.data()->text_color; open_theme_section(g,f,"Colors"); CHECK(f.data()->text_color==before); CHECK(f.panel.get_visibility());
    }

    TEST_CASE("CRI007 ThemeEditorPanel: expanded sizing controls preserve authored rounding") {
        GuiFrame g; ActiveThemeFixture f; f.data()->window_rounding=8; f.data()->frame_rounding=3; open_theme_section(g,f,"Sizing & Spacing"); CHECK(f.data()->window_rounding==8); CHECK(f.data()->frame_rounding==3);
    }

    TEST_CASE("CRI007 ThemeEditorPanel: expanded sizing controls retain nondefault vector spacing") {
        GuiFrame g; ActiveThemeFixture f; f.data()->window_padding=Vector2(7,11); f.data()->item_spacing=Vector2(2,4); open_theme_section(g,f,"Sizing & Spacing"); CHECK(f.data()->window_padding==Vector2(7,11)); CHECK(f.data()->item_spacing==Vector2(2,4));
    }

    TEST_CASE("CRI007 ThemeEditorPanel: expanded fonts tolerate unavailable font manager") {
        GuiFrame g; ActiveThemeFixture f; auto name=f.data()->font_family; open_theme_section(g,f,"Fonts"); CHECK(f.data()->font_family==name); CHECK(f.c.ctx.font_manager==nullptr);
    }

    TEST_CASE("CRI007 ThemeEditorPanel: expanded fonts with empty registry retain authored selection") {
        GuiFrame g; ActiveThemeFixture f; ImGuiFontManager fonts; f.c.ctx.font_manager=&fonts; f.data()->font_family="UnknownFamily"; f.data()->font_size=19; open_theme_section(g,f,"Fonts"); CHECK(f.data()->font_family=="UnknownFamily"); CHECK(f.data()->font_size==19);
    }

    TEST_CASE("CRI007 ThemeEditorPanel: expanded icons preserve current paths when registry empty") {
        GuiFrame g; ActiveThemeFixture f; f.data()->icon_paths["Entity"]="Assets/entity.png"; open_theme_section(g,f,"Icon Theme"); CHECK(f.data()->icon_paths.at("Entity")=="Assets/entity.png");
    }

    TEST_CASE("CRI007 ThemeEditorPanel: SDL icon table draws registered icon without changing path") {
        GuiFrame g; ActiveThemeFixture f; f.c.icons.registry["Entity"]=f.c.icons.unlocked; f.data()->icon_paths["Entity"]="authored.png"; f.c.init.renderer_type=RendererType::SDL; open_theme_section(g,f,"Icon Theme"); CHECK(f.data()->icon_paths.at("Entity")=="authored.png");
    }

    TEST_CASE("CRI007 ThemeEditorPanel: OpenGL icon table preserves registered icon and authored path") {
        GuiFrame g; ActiveThemeFixture f; f.c.icons.registry["Entity"]=f.c.icons.locked; f.data()->icon_paths["Entity"]="authored.png"; f.c.init.renderer_type=RendererType::OpenGL; open_theme_section(g,f,"Icon Theme"); CHECK(f.data()->icon_paths.at("Entity")=="authored.png"); CHECK(f.c.icons.registry.at("Entity").texture_id==f.c.icons.locked.texture_id);
    }

    TEST_CASE("CRI007 ThemeEditorPanel: icon section tolerates missing InitContext icon manager") {
        GuiFrame g; ActiveThemeFixture f; f.c.init.icon_manager=nullptr; open_theme_section(g,f,"Icon Theme"); CHECK(f.panel.get_visibility());
    }

    TEST_CASE("CRI007 ThemeEditorPanel: switching theme after open sections preserves new theme") {
        GuiFrame g; ActiveThemeFixture f; open_theme_section(g,f,"Colors"); auto t=std::make_unique<ImGuiTheme>("Second", "second.yaml"); REQUIRE(f.themes.register_theme(std::move(t))); REQUIRE(f.themes.set_active_theme("Second")); f.draw(g); CHECK(f.themes.get_active_theme()->get_name()=="Second"); CHECK(f.themes.get_active_theme()->get_file_path()=="second.yaml");
    }
}


namespace {
using namespace EditorCri;
struct ThemeFiles {
    TempRealm temp; ActiveThemeFixture f;
    std::string write(const std::string& yaml){return temp.write(yaml);}
    void save(){f.panel.save_theme_for_testing(temp.path());}
    void load(const char* name){f.panel.load_theme_for_testing(name,temp.path());}
};
}
TEST_SUITE("EditorCRI009_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 ThemeEditorPanel: save writes active theme metadata to isolated path") {
        ThemeFiles v; v.save(); REQUIRE(std::filesystem::exists(v.temp.path())); auto n=YAML::LoadFile(v.temp.path()); CHECK(n["name"].as<std::string>()==v.f.data()->name);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: save serializes authored font selection") {
        ThemeFiles v; v.f.data()->font_family="FixtureFamily"; v.f.data()->font_size=19; v.save(); auto n=YAML::LoadFile(v.temp.path()); CHECK(n["Font"]["font_family"].as<std::string>()=="FixtureFamily"); CHECK(n["Font"]["font_size"].as<float>()==19);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: save does not change active theme or request reload") {
        ThemeFiles v; auto* old=v.f.themes.get_active_theme(); v.save(); CHECK(v.f.themes.get_active_theme()==old); CHECK(v.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: missing active theme cannot create file") {
        TempRealm t; ThemeFixture f; f.panel.save_theme_for_testing(t.path()); CHECK_FALSE(std::filesystem::exists(t.path()));
    }

    TEST_CASE("CRI009 ThemeEditorPanel: missing load path retains old theme and reload count") {
        ThemeFiles v; auto* old=v.f.themes.get_active_theme(); v.load("Missing"); CHECK(v.f.themes.get_active_theme()==old); CHECK(v.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: malformed load retains current theme") {
        ThemeFiles v; auto* old=v.f.themes.get_active_theme(); v.write("name: ["); v.load("Broken"); CHECK(v.f.themes.get_active_theme()==old); CHECK(v.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: valid load switches registry identity and requests reload") {
        ThemeFiles v; v.write("name: Loaded\nFont: {font_family: FixtureFamily, font_size: 20}"); v.load("Loaded"); REQUIRE(v.f.themes.get_active_theme()); CHECK(v.f.themes.get_active_theme()->get_name()=="Loaded"); CHECK(v.f.data()->font_family=="FixtureFamily"); CHECK(v.f.c.gui.theme_reloads==1);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: reloading same key replaces data safely") {
        ThemeFiles v; v.write("name: CRI006\nFont: {font_family: Changed, font_size: 23}"); v.load("CRI006"); REQUIRE(v.f.data()); CHECK(v.f.data()->font_family=="Changed"); CHECK(v.f.c.gui.theme_reloads==1); GuiFrame g; v.f.draw(g); CHECK(g.current->DisabledStackSize==0);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: saved theme round trips font data under new registry key") {
        ThemeFiles v; v.f.data()->font_family="RoundTrip"; v.f.data()->font_size=21; v.save(); v.load("Copy"); REQUIRE(v.f.data()); CHECK(v.f.themes.get_active_theme()->get_name()=="Copy"); CHECK(v.f.data()->font_family=="RoundTrip"); CHECK(v.f.data()->font_size==21);
    }

    TEST_CASE("CRI009 ThemeEditorPanel: uninitialized panel file operations are harmless") {
        TempRealm t; ThemeEditorPanel p; CHECK_NOTHROW(p.save_theme_for_testing(t.path())); CHECK_NOTHROW(p.load_theme_for_testing("Missing",t.path())); CHECK_FALSE(std::filesystem::exists(t.path()));
    }
}

#include <Salix/gui/IGui.h>
namespace {
using namespace EditorCri;
struct IconDialogFixture {
    TempRealm temp; ScopedWorkingDir cwd; ActiveThemeFixture f;
    IconDialogFixture():cwd(temp) {f.data()->icon_paths["Entity"]="old.png";f.data()->icon_paths["Camera"]="camera.png";}
    Salix::DialogBox* browse(const std::string& type){f.panel.browse_icon_for_testing(type);auto* d=f.c.gui.get_dialog("SelectIconImage");REQUIRE(d);REQUIRE(d->get_callback());return d;}
    Salix::FileDialogResult result(bool accepted=true){Salix::FileDialogResult result{};result.is_ok=accepted;result.file_path_name=temp.write("fixture");result.folder_path=std::filesystem::current_path().string();return result;}
};
}
TEST_SUITE("EditorCRI011_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI011 ThemeEditorPanel: browse uses registered dialog and default directory") {
        IconDialogFixture x;auto* d=x.browse("Entity");CHECK(x.f.c.gui.shown_dialog=="SelectIconImage");CHECK(d->get_default_path()==".");
    }

    TEST_CASE("CRI011 ThemeEditorPanel: accepted icon path is relative and reloads theme") {
        IconDialogFixture x;auto* d=x.browse("Entity");d->get_callback()(x.result());CHECK(x.f.data()->icon_paths.at("Entity")==std::filesystem::path(x.temp.path()).filename().string());CHECK(x.f.c.gui.theme_reloads==1);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: cancel preserves authored icon and skips reload") {
        IconDialogFixture x;auto* d=x.browse("Entity");d->get_callback()(x.result(false));CHECK(x.f.data()->icon_paths.at("Entity")=="old.png");CHECK(x.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: next browse remembers last accepted directory") {
        IconDialogFixture x;auto* d=x.browse("Entity");auto r=x.result();d->get_callback()(r);auto* next=x.browse("Camera");CHECK(next->get_default_path()==r.folder_path);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: second browse edits only requested icon") {
        IconDialogFixture x;auto* d=x.browse("Camera");d->get_callback()(x.result());CHECK(x.f.data()->icon_paths.at("Entity")=="old.png");CHECK(x.f.data()->icon_paths.at("Camera")!="camera.png");
    }

    TEST_CASE("CRI011 ThemeEditorPanel: stale callback cannot overwrite more recent icon request") {
        IconDialogFixture x;auto previous=x.browse("Entity")->get_callback();auto* d=x.browse("Camera");previous(x.result());CHECK(x.f.c.gui.theme_reloads==0);d->get_callback()(x.result());CHECK(x.f.c.gui.theme_reloads==1);CHECK(x.f.data()->icon_paths.at("Entity")=="old.png");
    }

    TEST_CASE("CRI011 ThemeEditorPanel: duplicate callback invocation is consumed only once") {
        IconDialogFixture x;auto cb=x.browse("Entity")->get_callback();cb(x.result());cb(x.result());CHECK(x.f.c.gui.theme_reloads==1);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: switching active theme discards previous icon result") {
        IconDialogFixture x;auto cb=x.browse("Entity")->get_callback();REQUIRE(x.f.themes.register_theme(std::make_unique<ImGuiTheme>("Second")));REQUIRE(x.f.themes.set_active_theme("Second"));cb(x.result());CHECK(x.f.c.gui.theme_reloads==0);CHECK(x.f.data()->icon_paths.count("Entity")==0);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: missing selected file path does not erase icon") {
        IconDialogFixture x;auto* d=x.browse("Entity");auto r=x.result();r.file_path_name.clear();d->get_callback()(r);CHECK(x.f.data()->icon_paths.at("Entity")=="old.png");CHECK(x.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI011 ThemeEditorPanel: callback after panel destruction is inert") {
        IconDialogFixture x;auto p=std::make_unique<ThemeEditorPanel>();p->initialize(&x.f.c.ctx);p->browse_icon_for_testing("Entity");auto* d=x.f.c.gui.get_dialog("SelectIconImage");REQUIRE(d);auto cb=d->get_callback();p.reset();cb(x.result());CHECK(x.f.data()->icon_paths.at("Entity")=="old.png");CHECK(x.f.c.gui.theme_reloads==0);
    }
}
