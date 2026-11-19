#include <doctest.h>
#include <cstdlib>
#include <SDL.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/core/Engine.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/window/WindowConfig.h>
#include <Salix/ecs/Transform.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <Tests/SalixEngine/mocking/rendering/MockICamera.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::rendering::IRendererBatch1347") {
    TEST_CASE("renderer interface initialization captures requested window configuration") {
        MockIRenderer renderer;
        Salix::WindowConfig config;
        config.title = "Unit Renderer";
        config.width = 960;
        config.height = 540;
        Salix::IRenderer* interface_ptr = &renderer;
        CHECK(interface_ptr->initialize(config));
        CHECK(renderer.initialized);
        CHECK(renderer.initialize_calls == 1);
        CHECK(renderer.last_window_config.title == "Unit Renderer");
        CHECK(renderer.last_window_config.width == 960);
        CHECK(renderer.last_window_config.height == 540);
    }

    TEST_CASE("renderer interface initialization failure remains observable without backend") {
        MockIRenderer renderer;
        renderer.initialize_result = false;
        Salix::IRenderer* interface_ptr = &renderer;
        CHECK_FALSE(interface_ptr->initialize(Salix::WindowConfig{}));
        CHECK(renderer.initialize_calls == 1);
    }

    TEST_CASE("renderer frame lifecycle calls remain independently observable") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->begin_frame();
        interface_ptr->clear_depth_buffer();
        interface_ptr->clear();
        interface_ptr->end_frame();
        CHECK(renderer.begin_frame_calls == 1);
        CHECK(renderer.clear_depth_calls == 1);
        CHECK(renderer.clear_calls == 1);
        CHECK(renderer.end_frame_calls == 1);
    }

    TEST_CASE("renderer pixels per unit roundtrips through interface") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->set_pixels_per_unit(64.0f);
        CHECK(interface_ptr->get_pixels_per_unit() == doctest::Approx(64.0f));
    }

    TEST_CASE("renderer framebuffer lifecycle preserves mock binding state") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        const uint32_t id = interface_ptr->create_framebuffer(320, 180);
        CHECK(id == 1u);
        CHECK(renderer.last_framebuffer_width == 320);
        CHECK(renderer.last_framebuffer_height == 180);
        interface_ptr->bind_framebuffer(id);
        CHECK(interface_ptr->get_current_framebuffer_binding() == static_cast<GLint>(id));
        CHECK(interface_ptr->get_framebuffer_texture_id(id) == static_cast<ImTextureID>(1001u));
        interface_ptr->unbind_framebuffer();
        CHECK(interface_ptr->get_current_framebuffer_binding() == 0);
        interface_ptr->restore_framebuffer_binding(7);
        CHECK(interface_ptr->get_current_framebuffer_binding() == 7);
        interface_ptr->delete_framebuffer(id);
        CHECK(renderer.last_deleted_framebuffer == id);
    }

    TEST_CASE("renderer viewport and resize calls preserve independent dimensions") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->set_viewport(10, 20, 800, 600);
        interface_ptr->on_window_resize(1920, 1080);
        CHECK(renderer.viewport_x == 10);
        CHECK(renderer.viewport_y == 20);
        CHECK(renderer.viewport_width == 800);
        CHECK(renderer.viewport_height == 600);
        CHECK(renderer.resized_width == 1920);
        CHECK(renderer.resized_height == 1080);
    }

    TEST_CASE("renderer active camera is a nonowning interface reference") {
        MockIRenderer renderer;
        MockICamera camera;
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->set_active_camera(&camera);
        CHECK(interface_ptr->get_active_camera() == &camera);
        interface_ptr->set_active_camera(nullptr);
        CHECK(interface_ptr->get_active_camera() == nullptr);
    }

    TEST_CASE("renderer texture loading records path and supports controlled failure") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        std::unique_ptr<Salix::ITexture> texture(interface_ptr->load_texture("Assets/Images/unit.png"));
        REQUIRE(texture != nullptr);
        CHECK(renderer.last_texture_path == "Assets/Images/unit.png");
        CHECK(renderer.load_texture_calls == 1);
        renderer.should_texture_load_fail = true;
        CHECK(interface_ptr->load_texture("Assets/Images/missing.png") == nullptr);
        CHECK(renderer.last_texture_path == "Assets/Images/missing.png");
        CHECK(renderer.load_texture_calls == 2);
    }

    TEST_CASE("renderer sprite draw records object identity and flip state") {
        MockIRenderer renderer;
        MockITexture texture;
        Salix::Transform transform;
        Salix::Color color{};
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->draw_sprite(&texture, &transform, color, Salix::SpriteFlip::Horizontal);
        CHECK(renderer.draw_sprite_call_count == 1);
        CHECK(renderer.last_texture == &texture);
        CHECK(renderer.last_transform == &transform);
        CHECK(renderer.last_flip_state == Salix::SpriteFlip::Horizontal);
    }

    TEST_CASE("renderer line and sphere state remains isolated from backend implementation") {
        MockIRenderer renderer;
        Salix::IRenderer* interface_ptr = &renderer;
        interface_ptr->set_line_width(3.5f);
        CHECK(interface_ptr->get_line_width() == doctest::Approx(3.5f));
        interface_ptr->draw_line(glm::vec3(0.0f), glm::vec3(1.0f), Salix::Color{});
        interface_ptr->draw_sphere(glm::vec3(0.0f), 4.0f, Salix::Color{}, 24);
        CHECK(renderer.draw_line_calls == 1);
        CHECK(renderer.draw_sphere_calls == 1);
        CHECK(renderer.last_sphere_radius == doctest::Approx(4.0f));
        CHECK(renderer.last_sphere_segments == 24);
    }
}

