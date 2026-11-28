// Tests for Editor/states/EditorState.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/states/EditorState.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct EngineProbe:EngineInterface{
    EngineMode mode=EngineMode::None;bool running=false;float scale=1;
    void switch_state(AppStateType)override{}
    void set_mode(EngineMode m)override{mode=m;}
    EngineMode get_mode()const override{return mode;}
    bool is_running()const override{return running;}
    const bool is_running(bool keep)override{running=keep;return running;}
    float get_time_scale()const override{return scale;}
    void set_time_scale(float v)override{scale=v;}
};
struct StateFixture{TempRealm temp;ScopedWorkingDir cwd;EngineProbe engine;GuiProbe gui;RenderProbe renderer;AssetManager assets;EventManager events;ProjectManager projects;ImGuiThemeManager themes;ImGuiFontManager fonts;InitContext init{};EditorState state;
    StateFixture():cwd(temp){init.engine=&engine;init.gui=&gui;init.renderer=&renderer;init.asset_manager=&assets;init.event_manager=&events;init.project_manager=&projects;init.theme_manager=&themes;init.font_manager=&fonts;}
    void expect(const char* field){state.on_enter(init);auto log=read_text("editor_init.log");CHECK(log.find(std::string("[FATAL] engine_context.")+field+" is NULL!")!=std::string::npos);CHECK(log.find("Editor systems created.")==std::string::npos);CHECK(renderer.creates==0);state.on_exit();}
};
}
TEST_SUITE("EditorCRI002_EditorState") {
    TEST_CASE("CRI002 EditorState: entry rejects missing engine before any allocation") {
        StateFixture f;f.init.engine=nullptr;f.expect("engine");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing GUI") {
        StateFixture f;f.init.gui=nullptr;f.expect("gui");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing renderer") {
        StateFixture f;f.init.renderer=nullptr;f.expect("renderer");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing asset manager") {
        StateFixture f;f.init.asset_manager=nullptr;f.expect("asset_manager");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing event manager") {
        StateFixture f;f.init.event_manager=nullptr;f.expect("event_manager");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing project manager") {
        StateFixture f;f.init.project_manager=nullptr;f.expect("project_manager");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing theme manager") {
        StateFixture f;f.init.theme_manager=nullptr;f.expect("theme_manager");
    }

    TEST_CASE("CRI002 EditorState: entry rejects missing font manager") {
        StateFixture f;f.init.font_manager=nullptr;f.expect("font_manager");
    }

    TEST_CASE("CRI002 EditorState: entry fails safely when log path is a directory") {
        StateFixture f;REQUIRE(std::filesystem::create_directory("editor_init.log"));f.state.on_enter(f.init);CHECK(std::filesystem::is_directory("editor_init.log"));CHECK(f.renderer.creates==0);f.state.on_exit();
    }

    TEST_CASE("CRI002 EditorState: failed entry can exit and accept another rejected request") {
        StateFixture f;f.init.engine=nullptr;f.expect("engine");f.init.engine=&f.engine;f.init.gui=nullptr;f.expect("gui");f.state.render(nullptr);CHECK(f.renderer.clears==0);
    }
}

#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/ecs/Entity.h>

TEST_SUITE("EditorCRI006_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI006 EditorState: entry rejects missing window before creating panels") {
        StateFixture f; f.expect("window");
    }

    TEST_CASE("CRI006 EditorState: unentered update does not require ImGui") {
        EditorState s; s.update(.016f); s.on_exit();
    }

    TEST_CASE("CRI006 EditorState: unentered render does not invoke renderer") {
        EditorState s; RenderProbe renderer; s.render(&renderer); CHECK(renderer.clears==0); CHECK(renderer.begins==0);
    }

    TEST_CASE("CRI006 EditorState: unentered entity purge event is harmless") {
        EditorState s; Entity entity; BeforeEntityPurgedEvent e(&entity); s.on_event(e); CHECK_FALSE(entity.is_purged());
    }

    TEST_CASE("CRI006 EditorState: unentered element purge event is harmless") {
        EditorState s; BeforeElementPurgedEvent e(id(2),id(1)); s.on_event(e); CHECK_FALSE(e.handled);
    }

    TEST_CASE("CRI006 EditorState: failed engine entry cannot execute frame update") {
        StateFixture f; f.init.engine=nullptr; f.expect("engine"); f.state.update(.1f); CHECK(f.renderer.creates==0); CHECK(f.events.is_queue_empty());
    }

    TEST_CASE("CRI006 EditorState: failed renderer entry cannot issue render calls") {
        StateFixture f; f.init.renderer=nullptr; f.expect("renderer"); f.state.render(&f.renderer); CHECK(f.renderer.clears==0); CHECK(f.renderer.begins==0);
    }

    TEST_CASE("CRI006 EditorState: failed GUI entry ignores purge notification") {
        StateFixture f; f.init.gui=nullptr; f.expect("gui"); BeforeElementPurgedEvent e(id(2),id(1)); f.state.on_event(e); CHECK_FALSE(e.handled); CHECK(f.events.is_queue_empty());
    }

    TEST_CASE("CRI006 EditorState: failed font entry does not change engine mode") {
        StateFixture f; f.init.font_manager=nullptr; auto mode=f.engine.get_mode(); f.expect("font_manager"); f.state.update(.016f); CHECK(f.engine.get_mode()==mode);
    }

    TEST_CASE("CRI006 EditorState: log open failure leaves render and event callbacks inert") {
        StateFixture f; REQUIRE(std::filesystem::create_directory("editor_init.log")); f.state.on_enter(f.init); f.state.render(&f.renderer); f.state.update(0); BeforeEntityPurgedEvent e(nullptr); f.state.on_event(e); CHECK(f.renderer.clears==0); CHECK(f.events.is_queue_empty());
    }
}

