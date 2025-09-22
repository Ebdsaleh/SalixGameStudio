// =================================================================================
// Filename:    src/Tests/SalixEngine/ecs/BoxCollider.test.cpp
// Description: Contains unit tests for the BoxCollider class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/math/Vector3.h>
#include <cereal/archives/json.hpp>
#include <sstream>
#include <string>
#include <memory>



TEST_SUITE("Salix::ecs::BoxCollider") {

    // A helper for comparing Vector3 with tolerance, consistent with your other tests.
    inline void check_vector3_approximate(const Salix::Vector3& result, const Salix::Vector3& expected) {
        CHECK(result.x == doctest::Approx(expected.x));
        CHECK(result.y == doctest::Approx(expected.y));
        CHECK(result.z == doctest::Approx(expected.z));
    }
    TEST_CASE("initializes with default values correctly") {
        // ARRANGE
        Salix::SimpleGuid::reset_counter_for_testing();
        auto collider = std::make_unique<Salix::BoxCollider>(); 

        // ACT
        Salix::Vector3 expected_size = Salix::Vector3::One;
        const std::string expected_class_name = "BoxCollider";
        // ASSERT: size will be a salix::Vector3(1.0f, 1.0f, 1.0f)
        CHECK(collider->get_size() == expected_size);
        CHECK(collider->get_class_name() == expected_class_name);
        CHECK(collider->get_name() == expected_class_name);
        CHECK(collider->get_id().get_value() == 1);

        collider.reset();
    }

    TEST_CASE("update does nothing") {
        // ARRANGE
        Salix::SimpleGuid::reset_counter_for_testing();
        auto collider = std::make_unique<Salix::BoxCollider>(); 
        float delta_time = 1.0f;

        // ACT
        collider->update(delta_time);

        // ASSERT: collider will still have default values.
        CHECK(collider->get_size() == Salix::Vector3::One);
        CHECK(collider->get_class_name() == std::string("BoxCollider"));
        CHECK(collider->get_name() == "BoxCollider");
        CHECK(collider->get_id().get_value() == 1);
        CHECK(delta_time == 1.0f);
        collider.reset();

    }

    TEST_CASE("getters and setters") {
        // ARRANGE: Create a collider and a new size vector.
        Salix::BoxCollider collider;
        Salix::Vector3 new_size(5.0f, 10.0f, 2.0f);

        // ACT: Set the new size.
        collider.set_size(new_size);

        // ASSERT: The get_size() method should return the new size.
        check_vector3_approximate(collider.get_size(), new_size);
    }

    TEST_CASE("lifecycle methods run without crashing") {
        // ARRANGE: Create a collider and a default InitContext.
        Salix::BoxCollider collider;
        Salix::InitContext context; // An empty context is sufficient for this unit test.

        // ACT & ASSERT: Call the lifecycle methods. The test passes if they execute
        // without throwing exceptions or crashing.
        CHECK_NOTHROW(collider.initialize());
        CHECK_NOTHROW(collider.on_load(context));
        CHECK_NOTHROW(collider.update(0.016f));
    }

    TEST_CASE("can be serialized and deserialized with Cereal") {
        // ARRANGE: Create an original collider with non-default values.
        auto original_collider = std::make_unique<Salix::BoxCollider>();
        original_collider->set_size({2.5f, 3.5f, 4.5f});
        original_collider->set_name("TestCollider");

        std::stringstream ss;

        // ACT (Serialization): Save the object's state to the stringstream.
        {
            cereal::JSONOutputArchive output_archive(ss);
            // Directly serialize the unique_ptr to the concrete BoxCollider type.
            output_archive(*original_collider);
        }

        // ACT (Deserialization): Load the state into a new object.
            auto loaded_collider = std::make_unique<Salix::BoxCollider>() ;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(*loaded_collider);
        }

        // ASSERT: The loaded object's properties should match the original.
        REQUIRE(loaded_collider != nullptr);
        
        CHECK(loaded_collider->get_name() == original_collider->get_name());
        check_vector3_approximate(loaded_collider->get_size(), original_collider->get_size());
    }

}