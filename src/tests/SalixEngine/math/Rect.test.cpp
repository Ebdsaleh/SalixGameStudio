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