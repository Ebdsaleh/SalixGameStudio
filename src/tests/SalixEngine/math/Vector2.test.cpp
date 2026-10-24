// =================================================================================
// Filename:    src/Tests/SalixEngine/math/Vector2.test.cpp
// Description: Contains unit tests for the Vector2 functions.
// ================================================================================= 
// Tests/SalixEngine/math/Vector2.test.cpp
#include <doctest.h>
#include <Salix/math/Vector2.h>
#include <cereal/archives/json.hpp>
#include <sstream>
#include <string>
#include <yaml-cpp/yaml.h>

// This helper struct uses RAII (Resource Acquisition Is Initialization) to manage
// the redirection of std::cerr. When an object of this type is created, it
// redirects std::cerr. When it's destroyed (at the end of the scope), it
// automatically restores it.
struct CerrRedirect {
    CerrRedirect(std::stringstream& local_buffer) {
        // 1. Save the original buffer of std::cerr
        original_buffer = std::cerr.rdbuf();
        // 2. Redirect std::cerr to our local stringstream buffer
        std::cerr.rdbuf(local_buffer.rdbuf());
    }

    ~CerrRedirect() {
        // 3. Restore the original buffer when the object is destroyed
        std::cerr.rdbuf(original_buffer);
    }
    private:
    std::streambuf* original_buffer;
};

