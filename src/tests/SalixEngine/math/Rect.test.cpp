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

// Value and ownership contracts for the existing canonical suite.
#include <vector>
#include <cereal/types/vector.hpp>
TEST_SUITE("Salix::math::RectBatch647") {
    TEST_CASE("a double-to-float-to-double roundtrip retains dyadic fractional rectangles") {
        Salix::RectD original(-1.125,2.25,7.5,-8.75);
        const Salix::RectF as_float(original);
        const Salix::RectD restored(as_float);
        CHECK(restored==original);
    }

    TEST_CASE("integer conversion of signed fractional extents truncates toward zero") {
        Salix::RectD real(-8.75,3.5,-20.25,10.99);
        const Salix::RectI converted(real);
        CHECK(converted==Salix::RectI(-8,3,-20,10));
    }

    TEST_CASE("mixed numeric constructors preserve independently typed components") {
        Salix::RectD value(-4,3.25f,5ULL,-0.5);
        CHECK(value.x==-4.);
        CHECK(value.y==3.25);
        CHECK(value.w==5.);
        CHECK(value.h==-0.5);
    }

    TEST_CASE("rectangle equality depends on the position even for zero-area rectangles") {
        const Salix::RectI a(0,0,0,0),b(1,0,0,0),c(0,1,0,0);
        CHECK(a!=b);
        CHECK(a!=c);
        CHECK(b!=c);
    }

    TEST_CASE("rectangle assignment does not alias copied value fields") {
        Salix::RectI a(2,4,6,8),b=a;
        b.w=100;b.h=-50;
        CHECK(a.w==6);
        CHECK(a.h==8);
        CHECK(b!=a);
    }

    TEST_CASE("serialized JSON includes all four independently named members") {
        Salix::RectD original(-2.5,4.125,8.25,-16.5);
        std::stringstream stream;{cereal::JSONOutputArchive out(stream);out(original);}
        const std::string json=stream.str();
        for(const char* key:{"\"x\"","\"y\"","\"w\"","\"h\""})CHECK(json.find(key)!=std::string::npos);
    }

    TEST_CASE("JSON roundtrip maintains negative and positive double precision dimensions") {
        Salix::RectD original(-0.125,0.375,-9.625,1200.5);
        std::stringstream stream;{cereal::JSONOutputArchive out(stream);out(original);}
        Salix::RectD restored;{cereal::JSONInputArchive in(stream);in(restored);}
        CHECK(restored==original);
    }

    TEST_CASE("binary archive roundtrips multiple rectangles without losing order") {
        std::vector<Salix::RectI> values{Salix::RectI(1,2,3,4),Salix::RectI(-4,-3,-2,-1)};
        std::stringstream stream;{cereal::BinaryOutputArchive out(stream);out(values);}
        std::vector<Salix::RectI> loaded;{cereal::BinaryInputArchive in(stream);in(loaded);}
        REQUIRE(loaded.size()==2);
        CHECK(loaded[0]==values[0]);
        CHECK(loaded[1]==values[1]);
    }

    TEST_CASE("a float rectangle retains the same signed values in its stream representation") {
        std::ostringstream out;out<<Salix::RectF(-2.5f,0.f,3.25f,-4.f);
        CHECK(out.str()=="{ x: -2.5, y: 0, w: 3.25, h: -4 }");
    }

    TEST_CASE("restoring JSON into a populated rectangle replaces every coordinate") {
        Salix::RectI original(-20,-10,30,40),target(1,2,3,4);
        std::stringstream data;{cereal::JSONOutputArchive out(data);out(original);}
        {cereal::JSONInputArchive in(data);in(target);}
        CHECK(target==original);
        CHECK(original==Salix::RectI(-20,-10,30,40));
    }
}

