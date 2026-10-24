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

// Persistent identity collections and 64-bit archive bounds.
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <vector>
#include <cereal/types/vector.hpp>
TEST_SUITE("Salix::core::SimpleGuidExtended") {
    TEST_CASE("guid extended high-bit UUID values remain distinct from their low 32-bit parts") {
        const auto high = Salix::SimpleGuid::from_value((uint64_t{1} << 48) | 12345ULL);
        const auto low = Salix::SimpleGuid::from_value(12345ULL);
        CHECK(high.is_valid());
        CHECK(high != low);
        CHECK(high.get_value() == ((uint64_t{1} << 48) | 12345ULL));
    }

    TEST_CASE("guid extended maximum unsigned ID is valid without creating or advancing a new ID") {
        const auto max_guid = Salix::SimpleGuid::from_value(std::numeric_limits<uint64_t>::max());
        CHECK(max_guid.is_valid());
        CHECK(max_guid.get_value() == std::numeric_limits<uint64_t>::max());
        CHECK(Salix::SimpleGuid::invalid() < max_guid);
    }

    TEST_CASE("guid extended invalid zero sorts below every valid identifier") {
        const auto none = Salix::SimpleGuid::invalid();
        const auto lowest = Salix::SimpleGuid::from_value(1ULL);
        const auto largest = Salix::SimpleGuid::from_value(9000ULL);
        CHECK(none < lowest);
        CHECK(lowest < largest);
        CHECK_FALSE(lowest < none);
    }

    TEST_CASE("guid extended copy construction retains original identity without linking storage") {
        const auto original = Salix::SimpleGuid::from_value(404ULL);
        Salix::SimpleGuid copy(original);
        CHECK(copy == original);
        copy = Salix::SimpleGuid::from_value(405ULL);
        CHECK(copy != original);
        CHECK(original.get_value() == 404ULL);
    }

    TEST_CASE("guid extended sorting unsorted identities yields numeric stable ascending order") {
        std::vector<Salix::SimpleGuid> items{
            Salix::SimpleGuid::from_value(99ULL),
            Salix::SimpleGuid::invalid(),
            Salix::SimpleGuid::from_value(7ULL),
            Salix::SimpleGuid::from_value(14ULL)};
        std::sort(items.begin(), items.end());
        REQUIRE(items.size() == 4);
        CHECK(items[0].get_value() == 0ULL);
        CHECK(items[1].get_value() == 7ULL);
        CHECK(items[2].get_value() == 14ULL);
        CHECK(items[3].get_value() == 99ULL);
    }

    TEST_CASE("guid extended unordered map retrieves payload by copied identity value") {
        std::unordered_map<Salix::SimpleGuid, std::string> names;
        const auto id = Salix::SimpleGuid::from_value(999U);
        names.emplace(id, "player");
        const auto duplicate = Salix::SimpleGuid::from_value(999U);
        CHECK(names.at(duplicate) == "player");
        CHECK(names.count(Salix::SimpleGuid::from_value(1000U)) == 0);
    }

    TEST_CASE("guid extended JSON archive roundtrip preserves an ID beyond signed 32-bit range") {
        auto original = Salix::SimpleGuid::from_value((uint64_t{1} << 33) + 17ULL);
        std::stringstream json;
        { cereal::JSONOutputArchive out(json); out(original); }
        Salix::SimpleGuid loaded;
        { cereal::JSONInputArchive in(json); in(loaded); }
        CHECK(loaded == original);
        CHECK(loaded.get_value() == ((uint64_t{1} << 33) + 17ULL));
    }

    TEST_CASE("guid extended binary archive roundtrip preserves maximum unsigned 64-bit value") {
        auto original = Salix::SimpleGuid::from_value(std::numeric_limits<uint64_t>::max());
        std::stringstream bytes;
        { cereal::BinaryOutputArchive out(bytes); out(original); }
        Salix::SimpleGuid loaded;
        { cereal::BinaryInputArchive in(bytes); in(loaded); }
        CHECK(loaded == original);
    }

    TEST_CASE("guid extended vector of IDs survives binary persistence in original order") {
        std::vector<Salix::SimpleGuid> source{
            Salix::SimpleGuid::from_value(7), Salix::SimpleGuid::invalid(),
            Salix::SimpleGuid::from_value((uint64_t{1} << 42) + 1ULL)};
        std::stringstream bytes;
        { cereal::BinaryOutputArchive out(bytes); out(source); }
        std::vector<Salix::SimpleGuid> loaded;
        { cereal::BinaryInputArchive in(bytes); in(loaded); }
        REQUIRE(loaded.size() == source.size());
        for (std::size_t i = 0; i < source.size(); ++i) CHECK(loaded[i] == source[i]);
    }

    TEST_CASE("guid extended vector of IDs survives JSON persistence including invalid sentinels") {
        std::vector<Salix::SimpleGuid> source{
            Salix::SimpleGuid::invalid(), Salix::SimpleGuid::from_value(125),
            Salix::SimpleGuid::from_value(500)};
        std::stringstream json;
        { cereal::JSONOutputArchive out(json); out(source); }
        std::vector<Salix::SimpleGuid> loaded;
        { cereal::JSONInputArchive in(json); in(loaded); }
        REQUIRE(loaded.size() == source.size());
        for (std::size_t i = 0; i < source.size(); ++i) CHECK(loaded[i] == source[i]);
    }
}

