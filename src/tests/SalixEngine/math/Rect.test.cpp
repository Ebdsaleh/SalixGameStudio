// =================================================================================
// Filename:    src/Tests/SalixEngine/math/Rect.test.cpp
// Description: Contains unit tests for the Rect functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/math/Rect.h>
#include <type_traits>
#include <cereal/archives/json.hpp>
#include <sstream>

TEST_SUITE("Salix::math::Rect") {
    TEST_CASE("integer rect (RectI) constructors and equality") {
        // ARRANGE
        Salix::RectI r1 {10, 20, 30, 40};
        Salix::RectI r2 {10, 20, 30, 40};
        Salix::RectI r3 {0, 0, 0, 0};

        // ASSERT
        CHECK(r1.x == 10);
        CHECK(r1.y == 20);
        CHECK(r1.w == 30);
        CHECK(r1.h == 40);

        CHECK(r1 == r2);
        CHECK(r1 != r3);

    }
    TEST_CASE("floating point rect (RectF) constructors and equality") {
        // ARRANGE
        Salix::RectF r1{1.5f, 2.5f, 3.5f, 4.5f};

        // ASSERT
        CHECK(r1.x == 1.5f);
        CHECK(r1.y == 2.5f);
        CHECK(r1.w == 3.5f);
        CHECK(r1.h == 4.5f);
    }
    TEST_CASE("generic and converting constructors") {
        SUBCASE("int from mixed number types") {
            // ARRANGE: Create an integer Rect from mixed float/int types.
            // This tests the generic constructor and expects truncation.
            Salix::Rect r_from_mixed_number_types(10, 20.7f, 30, 40.2f);

            // ASSERT: x:10, y:20.7f, w:30, h:40.2 should be x:10, y:20, w:30 , h:40
            CHECK(r_from_mixed_number_types.x == 10);
            CHECK(r_from_mixed_number_types.y == 20);
            CHECK(r_from_mixed_number_types.w == 30);
            CHECK(r_from_mixed_number_types.h == 40);
        }
        SUBCASE("int from float") {
            // ARRANGE: Create a float Rect and then convert it to an integer Rect.
            Salix::RectF rf {5.8f, 6.3f, 7.9f, 8.1f};
            Salix::Rect r_from_float_conversion(rf);

            // ASSERT: x:5.8f, y:6.3f, w:7.9f, h:8.1f should be x:5, y:6, w:7 , h:8
            CHECK(r_from_float_conversion.x == 5);
            CHECK(r_from_float_conversion.y == 6);
            CHECK(r_from_float_conversion.w == 7);
            CHECK(r_from_float_conversion.h == 8);
        }
        SUBCASE("int from double") {
            // ARRANGE: Create a double Rect and then convert it to an integer Rect.
            Salix::RectD rd {22.5, 46.1, 5.9, 17.90};
            Salix::Rect r_from_double_conversion(rd);

            // ASSERT: x:22.5, y:46.1, w:5.9, h:17.90 should be x:22, y:46, w:5 , h:17
            CHECK(r_from_double_conversion.x == 22);
            CHECK(r_from_double_conversion.y == 46);
            CHECK(r_from_double_conversion.w == 5);
            CHECK(r_from_double_conversion.h == 17);
        }
    }
    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        Salix::Rect original_rect = {10, 20, 30, 40};
        std::stringstream ss;
        // ACT: serialization
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(original_rect);
        }

        
        // ACT: deserialization
        Salix::Rect loaded_rect;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(loaded_rect);
        }
        // ASSERT: loaded_rect will be equal to the original_rect.
        CHECK(loaded_rect.x == original_rect.x);
        CHECK(loaded_rect.y == original_rect.y);
        CHECK(loaded_rect.w == original_rect.w);
        CHECK(loaded_rect.h == original_rect.h);

    }
    TEST_CASE("can handle '<<' ostream correctly") {
        // ARRANGE
        Salix::Rect r1 = {10, 20, 30 ,40};
        std::stringstream ss;
        
        // ACT
        ss << r1;
        
        // ASSERT
        CHECK( ss.str() == "{ x: 10, y: 20, w: 30, h: 40 }");
    }
}