// Typed value and archive boundary tests added without touching runtime assets.
TEST_SUITE("Salix::Batch747::RectBatch747") {
    TEST_CASE("rectangle arithmetic constructor accepts boolean extents as zero and one") {
        const Salix::RectI r(-7,4,true,false);CHECK(r.x==-7);CHECK(r.y==4);CHECK(r.w==1);CHECK(r.h==0);
    }

    TEST_CASE("rectangle double coordinate conversion to float preserves exact power of two fractions") {
        const Salix::RectD src(-0.0625,1.125,4.5,-8.25);const Salix::RectF out(src);CHECK(out==Salix::RectF(-0.0625f,1.125f,4.5f,-8.25f));
    }

    TEST_CASE("rectangle converting signed decimal fractions into integers truncates each member individually") {
        const Salix::RectD src(-0.99,-3.01,5.99,-8.99);const Salix::RectI out(src);CHECK(out==Salix::RectI(0,-3,5,-8));
    }

    TEST_CASE("rectangle integral construction retains large representable signed dimensions") {
        const Salix::RectI r(-1048576,1048576,32768,-32768);CHECK(r.x==-1048576);CHECK(r.y==1048576);CHECK(r.w==32768);CHECK(r.h==-32768);
    }

    TEST_CASE("rectangle independent copies of floating extents do not share mutable storage") {
        Salix::RectF a(-2.f,3.f,4.f,-5.f),b=a;b.x=100.f;b.h=0.f;CHECK(a==Salix::RectF(-2.f,3.f,4.f,-5.f));CHECK(b!=a);
    }

    TEST_CASE("rectangle four-field equality is transitive across separately constructed double values") {
        const Salix::RectD a(2.125,-3.5,100.25,-1.125),b=a,c=b;CHECK(a==b);CHECK(b==c);CHECK(a==c);CHECK_FALSE(a!=c);
    }

    TEST_CASE("rectangle flipping width sign cannot preserve equality even at the same position") {
        const Salix::RectD positive(-4.,7.,16.,9.),flipped(-4.,7.,-16.,9.);CHECK(positive!=flipped);CHECK(positive.x==flipped.x);CHECK(positive.y==flipped.y);
    }

    TEST_CASE("rectangle binary consecutive archives restore float and integer values separately") {
        Salix::RectI ints(-1,2,-3,4);Salix::RectF floats(0.125f,-0.5f,3.25f,10.f);std::stringstream ss;{cereal::BinaryOutputArchive out(ss);out(ints,floats);}Salix::RectI a;Salix::RectF b;{cereal::BinaryInputArchive in(ss);in(a,b);}CHECK(a==ints);CHECK(b==floats);
    }

    TEST_CASE("rectangle JSON vector retains duplicate rectangles and a final negative-width rectangle") {
        std::vector<Salix::RectI> a{Salix::RectI(1,2,3,4),Salix::RectI(1,2,3,4),Salix::RectI(1,2,-3,4)};std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(a);}std::vector<Salix::RectI> b;{cereal::JSONInputArchive in(ss);in(b);}REQUIRE(b.size()==3);CHECK(b[0]==b[1]);CHECK(b[1]!=b[2]);
    }

    TEST_CASE("rectangle serializing and restoring an empty double rectangle replaces populated destination") {
        Salix::RectD empty(0.,0.,0.,0.),target(1.,2.,3.,4.);std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(empty);}{cereal::JSONInputArchive in(ss);in(target);}CHECK(target==empty);
    }
}

#include <cmath>
#include <cstdint>
#include <limits>
TEST_SUITE("Salix::math::RectBatch947") {
    TEST_CASE("Rect and RectI aliases resolve to the exact same integer rectangle type") {
        CHECK((std::is_same_v<Salix::Rect, Salix::RectI>));
        CHECK(sizeof(Salix::Rect) == sizeof(Salix::RectI));
    }

    TEST_CASE("cross-type rectangle conversion is constructible but deliberately explicit") {
        CHECK((std::is_constructible_v<Salix::RectI, Salix::RectD>));
        CHECK_FALSE((std::is_convertible_v<Salix::RectD, Salix::RectI>));
        CHECK((std::is_constructible_v<Salix::RectD, Salix::RectI>));
        CHECK_FALSE((std::is_convertible_v<Salix::RectI, Salix::RectD>));
    }

    TEST_CASE("long double constructor inputs convert independently into double rectangle fields") {
        Salix::RectD rect(1.25L, -2.5L, 4.0L, 8.125L);
        CHECK(rect.x == doctest::Approx(1.25));
        CHECK(rect.y == doctest::Approx(-2.5));
        CHECK(rect.w == doctest::Approx(4.0));
        CHECK(rect.h == doctest::Approx(8.125));
    }

    TEST_CASE("mixed short and unsigned short inputs preserve representable integer fields") {
        const short x = -123;
        const unsigned short y = 456;
        const short w = 789;
        const unsigned short h = 1024;
        Salix::RectI rect(x, y, w, h);
        CHECK(rect.x == -123);
        CHECK(rect.y == 456);
        CHECK(rect.w == 789);
        CHECK(rect.h == 1024);
    }

    TEST_CASE("integer to float to integer conversion is exact for modest signed coordinates") {
        const Salix::RectI source{-4096, 2048, 512, -256};
        const Salix::RectF as_float(source);
        const Salix::RectI restored(as_float);
        CHECK(restored == source);
    }

    TEST_CASE("floating rectangle preserves signed zero in each independently supplied field") {
        const Salix::RectF rect{-0.0f, 0.0f, -0.0f, 0.0f};
        CHECK(std::signbit(rect.x));
        CHECK_FALSE(std::signbit(rect.y));
        CHECK(std::signbit(rect.w));
        CHECK_FALSE(std::signbit(rect.h));
    }

    TEST_CASE("double rectangle accepts signed infinities without normalizing them") {
        const double inf = std::numeric_limits<double>::infinity();
        const Salix::RectD rect{-inf, inf, inf, -inf};
        CHECK(std::isinf(rect.x));
        CHECK(std::signbit(rect.x));
        CHECK(std::isinf(rect.y));
        CHECK_FALSE(std::signbit(rect.y));
        CHECK(std::isinf(rect.w));
        CHECK(std::isinf(rect.h));
        CHECK(std::signbit(rect.h));
    }

    TEST_CASE("JSON roundtrip preserves exact dyadic double rectangle values") {
        const Salix::RectD source{-0.125, 0.25, 1024.5, -2048.25};
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::RectD restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored == source);
    }

    TEST_CASE("binary roundtrip preserves signed fractional float rectangle values") {
        const Salix::RectF source{-1.5f, 2.25f, -3.75f, 4.5f};
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::RectF restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored == source);
    }

    TEST_CASE("self assignment preserves every double rectangle field") {
        Salix::RectD rect{-7.5, 8.25, 0.0, -11.125};
        const Salix::RectD expected = rect;
        rect = rect;
        CHECK(rect == expected);
    }
}

