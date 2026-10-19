// =================================================================================
// Filename:    src/Tests/SalixEngine/math/Color.test.cpp
// Description: Contains unit tests for the Color functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/math/Color.h>
#include <cereal/archives/json.hpp>
#include <sstream>
#include <yaml-cpp/yaml.h>

// Helper function to compare two colors with a tolerance
void check_color_approx(const Salix::Color& result, const Salix::Color& expected) {
    CHECK(result.r == doctest::Approx(expected.r));
    CHECK(result.g == doctest::Approx(expected.g));
    CHECK(result.b == doctest::Approx(expected.b));
    CHECK(result.a == doctest::Approx(expected.a));
}

TEST_SUITE("Salix::math::Color") {
    TEST_CASE("constructors and predefined colors") {
        // ARRANGE
        Salix::Color c1(0.1f, 0.2f, 0.3f, 0.4f);
        Salix::Color c2 = Salix::Color::from_rgba_int(255, 0, 0);
        Salix::Color c3 = Salix::Color();
        Salix::Color c4 = {0.0f, 0.0f, 0.0f, 1.0f};
        // ASSERT
        CHECK(c1.r == 0.1f);
        CHECK(c1.g == 0.2f);
        CHECK(c1.b == 0.3f);
        CHECK(c1.a == 0.4f);

        CHECK(Salix::White == Salix::Color(1.0f, 1.0f, 1.0f, 1.0f));
        check_color_approx(c2, Salix::Red);
        CHECK(c3 == Salix::White);
        CHECK(c4 == Salix::Black);
        CHECK(Salix::Blue != Salix::Green);
        CHECK(Salix::Magenta != Salix::Cyan);
        CHECK(Salix::Red != Salix::Yellow);

    }
    TEST_CASE("operators behave correctly") {
        // ARRANGE
        Salix::Color c1(0.1f, 0.2f, 0.3f, 0.4f);
        Salix::Color c2(0.5f, 0.5f, 0.5f, 0.5f);

        // ACT & ASSERT
        check_color_approx(c1 + c2, Salix::Color(0.6f, 0.7f, 0.8f, 0.9f));
        check_color_approx(c2 - c1, Salix::Color(0.4f, 0.3f, 0.2f, 0.1f));
        check_color_approx(c1 * 2.0f, Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        check_color_approx(2.0f * c1, Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
    }
    TEST_CASE("lerp interpolates between two colors") {
        // ARRANGE
        Salix::Color black(0.0f, 0.0f, 0.0f, 1.0f);
        Salix::Color white(1.0f, 1.0f, 1.0f, 1.0f);

        // ACT
        Salix::Color grey = Salix::Color::lerp(black, white, 0.5f);
        Salix::Color black_again = Salix::Color::lerp(black, white, 0.0f);
        Salix::Color white_again = Salix::Color::lerp(black, white, 1.0f);

        // ASSERT
        check_color_approx(grey, Salix::Color(0.5f, 0.5f, 0.5f, 1.0f));
        check_color_approx(black_again, black);
        check_color_approx(white_again, white);
    }
    TEST_CASE("from_hsv converts all cases correctly") {
        // This test is expanded to cover all branches inside the from_hsv function
        // ARRANGE
        Salix::Color grey_from_hsv = Salix::Color::from_hsv(100.0f, 0.0f, 0.5f); // Case: s <= 0.0f
        Salix::Color red_from_hsv = Salix::Color::from_hsv(0.0f, 1.0f, 1.0f);    // Case: 0
        Salix::Color yellow_from_hsv = Salix::Color::from_hsv(60.0f, 1.0f, 1.0f); // Case: 1
        Salix::Color green_from_hsv = Salix::Color::from_hsv(120.0f, 1.0f, 1.0f);// Case: 2
        Salix::Color cyan_from_hsv = Salix::Color::from_hsv(180.0f, 1.0f, 1.0f); // Case: 3
        Salix::Color blue_from_hsv = Salix::Color::from_hsv(240.0f, 1.0f, 1.0f); // Case: 4
        Salix::Color magenta_from_hsv = Salix::Color::from_hsv(300.0f, 1.0f, 1.0f); // Case: default

        // ASSERT
        check_color_approx(grey_from_hsv, {0.5f, 0.5f, 0.5f});
        check_color_approx(red_from_hsv, Salix::Red);
        check_color_approx(yellow_from_hsv, Salix::Yellow);
        check_color_approx(green_from_hsv, Salix::Green);
        check_color_approx(cyan_from_hsv, Salix::Cyan);
        check_color_approx(blue_from_hsv, Salix::Blue);
        check_color_approx(magenta_from_hsv, Salix::Magenta);
    }
    TEST_CASE("can convert to ImVec4") {
        // This tests the to_imvec4() method in Color.cpp
        // ARRANGE
        Salix::Color c1(0.1f, 0.2f, 0.3f, 0.4f);

        // ACT
        ImVec4 result = c1.to_imvec4();

        // ASSERT
        CHECK(result.x == doctest::Approx(0.1f));
        CHECK(result.y == doctest::Approx(0.2f));
        CHECK(result.z == doctest::Approx(0.3f));
        CHECK(result.w == doctest::Approx(0.4f));
    }
    TEST_CASE("can handle '<<' ostream correctly") {
        // ARRANGE
        Salix::Color c1 = {0.1f, 0.2f, 0.3f ,0.4f};
        std::stringstream ss;
        
        // ACT
        ss << c1;
        
        // ASSERT
        CHECK( ss.str() == "{ r: 0.1, g: 0.2, b: 0.3, a: 0.4 }");
    }
    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        Salix::Color c_original = {10.0f, 20.0f, 30.0f, 1.0f};
        std::stringstream ss;
        // ACT: serialization
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(c_original);
        }

        
        // ACT: deserialization
        Salix::Color c_loaded;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(c_loaded);
        }
        // ASSERT: loaded_rect will be equal to the original_rect.
        CHECK(c_loaded.r == c_original.r);
        CHECK(c_loaded.g == c_original.g);
        CHECK(c_loaded.b == c_original.b);
        CHECK(c_loaded.a == c_original.a);
    }
    
    TEST_CASE("can be serialized and deserialized with YAML") {
        // This tests the YAML operator<< and operator>> in Color.cpp
        // ARRANGE
        Salix::Color original_color(0.1f, 0.2f, 0.3f, 0.4f);
        
        // ACT (Serialization)
        YAML::Emitter out;
        out << original_color;
        
        // ACT (Deserialization)
        YAML::Node node = YAML::Load(out.c_str());
        Salix::Color loaded_color;
        node >> loaded_color;

        // ASSERT
        check_color_approx(loaded_color, original_color);
    }
}

