#include <doctest.h>
#include <Salix/math/Vector3.h>
#include <glm/glm.hpp>

TEST_SUITE("Salix::math::Vector3Closure1747") {
    TEST_CASE("batch1747 vector3 normalization converts two three six vector to unit length") {
        Salix::Vector3 value(2.0f, 3.0f, 6.0f);
        CHECK(value.length() == doctest::Approx(7.0f));
        value.normalize();
        CHECK(value.x == doctest::Approx(2.0f / 7.0f));
        CHECK(value.y == doctest::Approx(3.0f / 7.0f));
        CHECK(value.z == doctest::Approx(6.0f / 7.0f));
        CHECK(value.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch1747 vector3 normalization leaves exact zero vector unchanged") {
        Salix::Vector3 value = Salix::Vector3::Zero;
        value.normalize();
        CHECK(value == Salix::Vector3::Zero);
    }

    TEST_CASE("batch1747 vector3 lerp clamps fractions outside zero to one interval") {
        const Salix::Vector3 start(-1.0f, 2.0f, 3.0f);
        const Salix::Vector3 end(9.0f, -4.0f, 7.0f);
        CHECK(Salix::Vector3::lerp(start, end, -1.0f) == start);
        CHECK(Salix::Vector3::lerp(start, end, 2.0f) == end);
    }

    TEST_CASE("batch1747 vector3 GLM conversion roundtrip preserves all components") {
        const Salix::Vector3 source(-1.25f, 2.5f, -3.75f);
        const glm::vec3 glm_value = source.to_glm();
        const Salix::Vector3 restored = Salix::Vector3::from_glm(glm_value);
        CHECK(restored == source);
    }

    TEST_CASE("batch1747 vector3 component division zeroes only axes with zero divisors") {
        const auto result = Salix::Vector3(12.0f, 15.0f, 20.0f) /
                            Salix::Vector3(3.0f, 0.0f, 5.0f);
        CHECK(result.x == doctest::Approx(4.0f));
        CHECK(result.y == 0.0f);
        CHECK(result.z == doctest::Approx(4.0f));
    }

    TEST_CASE("batch1747 scalar divided by vector zeroes every zero component independently") {
        const auto result = 18.0f / Salix::Vector3(0.0f, 3.0f, 0.0f);
        CHECK(result.x == 0.0f);
        CHECK(result.y == doctest::Approx(6.0f));
        CHECK(result.z == 0.0f);
    }

    TEST_CASE("batch1747 vector3 compound division by zero resets all components and returns self") {
        Salix::Vector3 value(5.0f, -7.0f, 9.0f);
        Salix::Vector3& returned = (value /= 0.0f);
        CHECK(&returned == &value);
        CHECK(value == Salix::Vector3::Zero);
    }

    TEST_CASE("batch1747 vector3 dot product identifies orthogonal basis vectors") {
        const Salix::Vector3 x(1.0f, 0.0f, 0.0f);
        const Salix::Vector3 y(0.0f, 1.0f, 0.0f);
        const Salix::Vector3 z(0.0f, 0.0f, 1.0f);
        CHECK(Salix::dot(x, y) == 0.0f);
        CHECK(Salix::dot(y, z) == 0.0f);
        CHECK(Salix::dot(z, x) == 0.0f);
    }

    TEST_CASE("batch1747 vector3 cross product follows right handed basis orientation") {
        const Salix::Vector3 x(1.0f, 0.0f, 0.0f);
        const Salix::Vector3 y(0.0f, 1.0f, 0.0f);
        CHECK(Salix::cross(x, y) == Salix::Vector3(0.0f, 0.0f, 1.0f));
        CHECK(Salix::cross(y, x) == Salix::Vector3(0.0f, 0.0f, -1.0f));
    }

    TEST_CASE("batch1747 vector3 cross result is perpendicular to both source vectors") {
        const Salix::Vector3 a(2.0f, -3.0f, 4.0f);
        const Salix::Vector3 b(-5.0f, 7.0f, 11.0f);
        const auto c = Salix::cross(a, b);
        CHECK(Salix::dot(c, a) == doctest::Approx(0.0f).epsilon(0.0001));
        CHECK(Salix::dot(c, b) == doctest::Approx(0.0f).epsilon(0.0001));
    }
}