#include <Salix/window/IWindow.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/ecs/Camera.h>
#include <sstream>
#include <iostream>
namespace {
using namespace EditorCri;
struct WindowProbe : IWindow {
    int width=0,height=0,resizes=0;
    bool initialize(const WindowConfig&)override{return false;}
    void shutdown()override{}
    void* get_native_handle()const override{return nullptr;}
    void query_dimensions(int& w,int& h)const override{w=width;h=height;}
    int get_width()const override{return width;}
    int get_height()const override{return height;}
    void set_size(int w,int h)override{width=w;height=h;++resizes;}
    float get_dpi_scale()const override{return 1;}
};
struct StartupRenderer : OpenGLRenderer {
    int creates=0,resize_calls=0,width=0,height=0;
    int begins=0,ends=0,clears=0,depth_clears=0;
    OpenGLRenderer* as_opengl_renderer() override {return nullptr;}
    void begin_render_pass(uint32_t) override {++begins;}
    void end_render_pass() override {++ends;}
    void clear() override {++clears;}
    void clear_depth_buffer() override {++depth_clears;}
    void set_viewport(int,int,int,int) override {}
    void draw_line(const glm::vec3&,const glm::vec3&,const Color&) override {}
    ICamera* camera=nullptr;
    uint32_t create_framebuffer(int,int)override{return static_cast<uint32_t>(++creates);}
    void on_window_resize(int w,int h)override{++resize_calls;width=w;height=h;}
    void set_active_camera(ICamera* c)override{camera=c;}
    ICamera* get_active_camera()override{return camera;}
    ImTextureID get_framebuffer_texture_id(uint32_t)override{return ImTextureID(0);}
    void delete_framebuffer(uint32_t)override{}
};
struct StartupFixture {
    WindowProbe window; StartupRenderer renderer; ApplicationConfig app{};
    IconProbe icons;
    StateFixture f;
    StartupFixture() {
        f.gui.icons=&icons; f.init.icon_manager=&icons; f.init.window=&window;
        f.init.app_config=&app; f.init.renderer=&renderer;
        f.init.renderer_type=RendererType::OpenGL;
        app.window_config.width=640; app.window_config.height=480;
        auto root=std::filesystem::current_path();
        f.projects.set_project_to_load((root/"fixture.salix").string());
        f.projects.set_loaded_project_directory(root.string());
        std::filesystem::create_directories(root/"Assets"/"Scenes");
        std::ofstream out(root/"Assets"/"Scenes"/"default_realm.yaml");
        out<<"Entities: [{name: Startup, id: 11}]";
        out.close(); register_types();
    }
    ~StartupFixture(){f.state.on_exit();}
    void enter(){f.state.on_enter(f.init);}
    std::string log(){return read_text("editor_init.log");}
};
} // namespace
TEST_SUITE("EditorCRI007_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI007 EditorState: entry rejects missing application configuration") {
        StartupFixture s; s.f.init.app_config=nullptr; s.enter(); CHECK(s.log().find("engine_context.app_config is NULL!")!=std::string::npos); CHECK(s.renderer.creates==0);
    }

    TEST_CASE("CRI007 EditorState: entry rejects non OpenGL renderer after dependency checks") {
        StartupFixture s; s.f.init.renderer=&s.f.renderer; s.enter(); CHECK(s.log().find("Editor requires an OpenGL renderer")!=std::string::npos); CHECK(s.f.renderer.creates==0); CHECK(s.window.resizes==0);
    }

    TEST_CASE("CRI007 EditorState: successful startup changes engine mode and creates panels") {
        StartupFixture s; s.enter(); CHECK(s.f.engine.get_mode()==EngineMode::Editor); CHECK(s.renderer.creates==2); CHECK(s.log().find("FINISHED successfully")!=std::string::npos);
    }

    TEST_CASE("CRI007 EditorState: startup synchronizes window and renderer dimensions") {
        StartupFixture s; s.enter(); CHECK(s.window.resizes==1); CHECK(s.window.width==640); CHECK(s.window.height==480); CHECK(s.renderer.resize_calls==1); CHECK(s.renderer.width==640); CHECK(s.renderer.height==480);
    }

    TEST_CASE("CRI007 EditorState: startup installs actual editor camera") {
        StartupFixture s; s.enter(); REQUIRE(s.renderer.camera); CHECK(dynamic_cast<EditorCamera*>(s.renderer.camera)!=nullptr); CHECK(s.renderer.camera->get_is_active());
    }

    TEST_CASE("CRI007 EditorState: startup registers all six workflow panels") {
        StartupFixture s; s.enter(); auto log=s.log(); for(const char* name:{"WorldTreePanel registered","ScryingMirrorPanel registered","RealmDesignerPanel registered","RealmPotalPanel registered","ThemeEditorPanel registered","ProjectSettingsPanel registered"})CHECK(log.find(name)!=std::string::npos);
    }

    TEST_CASE("CRI007 EditorState: startup registers theme image dialog through GUI interface") {
        StartupFixture s; s.enter(); CHECK(s.f.gui.get_dialog("SelectIconImage")!=nullptr);
    }

    TEST_CASE("CRI007 EditorState: startup reads authored realm from selected project directory") {
        StartupFixture s; std::ostringstream out; auto* previous=std::cout.rdbuf(out.rdbuf()); try{s.enter();}catch(...){std::cout.rdbuf(previous);throw;} std::cout.rdbuf(previous); CHECK(out.str().find("Vector size: 1")!=std::string::npos); CHECK(out.str().find("Startup")!=std::string::npos);
    }

    TEST_CASE("CRI007 EditorState: purge notification clears renderer camera owned by purged entity") {
        StartupFixture s; s.enter(); Entity e; auto* cam=e.add_element<Camera>(); REQUIRE(cam); s.renderer.set_active_camera(cam); BeforeEntityPurgedEvent event(&e); s.f.state.on_event(event); CHECK(s.renderer.camera==nullptr);
    }

    TEST_CASE("CRI007 EditorState: purge notification leaves unrelated renderer camera intact") {
        StartupFixture s; s.enter(); Entity a,b; auto* cam=a.add_element<Camera>(); REQUIRE(cam); s.renderer.set_active_camera(cam); BeforeEntityPurgedEvent event(&b); s.f.state.on_event(event); CHECK(s.renderer.camera==cam); s.renderer.set_active_camera(nullptr);
    }
}

