#include <doctest.h>
#include <Salix/reflection/ReflectionTypes.h>
#include <cstdint>
#include <limits>
#include <string>
#include <variant>

TEST_SUITE("Salix::reflection::ReflectionTypesBatch1547") {
    TEST_CASE("property value defaults to integer zero alternative") {
        Salix::PropertyValue value{};
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
    }

    TEST_CASE("property value preserves signed and unsigned integer alternatives distinctly") {
        Salix::PropertyValue signed_value = -7;
        Salix::PropertyValue unsigned_value = uint64_t{9000000000ULL};
        CHECK(std::holds_alternative<int>(signed_value));
        CHECK(std::get<int>(signed_value) == -7);
        CHECK(std::holds_alternative<uint64_t>(unsigned_value));
        CHECK(std::get<uint64_t>(unsigned_value) == 9000000000ULL);
    }

    TEST_CASE("property value preserves float and boolean alternatives distinctly") {
        Salix::PropertyValue float_value = 3.25f;
        Salix::PropertyValue bool_value = true;
        CHECK(std::holds_alternative<float>(float_value));
        CHECK(std::get<float>(float_value) == doctest::Approx(3.25f));
        CHECK(std::holds_alternative<bool>(bool_value));
        CHECK(std::get<bool>(bool_value));
    }

    TEST_CASE("property value preserves strings including embedded null bytes") {
        const std::string source("abc\0xyz", 7);
        Salix::PropertyValue value = source;
        REQUIRE(std::holds_alternative<std::string>(value));
        CHECK(std::get<std::string>(value).size() == 7);
        CHECK(std::get<std::string>(value) == source);
    }

    TEST_CASE("property value preserves Vector2 and Vector3 alternatives") {
        Salix::PropertyValue two = Salix::Vector2{1.5f, -2.0f};
        Salix::PropertyValue three = Salix::Vector3{3.0f, 4.0f, 5.0f};
        CHECK(std::holds_alternative<Salix::Vector2>(two));
        CHECK(std::get<Salix::Vector2>(two).x == doctest::Approx(1.5f));
        CHECK(std::holds_alternative<Salix::Vector3>(three));
        CHECK(std::get<Salix::Vector3>(three).z == doctest::Approx(5.0f));
    }

    TEST_CASE("property value preserves color channel values") {
        Salix::PropertyValue value = Salix::Color{0.1f, 0.2f, 0.3f, 0.4f};
        REQUIRE(std::holds_alternative<Salix::Color>(value));
        const auto color = std::get<Salix::Color>(value);
        CHECK(color.r == doctest::Approx(0.1f));
        CHECK(color.g == doctest::Approx(0.2f));
        CHECK(color.b == doctest::Approx(0.3f));
        CHECK(color.a == doctest::Approx(0.4f));
    }

    TEST_CASE("property value preserves Point and Rect integer geometry") {
        Salix::PropertyValue point = Salix::Point{4, -3};
        Salix::PropertyValue rect = Salix::Rect{1, 2, 640, 480};
        CHECK(std::holds_alternative<Salix::Point>(point));
        CHECK(std::get<Salix::Point>(point).x == 4);
        CHECK(std::get<Salix::Point>(point).y == -3);
        CHECK(std::holds_alternative<Salix::Rect>(rect));
        CHECK(std::get<Salix::Rect>(rect).w == 640);
        CHECK(std::get<Salix::Rect>(rect).h == 480);
    }

    TEST_CASE("property value preserves full glm matrix alternative") {
        glm::mat4 matrix(1.0f);
        matrix[2][1] = 7.5f;
        Salix::PropertyValue value = matrix;
        REQUIRE(std::holds_alternative<glm::mat4>(value));
        CHECK(std::get<glm::mat4>(value)[0][0] == doctest::Approx(1.0f));
        CHECK(std::get<glm::mat4>(value)[2][1] == doctest::Approx(7.5f));
    }

    TEST_CASE("property value assignment can deliberately change active alternative") {
        Salix::PropertyValue value = 5;
        CHECK(std::holds_alternative<int>(value));
        value = std::string("five");
        CHECK(std::holds_alternative<std::string>(value));
        CHECK(std::get<std::string>(value) == "five");
        value = false;
        CHECK(std::holds_alternative<bool>(value));
        CHECK_FALSE(std::get<bool>(value));
    }

    TEST_CASE("property value copy is independent for mutable string storage") {
        Salix::PropertyValue source = std::string("source");
        Salix::PropertyValue copy = source;
        std::get<std::string>(copy) = "copy";
        CHECK(std::get<std::string>(source) == "source");
        CHECK(std::get<std::string>(copy) == "copy");
    }
}


