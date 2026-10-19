// Vector3 orientation, conversion, arithmetic and persistence tests.
#include <doctest.h>
#include <Salix/math/Vector3.h>
#include <cmath>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::Vector3Regression") {
    TEST_CASE("vector3 normalization of a three component vector preserves direction") {
        Salix::Vector3 v{2.f, 3.f, 6.f};
        v.normalize();
        CHECK(v.x == doctest::Approx(2.f / 7.f));
        CHECK(v.y == doctest::Approx(3.f / 7.f));
        CHECK(v.z == doctest::Approx(6.f / 7.f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector3 normalizing the zero vector leaves finite zero components") {
        Salix::Vector3 v{};
        v.normalize();
        CHECK(v == Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(std::isfinite(v.x));
        CHECK(std::isfinite(v.y));
        CHECK(std::isfinite(v.z));
    }

    TEST_CASE("vector3 linear interpolation clamps and handles three axes independently") {
        const Salix::Vector3 a{-10.f, 3.f, 8.f}, b{10.f, 13.f, -8.f};
        CHECK(Salix::Vector3::lerp(a, b, -0.5f) == a);
        CHECK(Salix::Vector3::lerp(a, b, 1.5f) == b);
        CHECK(Salix::Vector3::lerp(a, b, 0.5f) == Salix::Vector3(0.f, 8.f, 0.f));
    }

    TEST_CASE("vector3 scalar constructors replicate values and predefined constants remain stable") {
        CHECK(Salix::Vector3(4) == Salix::Vector3(4.f, 4.f, 4.f));
        CHECK(Salix::Vector3(2.5f) == Salix::Vector3(2.5f, 2.5f, 2.5f));
        CHECK(Salix::Vector3::Zero == Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(Salix::Vector3::One == Salix::Vector3(1.f, 1.f, 1.f));
    }

    TEST_CASE("vector3 GLM conversion roundtrip preserves signed coordinates") {
        const Salix::Vector3 source{-1.5f, 0.f, 12.25f};
        const glm::vec3 glm_value = source.to_glm();
        CHECK(glm_value.x == source.x);
        CHECK(glm_value.y == source.y);
        CHECK(glm_value.z == source.z);
        CHECK(Salix::Vector3::from_glm(glm_value) == source);
    }

    TEST_CASE("vector3 component arithmetic is reversible with integral coordinates") {
        const Salix::Vector3 a{8.f, -5.f, 2.f}, b{-3.f, 10.f, 4.f};
        CHECK((a + b) - b == a);
        CHECK((a - b) + b == a);
        CHECK(a * b == Salix::Vector3(-24.f, -50.f, 8.f));
        CHECK(a + 2.f == 2.f + a);
    }

    TEST_CASE("vector3 dot product discriminates orthogonal aligned and opposite directions") {
        const Salix::Vector3 a{1.f, 0.f, 0.f};
        CHECK(Salix::dot(a, Salix::Vector3(0.f, 1.f, 0.f)) == 0.f);
        CHECK(Salix::dot(a, Salix::Vector3(4.f, 0.f, 0.f)) == 4.f);
        CHECK(Salix::dot(a, Salix::Vector3(-4.f, 0.f, 0.f)) == -4.f);
    }

    TEST_CASE("vector3 cross product has right handed orientation and anti symmetry") {
        const Salix::Vector3 x{1.f, 0.f, 0.f}, y{0.f, 1.f, 0.f};
        CHECK(Salix::cross(x, y) == Salix::Vector3(0.f, 0.f, 1.f));
        CHECK(Salix::cross(y, x) == Salix::Vector3(0.f, 0.f, -1.f));
        CHECK(Salix::dot(Salix::cross(x, y), x) == 0.f);
        CHECK(Salix::cross(x, x) == Salix::Vector3(0.f, 0.f, 0.f));
    }

    TEST_CASE("vector3 division by zero in one axis does not corrupt other components") {
        const auto v = Salix::Vector3(12.f, 8.f, -9.f) / Salix::Vector3(3.f, 0.f, -3.f);
        CHECK(v == Salix::Vector3(4.f, 0.f, 3.f));
        CHECK(Salix::Vector3(3.f, 6.f, 9.f) / 0.f == Salix::Vector3(0.f, 0.f, 0.f));
        const auto reciprocal = 18.f / Salix::Vector3(3.f, 0.f, 9.f);
        CHECK(reciprocal == Salix::Vector3(6.f, 0.f, 2.f));
    }

    TEST_CASE("vector3 YAML roundtrip and absent coordinate handling preserve schema") {
        const Salix::Vector3 source{1.5f, -2.75f, 0.125f};
        YAML::Emitter out;
        out << source;
        YAML::Node node = YAML::Load(out.c_str());
        Salix::Vector3 restored;
        node >> restored;
        CHECK(restored.x == doctest::Approx(source.x));
        CHECK(restored.y == doctest::Approx(source.y));
        CHECK(restored.z == doctest::Approx(source.z));
        YAML::Node partial = YAML::Load("{x: 4, z: -3}");
        partial >> restored;
        CHECK(restored == Salix::Vector3(4.f, 0.f, -3.f));
    }
}
