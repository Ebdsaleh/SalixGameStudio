#include <doctest.h>
#include <Salix/rendering/sdl/SDLRenderer.h>
#include <Salix/window/WindowConfig.h>
#include <SDL.h>
#include <cstdlib>
#include <string>

namespace {
struct Batch7878SDLRendererEnv {
    bool had_video=false,had_render=false;std::string video,render;
    Batch7878SDLRendererEnv(const char* render_driver=nullptr){
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had_video=true;video=v;}
        if(const char* v=SDL_getenv("SDL_RENDER_DRIVER")){had_render=true;render=v;}
        SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);
        if(render_driver)SDL_setenv("SDL_RENDER_DRIVER",render_driver,1);else _putenv_s("SDL_RENDER_DRIVER","");
    }
    ~Batch7878SDLRendererEnv(){SDL_Quit();if(had_video)SDL_setenv("SDL_VIDEODRIVER",video.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");if(had_render)SDL_setenv("SDL_RENDER_DRIVER",render.c_str(),1);else _putenv_s("SDL_RENDER_DRIVER","");}
};
struct Batch7878SDLRendererLive {
    Batch7878SDLRendererEnv env;
    Salix::SDLRenderer renderer;
    bool ready=false;
    Batch7878SDLRendererLive(){Salix::WindowConfig c;c.title="Batch7878 SDLRenderer";c.width=96;c.height=64;ready=renderer.initialize(c);}
};
}

TEST_SUITE("Salix::rendering::SDLRendererBatch7878") {
    TEST_CASE("batch7878 concrete SDL renderer exposes native SDL renderer accessor") { Batch7878SDLRendererLive f;REQUIRE(f.ready);CHECK(f.renderer.get_sdl_renderer()!=nullptr); }
    TEST_CASE("batch7878 concrete SDL renderer native handle matches SDL accessor") { Batch7878SDLRendererLive f;REQUIRE(f.ready);CHECK(f.renderer.get_native_handle()==static_cast<void*>(f.renderer.get_sdl_renderer())); }
    TEST_CASE("batch7878 concrete SDL renderer owns live window") { Batch7878SDLRendererLive f;REQUIRE(f.ready);REQUIRE(f.renderer.get_window()!=nullptr);CHECK(f.renderer.get_window()->get_native_handle()!=nullptr); }
    TEST_CASE("batch7878 concrete SDL renderer clear color remains configured dark blue") { Batch7878SDLRendererLive f;REQUIRE(f.ready);auto c=f.renderer.get_clear_color();CHECK(c.r>=0.0f);CHECK(c.g>=0.0f);CHECK(c.b>=0.0f); }
    TEST_CASE("batch7878 concrete SDL renderer frame lifecycle remains safe") { Batch7878SDLRendererLive f;REQUIRE(f.ready);CHECK_NOTHROW(f.renderer.begin_frame());CHECK_NOTHROW(f.renderer.end_frame()); }
    TEST_CASE("batch7878 concrete SDL renderer positive logical resize remains safe") { Batch7878SDLRendererLive f;REQUIRE(f.ready);CHECK_NOTHROW(f.renderer.on_window_resize(78,78)); }
    TEST_CASE("batch7878 concrete SDL renderer framebuffer contract remains unsupported") { Batch7878SDLRendererLive f;REQUIRE(f.ready);CHECK(f.renderer.create_framebuffer(78,78)==0u);CHECK(f.renderer.get_framebuffer_texture_id(7878u)==static_cast<ImTextureID>(0)); }
    TEST_CASE("batch7878 concrete SDL renderer fixed pixels per unit ignores setter") { Batch7878SDLRendererLive f;REQUIRE(f.ready);f.renderer.set_pixels_per_unit(78.78f);CHECK(f.renderer.get_pixels_per_unit()==doctest::Approx(1.0f)); }
    TEST_CASE("batch7878 invalid SDL render driver fails after window creation and releases window") { Batch7878SDLRendererEnv env("__batch7878_invalid_driver__");Salix::SDLRenderer renderer;Salix::WindowConfig c;c.title="Batch7878 Invalid Renderer";c.width=64;c.height=48;CHECK_FALSE(renderer.initialize(c));CHECK(renderer.get_window()==nullptr);CHECK(renderer.get_sdl_renderer()==nullptr); }
    TEST_CASE("batch7878 concrete SDL renderer repeated shutdown is idempotent") { Batch7878SDLRendererLive f;REQUIRE(f.ready);f.renderer.shutdown();f.ready=false;CHECK(f.renderer.get_sdl_renderer()==nullptr);CHECK_NOTHROW(f.renderer.shutdown()); }
}
