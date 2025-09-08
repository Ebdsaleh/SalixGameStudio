// =================================================================================
// Filename:    src/Tests/SalixEngine/assets/Transform.test.cpp
// Description: Contains unit tests for the Transform class functions.
// =================================================================================
#pragma once
#include <doctest.h>
#include <Salix/ecs/Transform.h>
#include <Salix/math/Vector3.h>
#include <Salix/core/SimpleGuid.h>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cmath>
#include <memory>

// A helper for comparing Vector3 with tolerance
// This is needed because floating point math can have tiny precision errors.
void check_vector3_approximate(const Salix::Vector3& vec, float x, float y, float z) {
    CHECK(vec.x == doctest::Approx(x)); 
    CHECK(vec.y == doctest::Approx(y)); 
    CHECK(vec.z == doctest::Approx(z));
}

// This helper function is overloaded to compare two Vector3 objects equality directly.
// To make assertions much cleaner.
void check_vector3_approximate(const Salix::Vector3& result, const Salix::Vector3& expected) {
    CHECK(result.x == doctest::Approx(expected.x));
    CHECK(result.y == doctest::Approx(expected.y));
    CHECK(result.z == doctest::Approx(expected.z));
}


TEST_SUITE("Salix::ecs::Transform") {
    TEST_CASE("initialization and default values") {
        // ARRANGE
        std::unique_ptr<Salix::Transform> default_transform;
        default_transform = std::make_unique<Salix::Transform>();
        std::string class_name = "Transform";        //ASSERT: Check all default values.
        CHECK_FALSE(default_transform.get() == nullptr);
        CHECK(default_transform->get_class_name() == class_name);
        CHECK(default_transform->get_name() == "Transform");
        CHECK(default_transform->get_owner() == nullptr);
        CHECK(default_transform->get_id().get_value() == 1);
        CHECK(default_transform->get_parent() == nullptr);
        CHECK(default_transform->get_children().size() == 0);
        CHECK_FALSE(default_transform->get_id_as_ptr() == nullptr);
        CHECK(default_transform->get_model_matrix() == glm::mat4({1.0f}));
        // Check default transform local-space values.
        check_vector3_approximate(default_transform->get_position(), 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(default_transform->get_rotation(), 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(default_transform->get_scale(), 1.0f, 1.0f, 1.0f);
        // Check default transform world-space value.
        check_vector3_approximate(default_transform->get_world_position(), 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(default_transform->get_world_rotation(), 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(default_transform->get_world_scale(), 1.0f, 1.0f, 1.0f);
    }
    TEST_CASE("a root transform's local and world rotations are equivalent") {
        // ARRANGE
        Salix::Transform root;
        REQUIRE(root.get_parent() == nullptr);

        Salix::Vector3 new_rotation(45.f, 90.f, 180.f);

        // ACT
        root.set_rotation(new_rotation);

        // ASSERT: We can't compare the raw angles. Instead, we check that
        // both sets of angles produce the same final orientation. We do this
        // by rotating a base direction vector and checking if the results match.
        glm::vec3 test_vector(0.0f, 1.0f, 0.0f); // A simple 'up' vector

        // 1. Rotate the vector using the ORIGINAL local rotation angles
        glm::quat original_quat = glm::quat(glm::radians(root.get_rotation().to_glm()));
        glm::vec3 expected_result = original_quat * test_vector;

        // 2. Rotate the vector using the RESULTING world rotation angles
        glm::quat world_quat = glm::quat(glm::radians(root.get_world_rotation().to_glm()));
        glm::vec3 actual_result = world_quat * test_vector;

        // 3. The final rotated vectors should be nearly identical.
        check_vector3_approximate(
            Salix::Vector3(actual_result.x, actual_result.y, actual_result.z),
            Salix::Vector3(expected_result.x, expected_result.y, expected_result.z)
        );
    }

    TEST_CASE("set_world transform correctly modifies a child transform") {
        // ARRANGE: Create a parent with an offset transform and a child.
        Salix::Transform parent;
        parent.set_position(10.0f, 0.0f, 0.0f);
        parent.set_rotation(0.0f, 90.0f, 0.0f); // Rotated to face along the world +X axis
        parent.set_scale(2.0f, 2.0f, 2.0f);

        Salix::Transform child;
        child.set_parent(&parent);

        SUBCASE("for position") {
            // ACT: Set the child's desired WORLD position.
            Salix::Vector3 desired_world_pos(15.0f, 5.0f, 0.0f);
            child.set_world_position(desired_world_pos);

            // ASSERT: The child's world position should now be exactly what we set it to.
            // We don't need to check the local position, as that's an implementation detail.
            check_vector3_approximate(child.get_world_position(), 15.0f, 5.0f, 0.0f);
        }

        SUBCASE("for rotation") {
            // ACT: Set the child's desired WORLD rotation.
            Salix::Vector3 desired_world_rot(0.0f, 135.0f, 0.0f);
            child.set_world_rotation(desired_world_rot);

            // ASSERT: We test for EQUIVALENCE, not equality of the raw Euler angles.
            glm::vec3 test_vector(0.0f, 1.0f, 0.0f); // A simple 'up' vector

            // 1. Rotate the vector using the DESIRED world rotation
            glm::quat desired_quat = glm::quat(glm::radians(desired_world_rot.to_glm()));
            glm::vec3 expected_result = desired_quat * test_vector;

            // 2. Rotate the vector using the ACTUAL resulting world rotation
            glm::quat actual_quat = glm::quat(glm::radians(child.get_world_rotation().to_glm()));
            glm::vec3 actual_result = actual_quat * test_vector;

            // 3. The final rotated vectors should be nearly identical.
            check_vector3_approximate(
                Salix::Vector3(actual_result.x, actual_result.y, actual_result.z),
                Salix::Vector3(expected_result.x, expected_result.y, expected_result.z)
            );
        }

        SUBCASE("for scale") {
            // ACT: Set the child's desired WORLD scale.
            Salix::Vector3 desired_world_scale(3.0f, 3.0f, 3.0f);
            child.set_world_scale(desired_world_scale);

            // ASSERT: The child's world scale should be what we set it to.
            check_vector3_approximate(child.get_world_scale(), 3.0f, 3.0f, 3.0f);

            // ASSERT: The child's LOCAL scale should be 1.5.
            // Parent is scaled by 2. To get a final world scale of 3,
            // the local scale must be 1.5 (because 2 * 1.5 = 3).
            check_vector3_approximate(child.get_scale(), 1.5f, 1.5f, 1.5f);
        }
    }

    TEST_CASE("preserves world transform (position, rotation, scale) when reparenting") {
        // ARRANGE
        Salix::Transform root;
        root.set_position(10.0f, 0.0f, 0.0f);
        root.set_rotation(0.0f, 90.0f, 0.0f); // Add root rotation

        Salix::Transform parent;
        parent.set_parent(&root);
        parent.set_position(5.0f, 0.0f, 0.0f);
        parent.set_scale(2.0f, 2.0f, 2.0f); // Add parent scale

        Salix::Transform child;
        child.set_parent(&parent);
        child.set_position(2.0f, 0.0f, 0.0f);
        child.set_rotation(0.0f, 0.0f, 45.0f); // Add child rotation
        child.set_scale(1.0f, 1.0f, 1.0f);

        // Confirm the child's local transform values
        CHECK(child.get_position() == Salix::Vector3(2.0f, 0.0f, 0.0f));
        CHECK(child.get_rotation() == Salix::Vector3(0.0f, 0.0f, 45.0f));
        CHECK(child.get_scale() == Salix::Vector3(1.0f, 1.0f, 1.0f));

        // Store the child's initial world state
        Salix::Vector3 initial_world_pos = child.get_world_position();
        Salix::Vector3 initial_world_rot = child.get_world_rotation();
        Salix::Vector3 initial_world_scale = child.get_world_scale();

        // ACT: Reparent the child to the root
        child.set_parent(&root);

        // ASSERT: The child's world state should NOT have changed.
        check_vector3_approximate(child.get_world_position(), initial_world_pos.x, initial_world_pos.y, initial_world_pos.z);
        check_vector3_approximate(child.get_world_rotation(), initial_world_rot.x, initial_world_rot.y, initial_world_rot.z);
        check_vector3_approximate(child.get_world_scale(), initial_world_scale.x, initial_world_scale.y, initial_world_scale.z);
    }
    TEST_CASE("preserves world transform when released from parent") {
        // ARRANGE: Create a simple hierarchy: root -> parent -> child
        Salix::Transform root;
        root.set_position(10.0f, 0.0f, 0.0f);

        Salix::Transform parent;
        parent.set_parent(&root);
        parent.set_position(5.0f, 0.0f, 0.0f);

        Salix::Transform child;
        child.set_parent(&parent);
        child.set_position(2.0f, 0.0f, 0.0f);

        // The child's initial world position is 17
        Salix::Vector3 initial_world_pos = child.get_world_position();
        check_vector3_approximate(initial_world_pos, 17.0f, 0.0f, 0.0f);

        // ACT: Release the child, making it a root object.
        child.release_from_parent();

        // ASSERT: The child's world position should still be 17.
        Salix::Vector3 final_world_pos = child.get_world_position();
        check_vector3_approximate(final_world_pos, 17.0f, 0.0f, 0.0f);

        // ASSERT: Its local position should now BE its world position.
        Salix::Vector3 final_local_pos = child.get_position();
        check_vector3_approximate(final_local_pos, 17.0f, 0.0f, 0.0f);

        // ASSERT: Its parent should be null.
        CHECK(child.get_parent() == nullptr);
    }
    TEST_CASE("cannot set self as parent") {
        // ARRANGE
        Salix::Transform transform;

        //ACT
        transform.set_parent(&transform);

        // ASSERT
        CHECK(transform.get_parent() == nullptr);
    }
    TEST_CASE("'set_parent()' adds child to parent's children list") {
        // ARRANGE
        Salix::Transform parent;
        Salix::Transform child;

        // ACT
        child.set_parent(&parent);

        // ASSERT
        // Check that the parent's internal list now contains the child.
        CHECK(child.is_child_of(&parent));
        CHECK(parent.get_children().size() == 1);
        CHECK(parent.get_children()[0] == &child);
    }
    TEST_CASE("set_parent to nullptr removes child from old parent's list") {
        // ARRANGE
        Salix::Transform parent;
        Salix::Transform child;
        child.set_parent(&parent);

        // Pre-condition check to make sure the setup is correct
        REQUIRE(parent.get_children().size() == 1);

        // ACT
        child.set_parent(nullptr);

        // ASSERT
        // Check that the parent's internal list is now empty.
        CHECK(parent.get_children().empty() == true);
    }
    TEST_CASE("release child from parent when parent is destroyed") {
        // Arrange
        std::unique_ptr<Salix::Transform> parent;
        parent = std::make_unique<Salix::Transform>();
        std::unique_ptr<Salix::Transform> child;
        child = std::make_unique<Salix::Transform>();

        // Add child to parent and confirm.
        child->set_parent(parent.get());
        CHECK(child->get_parent() == parent.get());

        // Purge the parent and confirm the child no longer has a reference to the parent.
        parent.reset();
        CHECK(child->get_parent() == nullptr);

    }
    TEST_CASE("removes self from parent's children list when destroyed") {
        // ARRANGE
        auto parent = std::make_unique<Salix::Transform>();
        auto child = std::make_unique<Salix::Transform>();

        // Add child to parent and confirm the parent's list has one entry.
        child->set_parent(parent.get());
        REQUIRE(parent->get_children().size() == 1);

        // ACT: Destroy the CHILD object.
        child.reset();

        // ASSERT: The parent's list of children should now be empty.
        CHECK(parent->get_children().empty() == true);
    }
    TEST_CASE("child cannot be a parent of its parent") {
        // ARRANGE
        auto parent = std::make_unique<Salix::Transform>();
        auto child = std::make_unique<Salix::Transform>();

        // Add child to parent and confirm the parent's list has one entry.
        child->set_parent(parent.get());
        REQUIRE(parent->get_children().size() == 1);

        // Confirm the child is a child of the parent Transform.
        CHECK(child->is_child_of(parent.get()));

        // ASSERT: parent cannot become the child of its child.
        parent->set_parent(child.get());
        CHECK(parent->get_parent() == nullptr);
        CHECK_FALSE(parent->is_child_of(child.get()) == true);

        // Check that child is still a child parent.
        CHECK(child->is_child_of(parent.get()) == true);

    }
    TEST_CASE("translate correctly modifies a root transform's position") {
        // ARRANGE
        Salix::Transform root_0, root_1, root_2;
        Salix::Vector3 root_0_initial_position = root_0.get_position();
        Salix::Vector3 root_1_initial_position = root_1.get_position();
        Salix::Vector3 root_2_initial_position = root_2.get_position();

        check_vector3_approximate(root_0_initial_position, 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(root_1_initial_position, 0.0f, 0.0f, 0.0f);
        check_vector3_approximate(root_2_initial_position, 0.0f, 0.0f, 0.0f);

        Salix::Vector3 translation(10.0f, 0.0f, 5.0f);
        float translate_x = 15.0f;
        float translate_y = 2.0f;
        float translate_z = 10.0f;
        glm::vec3 delta_position = glm::vec3(23.5f, 5.1f, 1.3f);

        SUBCASE("with 'Salix::Vector3'") {
            // ACT
            root_0.translate(translation);
        
            // ASSERT
            Salix::Vector3 final_position_0 = root_0.get_position();
            check_vector3_approximate(final_position_0, 10.0f, 0.0f, 5.0f);
        }
        SUBCASE("with floats") {
            // ACT
            root_1.translate(translate_x, translate_y, translate_z);
        
            // ASSERT
            Salix::Vector3 final_position_1 = root_1.get_position();
            check_vector3_approximate(final_position_1, 15.0f, 2.0f, 10.0f);
        }
        SUBCASE("with 'glm::vec3'") {
            // ACT
            root_2.translate(delta_position);
        
            // ASSERT
            Salix::Vector3 final_position_2 = root_2.get_position();
            check_vector3_approximate(final_position_2, 23.5f, 5.1f, 1.3f);
        }
        
    }
}