TEST_SUITE("Salix::math::Vector2") {

    TEST_CASE("constructors and initalization") {
        // ARRANGE
        Salix::Vector2 vec_1 = Salix::Vector2();
        Salix::Vector2 vec_2 = Salix::Vector2(10.0f, 10.0f);
        Salix::Vector2 vec_3;
        Salix::Vector2 vec_4(vec_2);
        ImVec2 im_vec = vec_2.to_im_vec2();

        CHECK(vec_1 == Salix::Vector2(0.0f, 0.0f));
        CHECK(Salix::Vector2(10.0f, 10.0f) == vec_2);
        CHECK(Salix::Vector2(0.0f, 0.0f) == vec_3);
        CHECK(vec_4 == vec_2);
        CHECK(im_vec.x == vec_2.x);
        CHECK(im_vec.y == vec_2.y);
    }

    TEST_CASE ("can correctly converts to ImVec2") {
        Salix::Vector2 vec_1(2.0f, 1.0f);
        ImVec2 vec2 = vec_1.to_im_vec2();

        CHECK(vec2.x == vec_1.x);
        CHECK(vec2.y == vec_1.y);
    }

    TEST_CASE("correctly calculates magnitude (length)") {
        Salix::Vector2 vec(3.0f, 4.0f); // A 3-4-5 right triangle
        float magnitude = vec.length();
        CHECK(magnitude == doctest::Approx(5.0f));
    }

    TEST_CASE("correctly performs linear interpolation") {
        Salix::Vector2 start(0.0f, 10.0f);
        Salix::Vector2 end(10.0f, 0.0f);
        
        // Test midpoint
        Salix::Vector2 midpoint = Salix::Vector2::lerp(start, end, 0.5f);
        CHECK(midpoint.x == doctest::Approx(5.0f));
        CHECK(midpoint.y == doctest::Approx(5.0f));

        // Test clamping (t > 1)
        Salix::Vector2 clamped_end = Salix::Vector2::lerp(start, end, 1.2f);
        CHECK(clamped_end.x == doctest::Approx(10.0f));
        CHECK(clamped_end.y == doctest::Approx(0.0f));
    }

    TEST_CASE("correctly normalizes a vector") {
        Salix::Vector2 vec(0.0f, 5.0f);
        vec.normalize();
        CHECK(vec.x == 0.0f);
        CHECK(vec.y == 1.0f);
        CHECK(vec.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("correctly add two vectors") {
        // ARRANGE: Create two vectors we want to add.
        Salix::Vector2 a(5.0f, 1.0f);
        Salix::Vector2 b(4.0f, 5.0f);

        // ACT: Perform the addition.
        Salix::Vector2 result = a + b;

        // ASSERT: Check if each component of the result is correct.
        CHECK(result.x == 9.f);
        CHECK(result.y == 6.0f);
    }



    TEST_CASE("calculate the dot product of two vectors") {
        // ARRANGE: Create two vectors that are perpendicular (orthogonal).
        Salix::Vector2 forward(2.0f, 1.0f);
        Salix::Vector2 right(1.0f, 1.0f);

        // ACT: Calculate the dot product.
        float result = Salix::dot(forward, right);

        // ASSERT: The dot product of perpendicular vectors should be 0.
        CHECK(result == 3.0f);
    }

    TEST_CASE("dot product of perpendicular vectors is zero") {
        // ARRANGE: Create two vectors that are exactly perpendicular.
        Salix::Vector2 up(0.0f, 1.0f);
        Salix::Vector2 right(1.0f, 0.0f);

        // ACT: Calculate the dot product.
        float result = Salix::dot(up, right);

        // ASSERT: The dot product of perpendicular vectors must be 0.
        // (0.0 * 1.0) + (1.0 * 0.0) = 0
        CHECK(result == 0.0f);
    }

    TEST_CASE("dot product of parallel vectors is positive") {
        // ARRANGE: Create two vectors pointing in the same direction.
        Salix::Vector2 a(2.0f, 3.0f);
        Salix::Vector2 b(4.0f, 6.0f);

        // ACT: Calculate the dot product.
        float result = Salix::dot(a, b);

        // ASSERT: The result should be positive.
        // (2.0 * 4.0) + (3.0 * 6.0) = 8 + 18 = 26
        CHECK(result > 0.0f);
        CHECK(result == 26.0f);
    }

    TEST_CASE("dot product of opposite vectors is negative") {
        // ARRANGE: Create two vectors pointing in opposite directions.
        Salix::Vector2 a(2.0f, 3.0f);
        Salix::Vector2 b(-2.0f, -3.0f);

        // ACT: Calculate the dot product.
        float result = Salix::dot(a, b);

        // ASSERT: The result should be negative.
        // (2.0 * -2.0) + (3.0 * -3.0) = -4 + -9 = -13
        CHECK(result < 0.0f);
        CHECK(result == -13.0f);
    }

    TEST_CASE("correctly calculates 2D pseudo-cross product") {
        // ARRANGE
        Salix::Vector2 a(2.0f, 3.0f);
        Salix::Vector2 b(2.0f, 2.0f);

        // ACT: Calculate the cross product of 2 Vector2's
        float cross = Salix::cross(a, b);

        // ASSERT: a.x * b.y - a.y * b.x == 4 - 6
        CHECK(cross == -2.0f);
        CHECK((a.x * b.y) - (b.x * a.y) == -2.0f);
    }

    TEST_CASE("correctly returns a Vector2 that is perpendicular to the source Vector2") {
        // ARRANGE
        Salix::Vector2 source_vec(2.0f, 3.0f);
        

        // ACT: Return a Vector2 perpendicular to the source vector. Vector2(-v.y, v.x)
        Salix::Vector2 result_vec = Salix::perp(source_vec);

        // ASSERT: result_vec == Vector2(-3.0f, 2.0f)
        CHECK(result_vec == Salix::Vector2(-3.0f, 2.0f));

    }

    TEST_CASE("correctly handles operator overloading") {
        // ARRANGE
        std::stringstream captured_output;
        CerrRedirect CerrRedirect(captured_output);
        Salix::Vector2 vec_1 = Salix::Vector2(10.0f, 20.0f);
        Salix::Vector2 vec_2 = Salix::Vector2(2.0f, 4.0f);
        Salix::Vector2 vec_3 = Salix::Vector2(10.0f, 20.0f);

        SUBCASE(" '==' operator") {
            // ASSERT: vec_1 should equal to vec_3.
            CHECK(vec_1 == vec_3);
        }
        SUBCASE("'!=' operator") {
            // ASSERT: vec_1 should not equal vec_2.
            CHECK(vec_1 != vec_2);
        }
        SUBCASE("'+' operator with Vector2") {
            // ASSERT: vec_1 + vec_2 == Salix::Vector2(12.0f, 24.0f)
            CHECK(vec_1 + vec_2 == Salix::Vector2(12.0f, 24.0f));
        }
        SUBCASE("'+' operator with float") {
            // ACT
            Salix::Vector2 vec_4 = vec_1 + 2.0f;
            Salix::Vector2 vec_5 = 2.0f + vec_1;
            // ASSERT: vec_1 + 2.0f == Salix::Vector2(12.0f, 22.0f)
            CHECK(vec_4 == Salix::Vector2(12.0f, 22.0f));
            CHECK(vec_5 == Salix::Vector2(12.0f, 22.0f));
        }
        SUBCASE("'-' operator with Vector2") {
            // ASSERT: vec_1 - vec_2 == Salix::Vector2(8.0f, 16.0f)
            CHECK(vec_1 - vec_2 == Salix::Vector2(8.0f, 16.0f));
        }
        SUBCASE("'-' operator with float") {
            // ARRANGE
            Salix::Vector2 vec_4 = vec_1 - 2.0f;
            Salix::Vector2 vec_5 = 2.0f - vec_1;
            // ASSERT: vec_1 - 2.0f == Salix::Vector2(8.0f, 18.0f)
            CHECK(vec_4   == Salix::Vector2(8.0f, 18.0f));
            // ASSERT: vec_5 == Salix::Vector2(-8.0f, -18.0f)
            CHECK(vec_5   == Salix::Vector2(-8.0f, -18.0f));
        }
        SUBCASE("'*' operator with Vector2") {
            // ASSERT: vec_1 * vec_2 == Salix::Vector2(20.0f, 80.0f)
            CHECK(vec_1 * vec_2 == Salix::Vector2(20.0f, 80.0f));
        }
        SUBCASE("'*' operator with float") {
            // ARRANGE
            Salix::Vector2 vec_4 = vec_1 * 2.0f;
            Salix::Vector2 vec_5 = 2.0f * vec_1;
            // ASSERT: vec_1 * 2.0f == Salix::Vector2(20.0f, 40.0f)
            CHECK(vec_4 == Salix::Vector2(20.0f, 40.0f));
            CHECK(vec_5 == Salix::Vector2(20.0f, 40.0f));
        }
        SUBCASE("'/' operator with Vector2") {
            // ASSERT: vec_1 / vec_2 == Salix::Vector2(5.0f, 5.0f)
            CHECK(vec_1 / vec_2 == Salix::Vector2(5.0f, 5.0f));
        }
        SUBCASE("'/' operator with float") {
            // ACT
            Salix::Vector2 vec_4  = Salix::Vector2(vec_1) / 2.0f;
            vec_1 /= 2.0f;
            // ASSERT
            CHECK(vec_1 == Salix::Vector2(5.0f, 10.0f));
            CHECK( vec_4 == Salix::Vector2(5.0f, 10.f));
    
        }
         SUBCASE("'/' operator with float as first operand") {
            // ACT
            Salix::Vector2 result = 20.0f / vec_2; // 20/2=10, 20/4=5
            
            // ASSERT
            CHECK(result == Salix::Vector2(10.0f, 5.0f));
        }
        SUBCASE("division by zero is handled safely") {
            // THIS SUBCASE TESTS THE SAFEGUARDS in BOTH '/=' and '/'
            // Test vec / 0.0f
            Salix::Vector2 result1 = vec_1 / 0.0f;
            CHECK(result1 == Salix::Vector2(0.0f, 0.0f));
            CHECK(captured_output.str().find("Warning: Division by zero scalar/divisor!") != std::string::npos);
            captured_output.str(""); // Clear the buffer for the next test

            // Test vec /= 0.0f
            vec_1 /= 0.0f;
            CHECK(vec_1 == Salix::Vector2(0.0f, 0.0f));
            
            // Test 0.0f / vec
            Salix::Vector2 result2 = 0.0f / vec_2;
            CHECK(result2 == Salix::Vector2(0.0f, 0.0f));
        }
        SUBCASE("'+=' operator with Vector2") {
            // ACT
            Salix::Vector2 vec_4(vec_1 += vec_2);
            // ASSERT: vec_1 += vec_2 == Salix::Vector2(12.0f, 24.0f)
            CHECK(vec_4 == Salix::Vector2(12.0f, 24.0f));
        }
        SUBCASE("'-=' operator with Vector2") {
            // ACT
            Salix::Vector2 vec_4(vec_1 -= vec_2);
            // ASSERT: vec_1 =- vec_2 == Salix::Vector2(8.0f, 16.0f)
            CHECK(vec_4 == Salix::Vector2(8.0f, 16.0f));
        }
        SUBCASE("'*=' operator") {
            // ACT
            Salix::Vector2 vec_4 = vec_1 *= 2.0f;
            Salix::Vector2 vec_5(vec_1);
            vec_5 *= 2.0f;
            // ASSERT: vec_1 *= 2.0f == Salix::Vector2(20.0f, 40.0f)
            CHECK(vec_4 == Salix::Vector2(20.0f, 40.0f));
            // ASSERT: vec_5 *= 2.0f == Salix::Vector2(40.0f, 80.0f)
            CHECK(vec_5 == Salix::Vector2(40.0f, 80.0f));
        }
        SUBCASE("'/=' operator") {
            // ACT
            Salix::Vector2 vec_4 = vec_1 /= 2.0f;
            // ASSERT: vec_4 /= 2.0f == Salix::Vector2(5.0f, 10.0f)
            CHECK( vec_4 == Salix::Vector2(5.0f, 10.0f));
        }
    }

    TEST_CASE("division by zero prints correct warnings") {
        // ARRANGE
        std::stringstream captured_output;
        CerrRedirect redirect(captured_output); // Redirect std::cerr
        
        Salix::Vector2 vec_1(10.0f, 20.0f);
        Salix::Vector2 vec_zero(0.0f, 0.0f);

        SUBCASE("Vector2 / Vector2") {
            // ACT
            Salix::Vector2 result = vec_1 / vec_zero;

            // ASSERT
            std::string error_message = captured_output.str();
            CHECK(result == Salix::Vector2(0.0f, 0.0f));
            CHECK(error_message.find("Warning: Division by zero on X-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Y-axis!") != std::string::npos);
        }

        SUBCASE("Vector2 / float") {
            // ACT
            Salix::Vector2 result = vec_1 / 0.0f;

            // ASSERT
            std::string error_message = captured_output.str();
            CHECK(result == Salix::Vector2(0.0f, 0.0f));
            CHECK(error_message.find("Warning: Division by zero scalar/divisor!") != std::string::npos);
        }

        SUBCASE("float / Vector2") {
            // ACT
            Salix::Vector2 result = 1.0f / vec_zero;

            // ASSERT
            std::string error_message = captured_output.str();
            CHECK(result == Salix::Vector2(0.0f, 0.0f));
            CHECK(error_message.find("Warning: Division by zero on X-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Y-axis!") != std::string::npos);
        }
    }

    TEST_CASE("can serialize and deserialize correctly") {
        // ARRANGE
        Salix::Vector2 original_vec(10.0f, 30.0f);
        std::stringstream ss;

        // ACT: Serialization
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(original_vec);
        }

        // ACT: Deserialization
        Salix::Vector2 loaded_vec;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(loaded_vec);
        }

        // ASSERT
        CHECK(loaded_vec == original_vec);
        CHECK(loaded_vec.x == original_vec.x);
        CHECK(loaded_vec.y == original_vec.y);
    }
    TEST_CASE("can be serialized and deserialized with YAML") {
        // This tests the YAML operator<< and operator>> in Color.cpp
        // ARRANGE
        Salix::Vector2 original_vec(0.1f, 0.2f);
        
        // ACT (Serialization)
        YAML::Emitter out;
        out << original_vec;
        
        // ACT (Deserialization)
        YAML::Node node = YAML::Load(out.c_str());
        Salix::Vector2 loaded_vec;
        node >> loaded_vec;

        // ASSERT
        CHECK(loaded_vec == original_vec);
        CHECK(loaded_vec.x == original_vec.x);
        CHECK(loaded_vec.y == original_vec.y);
    }
    
    TEST_CASE("correctly handle '<<' outputting to ostream") {
        // ARRANGE
        Salix::Vector2 vec_1(10.5f, 20.8f);
        std::stringstream ss;

        // ACT
        ss << vec_1;

        // ASSERT
        CHECK(ss.str() == "{ x: 10.5, y: 20.8 }");
    }
}

