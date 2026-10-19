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

// Additional cases preserved from the separate regression suite.
// Ray and volume picking regression checks; all geometry is in memory.
#include <doctest.h>
#include <Salix/math/RayCasting.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

TEST_SUITE("Salix::math::RayCastingRegression") {
    TEST_CASE("raycasting forward ray hits nearest axis aligned face at expected distance") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, {0.f, 0.f, 1.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), distance));
        CHECK(distance == doctest::Approx(4.f));
    }

    TEST_CASE("raycasting ray pointed away from box reports miss without mutating output") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, {0.f, 0.f, -1.f}};
        float distance = 123.f;
        CHECK_FALSE(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), distance));
        CHECK(distance == 123.f);
    }

    TEST_CASE("raycasting ray starting inside box returns forward exit distance") {
        const Salix::Ray ray{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), distance));
        CHECK(distance == doctest::Approx(1.f));
    }

    TEST_CASE("raycasting parallel off axis ray misses and retains sentinel distance") {
        const Salix::Ray ray{{3.f, 0.f, -4.f}, {0.f, 0.f, 1.f}};
        float distance = 77.f;
        CHECK_FALSE(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), distance));
        CHECK(distance == 77.f);
    }

    TEST_CASE("raycasting diagonal normalized ray intersects the front face at a longer path") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, glm::normalize(glm::vec3(0.1f, 0.f, 1.f))};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), distance));
        CHECK(distance > 4.f);
        CHECK(distance < 4.1f);
    }

    TEST_CASE("raycasting Vector3 overload agrees with GLM overload on hit and distance") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, {0.f, 0.f, 1.f}};
        float glm_distance = -1.f;
        float vector_distance = -1.f;
        const bool glm_hit = Salix::Raycast::IntersectsAABB(ray, glm::vec3(-2.f, -2.f, -2.f), glm::vec3(2.f, 2.f, 2.f), glm_distance);
        const bool vector_hit = Salix::Raycast::IntersectsAABB(ray, Salix::Vector3(-2.f, -2.f, -2.f), Salix::Vector3(2.f, 2.f, 2.f), vector_distance);
        CHECK(glm_hit);
        CHECK(glm_hit == vector_hit);
        CHECK(glm_distance == doctest::Approx(vector_distance));
    }

    TEST_CASE("raycasting identity oriented box matches axis aligned box distance") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, {0.f, 0.f, 1.f}};
        float obb = -1.f;
        float aabb = -1.f;
        CHECK(Salix::Raycast::IntersectsOBB(ray, glm::mat4(1.f), glm::vec3(1.f), obb));
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f), glm::vec3(1.f), aabb));
        CHECK(obb == doctest::Approx(aabb));
    }

    TEST_CASE("raycasting translated oriented box is intersected at its translated near face") {
        const Salix::Ray ray{{0.f, 0.f, -5.f}, {0.f, 0.f, 1.f}};
        const glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 10.f));
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsOBB(ray, model, glm::vec3(1.f), distance));
        CHECK(distance == doctest::Approx(14.f));
    }

    TEST_CASE("raycasting rotating anisotropic oriented box changes its ray entry distance") {
        const Salix::Ray ray{{0.f, 0.f, -10.f}, {0.f, 0.f, 1.f}};
        const auto model = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsOBB(ray, model, glm::vec3(2.f, 1.f, 0.5f), distance));
        CHECK(distance == doctest::Approx(8.f).epsilon(0.001));
    }

    TEST_CASE("raycasting oriented box miss behind the ray leaves distance unchanged") {
        const Salix::Ray ray{{0.f, 0.f, 5.f}, {0.f, 0.f, 1.f}};
        const glm::mat4 model = glm::mat4(1.f);
        float distance = 99.f;
        CHECK_FALSE(Salix::Raycast::IntersectsOBB(ray, model, glm::vec3(1.f), distance));
        CHECK(distance == 99.f);
    }
}

