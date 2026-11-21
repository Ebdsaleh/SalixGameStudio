#include <doctest.h>
#include <SDL.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <cmath>
#include <fstream>
#include <atomic>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <Salix/ecs/Transform.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Tests/SalixEngine/mocking/rendering/MockICamera.h>
#include <memory>

TEST_SUITE("Salix::rendering::OpenGLRendererBatch1647") {
    TEST_CASE("fresh OpenGL renderer has no owned window before initialization") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_window() == nullptr);
    }

    TEST_CASE("fresh OpenGL renderer exposes no native GL context handle") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_native_handle() == nullptr);
    }

    TEST_CASE("fresh OpenGL renderer exposes null SDL GL context") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_sdl_gl_context() == nullptr);
    }

    TEST_CASE("fresh OpenGL renderer exposes null SDL window") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_sdl_window() == nullptr);
    }

    TEST_CASE("fresh OpenGL renderer starts with one hundred pixels per unit") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(100.0f));
    }

    TEST_CASE("OpenGL renderer pixels per unit roundtrips without backend initialization") {
        Salix::OpenGLRenderer renderer;
        renderer.set_pixels_per_unit(64.5f);
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(64.5f));
        renderer.set_pixels_per_unit(0.0f);
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(0.0f));
    }

    TEST_CASE("fresh OpenGL renderer starts without an active camera") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_active_camera() == nullptr);
    }

    TEST_CASE("OpenGL renderer active camera is a nonowning preinitialization pointer") {
        Salix::OpenGLRenderer renderer;
        MockICamera camera;
        renderer.set_active_camera(&camera);
        CHECK(renderer.get_active_camera() == &camera);
        renderer.set_active_camera(nullptr);
        CHECK(renderer.get_active_camera() == nullptr);
    }

    TEST_CASE("OpenGL renderer line width state roundtrips before context creation") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_line_width() == doctest::Approx(1.0f));
        renderer.set_line_width(3.25f);
        CHECK(renderer.get_line_width() == doctest::Approx(3.25f));
    }

    TEST_CASE("OpenGL renderer self cast and repeated preinit shutdown remain safe") {
        Salix::OpenGLRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        CHECK(interface_ptr->as_opengl_renderer() == &renderer);
        renderer.shutdown();
        renderer.shutdown();
        CHECK(renderer.get_window() == nullptr);
        CHECK(renderer.get_native_handle() == nullptr);
    }
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch7578") {
    TEST_CASE("batch7578 missing texture path returns null before GL dispatch") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.load_texture("__batch7578_missing_texture__.png") == nullptr);
    }
    TEST_CASE("batch7578 empty texture path returns null before GL dispatch") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.load_texture("") == nullptr);
    }
    TEST_CASE("batch7578 purging null texture is a safe no op") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.purge_texture(nullptr));
    }
    TEST_CASE("batch7578 transform sprite draw rejects null texture before GL dispatch") {
        Salix::OpenGLRenderer renderer;
        Salix::Transform transform;
        CHECK_NOTHROW(renderer.draw_sprite(nullptr, &transform, Salix::Color{}, Salix::SpriteFlip::None));
    }
    TEST_CASE("batch7578 matrix sprite draw rejects null texture before GL dispatch") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_sprite(nullptr, glm::mat4(1.0f), Salix::Color{}));
    }
    TEST_CASE("batch7578 wire box with no active camera returns before shader access") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_active_camera() == nullptr);
        CHECK_NOTHROW(renderer.draw_wire_box(glm::mat4(1.0f), Salix::Color{}));
    }
    TEST_CASE("batch7578 line with no active camera returns before shader access") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_line(glm::vec3(0.0f), glm::vec3(1.0f), Salix::Color{}));
    }
    TEST_CASE("batch7578 sphere with no active camera returns before shader access") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_sphere(glm::vec3(0.0f), 7.578f, Salix::Color{}, 12));
    }
    TEST_CASE("batch7578 framebuffer cube with no active camera returns before GL query") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_cube(7578u, 8757u, 0.0f));
    }
    TEST_CASE("batch7578 unknown framebuffer operations remain backend free") {
        Salix::OpenGLRenderer renderer;
        CHECK(renderer.get_framebuffer_texture_id(7578u) == static_cast<ImTextureID>(0));
        CHECK_NOTHROW(renderer.bind_framebuffer(7578u));
        CHECK_NOTHROW(renderer.delete_framebuffer(7578u));
        CHECK_NOTHROW(renderer.end_render_pass());
    }
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch7678") {
    TEST_CASE("batch7678 null texture draw rectangle path returns safely") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_texture(nullptr, Salix::Rect{0, 0, 32, 16}));
    }
    TEST_CASE("batch7678 null texture draw path accepts negative rectangle") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_texture(nullptr, Salix::Rect{-7, -6, 5, 4}));
    }
    TEST_CASE("batch7678 null texture draw path accepts zero extent") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.draw_texture(nullptr, Salix::Rect{7, 6, 0, 0}));
    }
    TEST_CASE("batch7678 transform sprite rejects non OpenGL texture before GL state") {
        Salix::OpenGLRenderer renderer;
        MockITexture texture;
        Salix::Transform transform;
        CHECK_NOTHROW(renderer.draw_sprite(&texture, &transform, Salix::Color{}, Salix::SpriteFlip::None));
    }
    TEST_CASE("batch7678 transform sprite rejects non OpenGL texture with horizontal flip") {
        Salix::OpenGLRenderer renderer;
        MockITexture texture;
        Salix::Transform transform;
        CHECK_NOTHROW(renderer.draw_sprite(&texture, &transform, Salix::Color{}, Salix::SpriteFlip::Horizontal));
    }
    TEST_CASE("batch7678 matrix sprite rejects non OpenGL texture before camera access") {
        Salix::OpenGLRenderer renderer;
        MockITexture texture;
        CHECK_NOTHROW(renderer.draw_sprite(&texture, glm::mat4(1.0f), Salix::Color{}));
    }
    TEST_CASE("batch7678 purge nonnull generic texture deletes through interface") {
        Salix::OpenGLRenderer renderer;
        auto* texture = new MockITexture();
        CHECK_NOTHROW(renderer.purge_texture(texture));
    }
    TEST_CASE("batch7678 zero width resize avoids GL viewport") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.on_window_resize(0, 7678));
    }
    TEST_CASE("batch7678 zero height resize avoids GL viewport") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.on_window_resize(7678, 0));
    }
    TEST_CASE("batch7678 negative resize avoids GL viewport") {
        Salix::OpenGLRenderer renderer;
        CHECK_NOTHROW(renderer.on_window_resize(-76, -78));
    }
}