// Typed value and archive boundary tests added without touching runtime assets.
TEST_SUITE("Salix::Batch747::GuidBatch747") {
    TEST_CASE("guid invalid identity roundtrips independently via JSON and binary") {
        auto src=Salix::SimpleGuid::invalid();std::stringstream json,bin;{cereal::JSONOutputArchive out(json);out(src);}{cereal::BinaryOutputArchive out(bin);out(src);}Salix::SimpleGuid a,b;{cereal::JSONInputArchive in(json);in(a);}{cereal::BinaryInputArchive in(bin);in(b);}CHECK_FALSE(a.is_valid());CHECK_FALSE(b.is_valid());CHECK(a==b);
    }

    TEST_CASE("guid value constructor preserves a bit exactly at bit sixty three") {
        const auto v=Salix::SimpleGuid::from_value(uint64_t{1}<<63);CHECK(v.is_valid());CHECK(v.get_value()==(uint64_t{1}<<63));
    }

    TEST_CASE("guid nonadjacent high bits preserve strict ordering without signed comparisons") {
        const auto a=Salix::SimpleGuid::from_value(uint64_t{1}<<40),b=Salix::SimpleGuid::from_value(uint64_t{1}<<60);CHECK(a<b);CHECK_FALSE(b<a);CHECK(a!=b);
    }

    TEST_CASE("guid two independently decoded identical values compare equal and hash the same") {
        const auto a=Salix::SimpleGuid::from_value((uint64_t{1}<<52)+23ULL),b=Salix::SimpleGuid::from_value((uint64_t{1}<<52)+23ULL);CHECK(a==b);CHECK(std::hash<Salix::SimpleGuid>{}(a)==std::hash<Salix::SimpleGuid>{}(b));
    }

    TEST_CASE("guid invalid key is distinct from a valid key in unordered maps") {
        std::unordered_map<Salix::SimpleGuid,int> counts;counts[Salix::SimpleGuid::invalid()]=4;counts[Salix::SimpleGuid::from_value(1ULL)]=7;REQUIRE(counts.size()==2);CHECK(counts.at(Salix::SimpleGuid::invalid())==4);CHECK(counts.at(Salix::SimpleGuid::from_value(1ULL))==7);
    }

    TEST_CASE("guid ordered collection sorts values straddling the signed 64 bit boundary") {
        std::vector<Salix::SimpleGuid> values{Salix::SimpleGuid::from_value(uint64_t{1}<<63),Salix::SimpleGuid::from_value(3ULL),Salix::SimpleGuid::from_value((uint64_t{1}<<63)-1ULL)};std::sort(values.begin(),values.end());CHECK(values[0].get_value()==3ULL);CHECK(values[1].get_value()==((uint64_t{1}<<63)-1ULL));CHECK(values[2].get_value()==(uint64_t{1}<<63));
    }

    TEST_CASE("guid JSON sequence preserves repeated keys without merging identities") {
        std::vector<Salix::SimpleGuid> originals{Salix::SimpleGuid::from_value(100ULL),Salix::SimpleGuid::from_value(100ULL),Salix::SimpleGuid::from_value(200ULL)};std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(originals);}std::vector<Salix::SimpleGuid> result;{cereal::JSONInputArchive in(ss);in(result);}REQUIRE(result.size()==3);CHECK(result[0]==result[1]);CHECK(result[1]!=result[2]);
    }

    TEST_CASE("guid binary sequence retains zero and high bit IDs in separate positions") {
        std::vector<Salix::SimpleGuid> originals{Salix::SimpleGuid::invalid(),Salix::SimpleGuid::from_value(uint64_t{1}<<63),Salix::SimpleGuid::invalid()};std::stringstream ss;{cereal::BinaryOutputArchive out(ss);out(originals);}std::vector<Salix::SimpleGuid> decoded;{cereal::BinaryInputArchive in(ss);in(decoded);}REQUIRE(decoded.size()==3);CHECK(decoded[0]==decoded[2]);CHECK(decoded[1].is_valid());
    }

    TEST_CASE("guid copy assignment cannot change a previously retained original identity") {
        const auto source=Salix::SimpleGuid::from_value(54321ULL);auto copy=source;copy=Salix::SimpleGuid::invalid();CHECK(source.is_valid());CHECK(source.get_value()==54321ULL);CHECK_FALSE(copy.is_valid());
    }

    TEST_CASE("guid binary serialization bytes are stable for a fixed loaded identity") {
        auto id=Salix::SimpleGuid::from_value((uint64_t{1}<<43)+99ULL);std::stringstream a,b;{cereal::BinaryOutputArchive out(a);out(id);}{cereal::BinaryOutputArchive out(b);out(id);}CHECK(a.str()==b.str());
    }
}

