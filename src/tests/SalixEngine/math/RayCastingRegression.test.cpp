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
