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

TEST_SUITE("Salix::ecs::CameraBatch2347") {
    TEST_CASE("batch2347 repeated activate calls remain idempotently active") {
        Salix::Camera camera; CHECK_FALSE(camera.get_is_active()); camera.activate(); camera.activate(); CHECK(camera.get_is_active());
    }
    TEST_CASE("batch2347 repeated deactivate calls remain idempotently inactive") {
        Salix::Camera camera; camera.activate(); REQUIRE(camera.get_is_active()); camera.deactivate(); camera.deactivate(); CHECK_FALSE(camera.get_is_active());
    }
    TEST_CASE("batch2347 zero viewport height leaves projection matrix unchanged") {
        Salix::Camera camera; const glm::mat4 before=camera.get_projection_matrix(); camera.set_viewport_size(640,0); const glm::mat4 after=camera.get_projection_matrix(); CHECK(after==before);
    }
    TEST_CASE("batch2347 perspective viewport aspect change rebuilds projection") {
        Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Perspective); camera.set_viewport_size(1600,900); const glm::mat4 wide=camera.get_projection_matrix(); camera.set_viewport_size(900,1600); const glm::mat4 tall=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(wide,tall));
    }
    TEST_CASE("batch2347 field of view mutation rebuilds perspective projection") {
        Salix::Camera camera; const glm::mat4 before=camera.get_projection_matrix(); camera.set_field_of_view(80.f); CHECK(camera.get_field_of_view()==doctest::Approx(80.f)); const glm::mat4 after=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(before,after));
    }
    TEST_CASE("batch2347 orthographic size mutation rebuilds orthographic projection") {
        Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Orthographic); const glm::mat4 before=camera.get_projection_matrix(); camera.set_orthographic_size(24.f); CHECK(camera.get_orthographic_size()==doctest::Approx(24.f)); const glm::mat4 after=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(before,after));
    }
    TEST_CASE("batch2347 switching projection modes produces distinct matrices") {
        Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Perspective); const glm::mat4 perspective=camera.get_projection_matrix(); camera.set_projection_mode(Salix::ProjectionMode::Orthographic); const glm::mat4 orthographic=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(perspective,orthographic));
    }
    TEST_CASE("batch2347 near and far clip getters expose stable storage across writes") {
        Salix::Camera camera; float* near_ptr=&camera.get_near_clip(); float* far_ptr=&camera.get_far_clip(); camera.set_near_clip(0.25f); camera.set_far_clip(500.f); CHECK(near_ptr==&camera.get_near_clip()); CHECK(far_ptr==&camera.get_far_clip()); CHECK(*near_ptr==doctest::Approx(0.25f)); CHECK(*far_ptr==doctest::Approx(500.f));
    }
    TEST_CASE("batch2347 owner transform getter returns exact entity transform pointer") {
        Salix::Entity entity; Salix::Camera camera; camera.set_owner(&entity); REQUIRE(entity.get_transform()!=nullptr); CHECK(camera.get_transform()==entity.get_transform());
    }
    TEST_CASE("batch2347 owner translation changes view matrix while preserving activation state") {
        Salix::Entity entity; Salix::Camera camera; camera.set_owner(&entity); camera.activate(); const glm::mat4 before=camera.get_view_matrix(); entity.get_transform()->translate(3.f,-2.f,5.f); const glm::mat4 after=camera.get_view_matrix(); CHECK(batch1947_camera_matrix_differs(before,after)); CHECK(camera.get_is_active());
    }
}

TEST_SUITE("Salix::ecs::CameraBatch2547") {
    TEST_CASE("batch2547 constructor exposes canonical numeric projection defaults") {
        Salix::Camera camera;
        CHECK(camera.get_field_of_view()==doctest::Approx(45.f));
        CHECK(camera.get_orthographic_size()==doctest::Approx(10.f));
        CHECK(camera.get_near_clip()==doctest::Approx(0.1f));
        CHECK(camera.get_far_clip()==doctest::Approx(1000.f));
    }
    TEST_CASE("batch2547 projection mode getter reference remains stable across mode writes") {
        Salix::Camera camera; const auto* ref=&camera.get_projection_mode();
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(ref==&camera.get_projection_mode()); CHECK(*ref==Salix::ProjectionMode::Orthographic);
    }
    TEST_CASE("batch2547 field of view getter reference remains stable across writes") {
        Salix::Camera camera; float* ref=&camera.get_field_of_view(); camera.set_field_of_view(67.5f);
        CHECK(ref==&camera.get_field_of_view()); CHECK(*ref==doctest::Approx(67.5f));
    }
    TEST_CASE("batch2547 orthographic size getter reference remains stable across writes") {
        Salix::Camera camera; float* ref=&camera.get_orthographic_size(); camera.set_orthographic_size(33.f);
        CHECK(ref==&camera.get_orthographic_size()); CHECK(*ref==doctest::Approx(33.f));
    }
    TEST_CASE("batch2547 zero viewport width with positive height rebuilds perspective projection") {
        Salix::Camera camera; const glm::mat4 before=camera.get_projection_matrix(); camera.set_viewport_size(0,480);
        const glm::mat4 after=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(before,after));
    }
    TEST_CASE("batch2547 orthographic projection reacts independently to near clip mutation") {
        Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Orthographic); const glm::mat4 before=camera.get_projection_matrix();
        camera.set_near_clip(2.f); const glm::mat4 after=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(before,after)); CHECK(camera.get_near_clip()==doctest::Approx(2.f));
    }
    TEST_CASE("batch2547 orthographic projection reacts independently to far clip mutation") {
        Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Orthographic); const glm::mat4 before=camera.get_projection_matrix();
        camera.set_far_clip(250.f); const glm::mat4 after=camera.get_projection_matrix(); CHECK(batch1947_camera_matrix_differs(before,after)); CHECK(camera.get_far_clip()==doctest::Approx(250.f));
    }
    TEST_CASE("batch2547 ownerless repeated view reads retain identity matrix storage") {
        Salix::Camera camera; const glm::mat4* first=&camera.get_view_matrix(); const glm::mat4* second=&camera.get_view_matrix();
        CHECK(first==second); CHECK(*first==glm::mat4(1.f));
    }
    TEST_CASE("batch2547 owner rotation changes computed view matrix") {
        Salix::Entity entity; Salix::Camera camera; camera.set_owner(&entity); auto* transform=entity.get_transform(); REQUIRE(transform!=nullptr);
        const glm::mat4 before=camera.get_view_matrix(); transform->set_rotation(0.f,45.f,0.f); const glm::mat4 after=camera.get_view_matrix();
        CHECK(batch1947_camera_matrix_differs(before,after));
    }
    TEST_CASE("batch2547 initialize keeps explicit numeric camera properties intact") {
        Salix::Camera camera; camera.set_field_of_view(72.f); camera.set_near_clip(0.25f); camera.set_far_clip(750.f); camera.set_orthographic_size(19.f);
        camera.initialize(); CHECK(camera.get_field_of_view()==doctest::Approx(72.f)); CHECK(camera.get_near_clip()==doctest::Approx(0.25f));
        CHECK(camera.get_far_clip()==doctest::Approx(750.f)); CHECK(camera.get_orthographic_size()==doctest::Approx(19.f));
    }
}