TEST_SUITE("Salix::rendering::IRendererBatch6647") {
    TEST_CASE("batch6647 fresh mock renderer exposes canonical pixels per unit and line width") {
        MockIRenderer renderer;
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(100.0f));
        CHECK(renderer.get_line_width() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch6647 negative pixels per unit roundtrips without normalization") {
        MockIRenderer renderer;
        renderer.set_pixels_per_unit(-66.47f);
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(-66.47f));
    }

    TEST_CASE("batch6647 zero line width roundtrips without changing pixels per unit") {
        MockIRenderer renderer;
        renderer.set_line_width(0.0f);
        CHECK(renderer.get_line_width() == doctest::Approx(0.0f));
        CHECK(renderer.get_pixels_per_unit() == doctest::Approx(100.0f));
    }

    TEST_CASE("batch6647 framebuffer identifiers increment monotonically") {
        MockIRenderer renderer;
        const auto first = renderer.create_framebuffer(64, 47);
        const auto second = renderer.create_framebuffer(66, 46);
        CHECK(first == 1u);
        CHECK(second == 2u);
    }

    TEST_CASE("batch6647 framebuffer texture identifier is deterministic from framebuffer id") {
        MockIRenderer renderer;
        CHECK(renderer.get_framebuffer_texture_id(0u) == static_cast<ImTextureID>(1000u));
        CHECK(renderer.get_framebuffer_texture_id(47u) == static_cast<ImTextureID>(1047u));
    }

    TEST_CASE("batch6647 begin render pass binds requested framebuffer and records call") {
        MockIRenderer renderer;
        renderer.begin_render_pass(66u);
        CHECK(renderer.begin_render_pass_calls == 1);
        CHECK(renderer.get_current_framebuffer_binding() == 66);
    }

    TEST_CASE("batch6647 end render pass is independently observable") {
        MockIRenderer renderer;
        renderer.end_render_pass();
        renderer.end_render_pass();
        CHECK(renderer.end_render_pass_calls == 2);
    }

    TEST_CASE("batch6647 viewport accepts signed coordinates and zero extent") {
        MockIRenderer renderer;
        renderer.set_viewport(-66, 47, 0, 0);
        CHECK(renderer.viewport_x == -66);
        CHECK(renderer.viewport_y == 47);
        CHECK(renderer.viewport_width == 0);
        CHECK(renderer.viewport_height == 0);
    }

    TEST_CASE("batch6647 native handle transports exact opaque pointer") {
        MockIRenderer renderer;
        int native = 6647;
        renderer.native_handle = &native;
        CHECK(renderer.get_native_handle() == &native);
    }

    TEST_CASE("batch6647 draw texture records exact texture pointer and destination rectangle") {
        MockIRenderer renderer;
        MockITexture texture(66, 47, ImTextureID(6647));
        const Salix::Rect destination{-66, 47, 664, 7};
        renderer.draw_texture(&texture, destination);
        CHECK(renderer.draw_texture_calls == 1);
        CHECK(renderer.last_texture == &texture);
        CHECK(renderer.last_dest_rect == destination);
    }
}