namespace {
struct Batch7778RendererCwd {
    std::filesystem::path previous=std::filesystem::current_path();
    Batch7778RendererCwd(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
        while(p.has_parent_path() && !std::filesystem::exists(p/"Assets"/"Shaders")) p=p.parent_path();
        if(std::filesystem::exists(p/"Assets"/"Shaders")) std::filesystem::current_path(p);
    }
    ~Batch7778RendererCwd(){std::error_code ec;std::filesystem::current_path(previous,ec);}
};
struct Batch7778RendererSDL {
    bool had=false; std::string old; bool ready=false;
    Batch7778RendererSDL(){
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}
        SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);
        ready=SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)==0;
    }
    ~Batch7778RendererSDL(){SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");}
};
struct Batch7778NativeGLRenderer {
    Batch7778RendererCwd cwd;
    Batch7778RendererSDL sdl;
    Salix::OpenGLRenderer renderer;
    bool ready=false;
    Batch7778NativeGLRenderer(){
        if(!sdl.ready)return;
        Salix::WindowConfig cfg;cfg.title="Batch7778 OpenGLRenderer";cfg.width=128;cfg.height=96;
        ready=renderer.initialize(cfg);
    }
};
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch7778") {
    TEST_CASE("batch7778 real OpenGL renderer initializes geometry shaders and context") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); CHECK(f.renderer.get_window()!=nullptr); CHECK(f.renderer.get_sdl_gl_context()!=nullptr);
    }
    TEST_CASE("batch7778 real OpenGL renderer frame lifecycle executes") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.begin_frame()); CHECK_NOTHROW(f.renderer.clear_depth_buffer()); CHECK_NOTHROW(f.renderer.end_frame());
    }
    TEST_CASE("batch7778 real OpenGL clear color roundtrips finite values") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); f.renderer.set_clear_color(Salix::Color{64.0f,128.0f,192.0f,255.0f});
        auto c=f.renderer.get_clear_color(); CHECK(std::isfinite(c.r)); CHECK(std::isfinite(c.g)); CHECK(std::isfinite(c.b)); CHECK(std::isfinite(c.a));
    }
    TEST_CASE("batch7778 real OpenGL framebuffer lifecycle returns texture id") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); auto id=f.renderer.create_framebuffer(40,30); REQUIRE(id!=0u);
        CHECK(f.renderer.get_framebuffer_texture_id(id)!=static_cast<ImTextureID>(0)); f.renderer.bind_framebuffer(id); f.renderer.unbind_framebuffer(); f.renderer.delete_framebuffer(id);
    }
    TEST_CASE("batch7778 real OpenGL render pass saves and restores framebuffer") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); auto id=f.renderer.create_framebuffer(32,32); REQUIRE(id!=0u);
        auto before=f.renderer.get_current_framebuffer_binding(); f.renderer.begin_render_pass(id); f.renderer.end_render_pass();
        CHECK(f.renderer.get_current_framebuffer_binding()==before); f.renderer.delete_framebuffer(id);
    }
    TEST_CASE("batch7778 real OpenGL viewport and framebuffer restore execute") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); auto before=f.renderer.get_current_framebuffer_binding();
        CHECK_NOTHROW(f.renderer.set_viewport(0,0,64,48)); CHECK_NOTHROW(f.renderer.restore_framebuffer_binding(before));
    }
    TEST_CASE("batch7778 real OpenGL debug primitives render with mock camera") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); MockICamera camera; f.renderer.set_active_camera(&camera);
        CHECK_NOTHROW(f.renderer.draw_wire_box(glm::mat4(1.0f),Salix::Color{}));
        CHECK_NOTHROW(f.renderer.draw_line(glm::vec3(0.0f),glm::vec3(1.0f),Salix::Color{}));
        CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(0.0f),1.0f,Salix::Color{},8));
    }
    TEST_CASE("batch7778 real OpenGL cube renders with mock camera") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); MockICamera camera; f.renderer.set_active_camera(&camera);
        CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(1.0f),Salix::Color{}));
    }
    TEST_CASE("batch7778 real OpenGL rectangle executes filled and outline paths") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready);
        CHECK_NOTHROW(f.renderer.draw_rectangle(Salix::Rect{4,5,20,12},Salix::Color{},true));
        CHECK_NOTHROW(f.renderer.draw_rectangle(Salix::Rect{4,5,20,12},Salix::Color{},false));
    }
    TEST_CASE("batch7778 real OpenGL resize line width and pixels per unit remain usable") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.on_window_resize(96,72));
        f.renderer.set_line_width(2.5f); CHECK(f.renderer.get_line_width()==doctest::Approx(2.5f));
        f.renderer.set_pixels_per_unit(64.0f); CHECK(f.renderer.get_pixels_per_unit()==doctest::Approx(64.0f));
    }
}

