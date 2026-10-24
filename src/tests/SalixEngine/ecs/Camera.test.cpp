#include <doctest.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <cmath>

namespace {
bool batch1247_matrix_differs(const glm::mat4& a, const glm::mat4& b) {
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            if (std::fabs(a[column][row] - b[column][row]) > 0.0001f) {
                return true;
            }
        }
    }
    return false;
}
}

TEST_SUITE("Salix::ecs::CameraBatch1247") {
    TEST_CASE("camera constructor establishes class identity and perspective defaults") {
        Salix::Camera camera;
        CHECK(std::string(camera.get_class_name()) == "Camera");
        CHECK(camera.get_name() == "Camera");
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("camera activation and deactivation are idempotent") {
        Salix::Camera camera;
        camera.activate();
        camera.activate();
        CHECK(camera.get_is_active());
        camera.deactivate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("camera projection mode roundtrips between perspective and orthographic") {
        Salix::Camera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Orthographic);
        camera.set_projection_mode(Salix::ProjectionMode::Perspective);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("camera field of view changes perspective projection") {
        Salix::Camera camera;
        const glm::mat4 before = camera.get_projection_matrix();
        camera.set_field_of_view(70.0f);
        CHECK(camera.get_field_of_view() == doctest::Approx(70.0f));
        const glm::mat4 after = camera.get_projection_matrix();
        CHECK(batch1247_matrix_differs(before, after));
    }

    TEST_CASE("camera orthographic size changes orthographic projection") {
        Salix::Camera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        const glm::mat4 before = camera.get_projection_matrix();
        camera.set_orthographic_size(24.0f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(24.0f));
        const glm::mat4 after = camera.get_projection_matrix();
        CHECK(batch1247_matrix_differs(before, after));
    }

    TEST_CASE("camera zero viewport height leaves existing projection unchanged") {
        Salix::Camera camera;
        const glm::mat4 before = camera.get_projection_matrix();
        camera.set_viewport_size(640, 0);
        const glm::mat4 after = camera.get_projection_matrix();
        CHECK_FALSE(batch1247_matrix_differs(before, after));
    }

    TEST_CASE("camera viewport aspect ratio changes projection matrix") {
        Salix::Camera camera;
        camera.set_viewport_size(1600, 900);
        const glm::mat4 wide = camera.get_projection_matrix();
        camera.set_viewport_size(900, 1600);
        const glm::mat4 tall = camera.get_projection_matrix();
        CHECK(batch1247_matrix_differs(wide, tall));
        CHECK(std::fabs(wide[0][0] - tall[0][0]) > 0.0001f);
    }

    TEST_CASE("camera near and far clip setters retain independent values") {
        Salix::Camera camera;
        camera.set_near_clip(0.5f);
        camera.set_far_clip(250.0f);
        CHECK(camera.get_near_clip() == doctest::Approx(0.5f));
        CHECK(camera.get_far_clip() == doctest::Approx(250.0f));
    }

    TEST_CASE("camera returns its owning entity transform through camera interface") {
        Salix::Entity entity;
        Salix::Camera camera;
        camera.set_owner(&entity);
        REQUIRE(entity.get_transform() != nullptr);
        CHECK(camera.get_transform() == entity.get_transform());
    }

    TEST_CASE("camera view matrix follows owning transform position") {
        Salix::Entity entity;
        Salix::Camera camera;
        camera.set_owner(&entity);
        Salix::Transform* transform = entity.get_transform();
        REQUIRE(transform != nullptr);

        const glm::mat4 before = camera.get_view_matrix();
        transform->set_position(5.0f, 2.0f, -3.0f);
        const glm::mat4 after = camera.get_view_matrix();

        CHECK(batch1247_matrix_differs(before, after));
    }
}

namespace {
bool batch1947_camera_matrix_differs(const glm::mat4& a, const glm::mat4& b) {
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (std::fabs(a[c][r] - b[c][r]) > 0.0001f) return true;
    return false;
}
}

TEST_SUITE("Salix::ecs::CameraBatch1947") {
    TEST_CASE("ownerless camera view matrix remains identity") {
        Salix::Camera camera;
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("direct active setter roundtrips both boolean states") {
        Salix::Camera camera;
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("initialize preserves explicit projection and activation selections") {
        Salix::Camera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_is_active(true);
        camera.initialize();
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_is_active());
    }

    TEST_CASE("inactive update is a safe no-op for public camera state") {
        Salix::Camera camera;
        camera.set_is_active(false);
        const auto mode = camera.get_projection_mode();
        const float fov = camera.get_field_of_view();
        CHECK_NOTHROW(camera.update(0.016f));
        CHECK(camera.get_projection_mode() == mode);
        CHECK(camera.get_field_of_view() == doctest::Approx(fov));
    }

    TEST_CASE("active update accepts zero and negative delta values without changing activation") {
        Salix::Camera camera;
        camera.activate();
        CHECK_NOTHROW(camera.update(0.0f));
        CHECK_NOTHROW(camera.update(-5.0f));
        CHECK(camera.get_is_active());
    }

    TEST_CASE("clean projection getter returns stable matrix storage across repeated reads") {
        Salix::Camera camera;
        const glm::mat4* first = &camera.get_projection_matrix();
        const glm::mat4* second = &camera.get_projection_matrix();
        CHECK(first == second);
        CHECK(*first == *second);
    }

    TEST_CASE("near clip change rebuilds perspective projection independently") {
        Salix::Camera camera;
        const glm::mat4 before = camera.get_projection_matrix();
        camera.set_near_clip(2.0f);
        CHECK(camera.get_near_clip() == doctest::Approx(2.0f));
        const glm::mat4 after = camera.get_projection_matrix();
        CHECK(batch1947_camera_matrix_differs(before, after));
    }

    TEST_CASE("far clip change rebuilds perspective projection independently") {
        Salix::Camera camera;
        const glm::mat4 before = camera.get_projection_matrix();
        camera.set_far_clip(75.0f);
        CHECK(camera.get_far_clip() == doctest::Approx(75.0f));
        const glm::mat4 after = camera.get_projection_matrix();
        CHECK(batch1947_camera_matrix_differs(before, after));
    }

    TEST_CASE("orthographic viewport changes alter horizontal projection scale") {
        Salix::Camera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_viewport_size(200, 100);
        const glm::mat4 wide = camera.get_projection_matrix();
        camera.set_viewport_size(100, 200);
        const glm::mat4 tall = camera.get_projection_matrix();
        CHECK(batch1947_camera_matrix_differs(wide, tall));
        CHECK(std::fabs(wide[0][0] - tall[0][0]) > 0.0001f);
    }

    TEST_CASE("view matrix storage stays stable while owner transform changes its contents") {
        Salix::Entity entity;
        Salix::Camera camera;
        camera.set_owner(&entity);
        const glm::mat4* storage = &camera.get_view_matrix();
        const glm::mat4 before = *storage;
        entity.get_transform()->set_position(2.0f, 3.0f, 4.0f);
        const glm::mat4* after_storage = &camera.get_view_matrix();
        CHECK(storage == after_storage);
        CHECK(batch1947_camera_matrix_differs(before, *after_storage));
    }
}

// ---- Relocated from src/tests/SalixEngine/ecs/RenderableElement3D.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/ecs/RenderableElement3D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElement3DBatch1547") {
    TEST_CASE("3D renderable defaults visible through its own base layer") {
        MockRenderableElement3D element;
        Salix::RenderableElement3D* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("3D renderable visibility writes remain observable through RenderableElement base") {
        MockRenderableElement3D element;
        Salix::RenderableElement* parent = &element;
        parent->set_visibility(false);
        CHECK_FALSE(element.is_visible());
        element.set_visibility(true);
        CHECK(parent->is_visible());
    }

    TEST_CASE("3D renderable render dispatch receives exact renderer pointer") {
        MockRenderableElement3D element;
        MockIRenderer renderer;
        Salix::RenderableElement3D* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("3D renderable on load receives exact initialization context") {
        MockRenderableElement3D element;
        Salix::InitContext context{};
        Salix::RenderableElement3D* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("3D renderable identity remains distinct from 2D renderable identity") {
        MockRenderableElement3D three_d;
        MockRenderableElement2D two_d;
        CHECK(std::string(three_d.get_class_name()) == "MockRenderableElement3D");
        CHECK(std::string(two_d.get_class_name()) == "MockRenderableElement2D");
    }

    TEST_CASE("3D renderable inherited display name changes independently of visibility") {
        MockRenderableElement3D element;
        element.set_visibility(false);
        element.set_name("Mesh Probe");
        CHECK(element.get_name() == "Mesh Probe");
        CHECK_FALSE(element.is_visible());
    }

    TEST_CASE("3D renderable owner pointer can be assigned and cleared") {
        MockRenderableElement3D element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        element.set_owner(nullptr);
        CHECK(element.get_owner() == nullptr);
    }

    TEST_CASE("3D renderable render accepts null without altering persistent metadata") {
        MockRenderableElement3D element;
        element.set_name("Null Renderer Probe");
        const auto id = element.get_id();
        element.render(nullptr);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == nullptr);
        CHECK(element.get_name() == "Null Renderer Probe");
        CHECK(element.get_id() == id);
    }

    TEST_CASE("separate 3D renderables retain independent load contexts") {
        MockRenderableElement3D first;
        MockRenderableElement3D second;
        Salix::InitContext first_context{};
        Salix::InitContext second_context{};
        first.on_load(first_context);
        second.on_load(second_context);
        CHECK(first.last_context == &first_context);
        CHECK(second.last_context == &second_context);
        CHECK(first.last_context != second.last_context);
    }

    TEST_CASE("3D renderable supports polymorphic destruction through Element base") {
        std::unique_ptr<Salix::Element> element =
            std::make_unique<MockRenderableElement3D>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElement3D");
        CHECK(element->is_visible());
    }
}
