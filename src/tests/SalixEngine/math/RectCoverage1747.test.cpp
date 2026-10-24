#include <doctest.h>
#include <Salix/math/Rect.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <sstream>
#include <type_traits>
#include <utility>

TEST_SUITE("Salix::math::RectClosure1747") {
    TEST_CASE("batch1747 rect aliases preserve their intended scalar member types") {
        CHECK((std::is_same_v<decltype(Salix::RectI{}.x), int>));
        CHECK((std::is_same_v<decltype(Salix::RectF{}.x), float>));
        CHECK((std::is_same_v<decltype(Salix::RectD{}.x), double>));
    }

    TEST_CASE("batch1747 rect converting constructor is explicit while remaining constructible") {
        CHECK((std::is_constructible_v<Salix::RectI, const Salix::RectF&>));
        CHECK_FALSE((std::is_convertible_v<Salix::RectF, Salix::RectI>));
        CHECK((std::is_constructible_v<Salix::RectD, const Salix::RectI&>));
    }

    TEST_CASE("batch1747 rect generic arithmetic constructor accepts bool and char members") {
        const Salix::RectI rect(true, false, static_cast<char>(7), static_cast<unsigned char>(9));
        CHECK(rect.x == 1);
        CHECK(rect.y == 0);
        CHECK(rect.w == 7);
        CHECK(rect.h == 9);
    }

    TEST_CASE("batch1747 rect mixed long long and unsigned construction converts each member independently") {
        const Salix::RectD rect(-123456789LL, 42ULL, -3.5f, 8);
        CHECK(rect.x == doctest::Approx(-123456789.0));
        CHECK(rect.y == doctest::Approx(42.0));
        CHECK(rect.w == doctest::Approx(-3.5));
        CHECK(rect.h == doctest::Approx(8.0));
    }

    TEST_CASE("batch1747 rect fractional to integer conversion truncates every component toward zero") {
        const Salix::RectD source(-0.999, 0.999, -7.999, 7.999);
        const Salix::RectI converted(source);
        CHECK(converted == Salix::RectI(0, 0, -7, 7));
    }

    TEST_CASE("batch1747 rect self assignment preserves all four members") {
        Salix::RectI rect(-5, 6, -7, 8);
        rect = rect;
        CHECK(rect == Salix::RectI(-5, 6, -7, 8));
    }

    TEST_CASE("batch1747 rect swapping values exchanges position and extents together") {
        Salix::RectF first(1.0f, 2.0f, 3.0f, 4.0f);
        Salix::RectF second(-1.0f, -2.0f, -3.0f, -4.0f);
        std::swap(first, second);
        CHECK(first == Salix::RectF(-1.0f, -2.0f, -3.0f, -4.0f));
        CHECK(second == Salix::RectF(1.0f, 2.0f, 3.0f, 4.0f));
    }

    TEST_CASE("batch1747 rect equality and inequality remain strict complements") {
        const Salix::RectD a(1.0, 2.0, 3.0, 4.0);
        const Salix::RectD b(1.0, 2.0, 3.0, 4.0);
        const Salix::RectD c(1.0, 2.0, 3.0, 4.5);
        CHECK(a == b);
        CHECK_FALSE(a != b);
        CHECK(a != c);
        CHECK_FALSE(a == c);
    }

    TEST_CASE("batch1747 rect JSON serialization is deterministic for identical values") {
        const Salix::RectI value(-12, 34, 56, -78);
        std::stringstream first, second;
        { cereal::JSONOutputArchive out(first); out(value); }
        { cereal::JSONOutputArchive out(second); out(value); }
        CHECK(first.str() == second.str());
    }

    TEST_CASE("batch1747 rect binary serialization is deterministic for identical double rectangles") {
        const Salix::RectD value(-0.125, 2.5, -16.75, 32.0);
        std::stringstream first, second;
        { cereal::BinaryOutputArchive out(first); out(value); }
        { cereal::BinaryOutputArchive out(second); out(value); }
        CHECK(first.str() == second.str());
    }
}