TEST_SUITE("Salix::core::SimpleGuidClosure1747") {
    TEST_CASE("batch1747 simple guid reset makes first two generated identifiers one and two") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK(Salix::SimpleGuid::generate().get_value() == 1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 2ULL);
    }

    TEST_CASE("batch1747 simple guid updating with zero after reset does not skip first identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(0);
        CHECK(Salix::SimpleGuid::generate().get_value() == 1ULL);
    }

    TEST_CASE("batch1747 simple guid stale high watermark never rewinds an advanced counter") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK(Salix::SimpleGuid::generate().get_value() == 1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 2ULL);
        Salix::SimpleGuid::update_next_id(1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 3ULL);
    }

    TEST_CASE("batch1747 simple guid watermark equal to next identifier advances past it") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 2ULL);
    }

    TEST_CASE("batch1747 simple guid high watermark advances generation to exactly following value") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(5000ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 5001ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 5002ULL);
    }

    TEST_CASE("batch1747 simple guid from value does not advance generation counter") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto loaded = Salix::SimpleGuid::from_value(9000ULL);
        CHECK(loaded.get_value() == 9000ULL);
        CHECK(Salix::SimpleGuid::generate().get_value() == 1ULL);
    }

    TEST_CASE("batch1747 simple guid invalid factory does not advance generation counter") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK_FALSE(Salix::SimpleGuid::invalid().is_valid());
        CHECK(Salix::SimpleGuid::generate().get_value() == 1ULL);
    }

    TEST_CASE("batch1747 simple guid copied identity produces the same standard hash") {
        const auto original = Salix::SimpleGuid::from_value(0x123456789ABCDEF0ULL);
        const auto copy = original;
        const std::hash<Salix::SimpleGuid> hasher;
        CHECK(copy == original);
        CHECK(hasher(copy) == hasher(original));
    }

    TEST_CASE("batch1747 simple guid unordered set deduplicates copied identities by value") {
        const auto a = Salix::SimpleGuid::from_value(77ULL);
        const auto b = Salix::SimpleGuid::from_value(77ULL);
        const auto c = Salix::SimpleGuid::from_value(78ULL);
        std::unordered_set<Salix::SimpleGuid> values{a, b, c};
        CHECK(values.size() == 2);
        CHECK(values.count(a) == 1);
        CHECK(values.count(c) == 1);
    }

    TEST_CASE("batch1747 simple guid ordered set uses unsigned identifier ordering") {
        std::set<Salix::SimpleGuid> values{
            Salix::SimpleGuid::from_value(100ULL),
            Salix::SimpleGuid::from_value(2ULL),
            Salix::SimpleGuid::from_value(50ULL)
        };
        REQUIRE(values.size() == 3);
        auto it = values.begin();
        CHECK(it++->get_value() == 2ULL);
        CHECK(it++->get_value() == 50ULL);
        CHECK(it->get_value() == 100ULL);
    }
}