#include <Editor/panels/PanelManager.h>
#include <Editor/panels/IPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
namespace {
using namespace EditorCri;
struct FrameFixture {
    GuiFrame gui; StartupFixture startup;
    FrameFixture(){ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable; startup.enter(); REQUIRE(context()); context()->grid_settings.snap_enabled=false;}
    EditorContext* context(){return startup.f.state.get_context_for_testing();}
    void frame(){gui.frame([&]{startup.f.state.update(.016f);});}
};
}
TEST_SUITE("EditorCRI009_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI009 EditorState: initial frame rebuilds authored preview exactly once") {
        FrameFixture f; REQUIRE(f.context()->preview_realm); CHECK(f.context()->preview_realm->get_entity_by_id(id(11))==nullptr); f.frame(); auto* live=f.context()->preview_realm->get_entity_by_id(id(11)); REQUIRE(live); CHECK(live->get_name()=="Startup"); CHECK_FALSE(f.context()->realm_is_dirty); f.frame(); CHECK(f.context()->preview_realm->get_entity_by_id(id(11))==live);
    }

    TEST_CASE("CRI009 EditorState: frame drains deferred commands in order") {
        FrameFixture f; std::vector<int> calls; f.context()->add_deferred_command([&]{calls.push_back(1);}); f.context()->add_deferred_command([&]{calls.push_back(2);}); f.frame(); REQUIRE(calls.size()==2); CHECK(calls[0]==1); CHECK(calls[1]==2); CHECK(f.context()->deferred_commands.empty());
    }

    TEST_CASE("CRI009 EditorState: second frame processes queued entity selection") {
        FrameFixture f; f.frame(); EntitySelectedEvent e(id(11)); f.startup.f.events.dispatch(e); f.frame(); CHECK(f.context()->selected_entity_id==id(11)); CHECK(f.startup.f.events.is_queue_empty());
    }

    TEST_CASE("CRI009 EditorState: render invokes two balanced passes then GUI and depth clear") {
        FrameFixture f; f.frame(); f.startup.f.state.render(&f.startup.renderer); CHECK(f.startup.renderer.begins==2); CHECK(f.startup.renderer.ends==2); CHECK(f.startup.renderer.clears==3); CHECK(f.startup.renderer.depth_clears==1); CHECK(f.startup.f.gui.renders==1); CHECK(f.startup.f.gui.platforms==1);
    }

    TEST_CASE("CRI009 EditorState: null render argument does not invoke renderer or GUI") {
        FrameFixture f; f.startup.f.state.render(nullptr); CHECK(f.startup.renderer.begins==0); CHECK(f.startup.renderer.clears==0); CHECK(f.startup.f.gui.renders==0);
    }

    TEST_CASE("CRI009 EditorState: each frame displays dialogs without native rendering") {
        FrameFixture f; f.frame(); f.frame(); CHECK(f.startup.f.gui.dialog_frames==2); CHECK(f.gui.current->StyleVarStack.Size==0); CHECK(f.gui.current->DisabledStackSize==0);
    }

    TEST_CASE("CRI009 EditorState: dirty preview rebuild replaces authored entity data") {
        FrameFixture f; f.frame(); auto* a=f.context()->editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->name="Edited"; f.context()->realm_is_dirty=true; f.frame(); REQUIRE(f.context()->preview_realm->get_entity_by_id(id(11))); CHECK(f.context()->preview_realm->get_entity_by_id(id(11))->get_name()=="Edited"); CHECK_FALSE(f.context()->realm_is_dirty);
    }

    TEST_CASE("CRI009 EditorState: selected element purge clears pointer and stable ID") {
        FrameFixture f; f.frame(); auto* live=f.context()->preview_realm->get_entity_by_id(id(11)); REQUIRE(live); f.context()->selected_element=live->get_transform(); f.context()->selected_element_id=live->get_transform()->get_id(); BeforeElementPurgedEvent e(f.context()->selected_element_id,id(11)); f.startup.f.state.on_event(e); CHECK(f.context()->selected_element==nullptr); CHECK_FALSE(f.context()->selected_element_id.is_valid());
    }

    TEST_CASE("CRI009 EditorState: selected entity purge clears borrowed live pointer") {
        FrameFixture f; f.frame(); auto* live=f.context()->preview_realm->get_entity_by_id(id(11)); REQUIRE(live); f.context()->selected_entity=live; BeforeEntityPurgedEvent e(live); f.startup.f.state.on_event(e); CHECK(f.context()->selected_entity==nullptr);
    }

    TEST_CASE("CRI009 EditorState: game mode frame updates preview without reloading authored model") {
        FrameFixture f; f.frame(); f.startup.f.init.engine_mode=EngineMode::Game; auto* live=f.context()->preview_realm->get_entity_by_id(id(11)); REQUIRE(live); f.frame(); CHECK(f.context()->preview_realm->get_entity_by_id(id(11))==live); CHECK_FALSE(f.context()->realm_is_dirty);
    }
}