TEST_SUITE("Salix::reflection::ReflectionTypesBatch2047") {
    TEST_CASE("integer property copy remains independent after source changes alternative") {
        Salix::PropertyValue source = 42; Salix::PropertyValue copy = source; source = std::string("changed");
        REQUIRE(std::holds_alternative<int>(copy)); CHECK(std::get<int>(copy) == 42);
    }
    TEST_CASE("property value preserves maximum unsigned 64 bit value exactly") {
        const uint64_t maximum = (std::numeric_limits<uint64_t>::max)(); Salix::PropertyValue value = maximum;
        REQUIRE(std::holds_alternative<uint64_t>(value)); CHECK(std::get<uint64_t>(value) == maximum);
    }
    TEST_CASE("boolean property assignment switches value without changing alternative") {
        Salix::PropertyValue value = false; REQUIRE(std::holds_alternative<bool>(value)); value = true;
        CHECK(std::holds_alternative<bool>(value)); CHECK(std::get<bool>(value));
    }
    TEST_CASE("Vector2 property copies own independent vector values") {
        Salix::PropertyValue source = Salix::Vector2{1.0f, 2.0f}; Salix::PropertyValue copy = source; std::get<Salix::Vector2>(source).x = 99.0f;
        CHECK(std::get<Salix::Vector2>(copy).x == doctest::Approx(1.0f)); CHECK(std::get<Salix::Vector2>(copy).y == doctest::Approx(2.0f));
    }
    TEST_CASE("Vector3 property copies own independent vector values") {
        Salix::PropertyValue source = Salix::Vector3{3.0f, 4.0f, 5.0f}; Salix::PropertyValue copy = source; std::get<Salix::Vector3>(source).z = -5.0f;
        CHECK(std::get<Salix::Vector3>(copy).z == doctest::Approx(5.0f));
    }
    TEST_CASE("Color property copies own independent channel values") {
        Salix::PropertyValue source = Salix::Color{0.1f, 0.2f, 0.3f, 0.4f}; Salix::PropertyValue copy = source; std::get<Salix::Color>(source).a = 1.0f;
        CHECK(std::get<Salix::Color>(copy).a == doctest::Approx(0.4f));
    }
    TEST_CASE("Point property copies own independent integer coordinates") {
        Salix::PropertyValue source = Salix::Point{7, 8}; Salix::PropertyValue copy = source; std::get<Salix::Point>(source).x = 70;
        CHECK(std::get<Salix::Point>(copy).x == 7); CHECK(std::get<Salix::Point>(copy).y == 8);
    }
    TEST_CASE("Rect property copies own independent geometry") {
        Salix::PropertyValue source = Salix::Rect{1, 2, 3, 4}; Salix::PropertyValue copy = source; std::get<Salix::Rect>(source).w = 300;
        CHECK(std::get<Salix::Rect>(copy).w == 3); CHECK(std::get<Salix::Rect>(copy).h == 4);
    }
    TEST_CASE("matrix property copy remains independent after source matrix mutation") {
        glm::mat4 matrix(1.0f); matrix[3][0] = 12.0f; Salix::PropertyValue source = matrix; Salix::PropertyValue copy = source;
        std::get<glm::mat4>(source)[3][0] = -12.0f; CHECK(std::get<glm::mat4>(copy)[3][0] == doctest::Approx(12.0f));
    }
    TEST_CASE("variant index follows declared PropertyValue alternative order for representative types") {
        Salix::PropertyValue integer = 1; Salix::PropertyValue text = std::string("text"); Salix::PropertyValue matrix = glm::mat4(1.0f);
        CHECK(integer.index() == 0); CHECK(text.index() == 4); CHECK(matrix.index() == 10);
    }
}
