#include <doctest.h>
#include <Salix/ecs/Transform.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/rendering/IRenderer.h>
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