#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IGui.h>
namespace {
using namespace EditorCri;
void check_utility_dialog(const char* key,const char* title){
    StartupFixture s;s.enter();auto* d=s.f.gui.get_dialog(key);REQUIRE(d);CHECK(d->get_key()==key);CHECK(d->get_title()==title);CHECK(d->get_type()==Salix::DialogType::File);CHECK_FALSE((d->get_config().flags&ImGuiFileDialogFlags_ConfirmOverwrite)!=0);
}
}
TEST_SUITE("EditorCRI011_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI011 EditorState: startup registers image utility dialog through GUI interface") {
        check_utility_dialog("SelectImageFile","Select Image File");
    }

    TEST_CASE("CRI011 EditorState: startup registers audio utility dialog through GUI interface") {
        check_utility_dialog("SelectAudioFile","Select Audio Clip");
    }

    TEST_CASE("CRI011 EditorState: startup registers source utility dialog through GUI interface") {
        check_utility_dialog("SelectSourceFile","Select Source File");
    }

    TEST_CASE("CRI011 EditorState: startup registers generic utility dialog through GUI interface") {
        check_utility_dialog("SelectFile","Select File");
    }

    TEST_CASE("CRI011 EditorState: utility registration logs success without invoking browser") {
        StartupFixture s;s.enter();CHECK(s.log().find("Utility dialogs registered.")!=std::string::npos);CHECK(s.f.gui.shown_dialog.empty());CHECK(s.f.gui.dialog_frames==0);
    }

    TEST_CASE("CRI011 EditorState: existing registered dialog retains callback and identity") {
        StartupFixture s;std::string k="SelectImageFile",title="Existing";auto* d=s.f.gui.create_dialog(k,title,Salix::DialogType::File,false);REQUIRE(d);d->set_filters("Existing filter");s.enter();CHECK(s.f.gui.get_dialog(k)==d);CHECK(d->get_filters()=="Existing filter");
    }

    TEST_CASE("CRI011 EditorState: all registered dialogs have independent storage") {
        StartupFixture s;s.enter();auto* a=s.f.gui.get_dialog("SelectImageFile");auto* b=s.f.gui.get_dialog("SelectAudioFile");REQUIRE(a);REQUIRE(b);CHECK(a!=b);a->set_default_path("images");CHECK(b->get_default_path()==".");
    }

    TEST_CASE("CRI011 EditorState: failed registration is reported without opening native dialog") {
        StartupFixture s;struct Reset{~Reset(){SDLImGui::set_force_dialog_insert_failure_for_testing(false);}} reset;SDLImGui::set_force_dialog_insert_failure_for_testing(true);s.enter();CHECK(s.log().find("Utility dialog registration incomplete.")!=std::string::npos);CHECK(s.f.gui.shown_dialog.empty());CHECK(s.f.gui.get_dialog("SelectFile")==nullptr);
    }

    TEST_CASE("CRI011 EditorState: startup failure does not register utility dialogs") {
        StartupFixture s;s.f.init.window=nullptr;s.enter();CHECK(s.f.gui.get_dialog("SelectFile")==nullptr);CHECK(s.f.gui.get_dialog("SelectImageFile")==nullptr);
    }

    TEST_CASE("CRI011 EditorState: frame updates retain registered dialog identity without opening it") {
        FrameFixture f;auto* d=f.startup.f.gui.get_dialog("SelectSourceFile");REQUIRE(d);f.frame();f.frame();CHECK(f.startup.f.gui.get_dialog("SelectSourceFile")==d);CHECK(f.startup.f.gui.shown_dialog.empty());CHECK(f.startup.f.gui.dialog_frames==2);
    }
}

