#include <doctest.h>
#include <Salix/math/Vector2.h>
#include <cmath>

TEST_SUITE("Salix::math::Vector2Closure1747") {
    TEST_CASE("batch1747 vector2 normalization converts three four vector to unit length") {
        Salix::Vector2 value(3.0f, 4.0f);
        value.normalize();
        CHECK(value.x == doctest::Approx(0.6f));
        CHECK(value.y == doctest::Approx(0.8f));
        CHECK(value.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch1747 vector2 normalization leaves exact zero vector unchanged") {
        Salix::Vector2 value(0.0f, 0.0f);
        value.normalize();
        CHECK(value.x == 0.0f);
        CHECK(value.y == 0.0f);
        CHECK(value.length() == 0.0f);
    }

    TEST_CASE("batch1747 vector2 lerp clamps fractions below zero and above one") {
        const Salix::Vector2 start(-2.0f, 8.0f);
        const Salix::Vector2 end(6.0f, -4.0f);
        CHECK(Salix::Vector2::lerp(start, end, -10.0f) == start);
        CHECK(Salix::Vector2::lerp(start, end, 10.0f) == end);
    }

    TEST_CASE("batch1747 vector2 lerp reversed endpoints mirrors complementary interpolation fraction") {
        const Salix::Vector2 a(-5.0f, 3.0f);
        const Salix::Vector2 b(7.0f, 11.0f);
        const auto forward = Salix::Vector2::lerp(a, b, 0.25f);
        const auto backward = Salix::Vector2::lerp(b, a, 0.75f);
        CHECK(forward.x == doctest::Approx(backward.x));
        CHECK(forward.y == doctest::Approx(backward.y));
    }

    TEST_CASE("batch1747 vector2 component division zeroes only axes with zero divisor") {
        const auto result = Salix::Vector2(8.0f, 9.0f) / Salix::Vector2(0.0f, 3.0f);
        CHECK(result.x == 0.0f);
        CHECK(result.y == doctest::Approx(3.0f));
    }

    TEST_CASE("batch1747 scalar divided by vector zeroes only axes whose vector component is zero") {
        const auto result = 12.0f / Salix::Vector2(4.0f, 0.0f);
        CHECK(result.x == doctest::Approx(3.0f));
        CHECK(result.y == 0.0f);
    }

    TEST_CASE("batch1747 vector2 compound division by zero resets both components") {
        Salix::Vector2 value(5.0f, -7.0f);
        Salix::Vector2& returned = (value /= 0.0f);
        CHECK(&returned == &value);
        CHECK(value == Salix::Vector2(0.0f, 0.0f));
    }

    TEST_CASE("batch1747 vector2 dot product identifies perpendicular axis basis vectors") {
        const Salix::Vector2 x_axis(1.0f, 0.0f);
        const Salix::Vector2 y_axis(0.0f, 1.0f);
        CHECK(Salix::dot(x_axis, y_axis) == 0.0f);
        CHECK(Salix::dot(x_axis, x_axis) == 1.0f);
    }

    TEST_CASE("batch1747 vector2 cross product changes sign when operand order is reversed") {
        const Salix::Vector2 a(2.0f, 3.0f);
        const Salix::Vector2 b(-4.0f, 5.0f);
        CHECK(Salix::cross(a, b) == doctest::Approx(-Salix::cross(b, a)));
        CHECK(Salix::cross(a, a) == doctest::Approx(0.0f));
    }

    TEST_CASE("batch1747 vector2 perpendicular operation is orthogonal and twice applied negates input") {
        const Salix::Vector2 value(3.5f, -2.0f);
        const auto once = Salix::perp(value);
        const auto twice = Salix::perp(once);
        CHECK(Salix::dot(value, once) == doctest::Approx(0.0f));
        CHECK(twice.x == doctest::Approx(-value.x));
        CHECK(twice.y == doctest::Approx(-value.y));
    }
}