// Signed axis slabs, inside-origin exit faces and offset viewport equivalence.
TEST_SUITE("Salix::math::RayCastingAdditional") {
    TEST_CASE("ray positive X ray reaches first X slab at predicted distance") {
        const Salix::Ray ray{{-7.f, 0.f, 0.f}, {1.f, 0.f, 0.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-2.f, -1.f, -1.f), glm::vec3(2.f, 1.f, 1.f), distance));
        CHECK(distance == doctest::Approx(5.f));
    }

    TEST_CASE("ray negative X ray reaches far-side X slab from the right") {
        const Salix::Ray ray{{7.f, 0.f, 0.f}, {-1.f, 0.f, 0.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-2.f, -1.f, -1.f), glm::vec3(2.f, 1.f, 1.f), distance));
        CHECK(distance == doctest::Approx(5.f));
    }

    TEST_CASE("ray positive Y ray intersects an elongated vertical box at lower face") {
        const Salix::Ray ray{{0.f, -8.f, 0.f}, {0.f, 1.f, 0.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f, -3.f, -1.f), glm::vec3(1.f, 3.f, 1.f), distance));
        CHECK(distance == doctest::Approx(5.f));
    }

    TEST_CASE("ray negative Y ray intersects at the upper face from above") {
        const Salix::Ray ray{{0.f, 8.f, 0.f}, {0.f, -1.f, 0.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f, -3.f, -1.f), glm::vec3(1.f, 3.f, 1.f), distance));
        CHECK(distance == doctest::Approx(5.f));
    }

    TEST_CASE("ray positive Z ray encounters a box whose near face is negative") {
        const Salix::Ray ray{{0.f, 0.f, -9.f}, {0.f, 0.f, 1.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f, -1.f, -4.f), glm::vec3(1.f, 1.f, 2.f), distance));
        CHECK(distance == doctest::Approx(5.f));
    }

    TEST_CASE("ray negative Z ray encounters a box whose near face is positive") {
        const Salix::Ray ray{{0.f, 0.f, 9.f}, {0.f, 0.f, -1.f}};
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f, -1.f, -4.f), glm::vec3(1.f, 1.f, 2.f), distance));
        CHECK(distance == doctest::Approx(7.f));
    }

    TEST_CASE("ray rays originating within asymmetric box use forward exit face") {
        const Salix::Ray ray_x{{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}};
        const Salix::Ray ray_y{{0.f, 0.f, 0.f}, {0.f, -1.f, 0.f}};
        float x = -1.f, y = -1.f;
        CHECK(Salix::Raycast::IntersectsAABB(ray_x, glm::vec3(-2.f, -3.f, -4.f), glm::vec3(5.f, 6.f, 7.f), x));
        CHECK(Salix::Raycast::IntersectsAABB(ray_y, glm::vec3(-2.f, -3.f, -4.f), glm::vec3(5.f, 6.f, 7.f), y));
        CHECK(x == doctest::Approx(5.f));
        CHECK(y == doctest::Approx(3.f));
    }

    TEST_CASE("ray ray with parallel axis outside slab is rejected without altering output") {
        const Salix::Ray ray{{0.f, 8.f, -5.f}, {0.f, 0.f, 1.f}};
        float distance = 314.f;
        CHECK_FALSE(Salix::Raycast::IntersectsAABB(ray, glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f), distance));
        CHECK(distance == 314.f);
    }

    TEST_CASE("ray translated oriented box along X reports correct near face") {
        const Salix::Ray ray{{-5.f, 0.f, 0.f}, {1.f, 0.f, 0.f}};
        const glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(10.f, 0.f, 0.f));
        float distance = -1.f;
        CHECK(Salix::Raycast::IntersectsOBB(ray, transform, glm::vec3(2.f, 1.f, 1.f), distance));
        CHECK(distance == doctest::Approx(13.f));
    }

    TEST_CASE("ray viewport offset produces identical center ray in both coordinate overloads") {
        MockICamera camera;
        const glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        const glm::mat4 projection = glm::perspective(glm::radians(60.f), 4.f / 3.f, 0.1f, 100.f);
        const_cast<glm::mat4&>(camera.get_view_matrix()) = view;
        const_cast<glm::mat4&>(camera.get_projection_matrix()) = projection;
        const auto vector_ray = Salix::Raycast::CreateRayFromScreen(&camera, Salix::Vector2(500.f, 350.f), Salix::Vector2(100.f, 50.f), Salix::Vector2(800.f, 600.f));
        const auto imgui_ray = Salix::Raycast::CreateRayFromScreen(&camera, ImVec2(500.f, 350.f), ImVec2(100.f, 50.f), ImVec2(800.f, 600.f));
        CHECK(vector_ray.origin.z == doctest::Approx(5.f));
        CHECK(vector_ray.direction.x == doctest::Approx(0.f).epsilon(0.001));
        CHECK(vector_ray.direction.y == doctest::Approx(0.f).epsilon(0.001));
        CHECK(vector_ray.direction.z == doctest::Approx(-1.f));
        CHECK(imgui_ray.direction.x == doctest::Approx(vector_ray.direction.x));
        CHECK(imgui_ray.direction.y == doctest::Approx(vector_ray.direction.y));
        CHECK(imgui_ray.direction.z == doctest::Approx(vector_ray.direction.z));
    }
}