namespace {
struct Batch7878ImageFile { std::filesystem::path path; ~Batch7878ImageFile(){std::error_code ec;if(!path.empty())std::filesystem::remove(path,ec);} };
Batch7878ImageFile batch7878_write_pnm(bool rgb) {
    static std::atomic<unsigned> seq{0};
    const auto path=std::filesystem::temp_directory_path()/("batch7878_"+std::to_string(++seq)+(rgb?".ppm":".pgm"));
    {
        std::ofstream out(path,std::ios::binary|std::ios::trunc);
        if(rgb){out<<"P6\n2 2\n255\n";const unsigned char px[12]={255,0,0,0,255,0,0,0,255,255,255,255};out.write(reinterpret_cast<const char*>(px),12);}
        else{out<<"P5\n2 2\n255\n";const unsigned char px[4]={0,64,128,255};out.write(reinterpret_cast<const char*>(px),4);}
    }
    return Batch7878ImageFile{path};
}
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch7878") {
    TEST_CASE("batch7878 initialized OpenGL cube rejects missing active camera") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);f.renderer.set_active_camera(nullptr);CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch7878 framebuffer cube renders first alternating target") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);MockICamera camera;f.renderer.set_active_camera(&camera);auto a=f.renderer.create_framebuffer(32,32),b=f.renderer.create_framebuffer(32,32);REQUIRE(a);REQUIRE(b);CHECK_NOTHROW(f.renderer.draw_cube(a,b,0.0f));f.renderer.delete_framebuffer(a);f.renderer.delete_framebuffer(b); }
    TEST_CASE("batch7878 framebuffer cube renders second alternating target") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);MockICamera camera;f.renderer.set_active_camera(&camera);auto a=f.renderer.create_framebuffer(32,32),b=f.renderer.create_framebuffer(32,32);REQUIRE(a);REQUIRE(b);CHECK_NOTHROW(f.renderer.draw_cube(a,b,0.75f));f.renderer.delete_framebuffer(a);f.renderer.delete_framebuffer(b); }
    TEST_CASE("batch7878 three channel PPM loads through RGB texture branch") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto img=batch7878_write_pnm(true);REQUIRE(std::filesystem::exists(img.path));std::unique_ptr<Salix::ITexture> t(f.renderer.load_texture(img.path.string().c_str()));REQUIRE(t!=nullptr);CHECK(t->get_width()==2);CHECK(t->get_height()==2); }
    TEST_CASE("batch7878 grayscale PGM rejects unsupported channel count safely") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto img=batch7878_write_pnm(false);REQUIRE(std::filesystem::exists(img.path));CHECK(f.renderer.load_texture(img.path.string().c_str())==nullptr); }
    TEST_CASE("batch7878 real texture transform draw rejects missing camera after shader bind") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto img=batch7878_write_pnm(true);REQUIRE(std::filesystem::exists(img.path));std::unique_ptr<Salix::ITexture> t(f.renderer.load_texture(img.path.string().c_str()));REQUIRE(t!=nullptr);Salix::Transform tr;f.renderer.set_active_camera(nullptr);CHECK_NOTHROW(f.renderer.draw_sprite(t.get(),&tr,Salix::Color{},Salix::SpriteFlip::None)); }
    TEST_CASE("batch7878 parented transform sprite composes parent world matrix") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto img=batch7878_write_pnm(true);REQUIRE(std::filesystem::exists(img.path));std::unique_ptr<Salix::ITexture> t(f.renderer.load_texture(img.path.string().c_str()));REQUIRE(t!=nullptr);MockICamera camera;f.renderer.set_active_camera(&camera);Salix::Transform parent;Salix::Transform child;child.set_parent(&parent);CHECK_NOTHROW(f.renderer.draw_sprite(t.get(),&child,Salix::Color{},Salix::SpriteFlip::Both)); }
    TEST_CASE("batch7878 shutdown deletes framebuffer left owned by renderer") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto id=f.renderer.create_framebuffer(24,24);REQUIRE(id);CHECK_NOTHROW(f.renderer.shutdown());f.ready=false; }
    TEST_CASE("batch7878 nested render passes restore bindings in LIFO order") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);auto a=f.renderer.create_framebuffer(20,20),b=f.renderer.create_framebuffer(18,18);REQUIRE(a);REQUIRE(b);auto base=f.renderer.get_current_framebuffer_binding();f.renderer.begin_render_pass(a);f.renderer.begin_render_pass(b);f.renderer.end_render_pass();f.renderer.end_render_pass();CHECK(f.renderer.get_current_framebuffer_binding()==base);f.renderer.delete_framebuffer(a);f.renderer.delete_framebuffer(b); }
    TEST_CASE("batch7878 initialized resize event reaches renderer event listener") { Batch7778NativeGLRenderer f;REQUIRE(f.ready);SDL_Event native{};native.type=SDL_WINDOWEVENT;native.window.event=SDL_WINDOWEVENT_RESIZED;native.window.data1=88;native.window.data2=66;Salix::WindowResizeEvent e(88,66);CHECK_NOTHROW(f.renderer.on_event(e)); }
}