// Additional cases preserved from the separate regression suite.
// Vector2 geometry, arithmetic and serialization regression checks.
#include <doctest.h>
#include <Salix/math/Vector2.h>
#include <cmath>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::Vector2Regression") {
    TEST_CASE("vector2 normalizing a three four vector preserves direction and gives unit length") {
        Salix::Vector2 v{3.f, 4.f};
        v.normalize();
        CHECK(v.x == doctest::Approx(0.6f));
        CHECK(v.y == doctest::Approx(0.8f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector2 normalizing zero vector does not produce NaN") {
        Salix::Vector2 v{0.f, 0.f};
        v.normalize();
        CHECK(v == Salix::Vector2(0.f, 0.f));
        CHECK(std::isfinite(v.x));
        CHECK(std::isfinite(v.y));
    }

    TEST_CASE("vector2 lerp respects both clamp boundaries and midpoint") {
        const Salix::Vector2 a{-10.f, 5.f}, b{10.f, 15.f};
        CHECK(Salix::Vector2::lerp(a, b, -1.f) == a);
        CHECK(Salix::Vector2::lerp(a, b, 2.f) == b);
        CHECK(Salix::Vector2::lerp(a, b, .5f) == Salix::Vector2(0.f, 10.f));
    }

    TEST_CASE("vector2 vector addition followed by subtraction restores both coordinates") {
        const Salix::Vector2 a{-7.f, 13.f}, b{4.f, -9.f};
        CHECK((a + b) - b == a);
        CHECK((a - b) + b == a);
        CHECK(a + 5.f == 5.f + a);
    }

    TEST_CASE("vector2 component multiplication handles opposite signs and zero components") {
        const Salix::Vector2 a{-3.f, 7.f}, b{2.f, 0.f};
        CHECK(a * b == Salix::Vector2(-6.f, 0.f));
        CHECK(2.f * a == a * 2.f);
        CHECK(a * 1.f == a);
    }

    TEST_CASE("vector2 component division zero affects only the corresponding axis") {
        const auto result = Salix::Vector2(20.f, -8.f) / Salix::Vector2(0.f, 4.f);
        CHECK(result.x == 0.f);
        CHECK(result.y == doctest::Approx(-2.f));
    }

    TEST_CASE("vector2 scalar reciprocal division handles a zero coordinate independently") {
        const auto result = 12.f / Salix::Vector2(4.f, 0.f);
        CHECK(result.x == doctest::Approx(3.f));
        CHECK(result.y == 0.f);
        CHECK(Salix::Vector2(2.f, 9.f) / 0.f == Salix::Vector2(0.f, 0.f));
    }

    TEST_CASE("vector2 compound operators return their operand and compose correctly") {
        Salix::Vector2 v{3.f, 5.f};
        Salix::Vector2* const identity = &v;
        CHECK(&(v += Salix::Vector2(1.f, -2.f)) == identity);
        CHECK(&(v -= Salix::Vector2(2.f, 1.f)) == identity);
        CHECK(&(v *= 3.f) == identity);
        CHECK(&(v /= 2.f) == identity);
        CHECK(v == Salix::Vector2(3.f, 3.f));
    }

    TEST_CASE("vector2 perpendicular vector has zero dot product and cross encodes orientation") {
        const Salix::Vector2 v{3.f, 4.f};
        const auto normal = Salix::perp(v);
        CHECK(normal == Salix::Vector2(-4.f, 3.f));
        CHECK(Salix::dot(v, normal) == doctest::Approx(0.f));
        CHECK(Salix::cross(v, normal) == doctest::Approx(25.f));
        CHECK(Salix::cross(normal, v) == doctest::Approx(-25.f));
    }

    TEST_CASE("vector2 YAML map roundtrip restores both fields and absent field defaults to zero") {
        const Salix::Vector2 original{12.25f, -3.5f};
        YAML::Emitter output;
        output << original;
        Salix::Vector2 loaded;
        YAML::Node parsed = YAML::Load(output.c_str());
        parsed >> loaded;
        CHECK(loaded.x == doctest::Approx(original.x));
        CHECK(loaded.y == doctest::Approx(original.y));
        YAML::Node partial = YAML::Load("{x: 9}");
        partial >> loaded;
        CHECK(loaded.x == 9.f);
        CHECK(loaded.y == 0.f);
    }
}

// Signed vector geometry, interpolation and data roundtrip invariants.
TEST_SUITE("Salix::math::Vector2Additional20261019") {
    TEST_CASE("vector2 extended length is invariant under swapping axes and negating both coordinates") {
        const Salix::Vector2 a(5.f,-12.f), b(-12.f,5.f), c(-5.f,12.f);
        CHECK(a.length() == doctest::Approx(13.f));
        CHECK(b.length() == doctest::Approx(a.length()));
        CHECK(c.length() == doctest::Approx(a.length()));
    }

    TEST_CASE("vector2 extended dot product is symmetric over signed inputs") {
        const Salix::Vector2 a(2.f,-7.f), b(-3.f,4.f);
        CHECK(Salix::dot(a,b) == Salix::dot(b,a));
        CHECK(Salix::dot(a,b) == doctest::Approx(-34.f));
    }

    TEST_CASE("vector2 extended two perpendicular axes have zero dot and unit orientation") {
        const Salix::Vector2 x(1.f,0.f), y(0.f,1.f);
        CHECK(Salix::dot(x,y) == 0.f);
        CHECK(Salix::cross(x,y) == 1.f);
        CHECK(Salix::cross(y,x) == -1.f);
    }

    TEST_CASE("vector2 extended length scales by absolute scalar factor") {
        const Salix::Vector2 a(-3.f,4.f);
        CHECK((a * -2.f).length() == doctest::Approx(10.f));
        CHECK((a * 0.5f).length() == doctest::Approx(2.5f));
    }

    TEST_CASE("vector2 extended perpendicular rotation applied twice negates the original vector") {
        const Salix::Vector2 original(4.f,-9.f);
        CHECK(Salix::perp(Salix::perp(original)) == Salix::Vector2(-4.f,9.f));
        CHECK(Salix::dot(original,Salix::perp(original)) == 0.f);
    }

    TEST_CASE("vector2 extended interpolation quarter points compose to the correct midpoint") {
        const Salix::Vector2 a(-16.f,8.f), b(16.f,-8.f);
        const auto left = Salix::Vector2::lerp(a,b,0.25f);
        const auto right = Salix::Vector2::lerp(a,b,0.75f);
        CHECK(Salix::Vector2::lerp(left,right,0.5f) == Salix::Vector2(0.f,0.f));
    }

    TEST_CASE("vector2 extended normalizing a negative-axis vector preserves direction") {
        Salix::Vector2 v(-15.f,0.f);
        v.normalize();
        CHECK(v == Salix::Vector2(-1.f,0.f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector2 extended normalization does not alter values after a second normalization") {
        Salix::Vector2 v(5.f,12.f);
        v.normalize();
        const auto first=v;
        v.normalize();
        CHECK(v.x == doctest::Approx(first.x));
        CHECK(v.y == doctest::Approx(first.y));
    }

    TEST_CASE("vector2 extended cereal JSON roundtrip preserves fractions with opposite signs") {
        Salix::Vector2 source(-8.125f,256.5f);
        std::stringstream buffer;
        { cereal::JSONOutputArchive writer(buffer); writer(source); }
        Salix::Vector2 restored;
        { cereal::JSONInputArchive reader(buffer); reader(restored); }
        CHECK(restored == source);
    }

    TEST_CASE("vector2 extended YAML reloading replaces both previously set coordinates") {
        Salix::Vector2 value(40.f,50.f);
        YAML::Node node = YAML::Load("{x: -1.25, y: 7.5}");
        node >> value;
        CHECK(value.x == doctest::Approx(-1.25f));
        CHECK(value.y == doctest::Approx(7.5f));
    }
}

// Deterministic geometry and persistence boundary scenarios.
TEST_SUITE("Salix::Batch747::Vector2Batch747") {
    TEST_CASE("vector2 dot and cross are zero for the zero displacement") {
        const Salix::Vector2 z(0.f,0.f), v(-123.f,45.f);CHECK(Salix::dot(z,v)==0.f);CHECK(Salix::cross(z,v)==0.f);CHECK(Salix::cross(v,z)==0.f);
    }

    TEST_CASE("vector2 parallel negative vectors have zero signed area") {
        const Salix::Vector2 a(3.f,-7.f), b(-6.f,14.f);CHECK(Salix::cross(a,b)==0.f);CHECK(Salix::dot(a,b)==doctest::Approx(-116.f));
    }

    TEST_CASE("vector2 four perpendicular rotations restore the original signed displacement") {
        const Salix::Vector2 a(-7.f,12.f);CHECK(Salix::perp(Salix::perp(Salix::perp(Salix::perp(a))))==a);
    }

    TEST_CASE("vector2 scalar-first subtraction changes signs independently across axes") {
        const Salix::Vector2 v(-6.f,9.f);CHECK(4.f-v==Salix::Vector2(10.f,-5.f));CHECK(v-4.f==Salix::Vector2(-10.f,5.f));
    }

    TEST_CASE("vector2 component division applies the zero rule independently to Y") {
        const auto result=Salix::Vector2(-15.f,8.f)/Salix::Vector2(3.f,0.f);CHECK(result.x==doctest::Approx(-5.f));CHECK(result.y==0.f);
    }

    TEST_CASE("vector2 scalar-over-vector reciprocal handles both signed components") {
        const auto v=24.f/Salix::Vector2(-6.f,8.f);CHECK(v.x==doctest::Approx(-4.f));CHECK(v.y==doctest::Approx(3.f));
    }

    TEST_CASE("vector2 normalization of a negatively scaled direction retains unit magnitude") {
        Salix::Vector2 v=Salix::Vector2(8.f,15.f)*-2.f;v.normalize();CHECK(v.length()==doctest::Approx(1.f));CHECK(v.x==doctest::Approx(-8.f/17.f));CHECK(v.y==doctest::Approx(-15.f/17.f));
    }

    TEST_CASE("vector2 three-quarter interpolation preserves distinct axis trajectories") {
        const auto v=Salix::Vector2::lerp(Salix::Vector2(-4.f,12.f),Salix::Vector2(8.f,-8.f),0.75f);CHECK(v.x==doctest::Approx(5.f));CHECK(v.y==doctest::Approx(-3.f));
    }

    TEST_CASE("vector2 YAML empty mapping zeros both old channels on decode") {
        Salix::Vector2 v(100.f,-200.f);const YAML::Node node=YAML::Load("{}");node >> v;CHECK(v==Salix::Vector2(0.f,0.f));
    }

    TEST_CASE("vector2 JSON array of consecutive values restores ordering and signed axes") {
        Salix::Vector2 a(3.125f,-7.5f),b(-0.25f,16.f);std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(a,b);}Salix::Vector2 x,y;{cereal::JSONInputArchive in(ss);in(x,y);}CHECK(x==a);CHECK(y==b);
    }
}

TEST_SUITE("Salix::math::Vector2Closure1747") {
    TEST_CASE("batch1747 vector2 normalization converts three four vector to unit length") {
        Salix::Vector2 value(3.0f, 4.0f);
        value.normalize();
        CHECK(value.x == doctest::Approx(0.6f));
        CHECK(value.y == doctest::Approx(0.8f));
        CHECK(value.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch1747 vector2 normalization leaves exact zero vector unchanged") {
        Salix::Vector2 value(0.0f, 0.0f);
        value.normalize();
        CHECK(value.x == 0.0f);
        CHECK(value.y == 0.0f);
        CHECK(value.length() == 0.0f);
    }

    TEST_CASE("batch1747 vector2 lerp clamps fractions below zero and above one") {
        const Salix::Vector2 start(-2.0f, 8.0f);
        const Salix::Vector2 end(6.0f, -4.0f);
        CHECK(Salix::Vector2::lerp(start, end, -10.0f) == start);
        CHECK(Salix::Vector2::lerp(start, end, 10.0f) == end);
    }

    TEST_CASE("batch1747 vector2 lerp reversed endpoints mirrors complementary interpolation fraction") {
        const Salix::Vector2 a(-5.0f, 3.0f);
        const Salix::Vector2 b(7.0f, 11.0f);
        const auto forward = Salix::Vector2::lerp(a, b, 0.25f);
        const auto backward = Salix::Vector2::lerp(b, a, 0.75f);
        CHECK(forward.x == doctest::Approx(backward.x));
        CHECK(forward.y == doctest::Approx(backward.y));
    }

    TEST_CASE("batch1747 vector2 component division zeroes only axes with zero divisor") {
        const auto result = Salix::Vector2(8.0f, 9.0f) / Salix::Vector2(0.0f, 3.0f);
        CHECK(result.x == 0.0f);
        CHECK(result.y == doctest::Approx(3.0f));
    }

    TEST_CASE("batch1747 scalar divided by vector zeroes only axes whose vector component is zero") {
        const auto result = 12.0f / Salix::Vector2(4.0f, 0.0f);
        CHECK(result.x == doctest::Approx(3.0f));
        CHECK(result.y == 0.0f);
    }

    TEST_CASE("batch1747 vector2 compound division by zero resets both components") {
        Salix::Vector2 value(5.0f, -7.0f);
        Salix::Vector2& returned = (value /= 0.0f);
        CHECK(&returned == &value);
        CHECK(value == Salix::Vector2(0.0f, 0.0f));
    }

    TEST_CASE("batch1747 vector2 dot product identifies perpendicular axis basis vectors") {
        const Salix::Vector2 x_axis(1.0f, 0.0f);
        const Salix::Vector2 y_axis(0.0f, 1.0f);
        CHECK(Salix::dot(x_axis, y_axis) == 0.0f);
        CHECK(Salix::dot(x_axis, x_axis) == 1.0f);
    }

    TEST_CASE("batch1747 vector2 cross product changes sign when operand order is reversed") {
        const Salix::Vector2 a(2.0f, 3.0f);
        const Salix::Vector2 b(-4.0f, 5.0f);
        CHECK(Salix::cross(a, b) == doctest::Approx(-Salix::cross(b, a)));
        CHECK(Salix::cross(a, a) == doctest::Approx(0.0f));
    }

    TEST_CASE("batch1747 vector2 perpendicular operation is orthogonal and twice applied negates input") {
        const Salix::Vector2 value(3.5f, -2.0f);
        const auto once = Salix::perp(value);
        const auto twice = Salix::perp(once);
        CHECK(Salix::dot(value, once) == doctest::Approx(0.0f));
        CHECK(twice.x == doctest::Approx(-value.x));
        CHECK(twice.y == doctest::Approx(-value.y));
    }
}

TEST_SUITE("Salix::math::Vector2Batch2147") {
    TEST_CASE("batch2147 vector2 length follows Pythagorean magnitude") {
        CHECK(Salix::Vector2(3.0f, 4.0f).length() == doctest::Approx(5.0f));
    }
    TEST_CASE("batch2147 vector2 normalization produces unit length and preserves direction") {
        Salix::Vector2 v(3.0f, 4.0f); v.normalize();
        CHECK(v.length() == doctest::Approx(1.0f)); CHECK(v.x == doctest::Approx(0.6f)); CHECK(v.y == doctest::Approx(0.8f));
    }
    TEST_CASE("batch2147 vector2 normalization leaves zero vector unchanged") {
        Salix::Vector2 v; v.normalize(); CHECK(v.x == 0.0f); CHECK(v.y == 0.0f);
    }
    TEST_CASE("batch2147 vector2 lerp clamps values below zero to start") {
        CHECK(Salix::Vector2::lerp({1.0f, 2.0f}, {9.0f, 10.0f}, -1.0f) == Salix::Vector2(1.0f, 2.0f));
    }
    TEST_CASE("batch2147 vector2 lerp clamps values above one to end") {
        CHECK(Salix::Vector2::lerp({1.0f, 2.0f}, {9.0f, 10.0f}, 2.0f) == Salix::Vector2(9.0f, 10.0f));
    }
    TEST_CASE("batch2147 vector2 lerp computes midpoint componentwise") {
        CHECK(Salix::Vector2::lerp({-2.0f, 4.0f}, {6.0f, 12.0f}, 0.5f) == Salix::Vector2(2.0f, 8.0f));
    }
    TEST_CASE("batch2147 vector2 ImGui conversion retains signed coordinates") {
        Salix::Vector2 v(-3.25f, 7.5f); const auto im = v.to_im_vec2();
        CHECK(im.x == v.x); CHECK(im.y == v.y);
    }
    TEST_CASE("batch2147 vector2 YAML input defaults missing x to zero") {
        const YAML::Node node = YAML::Load("{y: 9.5}"); Salix::Vector2 v(1.0f, 2.0f); node >> v;
        CHECK(v.x == 0.0f); CHECK(v.y == doctest::Approx(9.5f));
    }
    TEST_CASE("batch2147 vector2 YAML input defaults missing y to zero") {
        const YAML::Node node = YAML::Load("{x: -4.25}"); Salix::Vector2 v(1.0f, 2.0f); node >> v;
        CHECK(v.x == doctest::Approx(-4.25f)); CHECK(v.y == 0.0f);
    }
    TEST_CASE("batch2147 vector2 normalization handles negative coordinates without changing quadrant") {
        Salix::Vector2 v(-6.0f, -8.0f); v.normalize();
        CHECK(v.x == doctest::Approx(-0.6f)); CHECK(v.y == doctest::Approx(-0.8f)); CHECK(v.length() == doctest::Approx(1.0f));
    }
}

