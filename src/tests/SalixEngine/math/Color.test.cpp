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

// Color arithmetic and archive contracts, independent of the graphics backend.
#include <cereal/archives/binary.hpp>
#include <initializer_list>
TEST_SUITE("Salix::math::ColorAdditional") {
    TEST_CASE("color arithmetic addition combines RGBA channels without implicit clamping") {
        const Salix::Color a(0.5f, 0.25f, 0.f, 0.75f);
        const Salix::Color b(0.75f, 0.25f, 0.5f, 0.5f);
        const auto result = a + b;
        CHECK(result.r == doctest::Approx(1.25f));
        CHECK(result.g == doctest::Approx(0.5f));
        CHECK(result.b == doctest::Approx(0.5f));
        CHECK(result.a == doctest::Approx(1.25f));
    }

    TEST_CASE("color arithmetic subtraction restores each original channel after addition") {
        const Salix::Color a(0.2f, 0.4f, 0.6f, 0.8f);
        const Salix::Color b(0.1f, 0.3f, 0.2f, 0.1f);
        const auto recovered = (a + b) - b;
        CHECK(recovered.r == doctest::Approx(a.r));
        CHECK(recovered.g == doctest::Approx(a.g));
        CHECK(recovered.b == doctest::Approx(a.b));
        CHECK(recovered.a == doctest::Approx(a.a));
    }

    TEST_CASE("color arithmetic color addition is commutative including the alpha channel") {
        const Salix::Color a(0.25f, 0.125f, 0.5f, 0.125f);
        const Salix::Color b(0.5f, 0.25f, 0.125f, 0.5f);
        CHECK(a + b == b + a);
    }

    TEST_CASE("color arithmetic scalar multiplication has identical left and right overloads") {
        const Salix::Color c(0.125f, 0.25f, 0.5f, 1.f);
        CHECK(c * 2.f == 2.f * c);
        CHECK(c * 2.f == Salix::Color(0.25f, 0.5f, 1.f, 2.f));
    }

    TEST_CASE("color arithmetic scalar zero suppresses all channels including alpha") {
        const Salix::Color c(0.1f, 0.2f, 0.3f, 0.9f);
        CHECK(c * 0.f == Salix::Color(0.f, 0.f, 0.f, 0.f));
        CHECK(c * 1.f == c);
    }

    TEST_CASE("color arithmetic lerp preserves identical endpoints for every interpolation fraction") {
        const Salix::Color c(0.125f, 0.25f, 0.5f, 0.875f);
        for (float t : { -4.f, 0.f, 0.25f, 0.5f, 1.f, 5.f }) {
            CHECK(Salix::Color::lerp(c, c, t) == c);
        }
    }

    TEST_CASE("color arithmetic lerp symmetry t versus one minus t holds across RGB and alpha") {
        const Salix::Color a(0.125f, 0.25f, 0.5f, 0.f);
        const Salix::Color b(0.5f, 0.125f, 0.25f, 1.f);
        const auto ab = Salix::Color::lerp(a, b, 0.25f);
        const auto ba = Salix::Color::lerp(b, a, 0.75f);
        CHECK(ab.r == doctest::Approx(ba.r));
        CHECK(ab.g == doctest::Approx(ba.g));
        CHECK(ab.b == doctest::Approx(ba.b));
        CHECK(ab.a == doctest::Approx(ba.a));
    }

    TEST_CASE("color arithmetic transparent and opaque endpoints interpolate opacity independently") {
        const Salix::Color transparent(0.f, 0.f, 0.f, 0.f);
        const Salix::Color opaque(0.f, 0.f, 0.f, 1.f);
        const auto partial = Salix::Color::lerp(transparent, opaque, 0.125f);
        CHECK(partial.r == 0.f);
        CHECK(partial.g == 0.f);
        CHECK(partial.b == 0.f);
        CHECK(partial.a == doctest::Approx(0.125f));
    }

    TEST_CASE("color arithmetic binary archive preserves all four floating point channel values") {
        Salix::Color source(0.0625f, 0.375f, 0.625f, 0.9375f);
        std::stringstream bytes;
        { cereal::BinaryOutputArchive archive(bytes); archive(source); }
        Salix::Color loaded;
        { cereal::BinaryInputArchive archive(bytes); archive(loaded); }
        CHECK(loaded == source);
    }

    TEST_CASE("color arithmetic integer color conversion maps quarter and three-quarter intensities") {
        const auto c = Salix::Color::from_rgba_int(64, 128, 192, 32);
        CHECK(c.r == doctest::Approx(64.f / 255.f));
        CHECK(c.g == doctest::Approx(128.f / 255.f));
        CHECK(c.b == doctest::Approx(192.f / 255.f));
        CHECK(c.a == doctest::Approx(32.f / 255.f));
        CHECK(c.to_imvec4().w == doctest::Approx(c.a));
    }
}

