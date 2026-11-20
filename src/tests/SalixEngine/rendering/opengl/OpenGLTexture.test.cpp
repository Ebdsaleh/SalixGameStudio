#include <doctest.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/ecs/Transform.h>
#include <Tests/SalixEngine/mocking/rendering/MockICamera.h>
#include <SDL.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <memory>

namespace {
struct Batch7778OpenGLTextureFixture {
    std::filesystem::path previous=std::filesystem::current_path();
    std::filesystem::path root;
    bool had=false;std::string old;bool sdl_ready=false;
    Salix::OpenGLRenderer renderer;MockICamera camera;bool ready=false;
    Batch7778OpenGLTextureFixture(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();while(p.has_parent_path()&&!std::filesystem::exists(p/"Assets"/"Shaders"))p=p.parent_path();root=p;
        if(std::filesystem::exists(root/"Assets"/"Shaders"))std::filesystem::current_path(root);
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);sdl_ready=SDL_Init(SDL_INIT_VIDEO)==0;if(!sdl_ready)return;
        Salix::WindowConfig wc;wc.title="Batch7778 OpenGLTexture";wc.width=96;wc.height=64;ready=renderer.initialize(wc);if(ready)renderer.set_active_camera(&camera);
    }
    ~Batch7778OpenGLTextureFixture(){renderer.shutdown();SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");std::error_code ec;std::filesystem::current_path(previous,ec);}
    std::string image() const {return (root/"src"/"Salix"/"resources"/"templates"/"default"/"Assets"/"Images"/"sprites"/"test.png").string();}
};
}

TEST_SUITE("Salix::rendering::OpenGLTextureBatch7778") {
    TEST_CASE("batch7778 OpenGL texture loads through exported texture interface") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);CHECK(t->get_width()>0);CHECK(t->get_height()>0);f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture exposes nonzero ImGui texture id") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);CHECK(t->get_imgui_texture_id()!=static_cast<ImTextureID>(0));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 two OpenGL texture loads produce independent GPU ids") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* a=f.renderer.load_texture(f.image().c_str());Salix::ITexture* b=f.renderer.load_texture(f.image().c_str());REQUIRE(a!=nullptr);REQUIRE(b!=nullptr);CHECK(a->get_imgui_texture_id()!=b->get_imgui_texture_id());f.renderer.purge_texture(a);f.renderer.purge_texture(b); }
    TEST_CASE("batch7778 OpenGL texture draws through rectangle convenience path") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);CHECK_NOTHROW(f.renderer.draw_texture(t,Salix::Rect{0,0,32,32}));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture sprite draws without flip") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);Salix::Transform tr;CHECK_NOTHROW(f.renderer.draw_sprite(t,&tr,Salix::Color{},Salix::SpriteFlip::None));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture sprite draws with horizontal flip") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);Salix::Transform tr;CHECK_NOTHROW(f.renderer.draw_sprite(t,&tr,Salix::Color{},Salix::SpriteFlip::Horizontal));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture sprite draws with vertical flip") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);Salix::Transform tr;CHECK_NOTHROW(f.renderer.draw_sprite(t,&tr,Salix::Color{},Salix::SpriteFlip::Vertical));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture sprite draws with both flips") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);Salix::Transform tr;CHECK_NOTHROW(f.renderer.draw_sprite(t,&tr,Salix::Color{},Salix::SpriteFlip::Both));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture matrix sprite overload renders") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);Salix::ITexture* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);CHECK_NOTHROW(f.renderer.draw_sprite(t,glm::mat4(1.0f),Salix::Color{}));f.renderer.purge_texture(t); }
    TEST_CASE("batch7778 OpenGL texture destruction occurs through renderer purge") { Batch7778OpenGLTextureFixture f;REQUIRE(f.ready);auto* t=f.renderer.load_texture(f.image().c_str());REQUIRE(t!=nullptr);CHECK_NOTHROW(f.renderer.purge_texture(t)); }
}

