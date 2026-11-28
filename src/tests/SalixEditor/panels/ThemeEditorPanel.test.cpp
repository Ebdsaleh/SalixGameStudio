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

namespace {
using namespace EditorCri;
struct FontChoices : ImGuiFontManager {
    std::vector<std::string> get_unique_font_families() const override {return {"Alpha","Beta","Empty"};}
    std::vector<float> get_font_sizes_for_font_family(const std::string& name) const override {
        if(name=="Alpha")return {12,18,24};
        if(name=="Beta")return {16,20};
        return {};
    }
};
struct ThemeFonts {
    FontChoices fonts; ActiveThemeFixture f;
    ThemeFonts(){f.c.ctx.font_manager=&fonts;f.data()->font_family="Alpha";f.data()->font_size=18;}
};
}
TEST_SUITE("EditorCRI013_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI013 ThemeEditorPanel: populated font section renders without changing selected size") {
        GuiFrame g;ThemeFonts v;open_theme_section(g,v.f,"Fonts");CHECK(v.f.data()->font_family=="Alpha");CHECK(v.f.data()->font_size==18);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: selecting another family chooses its first available size") {
        ThemeFonts v;CHECK(v.f.panel.choose_font_family_for_testing("Beta"));CHECK(v.f.data()->font_family=="Beta");CHECK(v.f.data()->font_size==16);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: family with no sizes preserves current size") {
        ThemeFonts v;CHECK(v.f.panel.choose_font_family_for_testing("Empty"));CHECK(v.f.data()->font_family=="Empty");CHECK(v.f.data()->font_size==18);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: unknown family cannot modify current theme") {
        ThemeFonts v;CHECK_FALSE(v.f.panel.choose_font_family_for_testing("Unknown"));CHECK(v.f.data()->font_family=="Alpha");CHECK(v.f.data()->font_size==18);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: registered font size changes only size") {
        ThemeFonts v;CHECK(v.f.panel.choose_font_size_for_testing(24));CHECK(v.f.data()->font_size==24);CHECK(v.f.data()->font_family=="Alpha");
    }

    TEST_CASE("CRI013 ThemeEditorPanel: size from unrelated family is rejected") {
        ThemeFonts v;CHECK_FALSE(v.f.panel.choose_font_size_for_testing(20));CHECK(v.f.data()->font_size==18);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: changing font choices waits for explicit reload request") {
        ThemeFonts v;REQUIRE(v.f.panel.choose_font_family_for_testing("Beta"));REQUIRE(v.f.panel.choose_font_size_for_testing(20));CHECK(v.f.c.gui.theme_reloads==0);v.f.panel.apply_font_changes_for_testing();CHECK(v.f.c.gui.theme_reloads==1);CHECK(v.f.data()->font_size==20);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: locked panel rejects font changes and reload") {
        ThemeFonts v;v.f.panel.lock();CHECK_FALSE(v.f.panel.choose_font_family_for_testing("Beta"));CHECK_FALSE(v.f.panel.choose_font_size_for_testing(24));v.f.panel.apply_font_changes_for_testing();CHECK(v.f.data()->font_size==18);CHECK(v.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI013 ThemeEditorPanel: missing font manager rejects family and size safely") {
        ThemeFonts v;v.f.c.ctx.font_manager=nullptr;CHECK_FALSE(v.f.panel.choose_font_family_for_testing("Beta"));CHECK_FALSE(v.f.panel.choose_font_size_for_testing(24));CHECK(v.f.data()->font_family=="Alpha");
    }

    TEST_CASE("CRI013 ThemeEditorPanel: uninitialized panel font actions are inert") {
        ThemeEditorPanel p;CHECK_FALSE(p.choose_font_family_for_testing("Beta"));CHECK_FALSE(p.choose_font_size_for_testing(24));CHECK_NOTHROW(p.apply_font_changes_for_testing());
    }
}

