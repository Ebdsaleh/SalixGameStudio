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

// Persistence, identity and lifecycle invariants.
#include <Salix/ecs/Entity.h>
TEST_SUITE("Salix::ecs::BoxColliderBatch647") {
    TEST_CASE("JSON preserves negative and nonuniform collision dimensions") {
        Salix::BoxCollider original;original.set_size(Salix::Vector3(-2.f,3.5f,12.25f));
        std::stringstream data;
        {cereal::JSONOutputArchive writer(data);writer(original);}
        Salix::BoxCollider loaded;
        {cereal::JSONInputArchive reader(data);reader(loaded);}
        CHECK(loaded.get_size()==Salix::Vector3(-2.f,3.5f,12.25f));
    }

    TEST_CASE("binary archive restores an explicitly assigned collider identifier") {
        Salix::BoxCollider original;
        original.set_id(Salix::SimpleGuid::from_value(12345678ULL));
        std::stringstream bytes;
        {cereal::BinaryOutputArchive writer(bytes);writer(original);}
        Salix::BoxCollider restored;
        {cereal::BinaryInputArchive reader(bytes);reader(restored);}
        CHECK(restored.get_id()==original.get_id());
    }

    TEST_CASE("JSON load replaces preexisting size and name together") {
        Salix::BoxCollider original;original.set_name("NewPhysics");
        original.set_size(Salix::Vector3(3.f,4.f,5.f));
        std::stringstream data;{cereal::JSONOutputArchive out(data);out(original);}
        Salix::BoxCollider restored;restored.set_name("OldPhysics");restored.set_size(Salix::Vector3(9.f));
        {cereal::JSONInputArchive in(data);in(restored);}
        CHECK(restored.get_name()=="NewPhysics");
        CHECK(restored.get_size()==Salix::Vector3(3.f,4.f,5.f));
    }

    TEST_CASE("deserialization does not transfer a runtime-only owner pointer") {
        Salix::Entity entity;
        Salix::BoxCollider original;original.set_owner(&entity);
        std::stringstream bytes;{cereal::BinaryOutputArchive out(bytes);out(original);}
        Salix::BoxCollider restored;{cereal::BinaryInputArchive in(bytes);in(restored);}
        CHECK(restored.get_owner()==nullptr);
        CHECK(original.get_owner()==&entity);
    }

    TEST_CASE("collision volume updates do not change the element identity") {
        Salix::BoxCollider collider;
        const auto id=collider.get_id();
        collider.set_size(Salix::Vector3(-1.f,7.f,0.f));
        collider.set_size(Salix::Vector3(12.f,14.f,16.f));
        CHECK(collider.get_id()==id);
        CHECK(collider.get_size()==Salix::Vector3(12.f,14.f,16.f));
    }

    TEST_CASE("component visibility remains independent of collision size") {
        Salix::BoxCollider collider;collider.set_size(Salix::Vector3(8.f,2.f,1.f));
        collider.set_visibility(false);collider.update(0.5f);
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size()==Salix::Vector3(8.f,2.f,1.f));
        collider.set_visibility(true);
        CHECK(collider.is_visible());
    }

    TEST_CASE("two collider instances do not share their mutable size storage") {
        Salix::BoxCollider a,b;
        a.set_size(Salix::Vector3(2.f,4.f,6.f));
        b.set_size(Salix::Vector3(5.f,7.f,9.f));
        CHECK(a.get_size()!=b.get_size());
        b.set_size(a.get_size());
        CHECK(a.get_size()==b.get_size());
        a.set_size(Salix::Vector3(0.f));
        CHECK(b.get_size()==Salix::Vector3(2.f,4.f,6.f));
    }

    TEST_CASE("load without a renderer does not reset an explicitly assigned size") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(1.125f,-2.25f,9.5f));
        Salix::InitContext context{};
        collider.on_load(context);
        CHECK(collider.get_size()==Salix::Vector3(1.125f,-2.25f,9.5f));
    }

    TEST_CASE("negative and zero time updates preserve an asymmetric volume") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(0.f,-8.f,16.f));
        for(float delta:{-2.f,0.f,0.016f,1.f})collider.update(delta);
        CHECK(collider.get_size()==Salix::Vector3(0.f,-8.f,16.f));
    }

    TEST_CASE("binary deserialization keeps new GUID and label but no old runtime visibility") {
        Salix::BoxCollider original;original.set_name("SavedBounds");
        original.set_id(Salix::SimpleGuid::from_value(987654ULL));
        original.set_size(Salix::Vector3(6.f,7.f,8.f));
        std::stringstream bytes;{cereal::BinaryOutputArchive out(bytes);out(original);}
        Salix::BoxCollider restored;restored.set_visibility(false);
        {cereal::BinaryInputArchive in(bytes);in(restored);}
        CHECK(restored.get_id()==original.get_id());
        CHECK(restored.get_name()=="SavedBounds");
        CHECK(restored.get_size()==Salix::Vector3(6.f,7.f,8.f));
        CHECK_FALSE(restored.is_visible());
    }
}

