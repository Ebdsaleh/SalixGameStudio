#include <doctest.h>
#include <Salix/core/Engine.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/rendering/ITexture.h>
#include <SDL.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <memory>
#include <atomic>

namespace {
struct Batch7778SDLTextureVideo {
    bool had=false;std::string old;
    Batch7778SDLTextureVideo(){if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);_putenv_s("SDL_RENDER_DRIVER","");}
    ~Batch7778SDLTextureVideo(){SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");}
};
struct Batch7778SDLTextureEngine {
    Batch7778SDLTextureVideo video;Salix::Engine engine;bool ready=false;
    Batch7778SDLTextureEngine(){Salix::ApplicationConfig c;c.renderer_type=Salix::RendererType::SDL;c.gui_type=Salix::GuiType::None;c.initial_state=Salix::AppStateType::Launch;c.window_config.title="Batch7778 SDLTexture";c.window_config.width=96;c.window_config.height=64;ready=engine.initialize(c);}
    ~Batch7778SDLTextureEngine(){engine.shutdown();}
    Salix::IRenderer* renderer(){return engine.get_renderer();}
};
std::filesystem::path batch7778_texture_bmp(int w,int h){
    static std::atomic<unsigned> seq{0};auto path=std::filesystem::temp_directory_path()/("batch7778_sdltexture_"+std::to_string(++seq)+".bmp");
    SDL_Surface* s=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);if(!s)return {};
    SDL_FillRect(s,nullptr,SDL_MapRGBA(s->format,160,80,40,255));if(SDL_SaveBMP(s,path.string().c_str())!=0)path.clear();SDL_FreeSurface(s);return path;
}
struct Batch7778SDLTextureFile{std::filesystem::path path;~Batch7778SDLTextureFile(){std::error_code ec;if(!path.empty())std::filesystem::remove(path,ec);}};
}

TEST_SUITE("Salix::rendering::SDLTextureBatch7778") {
    TEST_CASE("batch7778 SDL texture constructor path records width") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(3,2)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK(t->get_width()==3); }
    TEST_CASE("batch7778 SDL texture constructor path records height") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(2,5)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK(t->get_height()==5); }
    TEST_CASE("batch7778 SDL texture exposes nonzero ImGui id") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(2,2)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK(t->get_imgui_texture_id()!=static_cast<ImTextureID>(0)); }
    TEST_CASE("batch7778 SDL texture draws through renderer translation layer") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(4,3)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK_NOTHROW(e.renderer()->draw_texture(t.get(),Salix::Rect{2,3,16,12})); }
    TEST_CASE("batch7778 SDL texture zero destination extent remains safe") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(2,2)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK_NOTHROW(e.renderer()->draw_texture(t.get(),Salix::Rect{0,0,0,0})); }
    TEST_CASE("batch7778 SDL texture negative destination origin remains safe") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(2,2)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);CHECK_NOTHROW(e.renderer()->draw_texture(t.get(),Salix::Rect{-2,-3,4,4})); }
    TEST_CASE("batch7778 two SDL textures keep independent dimensions") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile a{batch7778_texture_bmp(2,3)},b{batch7778_texture_bmp(5,1)};std::unique_ptr<Salix::ITexture> ta(e.renderer()->load_texture(a.path.string().c_str())),tb(e.renderer()->load_texture(b.path.string().c_str()));REQUIRE(ta!=nullptr);REQUIRE(tb!=nullptr);CHECK(ta->get_width()==2);CHECK(tb->get_width()==5); }
    TEST_CASE("batch7778 two SDL textures keep distinct native ids") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile a{batch7778_texture_bmp(2,2)},b{batch7778_texture_bmp(2,2)};std::unique_ptr<Salix::ITexture> ta(e.renderer()->load_texture(a.path.string().c_str())),tb(e.renderer()->load_texture(b.path.string().c_str()));REQUIRE(ta!=nullptr);REQUIRE(tb!=nullptr);CHECK(ta->get_imgui_texture_id()!=tb->get_imgui_texture_id()); }
    TEST_CASE("batch7778 SDL texture remains valid after logical renderer resize") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(6,4)};std::unique_ptr<Salix::ITexture> t(e.renderer()->load_texture(f.path.string().c_str()));REQUIRE(t!=nullptr);e.renderer()->on_window_resize(80,60);CHECK(t->get_width()==6);CHECK(t->get_height()==4); }
    TEST_CASE("batch7778 SDL texture virtual destructor releases native texture before engine teardown") { Batch7778SDLTextureEngine e;REQUIRE(e.ready);Batch7778SDLTextureFile f{batch7778_texture_bmp(1,1)};auto* t=e.renderer()->load_texture(f.path.string().c_str());REQUIRE(t!=nullptr);CHECK_NOTHROW(delete t); }
}