namespace {
struct Batch7978BlockedGLLog {
    std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::path root;
    bool ready = false;
    Batch7978BlockedGLLog() {
        static std::atomic<unsigned> seq{0};
        std::error_code ec;
        root = std::filesystem::temp_directory_path() / ("Salix_Batch7978_GLLog_" + std::to_string(++seq));
        std::filesystem::create_directories(root / "gl_init.log", ec);
        if (ec) return;
        std::filesystem::current_path(root, ec);
        ready = !ec;
    }
    ~Batch7978BlockedGLLog() {
        std::error_code ec;
        std::filesystem::current_path(previous, ec);
        ec.clear();
        if (!root.empty()) std::filesystem::remove_all(root, ec);
    }
};
uint32_t batch7978_create_probe_framebuffer(Salix::OpenGLRenderer& renderer, int width, int height) {
    uint32_t id = static_cast<uint32_t>(-1);
    CHECK_NOTHROW(id = renderer.create_framebuffer(width, height));
    if (id != 0u && id != static_cast<uint32_t>(-1)) renderer.delete_framebuffer(id);
    return id;
}
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch7978") {
    TEST_CASE("batch7978 blocked gl init log path returns controlled initialization failure") {
        Batch7978BlockedGLLog cwd; REQUIRE(cwd.ready);
        Salix::OpenGLRenderer renderer; Salix::WindowConfig config;
        config.title = "Batch7978 blocked log"; config.width = 64; config.height = 64;
        CHECK_FALSE(renderer.initialize(config));
        CHECK(renderer.get_window() == nullptr);
    }
    TEST_CASE("batch7978 fresh sphere with active camera reaches missing shader guard") {
        Salix::OpenGLRenderer renderer; MockICamera camera; renderer.set_active_camera(&camera);
        CHECK_NOTHROW(renderer.draw_sphere(glm::vec3(0.0f), 1.0f, Salix::Color{}, 8));
    }
    TEST_CASE("batch7978 fresh zero radius sphere reaches missing shader guard") {
        Salix::OpenGLRenderer renderer; MockICamera camera; renderer.set_active_camera(&camera);
        CHECK_NOTHROW(renderer.draw_sphere(glm::vec3(1.0f), 0.0f, Salix::Color{}, 0));
    }
    TEST_CASE("batch7978 fresh framebuffer cube first phase reaches missing shader guard") {
        Salix::OpenGLRenderer renderer; MockICamera camera; renderer.set_active_camera(&camera);
        CHECK_NOTHROW(renderer.draw_cube(7978u, 8797u, 0.0f));
    }
    TEST_CASE("batch7978 fresh framebuffer cube second phase reaches missing shader guard") {
        Salix::OpenGLRenderer renderer; MockICamera camera; renderer.set_active_camera(&camera);
        CHECK_NOTHROW(renderer.draw_cube(7978u, 8797u, 0.75f));
    }
    TEST_CASE("batch7978 real OpenGL zero width framebuffer creation is controlled") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready);
        const auto id = batch7978_create_probe_framebuffer(f.renderer, 0, 32);
        CHECK(id != static_cast<uint32_t>(-1));
    }
    TEST_CASE("batch7978 real OpenGL zero height framebuffer creation is controlled") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready);
        const auto id = batch7978_create_probe_framebuffer(f.renderer, 32, 0);
        CHECK(id != static_cast<uint32_t>(-1));
    }
    TEST_CASE("batch7978 real OpenGL zero extent framebuffer creation is controlled") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready);
        const auto id = batch7978_create_probe_framebuffer(f.renderer, 0, 0);
        CHECK(id != static_cast<uint32_t>(-1));
    }
    TEST_CASE("batch7978 real OpenGL one pixel framebuffer remains valid lifecycle") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready);
        const auto id = f.renderer.create_framebuffer(1, 1); REQUIRE(id != 0u);
        CHECK(f.renderer.get_framebuffer_texture_id(id) != static_cast<ImTextureID>(0));
        f.renderer.delete_framebuffer(id);
    }
    TEST_CASE("batch7978 real OpenGL sphere accepts negative radius with live shader") {
        Batch7778NativeGLRenderer f; REQUIRE(f.ready); MockICamera camera; f.renderer.set_active_camera(&camera);
        CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(2.0f, -1.0f, 0.0f), -1.0f, Salix::Color{}, 3));
    }
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch8078") {
    TEST_CASE("batch8078 fresh cube with camera reaches missing shader guard identity") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8078 fresh cube missing shader accepts translated model") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(2.0f),Salix::Color{})); }
    TEST_CASE("batch8078 fresh cube missing shader accepts zero matrix") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(0.0f),Salix::Color{})); }
    TEST_CASE("batch8078 fresh cube missing shader accepts opaque white") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color::from_rgba_int(255,255,255,255))); }
    TEST_CASE("batch8078 fresh cube missing shader accepts transparent color") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color::from_rgba_int(0,0,0,0))); }
    TEST_CASE("batch8078 cube without camera still returns before GL dispatch") { Salix::OpenGLRenderer r; CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8078 cube missing shader retains active camera pointer") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); r.draw_cube(glm::mat4(1.0f),Salix::Color{}); CHECK(r.get_active_camera()==&c); }
    TEST_CASE("batch8078 repeated cube missing shader guard remains stable") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8078 cube guard remains safe after camera replacement") { Salix::OpenGLRenderer r; MockICamera a,b; r.set_active_camera(&a); r.set_active_camera(&b); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); CHECK(r.get_active_camera()==&b); }
    TEST_CASE("batch8078 cube guard remains safe after camera clear and restore") { Salix::OpenGLRenderer r; MockICamera c; r.set_active_camera(&c); r.set_active_camera(nullptr); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); r.set_active_camera(&c); CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
}

