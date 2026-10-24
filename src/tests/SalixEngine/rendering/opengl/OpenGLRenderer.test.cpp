#include <doctest.h>
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