namespace {
using namespace EditorCri;
void open_workflow_panel(const char* name){
    StartupFixture s;s.enter();auto* panel=s.f.state.get_panel_for_testing(name);REQUIRE(panel);
    panel->set_visibility(false);CHECK_FALSE(panel->get_visibility());
    s.f.state.show_panel_for_testing(name);CHECK(panel->get_visibility());
    CHECK(s.f.state.get_panel_for_testing(name)==panel);
}
}
TEST_SUITE("EditorCRI013_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI013 EditorState: theme menu reveals existing theme editor") {
        open_workflow_panel("Theme Editor Panel");
    }

    TEST_CASE("CRI013 EditorState: settings menu reveals existing project settings") {
        open_workflow_panel("Project Settings Panel");
    }

    TEST_CASE("CRI013 EditorState: view menu reveals existing world tree") {
        open_workflow_panel("World Tree Panel");
    }

    TEST_CASE("CRI013 EditorState: view menu reveals existing inspector") {
        open_workflow_panel("Scrying Mirror Panel");
    }

    TEST_CASE("CRI013 EditorState: view menu reveals existing designer viewport") {
        open_workflow_panel("Realm Designer");
    }

    TEST_CASE("CRI013 EditorState: view menu reveals existing portal viewport") {
        open_workflow_panel("Realm Portal");
    }

    TEST_CASE("CRI013 EditorState: projection menu changes the live editor camera") {
        StartupFixture s;s.enter();auto* ctx=s.f.state.get_context_for_testing();REQUIRE(ctx);REQUIRE(ctx->editor_camera);s.f.state.choose_projection_for_testing(true);CHECK(ctx->editor_camera->get_projection_mode()==ProjectionMode::Orthographic);s.f.state.choose_projection_for_testing(false);CHECK(ctx->editor_camera->get_projection_mode()==ProjectionMode::Perspective);
    }

    TEST_CASE("CRI013 EditorState: exit menu requests engine shutdown without destroying context") {
        StartupFixture s;s.enter();s.f.engine.is_running(true);auto* context=s.f.state.get_context_for_testing();REQUIRE(context);s.f.state.request_exit_for_testing();CHECK_FALSE(s.f.engine.is_running());CHECK(s.f.state.get_context_for_testing()==context);
    }

    TEST_CASE("CRI013 EditorState: unknown panel action leaves existing panel unchanged") {
        StartupFixture s;s.enter();auto* p=s.f.state.get_panel_for_testing("World Tree Panel");REQUIRE(p);p->set_visibility(false);s.f.state.show_panel_for_testing("Missing");CHECK_FALSE(p->get_visibility());CHECK(s.f.state.get_panel_for_testing("Missing")==nullptr);
    }

    TEST_CASE("CRI013 EditorState: menu actions before startup are harmless") {
        EditorState s;CHECK_NOTHROW(s.show_panel_for_testing("Theme Editor Panel"));CHECK_NOTHROW(s.choose_projection_for_testing(true));CHECK_NOTHROW(s.request_exit_for_testing());CHECK(s.get_panel_for_testing("World Tree Panel")==nullptr);
    }
}

