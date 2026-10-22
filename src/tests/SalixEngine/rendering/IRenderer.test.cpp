#include <doctest.h>
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