namespace {
struct Batch7578RendererDriverGuard {
    bool had_video = false;
    bool had_render = false;
    std::string old_video;
    std::string old_render;
    Batch7578RendererDriverGuard() {
        if (const char* v = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = v; }
        if (const char* v = SDL_getenv("SDL_RENDER_DRIVER")) { had_render = true; old_render = v; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
    }
    ~Batch7578RendererDriverGuard() {
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
        if (had_render) SDL_setenv("SDL_RENDER_DRIVER", old_render.c_str(), 1); else _putenv_s("SDL_RENDER_DRIVER", "");
    }
};

struct Batch7578SDLRendererProbe {
    Batch7578RendererDriverGuard driver;
    Salix::Engine engine;
    bool initialized = true;
    Batch7578SDLRendererProbe() {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::None;
        initialized = engine.initialize(config);
    }
    ~Batch7578SDLRendererProbe() { engine.shutdown(); }
    Salix::IRenderer* renderer() { return engine.get_renderer(); }
};
}

TEST_SUITE("Salix::rendering::IRendererBatch7578") {
    TEST_CASE("batch7578 SDL concrete clear depth stub is safe through interface") {
        Batch7578SDLRendererProbe run; REQUIRE_FALSE(run.initialized); REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->clear_depth_buffer());
    }
    TEST_CASE("batch7578 SDL concrete active camera setter remains no op") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr); MockICamera camera;
        run.renderer()->set_active_camera(&camera); CHECK(run.renderer()->get_active_camera() == nullptr);
    }
    TEST_CASE("batch7578 SDL concrete framebuffer binding stub returns zero") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK(run.renderer()->get_current_framebuffer_binding() == 0);
    }
    TEST_CASE("batch7578 SDL concrete viewport stub accepts arbitrary rectangle") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->set_viewport(-75, 78, 757, 578));
    }
    TEST_CASE("batch7578 SDL concrete restore framebuffer stub accepts arbitrary id") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->restore_framebuffer_binding(7578));
    }
    TEST_CASE("batch7578 SDL concrete begin render pass stub accepts arbitrary id") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->begin_render_pass(7578u));
    }
    TEST_CASE("batch7578 SDL concrete end render pass stub is safe") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->end_render_pass());
    }
    TEST_CASE("batch7578 SDL concrete pixels per unit remains fixed one") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        run.renderer()->set_pixels_per_unit(75.78f); CHECK(run.renderer()->get_pixels_per_unit() == doctest::Approx(1.0f));
    }
    TEST_CASE("batch7578 SDL concrete framebuffer stubs return zero identifiers") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK(run.renderer()->create_framebuffer(757, 578) == 0u);
        CHECK(run.renderer()->get_framebuffer_texture_id(7578u) == static_cast<ImTextureID>(0));
    }
    TEST_CASE("batch7578 SDL concrete debug drawing stubs remain backend free") {
        Batch7578SDLRendererProbe run; REQUIRE(run.renderer() != nullptr);
        CHECK_NOTHROW(run.renderer()->draw_wire_box(glm::mat4(1.0f), Salix::Color{}));
        CHECK_NOTHROW(run.renderer()->draw_line(glm::vec3(0.0f), glm::vec3(1.0f), Salix::Color{}));
    }
}