TEST_SUITE("Salix::math::RectClosure1747") {
    TEST_CASE("batch1747 rect aliases preserve their intended scalar member types") {
        CHECK((std::is_same_v<decltype(Salix::RectI{}.x), int>));
        CHECK((std::is_same_v<decltype(Salix::RectF{}.x), float>));
        CHECK((std::is_same_v<decltype(Salix::RectD{}.x), double>));
    }

    TEST_CASE("batch1747 rect converting constructor is explicit while remaining constructible") {
        CHECK((std::is_constructible_v<Salix::RectI, const Salix::RectF&>));
        CHECK_FALSE((std::is_convertible_v<Salix::RectF, Salix::RectI>));
        CHECK((std::is_constructible_v<Salix::RectD, const Salix::RectI&>));
    }

    TEST_CASE("batch1747 rect generic arithmetic constructor accepts bool and char members") {
        const Salix::RectI rect(true, false, static_cast<char>(7), static_cast<unsigned char>(9));
        CHECK(rect.x == 1);
        CHECK(rect.y == 0);
        CHECK(rect.w == 7);
        CHECK(rect.h == 9);
    }

    TEST_CASE("batch1747 rect mixed long long and unsigned construction converts each member independently") {
        const Salix::RectD rect(-123456789LL, 42ULL, -3.5f, 8);
        CHECK(rect.x == doctest::Approx(-123456789.0));
        CHECK(rect.y == doctest::Approx(42.0));
        CHECK(rect.w == doctest::Approx(-3.5));
        CHECK(rect.h == doctest::Approx(8.0));
    }

    TEST_CASE("batch1747 rect fractional to integer conversion truncates every component toward zero") {
        const Salix::RectD source(-0.999, 0.999, -7.999, 7.999);
        const Salix::RectI converted(source);
        CHECK(converted == Salix::RectI(0, 0, -7, 7));
    }

    TEST_CASE("batch1747 rect self assignment preserves all four members") {
        Salix::RectI rect(-5, 6, -7, 8);
        rect = rect;
        CHECK(rect == Salix::RectI(-5, 6, -7, 8));
    }

    TEST_CASE("batch1747 rect swapping values exchanges position and extents together") {
        Salix::RectF first(1.0f, 2.0f, 3.0f, 4.0f);
        Salix::RectF second(-1.0f, -2.0f, -3.0f, -4.0f);
        std::swap(first, second);
        CHECK(first == Salix::RectF(-1.0f, -2.0f, -3.0f, -4.0f));
        CHECK(second == Salix::RectF(1.0f, 2.0f, 3.0f, 4.0f));
    }

    TEST_CASE("batch1747 rect equality and inequality remain strict complements") {
        const Salix::RectD a(1.0, 2.0, 3.0, 4.0);
        const Salix::RectD b(1.0, 2.0, 3.0, 4.0);
        const Salix::RectD c(1.0, 2.0, 3.0, 4.5);
        CHECK(a == b);
        CHECK_FALSE(a != b);
        CHECK(a != c);
        CHECK_FALSE(a == c);
    }

    TEST_CASE("batch1747 rect JSON serialization is deterministic for identical values") {
        const Salix::RectI value(-12, 34, 56, -78);
        std::stringstream first, second;
        { cereal::JSONOutputArchive out(first); out(value); }
        { cereal::JSONOutputArchive out(second); out(value); }
        CHECK(first.str() == second.str());
    }

    TEST_CASE("batch1747 rect binary serialization is deterministic for identical double rectangles") {
        const Salix::RectD value(-0.125, 2.5, -16.75, 32.0);
        std::stringstream first, second;
        { cereal::BinaryOutputArchive out(first); out(value); }
        { cereal::BinaryOutputArchive out(second); out(value); }
        CHECK(first.str() == second.str());
    }
}

// ---- Relocated from src/tests/SalixEngine/math/Point.test.cpp; canonical tests mirror production .cpp files ----
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
