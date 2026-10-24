#include <doctest.h>
#include <Salix/core/SimpleGuid.h>
#include <set>
#include <unordered_set>

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
