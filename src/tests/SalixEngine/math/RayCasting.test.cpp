// =================================================================================
// Filename:    src/Tests/SalixEngine/math/RayCasting.test.cpp
// Description: Contains unit tests for the RayCasting functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/math/RayCasting.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/rendering/ICamera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Tests/SalixEngine/mocking/rendering/MockICamera.h> // Assuming this mock is available

// Helper for comparing glm::vec3 with tolerance
void check_vec3_approx(const glm::vec3& result, const glm::vec3& expected) {
    CHECK(result.x == doctest::Approx(expected.x));
    CHECK(result.y == doctest::Approx(expected.y));
    CHECK(result.z == doctest::Approx(expected.z));
}

TEST_SUITE("Salix::math::RayCasting") {

    TEST_CASE("CreateRayFromScreen generates a correct world-space ray") {
        // ARRANGE
        MockICamera camera;
        auto* transform = camera.get_transform();

        // Position the camera at (0, 0, 5) looking towards the origin (0, 0, 0)
        transform->set_position(0.0f, 0.0f, 5.0f);

        // Manually set view and projection matrices for a predictable test
        glm::mat4& view = const_cast<glm::mat4&>(camera.get_view_matrix());
        view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glm::mat4& proj = const_cast<glm::mat4&>(camera.get_projection_matrix());
        proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        Salix::Vector2 viewport_pos(0.0f, 0.0f);
        Salix::Vector2 viewport_size(800.0f, 600.0f);
        SUBCASE("ray from center of screen points forward") {
            // ACT: Create a ray from the dead center of the viewport.
            Salix::Ray ray = Salix::Raycast::CreateRayFromScreen(&camera, {400.0f, 300.0f}, viewport_pos, viewport_size);

            // ASSERT: The ray should originate at the camera and point straight ahead.
            check_vec3_approx(ray.origin, {0.0f, 0.0f, 5.0f});
            check_vec3_approx(ray.direction, {0.0f, 0.0f, -1.0f});
        }
        SUBCASE("ray from top-left of screen points up and left") {
            // ACT: Create a ray from the top-left corner.
            Salix::Ray ray = Salix::Raycast::CreateRayFromScreen(&camera, {0.0f, 0.0f}, viewport_pos, viewport_size);

            // ASSERT: The ray's direction should be angled correctly.
            check_vec3_approx(ray.origin, {0.0f, 0.0f, 5.0f});
            CHECK(ray.direction.x < 0.0f); // Points left
            CHECK(ray.direction.y > 0.0f); // Points up
            CHECK(ray.direction.z < 0.0f); // Points forward (into the screen)
        }
        SUBCASE("ImVec2 overload correctly wraps the core function") {
            // ARRANGE: Use ImVec2 types for this test
            ImVec2 viewport_pos_imgui(0.0f, 0.0f);
            ImVec2 viewport_size_imgui(800.0f, 600.0f);
            ImVec2 mouse_pos_imgui(400.0f, 300.0f);

            // ACT: Call the ImVec2 overload.
            Salix::Ray ray = Salix::Raycast::CreateRayFromScreen(&camera, mouse_pos_imgui, viewport_pos_imgui, viewport_size_imgui);

            // ASSERT: The result should be identical to the core Vector2 version.
            check_vec3_approx(ray.origin, {0.0f, 0.0f, 5.0f});
            check_vec3_approx(ray.direction, {0.0f, 0.0f, -1.0f});
        }
    }

    TEST_CASE("IntersectsAABB correctly detects hits and misses") {
        // ARRANGE: A 2x2x2 cube centered at the origin.
        Salix::Vector3 box_min(-1.0f, -1.0f, -1.0f);
        Salix::Vector3 box_max(1.0f, 1.0f, 1.0f);
        float distance = 0.0f;
        SUBCASE("ray pointing at the box hits") {
            // ACT: A ray from (0,0,5) pointing straight at the box.
            Salix::Ray ray = {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}};
            bool hit = Salix::Raycast::IntersectsAABB(ray, box_min, box_max, distance);

            // ASSERT: Should hit the box at z=1, so distance is 4 (from 5 to 1).
            CHECK(hit == true);
            CHECK(distance == doctest::Approx(4.0f));
        }
        SUBCASE("ray pointing away from the box misses") {
            // ACT: A ray pointing away from the box.
            Salix::Ray ray = {{0.0f, 0.0f, 5.0f}, {0.0f, 1.0f, 0.0f}};
            bool hit = Salix::Raycast::IntersectsAABB(ray, box_min, box_max, distance);

            // ASSERT: Should not hit.
            CHECK(hit == false);
        }
    }

    TEST_CASE("IntersectsOBB correctly detects hits on a transformed box") {
        // ARRANGE: A 2x2x2 cube (half-extents of 1) moved and rotated.
        glm::vec3 half_extents = {1.0f, 1.0f, 1.0f};
        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        float distance = 0.0f;
        SUBCASE("ray pointing at the OBB hits") {
            // ACT: A ray aimed at the center of the transformed box.
            Salix::Ray ray = {{10.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}};
            bool hit = Salix::Raycast::IntersectsOBB(ray, model_matrix, half_extents, distance);

            // ASSERT: It should hit, and the distance should be positive.
            CHECK(hit == true);
            CHECK(distance > 0.0f);
        }
        SUBCASE("ray pointing away from the OBB misses") {
            // ACT: A ray aimed at the world origin, far from the box.
            Salix::Ray ray = {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}};
            bool hit = Salix::Raycast::IntersectsOBB(ray, model_matrix, half_extents, distance);

            // ASSERT
            CHECK(hit == false);
        }

    }
}