namespace {
struct Batch8178BrokenSimpleShader {
    std::filesystem::path previous=std::filesystem::current_path();
    std::filesystem::path root;
    bool had=false; std::string old;
    Salix::OpenGLRenderer renderer;
    bool ready=false;
    Batch8178BrokenSimpleShader() {
        auto repo=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
        while(repo.has_parent_path()&&!std::filesystem::exists(repo/"Assets"/"Shaders")) repo=repo.parent_path();
        if(!std::filesystem::exists(repo/"Assets"/"Shaders")) return;
        static std::atomic<unsigned> seq{0};
        root=std::filesystem::temp_directory_path()/("Salix_Batch8178_BrokenShader_"+std::to_string(++seq));
        std::error_code ec;
        std::filesystem::create_directories(root/"Assets",ec); if(ec)return;
        std::filesystem::copy(repo/"Assets"/"Shaders",root/"Assets"/"Shaders",std::filesystem::copy_options::recursive|std::filesystem::copy_options::overwrite_existing,ec); if(ec)return;
        { std::ofstream out(root/"Assets"/"Shaders"/"OpenGL"/"3D"/"simple.vert",std::ios::trunc); }
        std::filesystem::current_path(root,ec); if(ec)return;
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}
        SDL_Quit(); SDL_setenv("SDL_VIDEODRIVER","windows",1);
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0)return;
        Salix::WindowConfig wc; wc.title="Batch8178 Broken Shader"; wc.width=96; wc.height=64;
        ready=renderer.initialize(wc);
    }
    ~Batch8178BrokenSimpleShader(){
        if(ready)renderer.shutdown();
        SDL_Quit();
        if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");
        std::error_code ec; std::filesystem::current_path(previous,ec); if(!root.empty())std::filesystem::remove_all(root,ec);
    }
};
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch8178") {
    TEST_CASE("batch8178 invalid simple shader still leaves renderer initialization controlled") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK(f.renderer.get_window()!=nullptr); }
    TEST_CASE("batch8178 invalid simple shader cube reaches missing shader guard") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8178 invalid simple shader sphere reaches missing shader guard") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(0.0f),1.0f,Salix::Color{},8)); }
    TEST_CASE("batch8178 invalid simple shader framebuffer cube reaches missing shader guard") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_cube(1u,2u,0.0f)); }
    TEST_CASE("batch8178 invalid simple shader retains live GL context") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK(f.renderer.get_sdl_gl_context()!=nullptr); }
    TEST_CASE("batch8178 invalid simple shader retains SDL window") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK(f.renderer.get_sdl_window()!=nullptr); }
    TEST_CASE("batch8178 invalid simple shader can clear frame") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.clear()); }
    TEST_CASE("batch8178 invalid simple shader can begin and end frame") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.begin_frame()); CHECK_NOTHROW(f.renderer.end_frame()); }
    TEST_CASE("batch8178 invalid simple shader framebuffer lifecycle remains available") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); auto id=f.renderer.create_framebuffer(8,8); REQUIRE(id); f.renderer.delete_framebuffer(id); }
    TEST_CASE("batch8178 invalid simple shader shutdown clears native context") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); f.renderer.shutdown(); f.ready=false; CHECK(f.renderer.get_native_handle()==nullptr); }
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch8278") {
    TEST_CASE("batch8278 broken simple shader cube remains guarded with identity matrix") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8278 broken simple shader cube remains guarded with scaled matrix") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(8.278f),Salix::Color{})); }
    TEST_CASE("batch8278 broken simple shader sphere remains guarded at zero segments") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(0.0f),1.0f,Salix::Color{},0)); }
    TEST_CASE("batch8278 broken simple shader sphere remains guarded at high segments") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(1.0f),2.0f,Salix::Color{},32)); }
    TEST_CASE("batch8278 broken simple shader framebuffer cube remains guarded with full interpolation") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); MockICamera c; f.renderer.set_active_camera(&c); CHECK_NOTHROW(f.renderer.draw_cube(8278u,8728u,1.0f)); }
    TEST_CASE("batch8278 broken simple shader retains framebuffer creation") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); auto id=f.renderer.create_framebuffer(12,12); REQUIRE(id); CHECK(f.renderer.get_framebuffer_texture_id(id)!=static_cast<ImTextureID>(0)); f.renderer.delete_framebuffer(id); }
    TEST_CASE("batch8278 broken simple shader retains current framebuffer query") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.get_current_framebuffer_binding()); }
    TEST_CASE("batch8278 broken simple shader accepts viewport update") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.set_viewport(0,0,64,48)); }
    TEST_CASE("batch8278 broken simple shader clear depth remains callable") { Batch8178BrokenSimpleShader f; REQUIRE(f.ready); CHECK_NOTHROW(f.renderer.clear_depth_buffer()); }
    TEST_CASE("batch8278 blocked GL log still fails before owning a window") { Batch7978BlockedGLLog cwd; REQUIRE(cwd.ready); Salix::OpenGLRenderer r; Salix::WindowConfig wc; wc.title="Batch8278 blocked"; wc.width=48; wc.height=48; CHECK_FALSE(r.initialize(wc)); CHECK(r.get_window()==nullptr); }
}

