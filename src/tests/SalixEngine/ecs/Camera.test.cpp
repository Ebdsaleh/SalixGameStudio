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