#include <Salix/gui/imgui/ImGuiFont.h>
namespace {
using namespace EditorCri;
struct StateMenuFixture {
    GuiFrame gui; StartupFixture startup;
    StateMenuFixture(){startup.enter();startup.f.engine.is_running(true);}
    void frame(const char* open=nullptr){gui.frame([&]{
        ImGui::SetNextWindowPos(ImVec2(20,20),ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(800,600),ImGuiCond_Always);
        ImGui::Begin("CRI014 Menu Host",nullptr,ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoSavedSettings);
        if(open){ImGui::PushID("##MenuBar");ImGui::OpenPopup(open);ImGui::PopID();}
        startup.f.state.draw_main_menu_for_testing();
        ImGui::End();
    });}
    void open(const char* menu){
        frame();frame(menu);frame();frame();
        REQUIRE(gui.current->OpenPopupStack.Size==1);
        auto* w=gui.current->OpenPopupStack[0].Window;REQUIRE(w);
        CHECK((w->Flags&ImGuiWindowFlags_ChildMenu)!=0);
        CHECK(w->DC.CursorMaxPos.y>w->DC.CursorStartPos.y);
    }
};
}
TEST_SUITE("EditorCRI014_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI014 EditorState: new deferred commands execute next frame without invalidating current batch") {
        FrameFixture f;std::vector<int> order;f.context()->add_deferred_command([&]{order.push_back(1);f.context()->add_deferred_command([&]{order.push_back(3);});});f.context()->add_deferred_command([&]{order.push_back(2);});f.frame();CHECK((order==std::vector<int>{1,2}));REQUIRE(f.context()->deferred_commands.size()==1);f.frame();CHECK((order==std::vector<int>{1,2,3}));CHECK(f.context()->deferred_commands.empty());
    }

    TEST_CASE("CRI014 EditorState: large deferred enqueue remains intact for next frame") {
        FrameFixture f;int calls=0;f.context()->add_deferred_command([&]{for(int i=0;i<64;++i)f.context()->add_deferred_command([&]{++calls;});});f.frame();CHECK(calls==0);CHECK(f.context()->deferred_commands.size()==64);f.frame();CHECK(calls==64);CHECK(f.context()->deferred_commands.empty());
    }

    TEST_CASE("CRI014 EditorState: callback clearing next frame queue does not cancel current batch") {
        FrameFixture f;int calls=0;f.context()->add_deferred_command([&]{f.context()->add_deferred_command([&]{calls+=100;});f.context()->clear_deferred_commands();});f.context()->add_deferred_command([&]{++calls;});f.frame();CHECK(calls==1);f.frame();CHECK(calls==1);
    }

    TEST_CASE("CRI014 EditorState: active synthetic font is pushed and popped in actual frame") {
        FrameFixture f;auto font=std::make_unique<ImGuiFont>("","FixtureFont",13.0f);font->set_imgui_font_ptr(ImGui::GetIO().Fonts->Fonts[0]);REQUIRE(f.startup.f.fonts.register_font(std::move(font)));REQUIRE(f.startup.f.fonts.set_active_font("FixtureFont"));f.frame();f.frame();CHECK(f.gui.current->FontStack.Size==0);CHECK(f.startup.f.gui.dialog_frames==2);
    }

    TEST_CASE("CRI014 EditorState: font metadata without atlas pointer leaves font stack balanced") {
        FrameFixture f;auto font=std::make_unique<ImGuiFont>("","Unloaded",13.0f);REQUIRE(f.startup.f.fonts.register_font(std::move(font)));REQUIRE(f.startup.f.fonts.set_active_font("Unloaded"));f.frame();CHECK(f.gui.current->FontStack.Size==0);CHECK(f.startup.f.gui.dialog_frames==1);
    }

    TEST_CASE("CRI014 EditorState: File menu renders actual popup without stopping engine") {
        StateMenuFixture m;m.open("File");CHECK(m.startup.f.engine.is_running());CHECK(m.startup.f.state.get_context_for_testing()!=nullptr);
    }

    TEST_CASE("CRI014 EditorState: Edit menu renders actual popup without showing hidden settings") {
        StateMenuFixture m;auto* p=m.startup.f.state.get_panel_for_testing("Project Settings Panel");REQUIRE(p);p->set_visibility(false);m.open("Edit");CHECK_FALSE(p->get_visibility());
    }

    TEST_CASE("CRI014 EditorState: View menu renders actual popup without changing camera projection") {
        StateMenuFixture m;auto* c=m.startup.f.state.get_context_for_testing()->editor_camera;REQUIRE(c);auto original=c->get_projection_mode();m.open("View");CHECK(c->get_projection_mode()==original);
    }

    TEST_CASE("CRI014 EditorState: purging current main camera clears borrowed Editor pointer") {
        FrameFixture f;f.frame();auto* e=f.context()->preview_realm->create_entity(id(21),"Camera");auto* c=e->add_element<Camera>();REQUIRE(c);f.context()->main_camera=c;BeforeEntityPurgedEvent event(e);f.startup.f.state.on_event(event);CHECK(f.context()->main_camera==nullptr);
    }

    TEST_CASE("CRI014 EditorState: purging unrelated entity retains main camera pointer") {
        FrameFixture f;f.frame();auto* e=f.context()->preview_realm->create_entity(id(21),"Camera");auto* c=e->add_element<Camera>();REQUIRE(c);f.context()->main_camera=c;auto* other=f.context()->preview_realm->get_entity_by_id(id(11));REQUIRE(other);BeforeEntityPurgedEvent event(other);f.startup.f.state.on_event(event);CHECK(f.context()->main_camera==c);
    }
}

