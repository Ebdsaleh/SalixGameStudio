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

// Colliders are engine-owned components; these tests do not access the filesystem.
#include <cereal/archives/binary.hpp>
#include <Salix/core/InitContext.h>

TEST_SUITE("Salix::ecs::BoxColliderAdditional") {
    TEST_CASE("box collider zero sized bounds preserve all three dimensions") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(collider.get_size() == Salix::Vector3(0.f, 0.f, 0.f));
    }

    TEST_CASE("box collider asymmetric bounds retain component ordering for picking") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(0.25f, 8.5f, 42.f));
        CHECK(collider.get_size().x == doctest::Approx(0.25f));
        CHECK(collider.get_size().y == doctest::Approx(8.5f));
        CHECK(collider.get_size().z == doctest::Approx(42.f));
    }

    TEST_CASE("box collider negative dimensions are preserved rather than silently clamped") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(-5.f, 7.f, -2.f));
        CHECK(collider.get_size() == Salix::Vector3(-5.f, 7.f, -2.f));
    }

    TEST_CASE("box collider consecutive resizes replace the complete size rather than adding offsets") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(1.f, 2.f, 3.f));
        collider.set_size(Salix::Vector3(10.f, 20.f, 30.f));
        CHECK(collider.get_size() == Salix::Vector3(10.f, 20.f, 30.f));
    }

    TEST_CASE("box collider size getter returns stable storage while the collider stays alive") {
        Salix::BoxCollider collider;
        const Salix::Vector3* address = &collider.get_size();
        collider.set_size(Salix::Vector3(3.f, 2.f, 1.f));
        CHECK(&collider.get_size() == address);
        CHECK(address->x == 3.f);
    }

    TEST_CASE("box collider update with zero and negative delta leaves collision geometry intact") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(1.f, 4.f, 9.f));
        collider.update(0.f);
        collider.update(-0.5f);
        CHECK(collider.get_size() == Salix::Vector3(1.f, 4.f, 9.f));
    }

    TEST_CASE("box collider initialization and load without renderer preserve pending size") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(6.f, 7.f, 8.f));
        Salix::InitContext context{};
        collider.initialize();
        collider.on_load(context);
        CHECK(collider.get_size() == Salix::Vector3(6.f, 7.f, 8.f));
    }

    TEST_CASE("box collider inherited visibility toggles do not modify collider dimensions") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(3.f, 4.f, 5.f));
        collider.set_visibility(false);
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(3.f, 4.f, 5.f));
        collider.set_visibility(true);
        CHECK(collider.is_visible());
    }

    TEST_CASE("box collider inherited element identity may be restored without changing geometry") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(2.f, 4.f, 8.f));
        const auto restored_id = Salix::SimpleGuid::from_value(900001ULL);
        collider.set_id(restored_id);
        collider.set_name("PhysicsBoundary");
        CHECK(collider.get_id() == restored_id);
        CHECK(collider.get_name() == "PhysicsBoundary");
        CHECK(collider.get_size() == Salix::Vector3(2.f, 4.f, 8.f));
    }

    TEST_CASE("box collider binary archive preserves GUID name and nonuniform collision volume") {
        Salix::BoxCollider source;
        source.set_id(Salix::SimpleGuid::from_value(932100ULL));
        source.set_name("SavedCollider");
        source.set_size(Salix::Vector3(0.5f, 15.25f, -3.f));
        std::stringstream bytes;
        { cereal::BinaryOutputArchive archive(bytes); archive(source); }
        Salix::BoxCollider loaded;
        { cereal::BinaryInputArchive archive(bytes); archive(loaded); }
        CHECK(loaded.get_id() == source.get_id());
        CHECK(loaded.get_name() == source.get_name());
        CHECK(loaded.get_size() == source.get_size());
    }
}
