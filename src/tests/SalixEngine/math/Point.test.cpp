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

// Signed coordinate, value-semantics and archive compatibility cases.
#include <cereal/archives/binary.hpp>

TEST_SUITE("Salix::math::PointExtended") {
    TEST_CASE("point negative coordinates preserve their signs across equality") {
        Salix::Point a{-7, -11}, b{-7, -11}, c{7, -11};
        CHECK(a == b);
        CHECK(a != c);
        CHECK(b != c);
    }

    TEST_CASE("point changing only X breaks equality without changing Y") {
        Salix::Point a{2, 5}, b = a;
        b.x = -9;
        CHECK(a != b);
        CHECK(a.y == b.y);
        CHECK(b.x == -9);
    }

    TEST_CASE("point changing only Y breaks equality without changing X") {
        Salix::Point a{2, 5}, b = a;
        b.y = 15;
        CHECK(a != b);
        CHECK(a.x == b.x);
        CHECK(b.y == 15);
    }

    TEST_CASE("point copy construction produces independent coordinate storage") {
        Salix::Point original{-13, 24};
        Salix::Point copy(original);
        copy.x = 123;
        CHECK(copy != original);
        CHECK(original.x == -13);
        CHECK(original.y == 24);
    }

    TEST_CASE("point copy assignment replaces both coordinates but keeps source intact") {
        Salix::Point original{9, -8}, target{-4, 3};
        target = original;
        CHECK(target == original);
        target.y = 42;
        CHECK(original.y == -8);
        CHECK(target.x == 9);
    }

    TEST_CASE("point default points compare equal regardless of separate construction") {
        const Salix::Point a{}, b{};
        CHECK(a == b);
        CHECK_FALSE(a != b);
        CHECK(a.x == 0);
        CHECK(b.y == 0);
    }

    TEST_CASE("point stream formatting includes signed coordinates and no extra spaces") {
        std::ostringstream out;
        out << Salix::Point{-5, 17};
        CHECK(out.str() == "{ x: -5, y: 17 }");
    }

    TEST_CASE("point JSON archive retains large positive and negative coordinates") {
        Salix::Point original{1234567, -7654321};
        std::stringstream buffer;
        { cereal::JSONOutputArchive out(buffer); out(original); }
        Salix::Point loaded;
        { cereal::JSONInputArchive in(buffer); in(loaded); }
        CHECK(loaded == original);
    }

    TEST_CASE("point binary archive keeps coordinates for negative quadrant") {
        Salix::Point original{-256, -1024};
        std::stringstream bytes;
        { cereal::BinaryOutputArchive out(bytes); out(original); }
        Salix::Point restored;
        { cereal::BinaryInputArchive in(bytes); in(restored); }
        CHECK(restored == original);
    }

    TEST_CASE("point archive decoding replaces both prepopulated fields") {
        Salix::Point source{0, -99};
        std::stringstream buffer;
        { cereal::JSONOutputArchive out(buffer); out(source); }
        Salix::Point destination{555, 777};
        { cereal::JSONInputArchive in(buffer); in(destination); }
        CHECK(destination.x == 0);
        CHECK(destination.y == -99);
    }
}