namespace {
using namespace EditorCri;
struct StateMenuActionFixture : StateMenuFixture {
    void activate(const char* label) {
        REQUIRE(gui.current->OpenPopupStack.Size > 0);
        auto* popup = gui.current->OpenPopupStack.back().Window;
        REQUIRE(popup);
        // MenuItemEx and BeginMenuEx both use PushID(label) + Selectable("").
        ImGui::ActivateItemByID(ImHashStr("", 0, popup->GetID(label)));
        frame();
    }
    void submenu(const char* label) {
        activate(label);
        frame();
        frame();
        REQUIRE(gui.current->OpenPopupStack.Size == 2);
        REQUIRE(gui.current->OpenPopupStack.back().Window);
        CHECK((gui.current->OpenPopupStack.back().Window->Flags & ImGuiWindowFlags_ChildMenu) != 0);
    }
    void reveal(const char* parent, const char* entry, const char* panel, const char* child = nullptr) {
        auto* p = startup.f.state.get_panel_for_testing(panel);
        REQUIRE(p);
        p->set_visibility(false);
        open(parent);
        if (child) submenu(child);
        CHECK_FALSE(p->get_visibility());
        activate(entry);
        CHECK(p->get_visibility());
        CHECK(startup.f.state.get_panel_for_testing(panel) == p);
    }
};
}
TEST_SUITE("EditorCRI017_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI017 EditorState: actual Edit Theme menu reveals registered theme panel") {
        StateMenuActionFixture f;
        f.reveal("Edit", "Theme", "Theme Editor Panel");
    }

    TEST_CASE("CRI017 EditorState: actual Edit Project Settings menu reveals registered settings panel") {
        StateMenuActionFixture f;
        f.reveal("Edit", "Project Settings", "Project Settings Panel");
    }

    TEST_CASE("CRI017 EditorState: actual File Exit menu stops engine without replacing editor context") {
        StateMenuActionFixture f;
        auto* context = f.startup.f.state.get_context_for_testing();
        REQUIRE(context);
        f.open("File");
        REQUIRE(f.startup.f.engine.is_running());
        f.activate("Exit");
        CHECK_FALSE(f.startup.f.engine.is_running());
        CHECK(f.startup.f.state.get_context_for_testing() == context);
    }

    TEST_CASE("CRI017 EditorState: actual View Panels submenu reveals World Tree") {
        StateMenuActionFixture f;
        f.reveal("View", "World Tree", "World Tree Panel", "Panels");
    }

    TEST_CASE("CRI017 EditorState: actual View Panels submenu reveals Scrying Mirror") {
        StateMenuActionFixture f;
        f.reveal("View", "Scrying Mirror", "Scrying Mirror Panel", "Panels");
    }

    TEST_CASE("CRI017 EditorState: actual View Panels submenu reveals Realm Designer") {
        StateMenuActionFixture f;
        f.reveal("View", "Realm Designer", "Realm Designer", "Panels");
    }

    TEST_CASE("CRI017 EditorState: actual View Panels submenu reveals Realm Portal") {
        StateMenuActionFixture f;
        f.reveal("View", "Realm Portal", "Realm Portal", "Panels");
    }

    TEST_CASE("CRI017 EditorState: actual camera menu selects perspective projection") {
        StateMenuActionFixture f;
        auto* camera = f.startup.f.state.get_context_for_testing()->editor_camera;
        REQUIRE(camera);
        camera->set_projection_mode(ProjectionMode::Orthographic);
        f.open("View");
        f.submenu("Editor Camera");
        CHECK(camera->get_projection_mode() == ProjectionMode::Orthographic);
        f.activate("Perspective");
        CHECK(camera->get_projection_mode() == ProjectionMode::Perspective);
    }

    TEST_CASE("CRI017 EditorState: actual camera menu selects orthographic projection") {
        StateMenuActionFixture f;
        auto* camera = f.startup.f.state.get_context_for_testing()->editor_camera;
        REQUIRE(camera);
        camera->set_projection_mode(ProjectionMode::Perspective);
        f.open("View");
        f.submenu("Editor Camera");
        CHECK(camera->get_projection_mode() == ProjectionMode::Perspective);
        f.activate("Orthographic");
        CHECK(camera->get_projection_mode() == ProjectionMode::Orthographic);
    }
}

