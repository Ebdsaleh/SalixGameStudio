// =================================================================================
// Filename:    src/Tests/SalixEngine/ecs/Element.test.cpp
// Description: Contains unit tests for the base Element class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Entity.h> // Needed for owner tests
#include <Salix/core/SimpleGuid.h>
#include <memory>
#include <cereal/archives/json.hpp>
#include <sstream>

// A minimal, concrete class that inherits from Element for testing purposes.
class MockElement : public Salix::Element {
public:
    const char* get_class_name() const override { return "MockElement"; }
};

TEST_SUITE("Salix::ecs::Element") {

    TEST_CASE("initializes with correct default values") {
        // ARRANGE
        Salix::SimpleGuid::reset_counter_for_testing();
        MockElement element;

        // ASSERT
        CHECK(element.get_name() == "Element");
        CHECK(element.is_visible() == true);
        CHECK(element.get_owner() == nullptr);
        CHECK(element.get_id().get_value() == 1);
        CHECK(element.get_id().is_valid() == true);
    }

    TEST_CASE("getters and setters modify properties correctly") {
        // ARRANGE
        MockElement element;
        Salix::Entity mock_owner;

        // ACT & ASSERT for name
        element.set_name("New Name");
        CHECK(element.get_name() == "New Name");

        // ACT & ASSERT for visibility
        element.set_visibility(false);
        CHECK(element.is_visible() == false);

        // ACT & ASSERT for owner
        element.set_owner(&mock_owner);
        CHECK(element.get_owner() == &mock_owner);

        // ACT & ASSERT for ID
        Salix::SimpleGuid new_id = Salix::SimpleGuid::generate();
        element.set_id(new_id);
        CHECK(element.get_id() == new_id);
    }

    TEST_CASE("lifecycle methods run without crashing") {
        // ARRANGE
        MockElement element;
        Salix::InitContext context; // Default context is fine

        // ACT & ASSERT: The test passes if these virtual methods can be
        // called on the base class without throwing an exception.
        CHECK_NOTHROW(element.initialize());
        CHECK_NOTHROW(element.on_load(context));
        CHECK_NOTHROW(element.update(0.016f));
        CHECK_NOTHROW(element.shutdown());
    }

    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        auto original_element = std::make_unique<MockElement>();
        original_element->set_name("SerializedElement");
        // We'll use the ID it was generated with for the check.
        Salix::SimpleGuid original_id = original_element->get_id();

        std::stringstream ss;

        // ACT (Serialization)
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(*original_element);
        }

        // ACT (Deserialization)
        auto loaded_element = std::make_unique<MockElement>();
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(*loaded_element);
        }

        // ASSERT
        REQUIRE(loaded_element != nullptr);
        CHECK(loaded_element->get_name() == "SerializedElement");
        // The ID should also be restored from the serialized data.
        CHECK(loaded_element->get_id() == original_id);
    }
}