#include <cmath>
#include <limits>
TEST_SUITE("Salix::ecs::BoxColliderBatch947") {
    TEST_CASE("box collider class identity remains BoxCollider after display-name changes") {
        Salix::BoxCollider collider;
        collider.set_name("Player Hurtbox");
        CHECK(std::string(collider.get_class_name()) == "BoxCollider");
        CHECK(collider.get_name() == "Player Hurtbox");
    }

    TEST_CASE("changing collision size preserves identity name and visibility") {
        Salix::BoxCollider collider;
        collider.set_name("Bounds");
        collider.set_visibility(false);
        const auto id = collider.get_id();
        collider.set_size(Salix::Vector3(-3.0f, 8.0f, 0.5f));
        CHECK(collider.get_id() == id);
        CHECK(collider.get_name() == "Bounds");
        CHECK_FALSE(collider.is_visible());
    }

    TEST_CASE("size getter reference keeps stable storage across setter calls") {
        Salix::BoxCollider collider;
        const Salix::Vector3* address = &collider.get_size();
        collider.set_size(Salix::Vector3(2.0f, 3.0f, 4.0f));
        CHECK(&collider.get_size() == address);
        CHECK(*address == Salix::Vector3(2.0f, 3.0f, 4.0f));
    }

    TEST_CASE("signed zero collision dimensions preserve their floating point sign") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(-0.0f, 0.0f, -0.0f));
        const auto& size = collider.get_size();
        CHECK(std::signbit(size.x));
        CHECK_FALSE(std::signbit(size.y));
        CHECK(std::signbit(size.z));
    }

    TEST_CASE("maximum finite collision dimensions are stored without clamping") {
        Salix::BoxCollider collider;
        const float largest = std::numeric_limits<float>::max();
        collider.set_size(Salix::Vector3(largest, -largest, largest));
        CHECK(collider.get_size().x == largest);
        CHECK(collider.get_size().y == -largest);
        CHECK(collider.get_size().z == largest);
    }

    TEST_CASE("repeated lifecycle calls leave collision geometry unchanged") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(1.25f, -4.5f, 9.75f));
        Salix::InitContext context{};
        collider.initialize();
        collider.on_load(context);
        collider.update(-1.0f);
        collider.update(0.0f);
        collider.shutdown();
        CHECK(collider.get_size() == Salix::Vector3(1.25f, -4.5f, 9.75f));
    }

    TEST_CASE("JSON roundtrip preserves an explicitly invalid collider identifier") {
        Salix::BoxCollider original;
        original.set_id(Salix::SimpleGuid::invalid());
        original.set_size(Salix::Vector3(7.0f, 8.0f, 9.0f));
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(original); }
        Salix::BoxCollider restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.get_id().is_valid());
        CHECK(restored.get_size() == original.get_size());
    }

    TEST_CASE("binary roundtrip preserves an exact zero-sized collider") {
        Salix::BoxCollider original;
        original.set_size(Salix::Vector3(0.0f, 0.0f, 0.0f));
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(original); }
        Salix::BoxCollider restored;
        restored.set_size(Salix::Vector3(9.0f));
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_size() == Salix::Vector3(0.0f));
    }

    TEST_CASE("reassigning runtime owner never changes collider geometry or identifier") {
        Salix::Entity first, second;
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(3.0f, 5.0f, 7.0f));
        const auto id = collider.get_id();
        collider.set_owner(&first);
        collider.set_owner(&second);
        CHECK(collider.get_owner() == &second);
        CHECK(collider.get_id() == id);
        CHECK(collider.get_size() == Salix::Vector3(3.0f, 5.0f, 7.0f));
    }

    TEST_CASE("equal initial collider sizes remain independent after one instance changes") {
        Salix::BoxCollider first, second;
        first.set_size(Salix::Vector3(4.0f, 4.0f, 4.0f));
        second.set_size(first.get_size());
        REQUIRE(first.get_size() == second.get_size());
        first.set_size(Salix::Vector3(-1.0f, -2.0f, -3.0f));
        CHECK(second.get_size() == Salix::Vector3(4.0f, 4.0f, 4.0f));
        CHECK(first.get_size() != second.get_size());
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch2247") {
    TEST_CASE("batch2247 fresh collider retains canonical unit size on every axis") {
        Salix::BoxCollider collider;
        CHECK(collider.get_size().x==doctest::Approx(1.0f));
        CHECK(collider.get_size().y==doctest::Approx(1.0f));
        CHECK(collider.get_size().z==doctest::Approx(1.0f));
    }
    TEST_CASE("batch2247 fractional asymmetric size is stored without normalization") {
        Salix::BoxCollider collider; collider.set_size(Salix::Vector3(0.125f,-2.5f,99.75f));
        CHECK(collider.get_size()==Salix::Vector3(0.125f,-2.5f,99.75f));
    }
    TEST_CASE("batch2247 repeated update calls preserve geometry at extreme delta values") {
        Salix::BoxCollider collider; collider.set_size(Salix::Vector3(2.f,3.f,4.f));
        collider.update(-1000.f); collider.update(0.f); collider.update(1000.f);
        CHECK(collider.get_size()==Salix::Vector3(2.f,3.f,4.f));
    }
    TEST_CASE("batch2247 initialize and null-renderer load leave explicit size unchanged") {
        Salix::BoxCollider collider; collider.set_size(Salix::Vector3(7.f,8.f,9.f)); Salix::InitContext context{};
        collider.initialize(); collider.on_load(context);
        CHECK(collider.get_size()==Salix::Vector3(7.f,8.f,9.f));
    }
    TEST_CASE("batch2247 changing size never alters canonical class identity") {
        Salix::BoxCollider collider; collider.set_name("Editor Label"); collider.set_size(Salix::Vector3(-4.f,5.f,6.f));
        CHECK(std::string(collider.get_class_name())=="BoxCollider"); CHECK(collider.get_name()=="Editor Label");
    }
    TEST_CASE("batch2247 owner pointer survives geometry replacement unchanged") {
        Salix::Entity owner; Salix::BoxCollider collider; collider.set_owner(&owner); collider.set_size(Salix::Vector3(12.f,13.f,14.f));
        CHECK(collider.get_owner()==&owner); CHECK(collider.get_size()==Salix::Vector3(12.f,13.f,14.f));
    }
    TEST_CASE("batch2247 visibility survives repeated size assignments") {
        Salix::BoxCollider collider; collider.set_visibility(false);
        collider.set_size(Salix::Vector3(1.f)); collider.set_size(Salix::Vector3(2.f));
        CHECK_FALSE(collider.is_visible()); CHECK(collider.get_size()==Salix::Vector3(2.f));
    }
    TEST_CASE("batch2247 JSON roundtrip replaces preexisting size and preserves display name") {
        Salix::BoxCollider source; source.set_name("Json2247"); source.set_size(Salix::Vector3(-1.25f,2.5f,5.f));
        std::stringstream stream; { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored; restored.set_name("Old"); restored.set_size(Salix::Vector3(99.f));
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_name()=="Json2247"); CHECK(restored.get_size()==source.get_size());
    }
    TEST_CASE("batch2247 binary roundtrip replaces preexisting size exactly") {
        Salix::BoxCollider source; source.set_size(Salix::Vector3(0.25f,-0.5f,1024.f));
        std::stringstream stream; { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored; restored.set_size(Salix::Vector3(-9.f));
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_size()==source.get_size());
    }
    TEST_CASE("batch2247 size reference address remains stable through lifecycle calls and setter updates") {
        Salix::BoxCollider collider; const auto* storage=&collider.get_size(); Salix::InitContext context{};
        collider.initialize(); collider.set_size(Salix::Vector3(3.f,4.f,5.f)); collider.on_load(context); collider.update(0.5f);
        CHECK(&collider.get_size()==storage); CHECK(*storage==Salix::Vector3(3.f,4.f,5.f));
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch2847") {
    TEST_CASE("batch2847 scalar constructor-style size assignments remain exact") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(1.25f,2.5f,5.0f));
        CHECK(collider.get_size()==Salix::Vector3(1.25f,2.5f,5.0f));
    }
    TEST_CASE("batch2847 repeated identical size assignments are idempotent") {
        Salix::BoxCollider collider;
        const Salix::Vector3 size(-3.0f,4.0f,9.0f);
        collider.set_size(size); collider.set_size(size);
        CHECK(collider.get_size()==size);
    }
    TEST_CASE("batch2847 zero size survives lifecycle calls unchanged") {
        Salix::BoxCollider collider; Salix::InitContext context{};
        collider.set_size(Salix::Vector3::Zero);
        collider.initialize(); collider.on_load(context); collider.update(1.0f);
        CHECK(collider.get_size()==Salix::Vector3::Zero);
    }
    TEST_CASE("batch2847 negative dimensions survive lifecycle calls unchanged") {
        Salix::BoxCollider collider; Salix::InitContext context{};
        collider.set_size(Salix::Vector3(-1.f,-2.f,-3.f));
        collider.initialize(); collider.on_load(context); collider.update(-5.0f);
        CHECK(collider.get_size()==Salix::Vector3(-1.f,-2.f,-3.f));
    }
    TEST_CASE("batch2847 owner removal never alters collider dimensions") {
        Salix::Entity owner; Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3(2.f,4.f,8.f)); collider.set_owner(&owner); collider.set_owner(nullptr);
        CHECK(collider.get_owner()==nullptr); CHECK(collider.get_size()==Salix::Vector3(2.f,4.f,8.f));
    }
    TEST_CASE("batch2847 display-name replacement never alters collider dimensions") {
        Salix::BoxCollider collider; collider.set_size(Salix::Vector3(7.f,6.f,5.f));
        collider.set_name("Collider 2847");
        CHECK(collider.get_name()=="Collider 2847"); CHECK(collider.get_size()==Salix::Vector3(7.f,6.f,5.f));
    }
    TEST_CASE("batch2847 visibility off and on preserves dimensions") {
        Salix::BoxCollider collider; collider.set_size(Salix::Vector3(3.f,6.f,9.f));
        collider.set_visibility(false); collider.set_visibility(true);
        CHECK(collider.is_visible()); CHECK(collider.get_size()==Salix::Vector3(3.f,6.f,9.f));
    }
    TEST_CASE("batch2847 JSON roundtrip preserves zero and negative size components") {
        Salix::BoxCollider source; source.set_size(Salix::Vector3(0.f,-4.f,2.5f));
        std::stringstream ss; { cereal::JSONOutputArchive out(ss); out(source); }
        Salix::BoxCollider restored; { cereal::JSONInputArchive in(ss); in(restored); }
        CHECK(restored.get_size()==source.get_size());
    }
    TEST_CASE("batch2847 binary roundtrip preserves display name and asymmetric size") {
        Salix::BoxCollider source; source.set_name("Binary Collider 2847"); source.set_size(Salix::Vector3(0.5f,4.f,32.f));
        std::stringstream ss; { cereal::BinaryOutputArchive out(ss); out(source); }
        Salix::BoxCollider restored; { cereal::BinaryInputArchive in(ss); in(restored); }
        CHECK(restored.get_name()=="Binary Collider 2847"); CHECK(restored.get_size()==source.get_size());
    }
    TEST_CASE("batch2847 size storage remains stable after repeated setter replacements") {
        Salix::BoxCollider collider; const Salix::Vector3* storage=&collider.get_size();
        collider.set_size(Salix::Vector3(1.f)); collider.set_size(Salix::Vector3(2.f)); collider.set_size(Salix::Vector3(3.f));
        CHECK(&collider.get_size()==storage); CHECK(*storage==Salix::Vector3(3.f));
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch3247") {
 TEST_CASE("batch3247 default size reference remains stable across read access") { Salix::BoxCollider c; const auto* p=&c.get_size(); CHECK(&c.get_size()==p); CHECK(*p==Salix::Vector3::One); }
 TEST_CASE("batch3247 size setter updates stable backing storage") { Salix::BoxCollider c; const auto* p=&c.get_size(); c.set_size({3.25f,-4.5f,8.75f}); CHECK(&c.get_size()==p); CHECK(*p==Salix::Vector3(3.25f,-4.5f,8.75f)); }
 TEST_CASE("batch3247 initialize preserves identifier name and geometry") { Salix::BoxCollider c; c.set_name("Collider3247"); c.set_size({2.f,3.f,4.f}); const auto id=c.get_id(); c.initialize(); CHECK(c.get_id()==id); CHECK(c.get_name()=="Collider3247"); CHECK(c.get_size()==Salix::Vector3(2.f,3.f,4.f)); }
 TEST_CASE("batch3247 update with diagnostic delta preserves geometry") { Salix::BoxCollider c; c.set_size({-7.f,0.f,9.f}); c.update(-3247.f); CHECK(c.get_size()==Salix::Vector3(-7.f,0.f,9.f)); }
 TEST_CASE("batch3247 null renderer load preserves geometry and metadata") { Salix::BoxCollider c; Salix::InitContext context{}; c.set_name("Load3247"); c.set_size({1.f,5.f,25.f}); c.on_load(context); CHECK(c.get_name()=="Load3247"); CHECK(c.get_size()==Salix::Vector3(1.f,5.f,25.f)); }
 TEST_CASE("batch3247 owner and visibility changes are independent of size") { Salix::Entity owner; Salix::BoxCollider c; c.set_size({2.f,4.f,6.f}); c.set_owner(&owner); c.set_visibility(false); CHECK(c.get_owner()==&owner); CHECK_FALSE(c.is_visible()); CHECK(c.get_size()==Salix::Vector3(2.f,4.f,6.f)); }
 TEST_CASE("batch3247 zero negative and fractional components are stored verbatim") { Salix::BoxCollider c; c.set_size({0.f,-0.125f,3247.5f}); CHECK(c.get_size()==Salix::Vector3(0.f,-0.125f,3247.5f)); }
 TEST_CASE("batch3247 repeated identical size assignment is idempotent") { Salix::BoxCollider c; const Salix::Vector3 v(11.f,22.f,33.f); c.set_size(v); c.set_size(v); c.set_size(v); CHECK(c.get_size()==v); }
 TEST_CASE("batch3247 JSON roundtrip preserves name and geometry while visibility remains runtime local") { Salix::BoxCollider s; s.set_name("Json3247"); s.set_visibility(true); s.set_size({-1.f,2.5f,9.f}); std::stringstream ss; {cereal::JSONOutputArchive out(ss); out(s);} Salix::BoxCollider d; d.set_visibility(false); {cereal::JSONInputArchive in(ss); in(d);} CHECK(d.get_name()=="Json3247"); CHECK_FALSE(d.is_visible()); CHECK(d.get_size()==s.get_size()); }
 TEST_CASE("batch3247 two colliders remain independent after copying size value") { Salix::BoxCollider a,b; a.set_size({4.f,5.f,6.f}); b.set_size(a.get_size()); a.set_size({7.f,8.f,9.f}); CHECK(b.get_size()==Salix::Vector3(4.f,5.f,6.f)); CHECK(a.get_size()!=b.get_size()); }
}
TEST_SUITE("Salix::ecs::BoxColliderBatch3647") {
    TEST_CASE("batch3647 fresh collider combines canonical identity with unit geometry") {
        Salix::BoxCollider collider;
        CHECK(std::string(collider.get_class_name()) == "BoxCollider");
        CHECK(collider.get_name() == "BoxCollider");
        CHECK(collider.get_size() == Salix::Vector3::One);
    }

    TEST_CASE("batch3647 size reference remains stable while finite and infinite components are replaced") {
        Salix::BoxCollider collider;
        const Salix::Vector3* storage = &collider.get_size();
        collider.set_size(Salix::Vector3(
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            0.0f));
        CHECK(&collider.get_size() == storage);
        CHECK(std::isinf(storage->x));
        CHECK(std::isinf(storage->y));
        CHECK(storage->z == 0.0f);
    }

    TEST_CASE("batch3647 entity identifier replacement never changes collider geometry") {
        Salix::BoxCollider collider;
        collider.set_size({3.0f, 6.0f, 9.0f});
        collider.set_id(Salix::SimpleGuid::from_value(3647001ULL));
        CHECK(collider.get_id().get_value() == 3647001ULL);
        CHECK(collider.get_size() == Salix::Vector3(3.0f, 6.0f, 9.0f));
    }

    TEST_CASE("batch3647 customized display name survives geometry replacements") {
        Salix::BoxCollider collider;
        collider.set_name("Collision Volume 3647");
        collider.set_size({1.0f, 2.0f, 3.0f});
        collider.set_size({4.0f, 5.0f, 6.0f});
        CHECK(collider.get_name() == "Collision Volume 3647");
        CHECK(collider.get_size() == Salix::Vector3(4.0f, 5.0f, 6.0f));
    }

    TEST_CASE("batch3647 owner attach and detach preserve exact geometry") {
        Salix::Entity owner;
        Salix::BoxCollider collider;
        collider.set_size({-2.0f, 0.5f, 8.0f});
        collider.set_owner(&owner);
        CHECK(collider.get_owner() == &owner);
        collider.set_owner(nullptr);
        CHECK(collider.get_owner() == nullptr);
        CHECK(collider.get_size() == Salix::Vector3(-2.0f, 0.5f, 8.0f));
    }

    TEST_CASE("batch3647 lifecycle sequence preserves identifier display name visibility and geometry") {
        Salix::BoxCollider collider;
        Salix::InitContext context{};
        collider.set_name("Lifecycle3647");
        collider.set_visibility(false);
        collider.set_size({7.0f, -8.0f, 9.5f});
        const auto id = collider.get_id();
        collider.initialize();
        collider.on_load(context);
        collider.update(-3647.0f);
        CHECK(collider.get_id() == id);
        CHECK(collider.get_name() == "Lifecycle3647");
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(7.0f, -8.0f, 9.5f));
    }

    TEST_CASE("batch3647 JSON roundtrip preserves explicit identifier display name and geometry") {
        Salix::BoxCollider source;
        source.set_id(Salix::SimpleGuid::from_value(3647002ULL));
        source.set_name("Json Collider 3647");
        source.set_size({-1.25f, 2.5f, 10.0f});
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_name() == source.get_name());
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch3647 binary roundtrip preserves explicit identifier name and asymmetric geometry") {
        Salix::BoxCollider source;
        source.set_id(Salix::SimpleGuid::from_value(3647003ULL));
        source.set_name("Binary Collider 3647");
        source.set_size({0.125f, -64.0f, 1024.5f});
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_name() == source.get_name());
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch3647 deserialization leaves destination runtime owner and visibility local") {
        Salix::BoxCollider source;
        source.set_name("Serialized3647");
        source.set_visibility(true);
        source.set_size({4.0f, 8.0f, 16.0f});
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }

        Salix::Entity owner;
        Salix::BoxCollider restored;
        restored.set_owner(&owner);
        restored.set_visibility(false);
        { cereal::JSONInputArchive in(stream); in(restored); }

        CHECK(restored.get_owner() == &owner);
        CHECK_FALSE(restored.is_visible());
        CHECK(restored.get_name() == "Serialized3647");
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch3647 two colliders retain independent size storage addresses and values") {
        Salix::BoxCollider first, second;
        const auto* first_storage = &first.get_size();
        const auto* second_storage = &second.get_size();
        CHECK(first_storage != second_storage);
        first.set_size({1.0f, 3.0f, 5.0f});
        second.set_size({2.0f, 4.0f, 6.0f});
        CHECK(*first_storage == Salix::Vector3(1.0f, 3.0f, 5.0f));
        CHECK(*second_storage == Salix::Vector3(2.0f, 4.0f, 6.0f));
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch4047") {
    TEST_CASE("batch4047 fresh collider starts visible ownerless and unit sized") {
        Salix::BoxCollider collider;
        CHECK(collider.get_owner() == nullptr);
        CHECK(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3::One);
    }

    TEST_CASE("batch4047 assigning canonical zero size preserves class and display identity") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3::Zero);
        CHECK(collider.get_size() == Salix::Vector3::Zero);
        CHECK(std::string(collider.get_class_name()) == "BoxCollider");
        CHECK(collider.get_name() == "BoxCollider");
    }

    TEST_CASE("batch4047 identifier replacement leaves customized name and size untouched") {
        Salix::BoxCollider collider;
        collider.set_name("Collider4047");
        collider.set_size({4.0f, 0.0f, -7.0f});
        collider.set_id(Salix::SimpleGuid::from_value(4047001ULL));
        CHECK(collider.get_id().get_value() == 4047001ULL);
        CHECK(collider.get_name() == "Collider4047");
        CHECK(collider.get_size() == Salix::Vector3(4.0f, 0.0f, -7.0f));
    }

    TEST_CASE("batch4047 owner attach detach cycle preserves geometry and visibility") {
        Salix::Entity owner;
        Salix::BoxCollider collider;
        collider.set_size({2.0f, 3.0f, 5.0f});
        collider.set_visibility(false);
        collider.set_owner(&owner);
        collider.set_owner(nullptr);
        CHECK(collider.get_owner() == nullptr);
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(2.0f, 3.0f, 5.0f));
    }

    TEST_CASE("batch4047 repeated lifecycle calls preserve stable size storage") {
        Salix::BoxCollider collider;
        Salix::InitContext context{};
        const auto* storage = &collider.get_size();
        collider.set_size({1.25f, -2.5f, 10.0f});
        collider.initialize();
        collider.on_load(context);
        collider.update(0.0f);
        collider.update(-1.0f);
        CHECK(&collider.get_size() == storage);
        CHECK(*storage == Salix::Vector3(1.25f, -2.5f, 10.0f));
    }

    TEST_CASE("batch4047 repeated visibility toggles never change collider geometry") {
        Salix::BoxCollider collider;
        collider.set_size({9.0f, 8.0f, 7.0f});
        collider.set_visibility(false);
        collider.set_visibility(true);
        collider.set_visibility(false);
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(9.0f, 8.0f, 7.0f));
    }

    TEST_CASE("batch4047 JSON roundtrip preserves invalid identifier and zero geometry") {
        Salix::BoxCollider source;
        source.set_id(Salix::SimpleGuid::invalid());
        source.set_name("Json4047");
        source.set_size(Salix::Vector3::Zero);
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.get_id().is_valid());
        CHECK(restored.get_name() == "Json4047");
        CHECK(restored.get_size() == Salix::Vector3::Zero);
    }

    TEST_CASE("batch4047 JSON roundtrip replaces destination metadata and geometry") {
        Salix::BoxCollider source;
        source.set_id(Salix::SimpleGuid::from_value(4047002ULL));
        source.set_name("Source4047");
        source.set_size({-0.5f, 4.0f, 32.0f});
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        restored.set_id(Salix::SimpleGuid::from_value(1));
        restored.set_name("Destination");
        restored.set_size({99.0f, 99.0f, 99.0f});
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_name() == source.get_name());
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch4047 two colliders maintain independent display names and geometry") {
        Salix::BoxCollider first, second;
        first.set_name("First4047");
        second.set_name("Second4047");
        first.set_size({1.0f, 2.0f, 3.0f});
        second.set_size({4.0f, 5.0f, 6.0f});
        CHECK(first.get_name() == "First4047");
        CHECK(second.get_name() == "Second4047");
        CHECK(first.get_size() != second.get_size());
    }

    TEST_CASE("batch4047 fresh colliders receive independent valid element identifiers") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::BoxCollider first;
        Salix::BoxCollider second;
        CHECK(first.get_id().is_valid());
        CHECK(second.get_id().is_valid());
        CHECK(first.get_id() != second.get_id());
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch4347") {
    TEST_CASE("batch4347 zero geometry assignment remains exact and address stable") {
        Salix::BoxCollider collider;
        const auto* storage = &collider.get_size();
        collider.set_size(Salix::Vector3::Zero);
        CHECK(&collider.get_size() == storage);
        CHECK(collider.get_size() == Salix::Vector3::Zero);
    }

    TEST_CASE("batch4347 negative geometry components are preserved verbatim") {
        Salix::BoxCollider collider;
        collider.set_size({-1.0f, -2.5f, -4347.0f});
        CHECK(collider.get_size() == Salix::Vector3(-1.0f, -2.5f, -4347.0f));
    }

    TEST_CASE("batch4347 fractional geometry replacement overwrites all three axes") {
        Salix::BoxCollider collider;
        collider.set_size({1.0f, 2.0f, 3.0f});
        collider.set_size({0.125f, -0.25f, 4.347f});
        CHECK(collider.get_size() == Salix::Vector3(0.125f, -0.25f, 4.347f));
    }

    TEST_CASE("batch4347 display name and visibility stay independent from geometry") {
        Salix::BoxCollider collider;
        collider.set_name("Collider Label 4347");
        collider.set_visibility(false);
        collider.set_size({4.0f, 3.0f, 47.0f});
        CHECK(collider.get_name() == "Collider Label 4347");
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(4.0f, 3.0f, 47.0f));
    }

    TEST_CASE("batch4347 owner replacement preserves geometry and identifier") {
        Salix::Entity first, second;
        Salix::BoxCollider collider;
        collider.set_size({4.0f, 34.0f, 7.0f});
        const auto id = collider.get_id();
        collider.set_owner(&first);
        collider.set_owner(&second);
        CHECK(collider.get_owner() == &second);
        CHECK(collider.get_id() == id);
        CHECK(collider.get_size() == Salix::Vector3(4.0f, 34.0f, 7.0f));
    }

    TEST_CASE("batch4347 initialize and null renderer load preserve customized geometry") {
        Salix::BoxCollider collider;
        Salix::InitContext context{};
        collider.set_size({8.0f, 6.0f, 4.0f});
        collider.initialize();
        collider.on_load(context);
        CHECK(collider.get_size() == Salix::Vector3(8.0f, 6.0f, 4.0f));
    }

    TEST_CASE("batch4347 binary roundtrip preserves zero size and custom name") {
        Salix::BoxCollider source;
        source.set_name("Binary Zero 4347");
        source.set_size(Salix::Vector3::Zero);
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_name() == "Binary Zero 4347");
        CHECK(restored.get_size() == Salix::Vector3::Zero);
    }

    TEST_CASE("batch4347 JSON roundtrip preserves negative fractional geometry") {
        Salix::BoxCollider source;
        source.set_size({-0.125f, 4.347f, -64.5f});
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch4347 deserialization leaves destination owner pointer untouched") {
        Salix::BoxCollider source;
        source.set_size({1.0f, 4.0f, 7.0f});
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::Entity owner;
        Salix::BoxCollider restored;
        restored.set_owner(&owner);
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_owner() == &owner);
        CHECK(restored.get_size() == source.get_size());
    }

    TEST_CASE("batch4347 separate collider size references never alias") {
        Salix::BoxCollider first, second;
        CHECK(&first.get_size() != &second.get_size());
        first.set_size({1.0f, 2.0f, 3.0f});
        second.set_size({3.0f, 2.0f, 1.0f});
        CHECK(first.get_size() != second.get_size());
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch4847") {
    TEST_CASE("batch4847 setter copies caller vector storage independently") {
        Salix::BoxCollider collider;
        Salix::Vector3 source(4.0f, -8.0f, 47.0f);
        collider.set_size(source);
        source = Salix::Vector3::Zero;
        CHECK(collider.get_size() == Salix::Vector3(4.0f, -8.0f, 47.0f));
    }

    TEST_CASE("batch4847 positive and negative infinity size components are preserved") {
        Salix::BoxCollider collider;
        const float inf = std::numeric_limits<float>::infinity();
        collider.set_size({inf, -inf, 1.0f});
        CHECK(std::isinf(collider.get_size().x));
        CHECK(collider.get_size().x > 0.0f);
        CHECK(std::isinf(collider.get_size().y));
        CHECK(collider.get_size().y < 0.0f);
        CHECK(collider.get_size().z == 1.0f);
    }

    TEST_CASE("batch4847 NaN size component remains NaN without affecting peer axes") {
        Salix::BoxCollider collider;
        collider.set_size({2.0f, std::numeric_limits<float>::quiet_NaN(), -3.0f});
        CHECK(collider.get_size().x == 2.0f);
        CHECK(std::isnan(collider.get_size().y));
        CHECK(collider.get_size().z == -3.0f);
    }

    TEST_CASE("batch4847 infinite update delta never changes stored geometry") {
        Salix::BoxCollider collider;
        collider.set_size({4.847f, -48.47f, 484.7f});
        collider.update(std::numeric_limits<float>::infinity());
        CHECK(collider.get_size() == Salix::Vector3(4.847f, -48.47f, 484.7f));
    }

    TEST_CASE("batch4847 repeated initialize calls preserve identifier name visibility and geometry") {
        Salix::BoxCollider collider;
        collider.set_name("RepeatedInit4847");
        collider.set_visibility(false);
        collider.set_size({1.0f, 4.0f, 7.0f});
        const auto id = collider.get_id();
        collider.initialize();
        collider.initialize();
        CHECK(collider.get_id() == id);
        CHECK(collider.get_name() == "RepeatedInit4847");
        CHECK_FALSE(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(1.0f, 4.0f, 7.0f));
    }

    TEST_CASE("batch4847 null renderer on load is repeatable and geometry neutral") {
        Salix::BoxCollider collider;
        Salix::InitContext context{};
        collider.set_size({8.0f, 4.0f, 7.0f});
        CHECK_NOTHROW(collider.on_load(context));
        CHECK_NOTHROW(collider.on_load(context));
        CHECK(collider.get_size() == Salix::Vector3(8.0f, 4.0f, 7.0f));
    }

    TEST_CASE("batch4847 binary roundtrip preserves maximum finite geometry") {
        Salix::BoxCollider source;
        const float max = (std::numeric_limits<float>::max)();
        source.set_size({max, -max, max});
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_size().x == max);
        CHECK(restored.get_size().y == -max);
        CHECK(restored.get_size().z == max);
    }

    TEST_CASE("batch4847 binary roundtrip preserves signed zero geometry components") {
        Salix::BoxCollider source;
        source.set_size({-0.0f, 0.0f, -0.0f});
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::BoxCollider restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(std::signbit(restored.get_size().x));
        CHECK_FALSE(std::signbit(restored.get_size().y));
        CHECK(std::signbit(restored.get_size().z));
    }

    TEST_CASE("batch4847 changing identifier repeatedly never changes geometry storage address") {
        Salix::BoxCollider collider;
        collider.set_size({4.0f, 8.0f, 47.0f});
        const auto* address = &collider.get_size();
        collider.set_id(Salix::SimpleGuid::from_value(4847001ULL));
        collider.set_id(Salix::SimpleGuid::from_value(4847002ULL));
        CHECK(&collider.get_size() == address);
        CHECK(*address == Salix::Vector3(4.0f, 8.0f, 47.0f));
    }

    TEST_CASE("batch4847 two colliders can hold opposite infinite geometry independently") {
        Salix::BoxCollider first, second;
        const float inf = std::numeric_limits<float>::infinity();
        first.set_size({inf, 1.0f, 2.0f});
        second.set_size({-inf, 3.0f, 4.0f});
        CHECK(std::isinf(first.get_size().x));
        CHECK(first.get_size().x > 0.0f);
        CHECK(std::isinf(second.get_size().x));
        CHECK(second.get_size().x < 0.0f);
        CHECK(&first.get_size() != &second.get_size());
    }
}