TEST_SUITE("Salix::core::SimpleGuidBatch2147") {
    TEST_CASE("batch2147 default and explicit zero GUIDs are both invalid and equal") {
        const Salix::SimpleGuid a;
        const auto b = Salix::SimpleGuid::from_value(0);
        CHECK_FALSE(a.is_valid());
        CHECK_FALSE(b.is_valid());
        CHECK(a == b);
    }
    TEST_CASE("batch2147 nonzero explicit GUID retains its complete unsigned value") {
        const auto guid = Salix::SimpleGuid::from_value(0x123456789abcdef0ULL);
        CHECK(guid.is_valid());
        CHECK(guid.get_value() == 0x123456789abcdef0ULL);
    }
    TEST_CASE("batch2147 equality and inequality are strict complements") {
        const auto a = Salix::SimpleGuid::from_value(77);
        const auto b = Salix::SimpleGuid::from_value(77);
        const auto c = Salix::SimpleGuid::from_value(78);
        CHECK(a == b);
        CHECK_FALSE(a != b);
        CHECK(a != c);
        CHECK_FALSE(a == c);
    }
    TEST_CASE("batch2147 ordering follows raw numeric identifier order") {
        CHECK(Salix::SimpleGuid::from_value(1) < Salix::SimpleGuid::from_value(2));
        CHECK_FALSE(Salix::SimpleGuid::from_value(2) < Salix::SimpleGuid::from_value(1));
        CHECK_FALSE(Salix::SimpleGuid::from_value(2) < Salix::SimpleGuid::from_value(2));
    }
    TEST_CASE("batch2147 invalid factory matches default constructed invalid identity") {
        const auto invalid = Salix::SimpleGuid::invalid();
        const Salix::SimpleGuid fresh;
        CHECK(invalid == fresh);
        CHECK(invalid.get_value() == 0);
    }
    TEST_CASE("batch2147 reset makes the next generated identifier one again") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto first = Salix::SimpleGuid::generate();
        CHECK(first.get_value() == 1);
        CHECK(first.is_valid());
    }
    TEST_CASE("batch2147 generation increments identifiers monotonically after reset") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto a = Salix::SimpleGuid::generate();
        const auto b = Salix::SimpleGuid::generate();
        const auto c = Salix::SimpleGuid::generate();
        CHECK(a.get_value() == 1);
        CHECK(b.get_value() == 2);
        CHECK(c.get_value() == 3);
    }
    TEST_CASE("batch2147 update next id advances generation beyond a higher known identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(500);
        CHECK(Salix::SimpleGuid::generate().get_value() == 501);
    }
    TEST_CASE("batch2147 update next id never moves an already advanced counter backwards") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(100);
        CHECK(Salix::SimpleGuid::generate().get_value() == 101);
        Salix::SimpleGuid::update_next_id(10);
        CHECK(Salix::SimpleGuid::generate().get_value() == 102);
    }
    TEST_CASE("batch2147 standard hash agrees for equal GUID values") {
        const auto a = Salix::SimpleGuid::from_value(9999);
        const auto b = Salix::SimpleGuid::from_value(9999);
        CHECK(std::hash<Salix::SimpleGuid>{}(a) == std::hash<Salix::SimpleGuid>{}(b));
    }
}

TEST_SUITE("Salix::core::SimpleGuidBatch2647") {
    TEST_CASE("batch2647 reset restores deterministic first generated identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK(Salix::SimpleGuid::generate().get_value()==1ULL);
    }
    TEST_CASE("batch2647 explicit from value never advances generation counter") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto explicit_id=Salix::SimpleGuid::from_value(9000ULL);
        CHECK(explicit_id.get_value()==9000ULL);
        CHECK(Salix::SimpleGuid::generate().get_value()==1ULL);
    }
    TEST_CASE("batch2647 update at current counter advances exactly one past supplied watermark") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value()==2ULL);
    }
    TEST_CASE("batch2647 stale zero watermark never changes fresh generation sequence") {
        Salix::SimpleGuid::reset_counter_for_testing();
        Salix::SimpleGuid::update_next_id(0ULL);
        CHECK(Salix::SimpleGuid::generate().get_value()==1ULL);
        CHECK(Salix::SimpleGuid::generate().get_value()==2ULL);
    }
    TEST_CASE("batch2647 equality remains value based across independently constructed identifiers") {
        const auto a=Salix::SimpleGuid::from_value(2647001ULL);
        const auto b=Salix::SimpleGuid::from_value(2647001ULL);
        CHECK(a==b); CHECK_FALSE(a!=b);
    }
    TEST_CASE("batch2647 strict ordering handles adjacent high values without signed interpretation") {
        const std::uint64_t high=(std::uint64_t{1}<<63);
        const auto a=Salix::SimpleGuid::from_value(high);
        const auto b=Salix::SimpleGuid::from_value(high+1ULL);
        CHECK(a<b); CHECK_FALSE(b<a);
    }
    TEST_CASE("batch2647 invalid sentinel remains strictly below generated valid identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto invalid=Salix::SimpleGuid::invalid();
        const auto valid=Salix::SimpleGuid::generate();
        CHECK(invalid<valid); CHECK_FALSE(valid<invalid);
    }
    TEST_CASE("batch2647 JSON roundtrip preserves a high-bit identifier exactly") {
        const auto original=Salix::SimpleGuid::from_value((std::uint64_t{1}<<60)+2647ULL);
        std::stringstream stream; { cereal::JSONOutputArchive out(stream); out(original); }
        Salix::SimpleGuid restored; { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored==original); CHECK(restored.get_value()==original.get_value());
    }
    TEST_CASE("batch2647 binary roundtrip preserves zero invalid sentinel") {
        const auto original=Salix::SimpleGuid::invalid();
        std::stringstream stream; { cereal::BinaryOutputArchive out(stream); out(original); }
        Salix::SimpleGuid restored=Salix::SimpleGuid::from_value(99ULL);
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored==original); CHECK_FALSE(restored.is_valid());
    }
    TEST_CASE("batch2647 hash remains stable across copied identifier values") {
        const auto a=Salix::SimpleGuid::from_value(0xABCDEF1234567890ULL);
        const auto b=a;
        CHECK(std::hash<Salix::SimpleGuid>{}(a)==std::hash<Salix::SimpleGuid>{}(b));
    }
}

