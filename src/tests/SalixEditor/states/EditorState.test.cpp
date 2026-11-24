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