namespace {
using namespace EditorCri;
struct ThemeLockInteraction {
    GuiFrame gui; ActiveThemeFixture f; ImVec2 target{};
    void frame(){gui.draw([&]{f.panel.draw_lock_button_for_testing();auto a=ImGui::GetItemRectMin(),b=ImGui::GetItemRectMax();target=ImVec2((a.x+b.x)*.5f,(a.y+b.y)*.5f);});}
    void click(bool inside=true){frame();frame();auto& io=ImGui::GetIO();auto p=target;io.AddMousePosEvent(p.x+(inside?0.f:200.f),p.y);frame();io.AddMouseButtonEvent(0,true);frame();io.AddMouseButtonEvent(0,false);frame();}
};
}
TEST_SUITE("EditorCRI014_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI014 ThemeEditorPanel: real lock button toggles theme content lock") {
        ThemeLockInteraction x;x.click();CHECK(x.f.panel.is_locked());CHECK(x.f.panel.get_visibility());
    }

    TEST_CASE("CRI014 ThemeEditorPanel: locked theme can unlock through same button") {
        ThemeLockInteraction x;x.f.panel.lock();x.click();CHECK_FALSE(x.f.panel.is_locked());CHECK(x.f.panel.get_visibility());
    }

    TEST_CASE("CRI014 ThemeEditorPanel: lock button can toggle twice without changing active theme") {
        ThemeLockInteraction x;auto* old=x.f.themes.get_active_theme();x.click();REQUIRE(x.f.panel.is_locked());x.click();CHECK_FALSE(x.f.panel.is_locked());CHECK(x.f.themes.get_active_theme()==old);CHECK(x.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI014 ThemeEditorPanel: outside click does not toggle theme lock") {
        ThemeLockInteraction x;x.click(false);CHECK_FALSE(x.f.panel.is_locked());CHECK(x.f.c.gui.theme_reloads==0);
    }

    TEST_CASE("CRI014 ThemeEditorPanel: missing lock texture does not draw an interactive button") {
        ThemeLockInteraction x;x.f.c.icons.unlocked.texture_id=0;CHECK(x.gui.draw([&]{x.f.panel.draw_lock_button_for_testing();})==0);CHECK_FALSE(x.f.panel.is_locked());
    }

    TEST_CASE("CRI014 ThemeEditorPanel: icon service with no active theme renders balanced fallback window") {
        GuiFrame g;PanelContext c;ImGuiThemeManager themes;ThemeEditorPanel p;c.ctx.theme_manager=&themes;p.initialize(&c.ctx);p.set_visibility(true);g.frame([&]{ImGui::SetNextWindowSize(ImVec2(500,400));p.on_gui_update();});CHECK(ImGui::FindWindowByName(p.get_name().c_str())!=nullptr);CHECK(g.current->DisabledStackSize==0);CHECK(themes.get_active_theme()==nullptr);
    }

    TEST_CASE("CRI014 ThemeEditorPanel: locked active theme with missing data exits balanced layout") {
        GuiFrame g;ActiveThemeFixture f;auto* t=dynamic_cast<ImGuiTheme*>(f.themes.get_active_theme());REQUIRE(t);t->clear_data_for_testing();f.panel.lock();f.draw(g);CHECK(f.panel.is_locked());CHECK(g.current->DisabledStackSize==0);CHECK(t->get_data()==nullptr);
    }

    TEST_CASE("CRI014 ThemeEditorPanel: renamed metadata saves active theme using registry identity") {
        ThemeFiles v;v.f.data()->name="Renamed Theme";v.save();REQUIRE(std::filesystem::exists(v.temp.path()));CHECK(YAML::LoadFile(v.temp.path())["name"].as<std::string>()=="Renamed Theme");CHECK(v.f.themes.get_active_theme()->get_name()=="CRI006");
    }

    TEST_CASE("CRI014 ThemeEditorPanel: editable name matching another theme never saves the wrong object") {
        ThemeFiles v;auto other=std::make_unique<ImGuiTheme>("Other");other->get_data()->font_size=33;REQUIRE(v.f.themes.register_theme(std::move(other)));v.f.data()->name="Other";v.f.data()->font_size=17;v.save();auto n=YAML::LoadFile(v.temp.path());CHECK(n["name"].as<std::string>()=="Other");CHECK(n["Font"]["font_size"].as<float>()==17);CHECK(v.f.themes.get_active_theme()->get_name()=="CRI006");
    }

    TEST_CASE("CRI014 ThemeEditorPanel: saved renamed metadata can be loaded as independent registry entry") {
        ThemeFiles v;v.f.data()->name="Renamed";v.f.data()->font_size=23;v.save();v.load("Renamed");REQUIRE(v.f.themes.get_active_theme());CHECK(v.f.themes.get_active_theme()->get_name()=="Renamed");CHECK(v.f.data()->font_size==23);
    }
}