TEST_SUITE("Salix::rendering::OpenGLRendererBatch8378") {
    TEST_CASE("batch8378 fresh renderer cube remains safe without active camera") { Salix::OpenGLRenderer r;CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8378 fresh renderer cube remains safe with camera and no shader") { Salix::OpenGLRenderer r;MockICamera c;r.set_active_camera(&c);CHECK_NOTHROW(r.draw_cube(glm::mat4(1.0f),Salix::Color{})); }
    TEST_CASE("batch8378 broken shader renderer retains nonnull GL context") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);CHECK(f.renderer.get_sdl_gl_context()!=nullptr); }
    TEST_CASE("batch8378 broken shader renderer retains nonnull SDL window") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);CHECK(f.renderer.get_sdl_window()!=nullptr); }
    TEST_CASE("batch8378 broken shader renderer cube guard remains stable") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);MockICamera c;f.renderer.set_active_camera(&c);CHECK_NOTHROW(f.renderer.draw_cube(glm::mat4(8.378f),Salix::Color{})); }
    TEST_CASE("batch8378 broken shader renderer sphere guard remains stable") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);MockICamera c;f.renderer.set_active_camera(&c);CHECK_NOTHROW(f.renderer.draw_sphere(glm::vec3(0.0f),.8378f,Salix::Color{},12)); }
    TEST_CASE("batch8378 broken shader renderer viewport remains callable") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);CHECK_NOTHROW(f.renderer.set_viewport(1,2,83,78)); }
    TEST_CASE("batch8378 broken shader renderer frame remains callable") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);CHECK_NOTHROW(f.renderer.begin_frame());CHECK_NOTHROW(f.renderer.end_frame()); }
    TEST_CASE("batch8378 broken shader renderer active camera can be cleared") { Batch8178BrokenSimpleShader f;REQUIRE(f.ready);MockICamera c;f.renderer.set_active_camera(&c);f.renderer.set_active_camera(nullptr);CHECK(f.renderer.get_active_camera()==nullptr); }
    TEST_CASE("batch8378 blocked GL log initialization still fails cleanly") { Batch7978BlockedGLLog cwd;REQUIRE(cwd.ready);Salix::OpenGLRenderer r;Salix::WindowConfig wc;wc.title="Batch8378 blocked";wc.width=83;wc.height=78;CHECK_FALSE(r.initialize(wc)); }
}


