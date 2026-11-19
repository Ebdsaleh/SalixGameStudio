#include <doctest.h>
#include <Salix/core/Engine.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/rendering/IRenderer.h>
#include <SDL.h>
#include <filesystem>
#include <cstdlib>
#include <string>

namespace {
struct Batch7878EngineGuiCwd {
    std::filesystem::path previous=std::filesystem::current_path();
    Batch7878EngineGuiCwd(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
        while(p.has_parent_path()&&!std::filesystem::exists(p/"Assets"/"Shaders"))p=p.parent_path();
        if(std::filesystem::exists(p/"Assets"/"Shaders"))std::filesystem::current_path(p);
    }
    ~Batch7878EngineGuiCwd(){std::error_code ec;std::filesystem::current_path(previous,ec);}
};
struct Batch7878EngineGuiVideo {
    bool had=false;std::string old;
    Batch7878EngineGuiVideo(){if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);_putenv_s("SDL_RENDER_DRIVER","");}
    ~Batch7878EngineGuiVideo(){SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");}
};
struct Batch7878GuiEngine {
    Batch7878EngineGuiCwd cwd;
    Batch7878EngineGuiVideo video;
    Salix::Engine engine;
    bool ready=false;
    Batch7878GuiEngine(Salix::RendererType renderer, Salix::AppStateType state=Salix::AppStateType::Launch, Salix::TimerType timer=Salix::TimerType::SDL){
        Salix::ApplicationConfig c;c.renderer_type=renderer;c.gui_type=Salix::GuiType::ImGui;c.initial_state=state;c.timer_type=timer;
        c.window_config.title="Batch7878 Engine GUI";c.window_config.width=128;c.window_config.height=96;
        ready=engine.initialize(c);
    }
    ~Batch7878GuiEngine(){if(ready)engine.shutdown();}
    void quit_and_run(){SDL_Event e{};e.type=SDL_QUIT;REQUIRE(SDL_PushEvent(&e)==1);engine.run();}
};
}

TEST_SUITE("Salix::core::EngineGuiBatch7878") {
    TEST_CASE("batch7878 SDL ImGui engine initializes launch") { Batch7878GuiEngine f(Salix::RendererType::SDL); REQUIRE(f.ready); CHECK(f.engine.get_mode()==Salix::EngineMode::Launch); CHECK(f.engine.get_renderer()!=nullptr); }
    TEST_CASE("batch7878 OpenGL ImGui engine initializes launch") { Batch7878GuiEngine f(Salix::RendererType::OpenGL); REQUIRE(f.ready); CHECK(f.engine.get_mode()==Salix::EngineMode::Launch); CHECK(f.engine.get_renderer()->as_opengl_renderer()!=nullptr); }
    TEST_CASE("batch7878 SDL ImGui options state initializes") { Batch7878GuiEngine f(Salix::RendererType::SDL,Salix::AppStateType::Options); REQUIRE(f.ready); CHECK(f.engine.get_mode()==Salix::EngineMode::Options); }
    TEST_CASE("batch7878 OpenGL ImGui options state initializes") { Batch7878GuiEngine f(Salix::RendererType::OpenGL,Salix::AppStateType::Options); REQUIRE(f.ready); CHECK(f.engine.get_mode()==Salix::EngineMode::Options); }
    TEST_CASE("batch7878 SDL ImGui run processes quit through GUI event path") { Batch7878GuiEngine f(Salix::RendererType::SDL); REQUIRE(f.ready); f.quit_and_run(); CHECK_FALSE(f.engine.is_running()); }
    TEST_CASE("batch7878 OpenGL ImGui run processes quit through GUI event path") { Batch7878GuiEngine f(Salix::RendererType::OpenGL); REQUIRE(f.ready); f.quit_and_run(); CHECK_FALSE(f.engine.is_running()); }
    TEST_CASE("batch7878 SDL ImGui engine switches options and back to launch") { Batch7878GuiEngine f(Salix::RendererType::SDL); REQUIRE(f.ready); f.engine.switch_state(Salix::AppStateType::Options); CHECK(f.engine.get_mode()==Salix::EngineMode::Options); f.engine.switch_state(Salix::AppStateType::Launch); CHECK(f.engine.get_mode()==Salix::EngineMode::Launch); }
    TEST_CASE("batch7878 OpenGL ImGui engine switches to editor") { Batch7878GuiEngine f(Salix::RendererType::OpenGL); REQUIRE(f.ready); f.engine.switch_state(Salix::AppStateType::Editor); CHECK(f.engine.get_mode()==Salix::EngineMode::Editor); }
    TEST_CASE("batch7878 SDL ImGui engine switches to game input path") { Batch7878GuiEngine f(Salix::RendererType::SDL); REQUIRE(f.ready); auto* gui=f.engine.get_input_manager(); f.engine.switch_state(Salix::AppStateType::Game); CHECK(f.engine.get_mode()==Salix::EngineMode::Game); CHECK(f.engine.get_input_manager()!=nullptr); CHECK(f.engine.get_input_manager()!=gui); }
    TEST_CASE("batch7878 OpenGL ImGui chrono engine survives one quit frame") { Batch7878GuiEngine f(Salix::RendererType::OpenGL,Salix::AppStateType::Launch,Salix::TimerType::Chrono); REQUIRE(f.ready); f.quit_and_run(); CHECK_FALSE(f.engine.is_running()); }
}
