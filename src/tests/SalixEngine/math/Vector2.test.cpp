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