// HDR algebra, HSV invariants and alpha serialization contracts.
TEST_SUITE("Salix::math::ColorAdditional20261019") {
    TEST_CASE("color extended RGBA factory maps all four byte endpoints without cross-channel interference") {
        const auto c=Salix::Color::from_rgba_int(0,255,0,128);
        CHECK(c.r == 0.f);
        CHECK(c.g == 1.f);
        CHECK(c.b == 0.f);
        CHECK(c.a == doctest::Approx(128.f/255.f));
    }

    TEST_CASE("color extended integer conversion is monotone across a channel's byte domain") {
        float previous=-1.f;
        for (int n : {0,1,32,64,128,200,255}) {
            const auto c=Salix::Color::from_rgba_int(n,0,0);
            CHECK(c.r>=previous);
            previous=c.r;
        }
    }

    TEST_CASE("color extended HSV black is independent of saturation and hue") {
        const auto a=Salix::Color::from_hsv(30.f,1.f,0.f,0.25f);
        const auto b=Salix::Color::from_hsv(230.f,0.f,0.f,0.25f);
        CHECK(a == b);
        CHECK(a.r == 0.f);
        CHECK(a.g == 0.f);
        CHECK(a.b == 0.f);
    }

    TEST_CASE("color extended HSV grayscale preserves opacity across hue changes") {
        const auto x=Salix::Color::from_hsv(0.f,0.f,0.625f,0.75f);
        const auto y=Salix::Color::from_hsv(320.f,0.f,0.625f,0.75f);
        CHECK(x == y);
        CHECK(x.a == doctest::Approx(0.75f));
    }

    TEST_CASE("color extended HSV hue one complete cycle later produces identical RGB") {
        const auto a=Salix::Color::from_hsv(60.f,1.f,1.f);
        const auto b=Salix::Color::from_hsv(420.f,1.f,1.f);
        CHECK(b.r == doctest::Approx(a.r));
        CHECK(b.g == doctest::Approx(a.g));
        CHECK(b.b == doctest::Approx(a.b));
    }

    TEST_CASE("color extended linear interpolation keeps both endpoints when fraction is clamped") {
        const Salix::Color start(0.1f,0.2f,0.3f,0.4f), end(0.6f,0.7f,0.8f,0.9f);
        CHECK(Salix::Color::lerp(start,end,-3.f) == start);
        CHECK(Salix::Color::lerp(start,end,3.f) == end);
    }

    TEST_CASE("color extended linear RGBA interpolation separates transparency from RGB tint") {
        const Salix::Color start(0.25f,0.f,0.f,0.f), end(0.25f,0.f,0.f,1.f);
        const auto result=Salix::Color::lerp(start,end,0.25f);
        CHECK(result.r == doctest::Approx(0.25f));
        CHECK(result.g == 0.f);
        CHECK(result.a == doctest::Approx(0.25f));
    }

    TEST_CASE("color extended adding and subtracting colors retain unbounded HDR and alpha") {
        const Salix::Color a(2.f,0.5f,1.5f,1.25f), b(1.f,0.25f,0.5f,0.75f);
        const auto c=(a+b)-b;
        CHECK(c == a);
        CHECK((a+b).a == doctest::Approx(2.f));
    }

    TEST_CASE("color extended YAML decoding replaces a previously initialized alpha value") {
        Salix::Color value(1.f,1.f,1.f,1.f);
        const YAML::Node yaml=YAML::Load("{r: 0.125, g: 0.25, b: 0.5, a: 0.75}");
        yaml >> value;
        CHECK(value.r == doctest::Approx(0.125f));
        CHECK(value.g == doctest::Approx(0.25f));
        CHECK(value.a == doctest::Approx(0.75f));
    }

    TEST_CASE("color extended Cereal JSON roundtrip preserves non-byte-aligned opacity and tint") {
        Salix::Color source(0.125f,0.375f,0.625f,0.875f);
        std::stringstream data;
        { cereal::JSONOutputArchive out(data); out(source); }
        Salix::Color restored;
        { cereal::JSONInputArchive in(data); in(restored); }
        CHECK(restored == source);
    }
}

