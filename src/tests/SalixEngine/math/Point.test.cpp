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
#include <type_traits>
#include <utility>

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

// Persistence, identity and lifecycle invariants.
#include <limits>
#include <vector>
#include <cereal/types/vector.hpp>
TEST_SUITE("Salix::math::PointBatch647") {
    TEST_CASE("signed minimum and maximum coordinates serialize through JSON losslessly") {
        Salix::Point original{std::numeric_limits<int>::min(),std::numeric_limits<int>::max()};
        std::stringstream data;{cereal::JSONOutputArchive out(data);out(original);}
        Salix::Point restored;{cereal::JSONInputArchive in(data);in(restored);}
        CHECK(restored==original);
    }

    TEST_CASE("binary encoding retains the signed integer extrema") {
        Salix::Point original{std::numeric_limits<int>::max(),std::numeric_limits<int>::min()};
        std::stringstream data;{cereal::BinaryOutputArchive out(data);out(original);}
        Salix::Point restored;{cereal::BinaryInputArchive in(data);in(restored);}
        CHECK(restored==original);
    }

    TEST_CASE("Point equality is reflexive and symmetric for negative values") {
        Salix::Point a{-14,27},b{-14,27};
        CHECK(a==a);CHECK(a==b);CHECK(b==a);
        CHECK_FALSE(a!=b);
    }

    TEST_CASE("copying coordinates and then editing one cannot modify the original") {
        Salix::Point original{-200,300};
        Salix::Point copied=original;copied.x=25;copied.y=-35;
        CHECK(original.x==-200);
        CHECK(original.y==300);
        CHECK(copied!=original);
    }

    TEST_CASE("assignment replaces both components of an existing point") {
        const Salix::Point desired{0,-88};
        Salix::Point target{123,777};
        target=desired;
        CHECK(target==desired);
        CHECK(target.x==0);
        CHECK(target.y==-88);
    }

    TEST_CASE("stream output preserves a negative X and zero Y") {
        std::ostringstream out;out<<Salix::Point{-55,0};
        CHECK(out.str()=="{ x: -55, y: 0 }");
    }

    TEST_CASE("stream output preserves the largest representable positive X") {
        std::ostringstream out;out<<Salix::Point{std::numeric_limits<int>::max(),-1};
        CHECK(out.str().find(std::to_string(std::numeric_limits<int>::max()))!=std::string::npos);
        CHECK(out.str().find("y: -1")!=std::string::npos);
    }

    TEST_CASE("a vector of points persists in the original order using binary archive") {
        std::vector<Salix::Point> points{{0,0},{7,-8},{-90,200}};
        std::stringstream data;{cereal::BinaryOutputArchive out(data);out(points);}
        std::vector<Salix::Point> loaded;{cereal::BinaryInputArchive in(data);in(loaded);}
        REQUIRE(loaded.size()==points.size());
        for(size_t i=0;i<points.size();++i)CHECK(loaded[i]==points[i]);
    }

    TEST_CASE("a vector of signed points persists in the original order using JSON") {
        std::vector<Salix::Point> points{{-3,-2},{0,1},{500,-1000}};
        std::stringstream data;{cereal::JSONOutputArchive out(data);out(points);}
        std::vector<Salix::Point> loaded;{cereal::JSONInputArchive in(data);in(loaded);}
        REQUIRE(loaded.size()==points.size());
        for(size_t i=0;i<points.size();++i)CHECK(loaded[i]==points[i]);
    }

    TEST_CASE("loading JSON overwrites both fields but does not modify the source Point") {
        Salix::Point source{-7,0},destination{50,99};
        std::stringstream data;{cereal::JSONOutputArchive out(data);out(source);}
        {cereal::JSONInputArchive in(data);in(destination);}
        CHECK(destination==source);
        CHECK(source.x==-7);
        CHECK(source.y==0);
    }
}