// Additional contract tests: signed extents, typed conversion and archive fidelity.
#include <cereal/archives/binary.hpp>
#include <string>
#include <utility>

TEST_SUITE("Salix::math::RectAdditional") {
    TEST_CASE("rect negative fractional coordinates truncate toward zero in integer rectangles") {
        const Salix::RectI value(-1.9, -2.1, 3.9, -4.9);
        CHECK(value.x == -1);
        CHECK(value.y == -2);
        CHECK(value.w == 3);
        CHECK(value.h == -4);
    }

    TEST_CASE("rect negative extents remain signed instead of silently normalizing geometry") {
        const Salix::RectF value(12.f, 15.f, -3.f, -7.f);
        CHECK(value.w == -3.f);
        CHECK(value.h == -7.f);
        CHECK(value.x == 12.f);
        CHECK(value.y == 15.f);
    }

    TEST_CASE("rect converting integer rectangles to double preserves all coordinates") {
        const Salix::RectI original(-100, 250, 16, 80);
        const Salix::RectD converted(original);
        CHECK(converted.x == -100.0);
        CHECK(converted.y == 250.0);
        CHECK(converted.w == 16.0);
        CHECK(converted.h == 80.0);
    }

    TEST_CASE("rect converting fractional double rectangles to float preserves halves") {
        const Salix::RectD original(-3.5, 8.25, 12.5, -7.75);
        const Salix::RectF converted(original);
        CHECK(converted.x == doctest::Approx(-3.5f));
        CHECK(converted.y == doctest::Approx(8.25f));
        CHECK(converted.w == doctest::Approx(12.5f));
        CHECK(converted.h == doctest::Approx(-7.75f));
    }

    TEST_CASE("rect equality distinguishes each independently modified field") {
        const Salix::RectI baseline(1, 2, 3, 4);
        CHECK(baseline != Salix::RectI(5, 2, 3, 4));
        CHECK(baseline != Salix::RectI(1, 5, 3, 4));
        CHECK(baseline != Salix::RectI(1, 2, 5, 4));
        CHECK(baseline != Salix::RectI(1, 2, 3, 5));
        CHECK_FALSE(baseline != Salix::RectI(1, 2, 3, 4));
    }

    TEST_CASE("rect default floating and double rectangles are zero-valued") {
        const Salix::RectF floating;
        const Salix::RectD double_precision;
        CHECK(floating == Salix::RectF(0.f, 0.f, 0.f, 0.f));
        CHECK(double_precision == Salix::RectD(0.0, 0.0, 0.0, 0.0));
    }

    TEST_CASE("rect JSON roundtrip preserves signed and fractional rectangle members") {
        Salix::RectF original(-0.5f, 100.25f, 2.75f, -8.125f);
        std::stringstream stream;
        { cereal::JSONOutputArchive archive(stream); archive(original); }
        Salix::RectF restored;
        { cereal::JSONInputArchive archive(stream); archive(restored); }
        CHECK(restored == original);
    }

    TEST_CASE("rect binary roundtrip preserves negative coordinates and dimensions") {
        Salix::RectI original(-88, 555, -21, 32);
        std::stringstream stream;
        { cereal::BinaryOutputArchive archive(stream); archive(original); }
        Salix::RectI restored;
        { cereal::BinaryInputArchive archive(stream); archive(restored); }
        CHECK(restored == original);
    }

    TEST_CASE("rect stream output includes signed decimals and retains all labels") {
        const Salix::RectF value(-1.5f, 0.25f, 3.5f, -4.75f);
        std::ostringstream stream;
        stream << value;
        CHECK(stream.str() == "{ x: -1.5, y: 0.25, w: 3.5, h: -4.75 }");
    }

    TEST_CASE("rect rectangle constructors reject nonarithmetic fields at compile time") {
        CHECK((std::is_constructible_v<Salix::RectI, int, float, double, short>));
        CHECK_FALSE((std::is_constructible_v<Salix::RectI, int, std::string, int, int>));
        CHECK((std::is_constructible_v<Salix::RectD, const Salix::RectI&>));
    }
}

