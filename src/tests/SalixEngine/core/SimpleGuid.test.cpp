// =================================================================================
// Filename:    src/Tests/SalixEngine/core/SimpleGuid.test.cpp
// Description: Contains unit tests for the SimpleGuid functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/core/SimpleGuid.h> // The class we are testing
#include <cereal/archives/json.hpp> // For the JSON archive
#include <sstream>                  // For std::stringstream

TEST_SUITE("Salix::core::SimpleGuid") {
    TEST_CASE("constructors") {
        Salix::SimpleGuid guid_1;
        
        CHECK(guid_1.get_value() == 0);
        SUBCASE("from_value") {
            Salix::SimpleGuid guid_2 = Salix::SimpleGuid::from_value(2);
            CHECK(guid_2.is_valid());
            CHECK_FALSE(guid_2 == Salix::SimpleGuid::invalid());
        }
    }
    TEST_CASE("update_next_id correctly manages the counter") {
        
        SUBCASE("fast-forwards the counter when the new ID is higher") {
            Salix::SimpleGuid::reset_counter_for_testing(); // Reset state
            Salix::SimpleGuid::generate(); // Next is 2

            Salix::SimpleGuid::update_next_id(100);

            CHECK(Salix::SimpleGuid::generate().get_value() == 101);
        }

        SUBCASE("ignores a lower ID value") {
            Salix::SimpleGuid::reset_counter_for_testing(); // Reset state
            Salix::SimpleGuid::generate(); // Next is 2
            Salix::SimpleGuid::generate(); // Next is 3
            Salix::SimpleGuid::generate(); // Next is 4

            Salix::SimpleGuid::update_next_id(2);

            CHECK(Salix::SimpleGuid::generate().get_value() == 4);
        }
    }
    TEST_CASE("generation and uniqueness") {
        Salix::SimpleGuid guid_1 = Salix::SimpleGuid::generate();
        Salix::SimpleGuid guid_2 = Salix::SimpleGuid::generate();

        CHECK(guid_1.is_valid() == true);
        CHECK(guid_2.is_valid() == true);
        CHECK_FALSE(guid_1 == guid_2);
        CHECK(guid_1 != guid_2);
        CHECK(guid_1 < guid_2);
    }
    TEST_CASE("'invalid()' equals zero") {
        Salix::SimpleGuid invalid_guid = Salix::SimpleGuid::invalid();
        CHECK(invalid_guid.get_value() == 0);
        CHECK_FALSE(invalid_guid.is_valid());
    }
    TEST_CASE("can be serialized and deserialized correctly") {
        // This test ensures Cereal can save and load the GUID's state.
        Salix::SimpleGuid::reset_counter_for_testing();

        // ARRANGE
        Salix::SimpleGuid original_guid = Salix::SimpleGuid::from_value(1234);
        std::stringstream ss;  // memory buffer

        // ACT (Serialization)
        { 
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(original_guid);
        }   // Archive goes out of scope here, flushing data to the stringstream

        // ACT (Deserialization)
        Salix::SimpleGuid deserialized_guid;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(deserialized_guid);
        }

        // ASSERT
        CHECK(original_guid.get_value() == deserialized_guid.get_value());
    }
};