// Boundary behavior pinned against the implementation and public API.
TEST_SUITE("Salix::Batch747::ColorBatch747") {
    TEST_CASE("color HSV orange at thirty degrees splits red and green halfway") {
        const auto c=Salix::Color::from_hsv(30.f,1.f,1.f);check_color_approx(c,Salix::Color(1.f,0.5f,0.f));
    }

    TEST_CASE("color HSV chartreuse at ninety degrees splits green and red halfway") {
        const auto c=Salix::Color::from_hsv(90.f,1.f,1.f);check_color_approx(c,Salix::Color(0.5f,1.f,0.f));
    }

    TEST_CASE("color HSV spring green at one hundred fifty degrees adds half blue") {
        const auto c=Salix::Color::from_hsv(150.f,1.f,1.f);check_color_approx(c,Salix::Color(0.f,1.f,0.5f));
    }

    TEST_CASE("color HSV azure at two hundred ten degrees adds half green") {
        const auto c=Salix::Color::from_hsv(210.f,1.f,1.f);check_color_approx(c,Salix::Color(0.f,0.5f,1.f));
    }

    TEST_CASE("color HSV violet at two hundred seventy degrees adds half red") {
        const auto c=Salix::Color::from_hsv(270.f,1.f,1.f);check_color_approx(c,Salix::Color(0.5f,0.f,1.f));
    }

    TEST_CASE("color HSV rose at three hundred thirty degrees adds half blue") {
        const auto c=Salix::Color::from_hsv(330.f,1.f,1.f);check_color_approx(c,Salix::Color(1.f,0.f,0.5f));
    }

    TEST_CASE("color negative HSV hue wraps to the corresponding positive hue") {
        const auto a=Salix::Color::from_hsv(-30.f,1.f,1.f,0.3f);const auto b=Salix::Color::from_hsv(330.f,1.f,1.f,0.3f);check_color_approx(a,b);
    }

    TEST_CASE("color zero and negative saturation produce grayscale with user alpha") {
        const auto a=Salix::Color::from_hsv(123.f,-0.1f,0.375f,0.625f);const auto b=Salix::Color::from_hsv(15.f,0.f,0.375f,0.625f);check_color_approx(a,b);check_color_approx(a,Salix::Color(0.375f,0.375f,0.375f,0.625f));
    }

    TEST_CASE("color HSV supports overbright value without silently clipping HDR") {
        const auto c=Salix::Color::from_hsv(0.f,1.f,2.f,0.125f);check_color_approx(c,Salix::Color(2.f,0.f,0.f,0.125f));
    }

    TEST_CASE("color YAML missing alpha explicitly clears preexisting opacity instead of keeping white default") {
        Salix::Color c(1.f,1.f,1.f,1.f);const YAML::Node n=YAML::Load("{r: 0.25, g: 0.5, b: 0.75}");n >> c;check_color_approx(c,Salix::Color(0.25f,0.5f,0.75f,0.f));
    }
}