// Rectangle value semantics, precision-preserving archives and signed extents.
TEST_SUITE("Salix::math::RectExtended") {
    TEST_CASE("rect extended double precision rectangles preserve subunit coordinates") {
        const Salix::RectD rect(-1.125, 0.0625, 5.375, -8.875);
        CHECK(rect.x == -1.125);
        CHECK(rect.y == 0.0625);
        CHECK(rect.w == 5.375);
        CHECK(rect.h == -8.875);
    }

    TEST_CASE("rect extended copying rectangles preserves value state without aliasing") {
        Salix::RectI original(-20, 15, 12, 9);
        Salix::RectI copy(original);
        copy.w = 0;
        CHECK(original.w == 12);
        CHECK(copy != original);
        CHECK(copy.x == original.x);
    }

    TEST_CASE("rect extended assignment replaces all members of destination rectangle") {
        const Salix::RectI src(4, -9, -7, 21);
        Salix::RectI dst(8, 8, 8, 8);
        dst = src;
        CHECK(dst == src);
        CHECK(dst.x == 4);
        CHECK(dst.y == -9);
        CHECK(dst.w == -7);
        CHECK(dst.h == 21);
    }

    TEST_CASE("rect extended conversion from floats to ints truncates negative fractions toward zero") {
        const Salix::RectF original(-1.75f, -2.25f, -3.875f, -4.125f);
        const Salix::RectI result(original);
        CHECK(result == Salix::RectI(-1, -2, -3, -4));
    }

    TEST_CASE("rect extended int to double and back conversion preserves representable coordinates") {
        const Salix::RectI original(-300, 200, 640, 480);
        const Salix::RectD doubles(original);
        const Salix::RectI restored(doubles);
        CHECK(restored == original);
    }

    TEST_CASE("rect extended zero width and height still participate in strict value equality") {
        const Salix::RectI empty(5, 7, 0, 0);
        CHECK(empty == Salix::RectI(5, 7, 0, 0));
        CHECK(empty != Salix::RectI(5, 8, 0, 0));
        CHECK(empty != Salix::RectI(5, 7, 0, 1));
    }

    TEST_CASE("rect extended negative rectangle extents are distinct from their positive counterparts") {
        const Salix::RectI flipped(0, 0, -64, -32);
        const Salix::RectI normal(0, 0, 64, 32);
        CHECK(flipped != normal);
        CHECK(flipped.w == -64);
        CHECK(flipped.h == -32);
    }

    TEST_CASE("rect extended binary serialization of double rectangle roundtrips exact values") {
        Salix::RectD original(-0.125, 22.5, 12345.75, -333.0625);
        std::stringstream bytes;
        { cereal::BinaryOutputArchive out(bytes); out(original); }
        Salix::RectD restored;
        { cereal::BinaryInputArchive in(bytes); in(restored); }
        CHECK(restored == original);
    }

    TEST_CASE("rect extended JSON serialization uses named coordinate fields for portability") {
        Salix::RectI original(17, -8, 99, 201);
        std::stringstream json;
        { cereal::JSONOutputArchive out(json); out(original); }
        const std::string data = json.str();
        CHECK(data.find("\"x\"") != std::string::npos);
        CHECK(data.find("\"y\"") != std::string::npos);
        CHECK(data.find("\"w\"") != std::string::npos);
        CHECK(data.find("\"h\"") != std::string::npos);
    }

    TEST_CASE("rect extended rectangle stream output handles zero negative dimensions and signs") {
        std::ostringstream stream;
        stream << Salix::RectI(0, -7, 0, -10);
        CHECK(stream.str() == "{ x: 0, y: -7, w: 0, h: -10 }");
    }
}
