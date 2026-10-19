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