// Additional cases preserved from the separate regression suite.
// Color channel conversion, hue sectors, alpha and YAML regression coverage.
#include <doctest.h>
#include <Salix/math/Color.h>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::ColorRegression") {
    TEST_CASE("color integer RGBA normalization preserves channel endpoints") {
        const Salix::Color color = Salix::Color::from_rgba_int(255, 0, 128, 255);
        CHECK(color.r == 1.f);
        CHECK(color.g == 0.f);
        CHECK(color.b == doctest::Approx(128.f / 255.f));
        CHECK(color.a == 1.f);
    }

    TEST_CASE("color integer RGBA default alpha is opaque and partial alpha scales") {
        const auto opaque = Salix::Color::from_rgba_int(10, 20, 30);
        const auto translucent = Salix::Color::from_rgba_int(10, 20, 30, 64);
        CHECK(opaque.a == 1.f);
        CHECK(translucent.a == doctest::Approx(64.f / 255.f));
        CHECK(translucent.r == opaque.r);
    }

    TEST_CASE("color zero saturation creates grayscale independent of hue") {
        const auto gray1 = Salix::Color::from_hsv(10.f, 0.f, 0.35f, 0.4f);
        const auto gray2 = Salix::Color::from_hsv(271.f, 0.f, 0.35f, 0.4f);
        CHECK(gray1 == gray2);
        CHECK(gray1.r == doctest::Approx(0.35f));
        CHECK(gray1.g == doctest::Approx(0.35f));
        CHECK(gray1.b == doctest::Approx(0.35f));
        CHECK(gray1.a == doctest::Approx(0.4f));
    }

    TEST_CASE("color HSV primary red green and blue retain channel order") {
        CHECK(Salix::Color::from_hsv(0.f, 1.f, 1.f) == Salix::Red);
        CHECK(Salix::Color::from_hsv(120.f, 1.f, 1.f) == Salix::Green);
        CHECK(Salix::Color::from_hsv(240.f, 1.f, 1.f) == Salix::Blue);
    }

    TEST_CASE("color HSV secondary colors cover all hue sectors") {
        CHECK(Salix::Color::from_hsv(60.f, 1.f, 1.f) == Salix::Yellow);
        CHECK(Salix::Color::from_hsv(180.f, 1.f, 1.f) == Salix::Cyan);
        CHECK(Salix::Color::from_hsv(300.f, 1.f, 1.f) == Salix::Magenta);
    }

    TEST_CASE("color negative hue wraps correctly while preserving alpha") {
        const auto wrapped = Salix::Color::from_hsv(-60.f, 1.f, 1.f, 0.25f);
        CHECK(wrapped.r == doctest::Approx(1.f));
        CHECK(wrapped.g == doctest::Approx(0.f));
        CHECK(wrapped.b == doctest::Approx(1.f));
        CHECK(wrapped.a == doctest::Approx(0.25f));
    }

    TEST_CASE("color multi revolution positive hue matches equivalent hue") {
        const auto reference = Salix::Color::from_hsv(30.f, 0.5f, 0.8f);
        const auto wrapped = Salix::Color::from_hsv(750.f, 0.5f, 0.8f);
        CHECK(wrapped.r == doctest::Approx(reference.r));
        CHECK(wrapped.g == doctest::Approx(reference.g));
        CHECK(wrapped.b == doctest::Approx(reference.b));
    }

    TEST_CASE("color color interpolation clamps outside both endpoints") {
        const Salix::Color a{0.1f, 0.2f, 0.3f, 0.4f};
        const Salix::Color b{0.5f, 0.6f, 0.7f, 0.8f};
        CHECK(Salix::Color::lerp(a, b, -1.f) == a);
        CHECK(Salix::Color::lerp(a, b, 2.f) == b);
    }

    TEST_CASE("color color interpolation treats opacity as its own fourth channel") {
        const Salix::Color a{1.f, 0.f, 0.f, 0.f};
        const Salix::Color b{0.f, 0.f, 1.f, 1.f};
        const auto halfway = Salix::Color::lerp(a, b, 0.5f);
        CHECK(halfway.r == doctest::Approx(0.5f));
        CHECK(halfway.g == 0.f);
        CHECK(halfway.b == doctest::Approx(0.5f));
        CHECK(halfway.a == doctest::Approx(0.5f));
    }

    TEST_CASE("color YAML color representation restores every channel and defaults omissions") {
        const Salix::Color original{0.2f, 0.4f, 0.6f, 0.8f};
        YAML::Emitter out;
        out << original;
        Salix::Color restored;
        YAML::Node node = YAML::Load(out.c_str());
        node >> restored;
        CHECK(restored.r == doctest::Approx(original.r));
        CHECK(restored.g == doctest::Approx(original.g));
        CHECK(restored.b == doctest::Approx(original.b));
        CHECK(restored.a == doctest::Approx(original.a));
        YAML::Node partial = YAML::Load("{r: 1, b: 0.25}");
        partial >> restored;
        CHECK(restored.g == 0.f);
        CHECK(restored.a == 0.f);
    }
}