TEST_SUITE("Salix::ecs::BoxColliderBatch5147") {
    TEST_CASE("batch5147 default size reference remains stable after replacement") {
        Salix::BoxCollider collider;
        const auto* address = &collider.get_size();
        REQUIRE(*address == Salix::Vector3::One);
        collider.set_size({5.0f, 1.0f, 47.0f});
        CHECK(&collider.get_size() == address);
        CHECK(*address == Salix::Vector3(5.0f, 1.0f, 47.0f));
    }

    TEST_CASE("batch5147 zero geometry is stored exactly") {
        Salix::BoxCollider collider;
        collider.set_size(Salix::Vector3::Zero);
        CHECK(collider.get_size() == Salix::Vector3::Zero);
    }

    TEST_CASE("batch5147 signed geometry components remain independent") {
        Salix::BoxCollider collider;
        collider.set_size({-5.0f, 14.0f, -7.0f});
        CHECK(collider.get_size().x == -5.0f);
        CHECK(collider.get_size().y == 14.0f);
        CHECK(collider.get_size().z == -7.0f);
    }

    TEST_CASE("batch5147 finite update is geometry neutral") {
        Salix::BoxCollider collider;
        collider.set_size({5.147f, -51.47f, 514.7f});
        collider.update(0.016f);
        CHECK(collider.get_size() == Salix::Vector3(5.147f, -51.47f, 514.7f));
    }

    TEST_CASE("batch5147 repeated initialize is geometry neutral") {
        Salix::BoxCollider collider;
        collider.set_size({1.0f, 5.0f, 7.0f});
        collider.initialize();
        collider.initialize();
        CHECK(collider.get_size() == Salix::Vector3(1.0f, 5.0f, 7.0f));
    }

    TEST_CASE("batch5147 null renderer load leaves geometry unchanged") {
        Salix::BoxCollider collider;
        Salix::InitContext context{};
        collider.set_size({7.0f, 1.0f, 5.0f});
        collider.on_load(context);
        CHECK(collider.get_size() == Salix::Vector3(7.0f, 1.0f, 5.0f));
    }

    TEST_CASE("batch5147 visibility changes never rewrite geometry") {
        Salix::BoxCollider collider;
        collider.set_size({5.0f, 14.0f, 7.0f});
        collider.set_visibility(false);
        REQUIRE_FALSE(collider.is_visible());
        collider.set_visibility(true);
        CHECK(collider.is_visible());
        CHECK(collider.get_size() == Salix::Vector3(5.0f, 14.0f, 7.0f));
    }

    TEST_CASE("batch5147 display name changes never rewrite geometry") {
        Salix::BoxCollider collider;
        collider.set_size({5.0f, 1.0f, 47.0f});
        collider.set_name("Collider5147");
        CHECK(collider.get_name() == "Collider5147");
        CHECK(collider.get_size() == Salix::Vector3(5.0f, 1.0f, 47.0f));
    }

    TEST_CASE("batch5147 identifier changes never rewrite geometry") {
        Salix::BoxCollider collider;
        collider.set_size({5.0f, 14.0f, 7.0f});
        collider.set_id(Salix::SimpleGuid::from_value(5147001ULL));
        CHECK(collider.get_id().get_value() == 5147001ULL);
        CHECK(collider.get_size() == Salix::Vector3(5.0f, 14.0f, 7.0f));
    }

    TEST_CASE("batch5147 separate colliders own independent geometry storage") {
        Salix::BoxCollider first, second;
        first.set_size({1.0f, 2.0f, 3.0f});
        second.set_size({4.0f, 5.0f, 6.0f});
        CHECK(&first.get_size() != &second.get_size());
        CHECK(first.get_size() == Salix::Vector3(1.0f, 2.0f, 3.0f));
        CHECK(second.get_size() == Salix::Vector3(4.0f, 5.0f, 6.0f));
    }
}