namespace {
using namespace EditorCri;
struct ThemeInputFixture {
    GuiFrame gui;
    ActiveThemeFixture& f;
    explicit ThemeInputFixture(ActiveThemeFixture& fixture):f(fixture){}
    void frame(){
        gui.frame([&]{
            ImGui::SetNextWindowPos(ImVec2(20,20),ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(900,650),ImGuiCond_Always);
            f.panel.on_gui_update();
        });
    }
    ImGuiWindow* window(){
        auto* w=ImGui::FindWindowByName(f.panel.get_name().c_str());
        REQUIRE(w); return w;
    }
    void show(){frame();frame();ImGui::FocusWindow(window());frame();}
    void section(const char* name){
        window()->StateStorage.SetInt(window()->GetID(name),1);
        frame();frame();
    }
    void activate(ImGuiID id){ImGui::ActivateItemByID(id);frame();}
    void activate(const char* label){activate(window()->GetID(label));}
    void replace(const char* label,const std::string& value){
        replace_id(window()->GetID(label),value);
    }
    void replace_id(ImGuiID id,const std::string& value){
        ImGui::ActivateItemByID(id);
        // Match Enter-key navigation: text fields need input activation,
        // not the default button-style activation.
        gui.current->NavNextActivateFlags=ImGuiActivateFlags_PreferInput;
        frame();
        REQUIRE(gui.current->ActiveId==id);
        auto& io=ImGui::GetIO();
        io.AddKeyEvent(ImGuiMod_Ctrl,true);
        io.AddKeyEvent(ImGuiKey_A,true);frame();
        io.AddKeyEvent(ImGuiKey_A,false);
        io.AddKeyEvent(ImGuiMod_Ctrl,false);frame();
        io.AddInputCharactersUTF8(value.c_str());frame();
        io.AddKeyEvent(ImGuiKey_Enter,true);frame();
        io.AddKeyEvent(ImGuiKey_Enter,false);frame();
    }
    void combo(const char* label,int index,const char* choice){
        const auto seed=window()->GetID("Fonts");
        activate(ImHashStr(label,0,seed));
        frame();
        REQUIRE(gui.current->OpenPopupStack.Size==1);
        auto* popup=gui.current->OpenPopupStack.back().Window;
        REQUIRE(popup);
        activate(ImHashStr(choice,0,popup->GetID(index)));
        CHECK(gui.current->OpenPopupStack.Size==0);
    }
};
}
TEST_SUITE("EditorCRI020_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI020 ThemeEditorPanel: real name input changes metadata but not registry key"){
        ActiveThemeFixture f;
        ThemeInputFixture ui(f);ui.show();
        ui.replace("Name","Edited Theme");
        CHECK(f.data()->name=="Edited Theme");
        CHECK(f.themes.get_active_theme()->get_name()=="CRI006");
        CHECK(f.c.gui.theme_reloads==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: real Save button writes edited theme to isolated path"){
        ThemeFiles files;
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("Name","Saved Through UI");
        ui.replace("File Path",files.temp.path());
        CHECK_FALSE(std::filesystem::exists(files.temp.path()));
        ui.activate("Save Theme");
        REQUIRE(std::filesystem::exists(files.temp.path()));
        CHECK(YAML::LoadFile(files.temp.path())["name"].as<std::string>()=="Saved Through UI");
        CHECK(files.f.themes.get_active_theme()->get_name()=="CRI006");
    }
    TEST_CASE("CRI020 ThemeEditorPanel: real Load button replaces theme and refreshes layout"){
        ThemeFiles files;
        files.write("name: LoadedUI\nFont: {font_family: FixtureFamily, font_size: 23}");
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("Name","LoadedUI");
        ui.replace("File Path",files.temp.path());
        ui.activate("Load Theme");ui.frame();
        REQUIRE(files.f.data());
        CHECK(files.f.themes.get_active_theme()->get_name()=="LoadedUI");
        CHECK(files.f.data()->font_family=="FixtureFamily");
        CHECK(files.f.data()->font_size==23);
        CHECK(files.f.c.gui.theme_reloads==1);
        CHECK(ui.gui.current->DisabledStackSize==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: invalid path through real Load preserves active data"){
        ThemeFiles files;
        auto* original=files.f.themes.get_active_theme();
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("File Path",files.temp.path());
        ui.activate("Load Theme");
        CHECK(files.f.themes.get_active_theme()==original);
        CHECK(files.f.c.gui.theme_reloads==0);
        CHECK(ui.gui.current->DisabledStackSize==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: locked real Save button cannot write"){
        ThemeFiles files;
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("File Path",files.temp.path());
        files.f.panel.lock();
        ui.frame();ui.activate("Save Theme");
        CHECK_FALSE(std::filesystem::exists(files.temp.path()));
        CHECK(ui.gui.current->DisabledStackSize==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: actual family dropdown selects Beta and default size"){
        ThemeFonts fonts;
        ThemeInputFixture ui(fonts.f);ui.show();ui.section("Fonts");
        ui.combo("Font Family",1,"Beta");
        CHECK(fonts.f.data()->font_family=="Beta");
        CHECK(fonts.f.data()->font_size==16);
        CHECK(fonts.f.c.gui.theme_reloads==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: actual size dropdown changes size without changing family"){
        ThemeFonts fonts;
        ThemeInputFixture ui(fonts.f);ui.show();ui.section("Fonts");
        ui.combo("Font Size",2,"24px");
        CHECK(fonts.f.data()->font_family=="Alpha");
        CHECK(fonts.f.data()->font_size==24);
        CHECK(fonts.f.c.gui.theme_reloads==0);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: actual Apply Font Changes requests one reload"){
        ThemeFonts fonts;
        ThemeInputFixture ui(fonts.f);ui.show();ui.section("Fonts");
        ui.activate(ImHashStr("Apply Font Changes",0,ui.window()->GetID("Fonts")));
        CHECK(fonts.f.c.gui.theme_reloads==1);
        CHECK(fonts.f.data()->font_family=="Alpha");
        CHECK(fonts.f.data()->font_size==18);
    }
    TEST_CASE("CRI020 ThemeEditorPanel: locked window without active theme keeps disabled stack balanced"){
        GuiFrame g; PanelContext c; ImGuiThemeManager themes; ThemeEditorPanel p;
        c.ctx.theme_manager=&themes;p.initialize(&c.ctx);p.set_visibility(true);p.lock();
        g.frame([&]{ImGui::SetNextWindowSize(ImVec2(500,400));p.on_gui_update();});
        CHECK(g.current->DisabledStackSize==0);
        CHECK(p.is_locked());
        CHECK(themes.get_active_theme()==nullptr);
    }
}

TEST_SUITE("EditorCRI021_ThemeEditorPanel") {
    using namespace EditorCri;

    TEST_CASE("CRI021 ThemeEditorPanel: locked load activation cannot replace the active theme") {
        ThemeFiles files;
        files.write("name: LoadedUI\nFont: {font_family: FixtureFamily, font_size: 23}");
        auto* original=files.f.themes.get_active_theme();
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("Name","LoadedUI");
        ui.replace("File Path",files.temp.path());
        files.f.panel.lock();
        ui.frame();
        REQUIRE(files.f.panel.is_locked());
        ui.activate("Load Theme");
        CHECK(files.f.themes.get_active_theme()==original);
        CHECK(files.f.c.gui.theme_reloads==0);
        CHECK(ui.gui.current->DisabledStackSize==0);
    }

    TEST_CASE("CRI021 ThemeEditorPanel: save succeeds after unlocking previously blocked widget") {
        ThemeFiles files;
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("File Path",files.temp.path());
        files.f.panel.lock();ui.frame();
        REQUIRE(files.f.panel.is_locked());
        ui.activate("Save Theme");
        REQUIRE_FALSE(std::filesystem::exists(files.temp.path()));
        files.f.panel.unlock();ui.frame();
        REQUIRE_FALSE(files.f.panel.is_locked());
        ui.activate("Save Theme");
        REQUIRE(std::filesystem::exists(files.temp.path()));
        CHECK(YAML::LoadFile(files.temp.path())["name"].as<std::string>()==files.f.data()->name);
        CHECK(ui.gui.current->DisabledStackSize==0);
    }

    TEST_CASE("CRI021 ThemeEditorPanel: load succeeds after unlocking previously blocked widget") {
        ThemeFiles files;
        files.write("name: LoadedUI\nFont: {font_family: FixtureFamily, font_size: 23}");
        auto* original=files.f.themes.get_active_theme();
        ThemeInputFixture ui(files.f);ui.show();
        ui.replace("Name","LoadedUI");
        ui.replace("File Path",files.temp.path());
        files.f.panel.lock();ui.frame();
        ui.activate("Load Theme");
        REQUIRE(files.f.themes.get_active_theme()==original);
        REQUIRE(files.f.c.gui.theme_reloads==0);
        files.f.panel.unlock();ui.frame();
        ui.activate("Load Theme");ui.frame();
        REQUIRE(files.f.data());
        CHECK(files.f.themes.get_active_theme()->get_name()=="LoadedUI");
        CHECK(files.f.data()->font_family=="FixtureFamily");
        CHECK(files.f.data()->font_size==23);
        CHECK(files.f.c.gui.theme_reloads==1);
        CHECK(ui.gui.current->DisabledStackSize==0);
    }
}

TEST_SUITE("EditorCRI022_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI022 ThemeEditorPanel: actual icon Browse button opens registered image dialog") {
        IconDialogFixture f;
        f.f.c.icons.registry["Entity"]=f.f.c.icons.unlocked;
        ThemeInputFixture ui(f.f);ui.show();ui.section("Icon Theme");
        const auto table=ui.window()->GetID("icon_table");
        ui.activate(ImHashStr("Browse##Entity",0,table));
        CHECK(f.f.c.gui.shown_dialog=="SelectIconImage");
        auto* d=f.f.c.gui.get_dialog("SelectIconImage");REQUIRE(d);
        REQUIRE(d->get_callback());
        CHECK(f.f.data()->icon_paths.at("Entity")=="old.png");
        CHECK(f.f.c.gui.theme_reloads==0);
    }
}

TEST_SUITE("EditorCRI024_ThemeEditorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI024 ThemeEditorPanel: actual hex color input updates authored color and ImGui style") {
        ActiveThemeFixture f;
        f.data()->text_color=Color(1.f,1.f,1.f,1.f);
        ThemeInputFixture ui(f);
        ImGui::SetColorEditOptions(ImGuiColorEditFlags_DisplayHex |
                                   ImGuiColorEditFlags_Uint8 |
                                   ImGuiColorEditFlags_InputRGB);
        ui.show();ui.section("Colors");
        const auto colors=ui.window()->GetID("Colors");
        const auto text_color=ImHashStr("Text",0,colors);
        ui.replace_id(ImHashStr("##Text",0,text_color),"#336699CC");
        const auto& authored=f.data()->text_color;
        const auto& applied=ImGui::GetStyle().Colors[ImGuiCol_Text];
        CHECK(authored.r==doctest::Approx(.2));
        CHECK(authored.g==doctest::Approx(.4));
        CHECK(authored.b==doctest::Approx(.6));
        CHECK(authored.a==doctest::Approx(.8));
        CHECK(applied.x==doctest::Approx(authored.r));
        CHECK(applied.y==doctest::Approx(authored.g));
        CHECK(applied.z==doctest::Approx(authored.b));
        CHECK(applied.w==doctest::Approx(authored.a));
        CHECK(f.c.gui.theme_reloads==0);
    }
}
