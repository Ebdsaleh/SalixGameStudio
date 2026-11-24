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