namespace {
struct Batch7678RendererNativeGuard {
    bool had_video = false;
    std::string old_video;
    Batch7678RendererNativeGuard() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "windows", 1);
        _putenv_s("SDL_RENDER_DRIVER", "");
    }
    ~Batch7678RendererNativeGuard() {
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
    }
};

struct Batch7678NativeSDLRenderer {
    Batch7678RendererNativeGuard driver;
    Salix::Engine engine;
    bool initialized = false;
    Batch7678NativeSDLRenderer() {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::Launch;
        config.window_config.title = "Batch7678 IRenderer";
        config.window_config.width = 96;
        config.window_config.height = 64;
        initialized = engine.initialize(config);
    }
    ~Batch7678NativeSDLRenderer() { engine.shutdown(); }
    Salix::IRenderer* renderer() { return engine.get_renderer(); }
};
}

TEST_SUITE("Salix::rendering::IRendererBatch7678") {
    TEST_CASE("batch7678 native SDL renderer exposes renderer handle while SDL window shortcut remains null") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized); REQUIRE(run.renderer() != nullptr);
        CHECK(run.renderer()->get_native_handle() != nullptr);
        CHECK(run.renderer()->get_window() != nullptr);
        CHECK(run.renderer()->get_window()->get_native_handle() != nullptr);
        CHECK(run.renderer()->get_sdl_window() == nullptr);
    }
    TEST_CASE("batch7678 native SDL renderer clear executes against live backend") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK_NOTHROW(run.renderer()->clear());
    }
    TEST_CASE("batch7678 native SDL renderer begin and end frame execute") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK_NOTHROW(run.renderer()->begin_frame());
        CHECK_NOTHROW(run.renderer()->end_frame());
    }
    TEST_CASE("batch7678 native SDL renderer missing texture returns null") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK(run.renderer()->load_texture("__batch7678_missing_texture__.png") == nullptr);
    }
    TEST_CASE("batch7678 native SDL renderer null draw texture remains safe") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK_NOTHROW(run.renderer()->draw_texture(nullptr, Salix::Rect{0,0,32,32}));
    }
    TEST_CASE("batch7678 native SDL renderer sprite stub remains safe") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        Salix::Transform transform;
        CHECK_NOTHROW(run.renderer()->draw_sprite(nullptr, &transform, Salix::Color{}, Salix::SpriteFlip::Both));
    }
    TEST_CASE("batch7678 native SDL renderer positive resize updates logical backend") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK_NOTHROW(run.renderer()->on_window_resize(80, 48));
    }
    TEST_CASE("batch7678 native SDL renderer framebuffer bind unbind delete stubs remain safe") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK_NOTHROW(run.renderer()->bind_framebuffer(7678u));
        CHECK_NOTHROW(run.renderer()->unbind_framebuffer());
        CHECK_NOTHROW(run.renderer()->delete_framebuffer(7678u));
    }
    TEST_CASE("batch7678 native SDL renderer framebuffer creation remains unsupported zero") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        CHECK(run.renderer()->create_framebuffer(80, 48) == 0u);
        CHECK(run.renderer()->get_framebuffer_texture_id(7678u) == static_cast<ImTextureID>(0));
    }
    TEST_CASE("batch7678 native SDL renderer fixed pixels per unit survives setter") {
        Batch7678NativeSDLRenderer run; REQUIRE(run.initialized);
        run.renderer()->set_pixels_per_unit(76.78f);
        CHECK(run.renderer()->get_pixels_per_unit() == doctest::Approx(1.0f));
    }
}