TEST_SUITE("EditorCRI018_EditorState") {
    TEST_CASE("CRI018 EditorState: actual camera debug menu renders diagnostic window without changing projection") {
        StateMenuActionFixture f;
        auto* context = f.startup.f.state.get_context_for_testing();
        REQUIRE(context);
        REQUIRE(context->editor_camera);
        const auto projection = context->editor_camera->get_projection_mode();
        f.open("View");
        f.submenu("Editor Camera");
        f.activate("Debug Window");
        auto* window = ImGui::FindWindowByName("Camera Debug Info");
        REQUIRE(window);
        CHECK((window->WasActive || window->Active));
        CHECK(context->editor_camera->get_projection_mode() == projection);
        CHECK(f.startup.f.engine.is_running());
    }
}

TEST_SUITE("EditorCRI023_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI023 EditorState: running engine initializes gizmo frame and preserves preview across frames") {
        FrameFixture f;
        f.startup.f.engine.running=true;
        f.frame();
        auto* live=f.context()->preview_realm->get_entity_by_id(id(11));
        REQUIRE(live);
        auto* gizmo=ImGui::FindWindowByName("gizmo");
        REQUIRE(gizmo);
        CHECK(gizmo->Active);
        CHECK((gizmo->Flags & ImGuiWindowFlags_NoInputs)==ImGuiWindowFlags_NoInputs);
        f.frame();
        CHECK(f.context()->preview_realm->get_entity_by_id(id(11))==live);
        CHECK(f.startup.f.gui.dialog_frames==2);
        CHECK(f.gui.current->StyleVarStack.Size==0);
        CHECK(f.startup.f.engine.is_running());
    }
    TEST_CASE("CRI023 EditorState: stopped engine does not begin gizmo frame after running frame") {
        FrameFixture f;
        f.startup.f.engine.running=true;f.frame();
        auto* gizmo=ImGui::FindWindowByName("gizmo");
        REQUIRE(gizmo);
        REQUIRE(gizmo->Active);
        f.startup.f.engine.running=false;f.frame();
        CHECK_FALSE(gizmo->Active);
        CHECK(f.context()->preview_realm->get_entity_by_id(id(11))!=nullptr);
        CHECK(f.startup.f.gui.dialog_frames==2);
    }
}

namespace {
// Keep the existing headless render-pass probe, but expose the actual
// OpenGL backend so its diagnostic cube dependency guards are exercised.
// The real OpenGLRenderer is never initialized with GPU resources.
struct DiagnosticRouteRenderer : StartupRenderer {
    bool bind_backend_camera=true;
    int boxes=0;
    OpenGLRenderer* as_opengl_renderer() override {return this;}
    void set_active_camera(ICamera* c) override {
        StartupRenderer::set_active_camera(c);
        OpenGLRenderer::set_active_camera(bind_backend_camera?c:nullptr);
    }
    void draw_wire_box(const glm::mat4&,const Color&) override {++boxes;}
};
struct DiagnosticErrorCapture {
    std::ostringstream output;
    std::streambuf* previous;
    DiagnosticErrorCapture():previous(std::cerr.rdbuf(output.rdbuf())){}
    ~DiagnosticErrorCapture(){std::cerr.rdbuf(previous);}
};
}
TEST_SUITE("EditorCRI026_EditorState") {
    using namespace EditorCri;
    TEST_CASE("CRI026 EditorState: diagnostic cube missing dependencies does not interrupt headless render passes") {
        GuiFrame gui;
        DiagnosticRouteRenderer renderer;
        StartupFixture s;
        s.f.init.renderer=&renderer;
        ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;
        SUBCASE("backend camera missing") {renderer.bind_backend_camera=false;}
        SUBCASE("backend camera present but shader missing") {renderer.bind_backend_camera=true;}
        s.enter();
        REQUIRE(s.f.state.get_context_for_testing());
        s.f.state.get_context_for_testing()->grid_settings.snap_enabled=false;
        gui.frame([&]{s.f.state.update(.016f);});
        DiagnosticErrorCapture errors;
        s.f.state.render(&renderer);
        const auto log=errors.output.str();
        if(renderer.bind_backend_camera)
            CHECK(log.find("simple_3d_shader is null in draw_cube()")!=std::string::npos);
        else
            CHECK(log.find("Attempted to draw_cube with no active camera set")!=std::string::npos);
        CHECK(renderer.begins==2);
        CHECK(renderer.ends==2);
        CHECK(renderer.clears==3);
        CHECK(renderer.depth_clears==1);
        CHECK(s.f.gui.renders==1);
        CHECK(s.f.gui.platforms==1);
    }
}
