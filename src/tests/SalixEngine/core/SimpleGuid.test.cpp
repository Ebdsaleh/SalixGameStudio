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

// Additional cases preserved from the separate regression suite.
// Regression checks for persistent GUID identity, collision prevention and archives.
#include <doctest.h>
#include <Salix/core/SimpleGuid.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cstdint>
#include <set>
#include <unordered_set>
#include <sstream>

TEST_SUITE("Salix::core::SimpleGuidRegression") {
    TEST_CASE("guid default and invalid factories produce the same sentinel") {
        const Salix::SimpleGuid initial{};
        const auto invalid = Salix::SimpleGuid::invalid();
        CHECK_FALSE(initial.is_valid());
        CHECK_FALSE(invalid.is_valid());
        CHECK(initial == invalid);
        CHECK(initial.get_value() == 0ULL);
    }

    TEST_CASE("guid from_value zero remains invalid and roundtrips exactly") {
        const auto id = Salix::SimpleGuid::from_value(0ULL);
        CHECK_FALSE(id.is_valid());
        CHECK(id.get_value() == 0ULL);
        CHECK(id == Salix::SimpleGuid::invalid());
    }

    TEST_CASE("guid from_value preserves high 64-bit identifier bits") {
        const std::uint64_t raw = (std::uint64_t{1} << 55) + 123ULL;
        const auto id = Salix::SimpleGuid::from_value(raw);
        CHECK(id.is_valid());
        CHECK(id.get_value() == raw);
    }

    TEST_CASE("guid generated identifiers are unique and strictly increasing") {
        const auto a = Salix::SimpleGuid::generate();
        const auto b = Salix::SimpleGuid::generate();
        const auto c = Salix::SimpleGuid::generate();
        CHECK(a.is_valid());
        CHECK(a < b);
        CHECK(b < c);
        CHECK(a != c);
    }

    TEST_CASE("guid value equality is reflexive symmetric and differs by ID") {
        const auto a = Salix::SimpleGuid::from_value(17);
        const auto b = Salix::SimpleGuid::from_value(17);
        const auto c = Salix::SimpleGuid::from_value(18);
        CHECK(a == a);
        CHECK(a == b);
        CHECK(b == a);
        CHECK(a != c);
        CHECK_FALSE(a != b);
    }

    TEST_CASE("guid strict ordering is transitive for stable entity containers") {
        const auto low = Salix::SimpleGuid::from_value(2);
        const auto middle = Salix::SimpleGuid::from_value(10);
        const auto high = Salix::SimpleGuid::from_value(50);
        CHECK(low < middle);
        CHECK(middle < high);
        CHECK(low < high);
        CHECK_FALSE(high < low);
        CHECK_FALSE(low < low);
    }

    TEST_CASE("guid ordered set distinguishes IDs and deduplicates same values") {
        std::set<Salix::SimpleGuid> ids;
        ids.insert(Salix::SimpleGuid::from_value(9));
        ids.insert(Salix::SimpleGuid::from_value(2));
        ids.insert(Salix::SimpleGuid::from_value(9));
        REQUIRE(ids.size() == 2);
        CHECK(ids.begin()->get_value() == 2ULL);
        CHECK(ids.rbegin()->get_value() == 9ULL);
    }

    TEST_CASE("guid unordered lookup uses hash consistent with value equality") {
        std::unordered_set<Salix::SimpleGuid> ids;
        ids.insert(Salix::SimpleGuid::from_value(456));
        ids.insert(Salix::SimpleGuid::from_value(456));
        ids.insert(Salix::SimpleGuid::from_value(999));
        CHECK(ids.size() == 2);
        CHECK(ids.count(Salix::SimpleGuid::from_value(456)) == 1);
        CHECK(ids.count(Salix::SimpleGuid::from_value(457)) == 0);
    }

    TEST_CASE("guid updating next ID from a loaded high watermark prevents collisions") {
        const auto old = Salix::SimpleGuid::generate();
        const std::uint64_t high = old.get_value() + 10000ULL;
        Salix::SimpleGuid::update_next_id(high);
        const auto generated = Salix::SimpleGuid::generate();
        CHECK(generated.get_value() == high + 1ULL);
        CHECK(generated != old);
    }

    TEST_CASE("guid stale watermark never rewinds generation and both archive formats preserve identity") {
        const auto first = Salix::SimpleGuid::generate();
        Salix::SimpleGuid::update_next_id(first.get_value());
        const auto second = Salix::SimpleGuid::generate();
        CHECK(first < second);
        std::stringstream json;
        { cereal::JSONOutputArchive out(json); out(second); }
        Salix::SimpleGuid from_json;
        { cereal::JSONInputArchive in(json); in(from_json); }
        CHECK(from_json == second);
        std::stringstream binary;
        { cereal::BinaryOutputArchive out(binary); out(second); }
        Salix::SimpleGuid from_binary;
        { cereal::BinaryInputArchive in(binary); in(from_binary); }
        CHECK(from_binary == second);
    }
}