TEST_SUITE("Salix::core::SimpleGuidBatch2747") {
    TEST_CASE("batch2747 explicit one identifier is valid without touching generator state") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto explicit_one=Salix::SimpleGuid::from_value(1);
        CHECK(explicit_one.is_valid());
        CHECK(Salix::SimpleGuid::generate().get_value()==1);
    }
    TEST_CASE("batch2747 two generated identifiers compare in creation order") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto first=Salix::SimpleGuid::generate();
        const auto second=Salix::SimpleGuid::generate();
        CHECK(first<second);
        CHECK_FALSE(second<first);
    }
    TEST_CASE("batch2747 updating watermark after one generated identifier skips forward") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK(Salix::SimpleGuid::generate().get_value()==1);
        Salix::SimpleGuid::update_next_id(10);
        CHECK(Salix::SimpleGuid::generate().get_value()==11);
    }
    TEST_CASE("batch2747 stale watermark equal to last issued id does not rewind") {
        Salix::SimpleGuid::reset_counter_for_testing();
        CHECK(Salix::SimpleGuid::generate().get_value()==1);
        CHECK(Salix::SimpleGuid::generate().get_value()==2);
        Salix::SimpleGuid::update_next_id(2);
        CHECK(Salix::SimpleGuid::generate().get_value()==3);
    }
    TEST_CASE("batch2747 maximum explicit identifier remains valid") {
        const auto id=Salix::SimpleGuid::from_value(std::numeric_limits<std::uint64_t>::max());
        CHECK(id.is_valid());
        CHECK(id.get_value()==std::numeric_limits<std::uint64_t>::max());
    }
    TEST_CASE("batch2747 copied invalid identifier remains invalid and equal") {
        const auto a=Salix::SimpleGuid::invalid();
        const auto b=a;
        CHECK_FALSE(b.is_valid());
        CHECK(a==b);
    }
    TEST_CASE("batch2747 adjacent explicit identifiers are unequal") {
        const auto a=Salix::SimpleGuid::from_value(2747000ULL);
        const auto b=Salix::SimpleGuid::from_value(2747001ULL);
        CHECK(a!=b);
        CHECK_FALSE(a==b);
    }
    TEST_CASE("batch2747 JSON roundtrip preserves ordinary generated identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto original=Salix::SimpleGuid::generate();
        std::stringstream ss; { cereal::JSONOutputArchive out(ss); out(original); }
        Salix::SimpleGuid restored; { cereal::JSONInputArchive in(ss); in(restored); }
        CHECK(restored==original);
    }
    TEST_CASE("batch2747 binary roundtrip preserves ordinary generated identifier") {
        Salix::SimpleGuid::reset_counter_for_testing();
        const auto original=Salix::SimpleGuid::generate();
        std::stringstream ss; { cereal::BinaryOutputArchive out(ss); out(original); }
        Salix::SimpleGuid restored; { cereal::BinaryInputArchive in(ss); in(restored); }
        CHECK(restored==original);
    }
    TEST_CASE("batch2747 hash is stable for independently constructed equal identifiers") {
        const auto a=Salix::SimpleGuid::from_value(1001);
        const auto b=Salix::SimpleGuid::from_value(1001);
        CHECK(a==b);
        CHECK(std::hash<Salix::SimpleGuid>{}(a)==std::hash<Salix::SimpleGuid>{}(b));
    }
}

