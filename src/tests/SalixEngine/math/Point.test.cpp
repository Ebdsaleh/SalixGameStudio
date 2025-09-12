// =================================================================================
// Filename:    src/Tests/SalixEngine/math/Point.test.cpp
// Description: Contains unit tests for the Point functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/math/Point.h>
#include <sstream>
#include <cereal/archives/json.hpp>


TEST_SUITE("Salix::math::Point") {
    TEST_CASE("constructors and equality operators") {
        // ARRANGE
        Salix::Point p1{10, 20};
        Salix::Point p2{10, 20};
        Salix::Point p3{-5, 15};
        Salix::Point p_default;

        // ASSERT
        CHECK(p1.x == 10);
        CHECK(p1.y == 20);
        
        CHECK(p_default.x == 0);
        CHECK(p_default.y == 0);

        CHECK(p1 == p2);
        CHECK(p1 != p3);
    }
    TEST_CASE("can handle '<<' ostream correctly") {
        // ARRANGE
        Salix::Point p1 = {10, 20};
        std::stringstream ss;
        
        // ACT
        ss << p1;
        
        // ASSERT
        CHECK( ss.str() == "{ x: 10, y: 20 }");
    }
    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        Salix::Point original_point = {10, 20};
        std::stringstream ss;
        // ACT: serialization
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(original_point);
        }

        // ACT: deserialization
        Salix::Point loaded_point;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(loaded_point);
        }
        // ASSERT: loaded_rect will be equal to the original_rect.
        CHECK(loaded_point.x == original_point.x);
        CHECK(loaded_point.y == original_point.y);
    }
}