// Vector2 geometry, arithmetic and serialization regression checks.
#include <doctest.h>
#include <Salix/math/Vector2.h>
#include <cmath>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::Vector2Regression") {
    TEST_CASE("vector2 normalizing a three four vector preserves direction and gives unit length") {
        Salix::Vector2 v{3.f, 4.f};
        v.normalize();
        CHECK(v.x == doctest::Approx(0.6f));
        CHECK(v.y == doctest::Approx(0.8f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector2 normalizing zero vector does not produce NaN") {
        Salix::Vector2 v{0.f, 0.f};
        v.normalize();
        CHECK(v == Salix::Vector2(0.f, 0.f));
        CHECK(std::isfinite(v.x));
        CHECK(std::isfinite(v.y));
    }

    TEST_CASE("vector2 lerp respects both clamp boundaries and midpoint") {
        const Salix::Vector2 a{-10.f, 5.f}, b{10.f, 15.f};
        CHECK(Salix::Vector2::lerp(a, b, -1.f) == a);
        CHECK(Salix::Vector2::lerp(a, b, 2.f) == b);
        CHECK(Salix::Vector2::lerp(a, b, .5f) == Salix::Vector2(0.f, 10.f));
    }

    TEST_CASE("vector2 vector addition followed by subtraction restores both coordinates") {
        const Salix::Vector2 a{-7.f, 13.f}, b{4.f, -9.f};
        CHECK((a + b) - b == a);
        CHECK((a - b) + b == a);
        CHECK(a + 5.f == 5.f + a);
    }

    TEST_CASE("vector2 component multiplication handles opposite signs and zero components") {
        const Salix::Vector2 a{-3.f, 7.f}, b{2.f, 0.f};
        CHECK(a * b == Salix::Vector2(-6.f, 0.f));
        CHECK(2.f * a == a * 2.f);
        CHECK(a * 1.f == a);
    }

    TEST_CASE("vector2 component division zero affects only the corresponding axis") {
        const auto result = Salix::Vector2(20.f, -8.f) / Salix::Vector2(0.f, 4.f);
        CHECK(result.x == 0.f);
        CHECK(result.y == doctest::Approx(-2.f));
    }

    TEST_CASE("vector2 scalar reciprocal division handles a zero coordinate independently") {
        const auto result = 12.f / Salix::Vector2(4.f, 0.f);
        CHECK(result.x == doctest::Approx(3.f));
        CHECK(result.y == 0.f);
        CHECK(Salix::Vector2(2.f, 9.f) / 0.f == Salix::Vector2(0.f, 0.f));
    }

    TEST_CASE("vector2 compound operators return their operand and compose correctly") {
        Salix::Vector2 v{3.f, 5.f};
        Salix::Vector2* const identity = &v;
        CHECK(&(v += Salix::Vector2(1.f, -2.f)) == identity);
        CHECK(&(v -= Salix::Vector2(2.f, 1.f)) == identity);
        CHECK(&(v *= 3.f) == identity);
        CHECK(&(v /= 2.f) == identity);
        CHECK(v == Salix::Vector2(3.f, 3.f));
    }

    TEST_CASE("vector2 perpendicular vector has zero dot product and cross encodes orientation") {
        const Salix::Vector2 v{3.f, 4.f};
        const auto normal = Salix::perp(v);
        CHECK(normal == Salix::Vector2(-4.f, 3.f));
        CHECK(Salix::dot(v, normal) == doctest::Approx(0.f));
        CHECK(Salix::cross(v, normal) == doctest::Approx(25.f));
        CHECK(Salix::cross(normal, v) == doctest::Approx(-25.f));
    }

    TEST_CASE("vector2 YAML map roundtrip restores both fields and absent field defaults to zero") {
        const Salix::Vector2 original{12.25f, -3.5f};
        YAML::Emitter output;
        output << original;
        Salix::Vector2 loaded;
        YAML::Node parsed = YAML::Load(output.c_str());
        parsed >> loaded;
        CHECK(loaded.x == doctest::Approx(original.x));
        CHECK(loaded.y == doctest::Approx(original.y));
        YAML::Node partial = YAML::Load("{x: 9}");
        partial >> loaded;
        CHECK(loaded.x == 9.f);
        CHECK(loaded.y == 0.f);
    }
}