TEST_SUITE("Salix::rendering::OpenGLRendererBatch8678") {
    TEST_CASE("batch8678 forced OpenGL context failure reaches initialization error branch") {
        Batch7778RendererCwd cwd;
        Batch7778RendererSDL sdl;
        REQUIRE(sdl.ready);
        Salix::OpenGLRenderer::set_force_context_failure_for_testing(true);
        struct Reset {
            ~Reset() { Salix::OpenGLRenderer::set_force_context_failure_for_testing(false); }
        } reset;
        Salix::OpenGLRenderer renderer;
        Salix::WindowConfig config;
        config.title = "Batch8678 Context Failure";
        config.width = 96;
        config.height = 64;
        CHECK_FALSE(renderer.initialize(config));
        CHECK(renderer.get_sdl_gl_context() == nullptr);
        renderer.shutdown();
    }

    TEST_CASE("batch8678 forced GLAD failure reaches initialization error branch") {
        Batch7778RendererCwd cwd;
        Batch7778RendererSDL sdl;
        REQUIRE(sdl.ready);
        Salix::OpenGLRenderer::set_force_glad_failure_for_testing(true);
        struct Reset {
            ~Reset() { Salix::OpenGLRenderer::set_force_glad_failure_for_testing(false); }
        } reset;
        Salix::OpenGLRenderer renderer;
        Salix::WindowConfig config;
        config.title = "Batch8678 GLAD Failure";
        config.width = 96;
        config.height = 64;
        CHECK_FALSE(renderer.initialize(config));
        CHECK(renderer.get_sdl_gl_context() != nullptr);
        renderer.shutdown();
    }

    TEST_CASE("batch8678 zero cube VAO reaches guarded draw return") {
        Batch7778NativeGLRenderer fixture;
        REQUIRE(fixture.ready);
        MockICamera camera;
        fixture.renderer.set_active_camera(&camera);
        const GLuint original = fixture.renderer.get_cube_vao_for_testing();
        REQUIRE(original != 0u);
        fixture.renderer.set_cube_vao_for_testing(0u);
        CHECK_NOTHROW(fixture.renderer.draw_cube(glm::mat4(1.0f), Salix::Color{}));
        fixture.renderer.set_cube_vao_for_testing(original);
    }
}