// Deterministic geometry and persistence boundary scenarios.
TEST_SUITE("Salix::Batch747::PointBatch747") {
    TEST_CASE("point JSON uses explicit named axes instead of positional-only fields") {
        Salix::Point p{-25,128};std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(p);}CHECK(ss.str().find("\"x\"")!=std::string::npos);CHECK(ss.str().find("\"y\"")!=std::string::npos);
    }

    TEST_CASE("point equality distinguishes diagonal mirror coordinates") {
        const Salix::Point p{12,-12},q{-12,12};CHECK(p!=q);CHECK_FALSE(p==q);
    }

    TEST_CASE("point equality distinguishes swapped coordinates even when the sum agrees") {
        const Salix::Point p{1,99},q{99,1};CHECK(p!=q);CHECK(p.x+p.y==q.x+q.y);
    }

    TEST_CASE("point clearing one copied coordinate does not clear its independent source") {
        Salix::Point first{3,7},second=first;second.x=0;CHECK((first==Salix::Point{3,7}));CHECK((second==Salix::Point{0,7}));
    }

    TEST_CASE("point self assignment preserves both axes of a signed location") {
        Salix::Point p{-256,32767};p=p;CHECK(p.x==-256);CHECK(p.y==32767);
    }

    TEST_CASE("point output stream can concatenate two coordinates without corrupting formatting") {
        std::ostringstream out;out<<Salix::Point{1,-2}<<" | "<<Salix::Point{-3,4};CHECK(out.str()=="{ x: 1, y: -2 } | { x: -3, y: 4 }");
    }

    TEST_CASE("point JSON consecutive archive values restore two separate positions") {
        Salix::Point a{-9,5},b{27,-4};std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(a,b);}Salix::Point x,y;{cereal::JSONInputArchive in(ss);in(x,y);}CHECK(x==a);CHECK(y==b);
    }

    TEST_CASE("point binary consecutive archive values restore two separate positions") {
        Salix::Point a{0,-300},b{-10,0};std::stringstream ss;{cereal::BinaryOutputArchive out(ss);out(a,b);}Salix::Point x,y;{cereal::BinaryInputArchive in(ss);in(x,y);}CHECK(x==a);CHECK(y==b);
    }

    TEST_CASE("point vector JSON archive preserves repeated equal positions without deduplication") {
        std::vector<Salix::Point> source{Salix::Point{2,3},Salix::Point{2,3},Salix::Point{-1,8}};std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(source);}std::vector<Salix::Point> loaded;{cereal::JSONInputArchive in(ss);in(loaded);}REQUIRE(loaded.size()==3);CHECK(loaded[0]==loaded[1]);CHECK(loaded[2]==source[2]);
    }

    TEST_CASE("point binary archive output remains deterministic for identical values") {
        Salix::Point source{42,-99};std::stringstream a,b;{cereal::BinaryOutputArchive out(a);out(source);}{cereal::BinaryOutputArchive out(b);out(source);}CHECK(a.str()==b.str());
    }
}

TEST_SUITE("Salix::math::PointClosure1747") {
    TEST_CASE("batch1747 point remains an aggregate with standard layout") {
        CHECK((std::is_aggregate_v<Salix::Point>));
        CHECK((std::is_standard_layout_v<Salix::Point>));
    }

    TEST_CASE("batch1747 point remains trivially copyable for lightweight coordinate transport") {
        CHECK((std::is_trivially_copyable_v<Salix::Point>));
    }

    TEST_CASE("batch1747 point structured binding exposes copied X and Y values") {
        const Salix::Point point{-12, 34};
        const auto [x, y] = point;
        CHECK(x == -12);
        CHECK(y == 34);
    }

    TEST_CASE("batch1747 point equality is transitive across three separately created values") {
        const Salix::Point a{-4, 9}, b{-4, 9}, c{-4, 9};
        CHECK(a == b);
        CHECK(b == c);
        CHECK(a == c);
    }

    TEST_CASE("batch1747 point inequality remains exact complement of equality after one axis mutation") {
        Salix::Point a{8, 13};
        Salix::Point b = a;
        CHECK(a == b);
        b.y += 1;
        CHECK(a != b);
        CHECK_FALSE(a == b);
    }

    TEST_CASE("batch1747 point swapping two values exchanges both coordinates atomically") {
        Salix::Point a{1, 2}, b{-3, -4};
        std::swap(a, b);
        CHECK((a == Salix::Point{-3, -4}));
        CHECK((b == Salix::Point{1, 2}));
    }

    TEST_CASE("batch1747 point stream prints signed integer extrema without losing labels") {
        std::ostringstream out;
        out << Salix::Point{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
        CHECK(out.str().find("x: " + std::to_string(std::numeric_limits<int>::min())) != std::string::npos);
        CHECK(out.str().find("y: " + std::to_string(std::numeric_limits<int>::max())) != std::string::npos);
    }

    TEST_CASE("batch1747 point JSON archive restores extrema into a prepopulated destination") {
        const Salix::Point source{std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::Point target{7, 8};
        { cereal::JSONInputArchive in(stream); in(target); }
        CHECK(target == source);
    }

    TEST_CASE("batch1747 point binary archive is deterministic for identical coordinate values") {
        const Salix::Point source{-123456, 654321};
        std::stringstream first, second;
        { cereal::BinaryOutputArchive out(first); out(source); }
        { cereal::BinaryOutputArchive out(second); out(source); }
        CHECK(first.str() == second.str());
    }

    TEST_CASE("batch1747 point self assignment retains both coordinate fields") {
        Salix::Point point{-17, 29};
        point = point;
        CHECK((point == Salix::Point{-17, 29}));
    }
}
