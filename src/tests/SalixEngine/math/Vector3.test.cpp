// =================================================================================
// Filename:    src/Tests/SalixEngine/math/Vector3.test.cpp
// Description: Contains unit tests for the Vector3 functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/math/Vector3.h>
#include <cereal/archives/json.hpp>
#include <sstream>
#include <string>
#include <yaml-cpp/yaml.h>

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
TEST_SUITE("Salix::math::Vector3") {
    
    TEST_CASE("constructors and instantiation") {
        SUBCASE("when using static const 'Zero'") {
            // ARRANGE
            Salix::Vector3 vec_zero = Salix::Vector3::Zero;
            // ASSERT
            CHECK(vec_zero.x == 0.0f);
            CHECK(vec_zero.y == 0.0f);
            CHECK(vec_zero.z == 0.0f);

            CHECK(Salix::Vector3::Zero.x == doctest::Approx(0.0f));
            CHECK(Salix::Vector3::Zero.y == doctest::Approx(0.0f));
            CHECK(Salix::Vector3::Zero.z == doctest::Approx(0.0f));
        }
        SUBCASE("when using static const 'One'") {
            // ARRANGE
            Salix::Vector3 vec_one = Salix::Vector3::One;
            // ASSERT
            CHECK(vec_one.x == 1.0f);
            CHECK(vec_one.y == 1.0f);
            CHECK(vec_one.z == 1.0f);

            CHECK(Salix::Vector3::One.x == doctest::Approx(1.0f));
            CHECK(Salix::Vector3::One.y == doctest::Approx(1.0f));
            CHECK(Salix::Vector3::One.z == doctest::Approx(1.0f));

        }
        SUBCASE("default contructor") {
            // ARRANGE
            Salix::Vector3 vec_default = Salix::Vector3();
            // ASSERT
            CHECK(vec_default.x == 0.0f);
            CHECK(vec_default.y == 0.0f);
            CHECK(vec_default.z == 0.0f);
        }
        SUBCASE("when using float values") {
            // ARRANGE
            Salix::Vector3 vec_floats = Salix::Vector3(0.0f, 1.0f, 2.0f);
            // ASSERT
            CHECK(vec_floats.x == 0.0f);
            CHECK(vec_floats.y == 1.0f);
            CHECK(vec_floats.z == 2.0f);
        }
        SUBCASE("when using 'glm::vec3'") {
            // ARRANGE
            glm::vec3 glm_vec = glm::vec3(1.0f, 2.0f, 3.0f);
            Salix::Vector3 vec_glm = Salix::Vector3(glm_vec);
            Salix::Vector3 vec_glm_1 = Salix::Vector3(glm::vec3(3.0f, 2.0f, 1.0f));
            // ASSERT
            CHECK(vec_glm.x == 1.0f);
            CHECK(vec_glm.y == 2.0f);
            CHECK(vec_glm.z == 3.0f);

            CHECK(vec_glm_1.x == 3.0f);
            CHECK(vec_glm_1.y == 2.0f);
            CHECK(vec_glm_1.z == 1.0f);
        }
        SUBCASE("when using float scalar") {
            // ARRANGE
            Salix::Vector3 vec_f_scalar = Salix::Vector3(3.0f);
            // ASSERT
            CHECK(vec_f_scalar.x == 3.0f);
            CHECK(vec_f_scalar.y == 3.0f);
            CHECK(vec_f_scalar.z == 3.0f);
        }
        SUBCASE("when using int scalar") {
            // ARRANGE
            Salix::Vector3 vec_i_scalar = Salix::Vector3(6);
            Salix::Vector3 vec_i_scalar_1(1);
            // ASSERT
            CHECK(vec_i_scalar.x == 6.0f);
            CHECK(vec_i_scalar.y == 6.0f);
            CHECK(vec_i_scalar.z == 6.0f);

            CHECK(vec_i_scalar_1.x == 1.0f);
            CHECK(vec_i_scalar_1.y == 1.0f);
            CHECK(vec_i_scalar_1.z == 1.0f);

            CHECK(Salix::Vector3(5).x == 5.0f);
            CHECK(Salix::Vector3(5).y == 5.0f);
            CHECK(Salix::Vector3(5).z == 5.0f);
        }
    }
    TEST_CASE("correctly handle operator overloads") {
        // ARRANGE
        std::stringstream captured_output;
        CerrRedirect CerrRedirect(captured_output);
        Salix::Vector3 vec_1(1.0f, 2.0f, 3.0f);
        Salix::Vector3 vec_2(4.0f, 5.0f, 6.0f);

        SUBCASE("'+' operator with Vector3") {
            // ARRANGE: Create two vectors we want to add.
            Salix::Vector3 a(1.0f, 2.0f, 3.0f);
            Salix::Vector3 b(4.0f, 5.0f, 6.0f);
        
            // ACT: Perform the addition.
            Salix::Vector3 result = a + b;

            // ASSERT: Check if each component of the result is correct.
            CHECK(result.x == 5.0f);
            CHECK(result.y == 7.0f);
            CHECK(result.z == 9.0f);
        }
        SUBCASE("'+' operator with float") {
            // ACT
            Salix::Vector3 result = vec_1 + 1.0f;
            Salix::Vector3 result_1 = 1.0f + vec_1;
            // ASSERT
            CHECK(result.x == 2.0f);
            CHECK(result.y == 3.0f);
            CHECK(result.z == 4.0f);

            CHECK(result_1.x == 2.0f);
            CHECK(result_1.y == 3.0f);
            CHECK(result_1.z == 4.0f);
        }
        SUBCASE("'-' operator Vector3") {
            // ARRANGE: Create two vectors we want to add.
            Salix::Vector3 a(1.0f, 2.0f, 3.0f);
            Salix::Vector3 b(4.0f, 5.0f, 6.0f);

            // ACT: Perform the subtraction
            Salix::Vector3 result = b - a;

            // ASSERT: Check if each component of the result is correct.
            CHECK(result.x == 3.0f);
            CHECK(result.y == 3.0f);
            CHECK(result.z == 3.0f);
        }
        SUBCASE("'-' operator with float") {
            // ACT
            Salix::Vector3 result = vec_1 - 1.0f;
            Salix::Vector3 result_1 = 1.0f - vec_1;
            // ASSERT
            CHECK(result.x == 0.0f);
            CHECK(result.y == 1.0f);
            CHECK(result.z == 2.0f);

            CHECK(result_1.x == 0.0f);
            CHECK(result_1.y == -1.0f);
            CHECK(result_1.z == -2.0f);
        }
        SUBCASE("'*' operator with Vector3") {
            // ARRANGE: Create two vectors we want to add.
            Salix::Vector3 a(1.0f, 2.0f, 3.0f);
            Salix::Vector3 b(4.0f, 5.0f, 6.0f);
        
            // ACT: Perform the addition.
            Salix::Vector3 result = a * b;

            // ASSERT: Check if each component of the result is correct.
            CHECK(result.x == 4.0f);
            CHECK(result.y == 10.0f);
            CHECK(result.z == 18.0f);
        }
        SUBCASE("'*' operator with float") {
            // ACT
            Salix::Vector3 result = vec_1 * 1.0f;
            Salix::Vector3 result_1 = 2.0f * vec_1;
            // ASSERT
            CHECK(result.x == 1.0f);
            CHECK(result.y == 2.0f);
            CHECK(result.z == 3.0f);

            CHECK(result_1.x == 2.0f);
            CHECK(result_1.y == 4.0f);
            CHECK(result_1.z == 6.0f);
        }
        SUBCASE(" '/' operator with Vector3") {
            // ARRANGE: Create two vectors we want to add.
            Salix::Vector3 a(4.0f, 6.0f, 24.0f);
            Salix::Vector3 b(1.0f, 3.0f, 6.0f);
        
            // ACT: Perform the addition.
            Salix::Vector3 result = a / b;

            // ASSERT: Check if each component of the result is correct.
            CHECK(result.x == 4.0f);
            CHECK(result.y == 2.0f);
            CHECK(result.z == 4.0f);
        }
        SUBCASE(" '/' operator with float") {
            // ACT
            Salix::Vector3 result = vec_1 / 1.0f;
            Salix::Vector3 result_1 = 6.0f / vec_1;
            // ASSERT
            CHECK(result.x == 1.0f);
            CHECK(result.y == 2.0f);
            CHECK(result.z == 3.0f);

            CHECK(result_1.x == 6.0f);
            CHECK(result_1.y == 3.0f);
            CHECK(result_1.z == 2.0f);

        }
        SUBCASE("division by zero is handled safely") {
            // ARRANGE
            // THIS SUBCASE TESTS THE SAFEGUARDS in BOTH '/=' and '/'
            // Test vec / 0.0f
            Salix::Vector3 result1 = vec_1 / 0.0f;
            CHECK(result1 == Salix::Vector3(0.0f, 0.0f, 0.0f));
            CHECK(captured_output.str().find("Warning: Division by zero scalar/divisor!") != std::string::npos);
            captured_output.str(""); // Clear the buffer for the next test

            // Test vec /= 0.0f
            vec_1 /= 0.0f;
            CHECK(vec_1 == Salix::Vector3(0.0f, 0.0f, 0.0f));
            
            // Test 0.0f / vec
            Salix::Vector3 result2 = 0.0f / vec_2;
            CHECK(result2 == Salix::Vector3(0.0f, 0.0f, 0.0f));
        }
        SUBCASE("'!= operator") {
            // ACT
            bool result = vec_1 != vec_2;

            // ASSERT
            CHECK(result == true);
        }
        SUBCASE("'==' operator") {
            // ARRANGE
            Salix::Vector3 vec_3(1.0f, 2.0f, 3.0f);

            // ACT
            bool result = vec_1 == vec_3;

            // ASSERT
            CHECK(result == true);
        }
        SUBCASE("'+=' operator") {
            // ACT
            vec_1 += vec_2;

            // ASSERT
            CHECK(vec_1 == Salix::Vector3(5.0f, 7.0f, 9.0f));
        }
        SUBCASE("'-=' operator") {
            // ARRANGE
            Salix::Vector3 vec_3(100);

            // ACT: 4.0f, 5.0f, 6.0f -= 1.0f, 2.0f, 3.0f   
            vec_3 =  vec_2 -= vec_1;

            // ASSERT: 4.0f, 5.0f, 6.0f -= 1.0f, 2.0f, 3.0f == Salix::Vector3(3.0f, 3.0f, 3.0f)
            CHECK(vec_3 == Salix::Vector3(3.0f, 3.0f, 3.0f));
        }
        SUBCASE("'*=' operator") {
            // ARRANGE
            Salix::Vector3 vec_3(100);
            
            // ACT: 4.0f, 5.0f, 6.0f *= 2.0f
            vec_3 =  vec_2 *= 2.0f;

            // ASSERT: 4.0f, 5.0f, 6.0f *= 2.0f == Salix::Vector3(8.0f, 10.0f, 12.0f)
            CHECK(vec_3 == Salix::Vector3(8.0f, 10.0f, 12.0f));
        }
        SUBCASE("'/=' operator") {
            // ARRANGE
            Salix::Vector3 vec_3(100);

            // ACT
            vec_3 /= 2.0f;

            // ASSERT
            CHECK(vec_3 == Salix::Vector3(50.0f, 50.0f, 50.0f));
        }
    }
    TEST_CASE("division by zero prints correct warnings") {
        // ARRANGE
        std::stringstream captured_output;
        CerrRedirect redirect(captured_output); // Redirect std::cerr
        
        Salix::Vector3 vec_1(10.0f, 20.0f, 30.0f);
        Salix::Vector3 vec_zero(0.0f, 0.0f, 0.0f);

        SUBCASE("Vector3 / Vector3") {
            // ACT
            Salix::Vector3 result = vec_1 / vec_zero;
            

            // ASSERT
            std::string error_message = captured_output.str();
            CHECK(result == Salix::Vector3(0.0f, 0.0f, 0.0f));
            CHECK(error_message.find("Warning: Division by zero on X-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Y-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Z-axis!") != std::string::npos);


        }
        SUBCASE("float / Vector3") {
            // ACT
            Salix::Vector3 result = 2.0f / vec_zero;

            // ASSERT
            std::string error_message = captured_output.str();
            CHECK(result == Salix::Vector3(0.0f, 0.0f, 0.0f));
            CHECK(error_message.find("Warning: Division by zero on X-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Y-axis!") != std::string::npos);
            CHECK(error_message.find("Warning: Division by zero on Z-axis!") != std::string::npos);
        }
    }
    
    TEST_CASE("correctly normalize a vector") {
        // ARRANGE: Create a vector that does not have a length of 1.
        Salix::Vector3 vec(0.0f, 5.0f, 0.0f);

        // ACT: Call the normalize function.
        vec.normalize();

        // ASSERT: Check that the new vector is a unit vector pointing up.
        CHECK(vec.x == 0.0f);
        CHECK(vec.y == 1.0f);
        CHECK(vec.z == 0.0f);

        // For floating-point numbers, it's best to check they are "approximately" equal.
        // doctest provides a handy Approx() tool for this.
        CHECK(vec.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("calculate the dot product of two vectors") {
        // ARRANGE: Create two vectors that are perpendicular (orthogonal).
        Salix::Vector3 forward(0.0f, 0.0f, 1.0f);
        Salix::Vector3 right(1.0f, 0.0f, 0.0f);

        // ACT: Calculate the dot product.
        float result = Salix::dot(forward, right);

        // ASSERT: The dot product of perpendicular vectors should be 0.
        CHECK(result == 0.0f);
    }

    TEST_CASE("calculate the product of two vectors") {
        // ARRANGE
        Salix::Vector3 vec_1(1.0f, 2.0f, 3.0f);
        Salix::Vector3 vec_2(2);

        // ACT: 
        Salix::Vector3 result = Salix::cross(vec_1, vec_2);
        // returns a Vector3(
        //  a.y * b.z - a.z * b.y,
        //  2 * 2 - 3 * 2 = -2
        //  a.z * b.x - a.x * b.z,
        //  3 * 2 - 1 * 2 = 4
        //  a.x * b.y - a.y * b.x )
        //  1 * 2 - 2 * 2 = -2

        // ASSERT: result == Salix::Vector3(-2, 4, -2)
        CHECK(result == Salix::Vector3(-2, 4, -2));

    }

    TEST_CASE("correctly calculates magnitude (length)") {
        Salix::Vector3 vec(3.0f, 4.0f, 0.0f);
        // A 3-4-5 right triangle in 3D space
        CHECK(vec.length() == doctest::Approx(5.0f));

        Salix::Vector3 vec2(1.0f, 1.0f, 1.0f);
        CHECK(vec2.length() == doctest::Approx(sqrt(3.0f)));
    }

    TEST_CASE("correctly performs linear interpolation (lerp)") {
        Salix::Vector3 start(0.0f, 10.0f, -5.0f);
        Salix::Vector3 end(10.0f, 0.0f, 5.0f);

        SUBCASE("midpoint interpolation") {
            Salix::Vector3 midpoint = Salix::Vector3::lerp(start, end, 0.5f);
            CHECK(midpoint.x == doctest::Approx(5.0f));
            CHECK(midpoint.y == doctest::Approx(5.0f));
            CHECK(midpoint.z == doctest::Approx(0.0f));
        }

        SUBCASE("clamps t value when greater than 1") {
            Salix::Vector3 clamped_end = Salix::Vector3::lerp(start, end, 1.5f);
            CHECK(clamped_end == end);
        }

        SUBCASE("clamps t value when less than 0") {
            Salix::Vector3 clamped_start = Salix::Vector3::lerp(start, end, -0.5f);
            CHECK(clamped_start == start);
        }
    }

    TEST_CASE("correctly converts to and from glm::vec3") {
        SUBCASE("to_glm") {
            Salix::Vector3 salix_vec(1.2f, 3.4f, 5.6f);
            glm::vec3 glm_vec = salix_vec.to_glm();
            CHECK(glm_vec.x == doctest::Approx(1.2f));
            CHECK(glm_vec.y == doctest::Approx(3.4f));
            CHECK(glm_vec.z == doctest::Approx(5.6f));
        }
        SUBCASE("from_glm") {
            glm::vec3 glm_vec(7.8f, 9.0f, 1.2f);
            Salix::Vector3 salix_vec = Salix::Vector3::from_glm(glm_vec);
            CHECK(salix_vec.x == doctest::Approx(7.8f));
            CHECK(salix_vec.y == doctest::Approx(9.0f));
            CHECK(salix_vec.z == doctest::Approx(1.2f));
        }
    }

    TEST_CASE("normalize handles zero vector safely") {
        // ARRANGE
        Salix::Vector3 zero_vec(0.0f, 0.0f, 0.0f);
        
        // ACT
        zero_vec.normalize(); // This should not cause a division by zero.
        
        // ASSERT
        // The vector should remain unchanged.
        CHECK(zero_vec.x == 0.0f);
        CHECK(zero_vec.y == 0.0f);
        CHECK(zero_vec.z == 0.0f);
    }

    TEST_CASE("can be serialized and deserialized") {
        SUBCASE("with Cereal (JSON)") {
            // ARRANGE
            Salix::Vector3 original_vec(10.1f, -20.2f, 30.3f);
            std::stringstream ss;

            // ACT: Serialization
            {
                cereal::JSONOutputArchive output_archive(ss);
                output_archive(original_vec);
            }

            // ACT: Deserialization
            Salix::Vector3 loaded_vec;
            {
                cereal::JSONInputArchive input_archive(ss);
                input_archive(loaded_vec);
            }

            // ASSERT
            CHECK(loaded_vec == original_vec);
        }
        SUBCASE("with YAML") {
            // ARRANGE
            Salix::Vector3 original_vec(1.1f, 2.2f, 3.3f);

            // ACT (Serialization)
            YAML::Emitter out;
            out << original_vec;

            // ACT (Deserialization)
            YAML::Node node = YAML::Load(out.c_str());
            Salix::Vector3 loaded_vec;
            node >> loaded_vec;

            // ASSERT
            CHECK(loaded_vec == original_vec);
        }
    }

    TEST_CASE("correctly outputs to ostream") {
        // ARRANGE
        Salix::Vector3 vec(1.5f, -2.5f, 3.0f);
        std::stringstream ss;

        // ACT
        ss << vec;

        // ASSERT
        CHECK(ss.str() == "{ x: 1.5, y: -2.5, z: 3 }");
    }

    TEST_CASE("glm::mat4 ostream operator works correctly") {
        // ARRANGE
        glm::mat4 identity_matrix(1.0f);
        std::stringstream ss;

        // ACT: Call the ostream operator from Vector3.h
        ss << identity_matrix;

        // ASSERT: Check that the output is not empty and matches the expected format from glm::to_string.
        // We don't need to check the exact numbers, just that the function ran and produced output.
        CHECK_FALSE(ss.str().empty());
        CHECK(ss.str().find("mat4x4") != std::string::npos);
    }

}

// Additional cases preserved from the separate regression suite.
// Vector3 orientation, conversion, arithmetic and persistence tests.
#include <doctest.h>
#include <Salix/math/Vector3.h>
#include <cmath>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::Vector3Regression") {
    TEST_CASE("vector3 normalization of a three component vector preserves direction") {
        Salix::Vector3 v{2.f, 3.f, 6.f};
        v.normalize();
        CHECK(v.x == doctest::Approx(2.f / 7.f));
        CHECK(v.y == doctest::Approx(3.f / 7.f));
        CHECK(v.z == doctest::Approx(6.f / 7.f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector3 normalizing the zero vector leaves finite zero components") {
        Salix::Vector3 v{};
        v.normalize();
        CHECK(v == Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(std::isfinite(v.x));
        CHECK(std::isfinite(v.y));
        CHECK(std::isfinite(v.z));
    }

    TEST_CASE("vector3 linear interpolation clamps and handles three axes independently") {
        const Salix::Vector3 a{-10.f, 3.f, 8.f}, b{10.f, 13.f, -8.f};
        CHECK(Salix::Vector3::lerp(a, b, -0.5f) == a);
        CHECK(Salix::Vector3::lerp(a, b, 1.5f) == b);
        CHECK(Salix::Vector3::lerp(a, b, 0.5f) == Salix::Vector3(0.f, 8.f, 0.f));
    }

    TEST_CASE("vector3 scalar constructors replicate values and predefined constants remain stable") {
        CHECK(Salix::Vector3(4) == Salix::Vector3(4.f, 4.f, 4.f));
        CHECK(Salix::Vector3(2.5f) == Salix::Vector3(2.5f, 2.5f, 2.5f));
        CHECK(Salix::Vector3::Zero == Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(Salix::Vector3::One == Salix::Vector3(1.f, 1.f, 1.f));
    }

    TEST_CASE("vector3 GLM conversion roundtrip preserves signed coordinates") {
        const Salix::Vector3 source{-1.5f, 0.f, 12.25f};
        const glm::vec3 glm_value = source.to_glm();
        CHECK(glm_value.x == source.x);
        CHECK(glm_value.y == source.y);
        CHECK(glm_value.z == source.z);
        CHECK(Salix::Vector3::from_glm(glm_value) == source);
    }

    TEST_CASE("vector3 component arithmetic is reversible with integral coordinates") {
        const Salix::Vector3 a{8.f, -5.f, 2.f}, b{-3.f, 10.f, 4.f};
        CHECK((a + b) - b == a);
        CHECK((a - b) + b == a);
        CHECK(a * b == Salix::Vector3(-24.f, -50.f, 8.f));
        CHECK(a + 2.f == 2.f + a);
    }

    TEST_CASE("vector3 dot product discriminates orthogonal aligned and opposite directions") {
        const Salix::Vector3 a{1.f, 0.f, 0.f};
        CHECK(Salix::dot(a, Salix::Vector3(0.f, 1.f, 0.f)) == 0.f);
        CHECK(Salix::dot(a, Salix::Vector3(4.f, 0.f, 0.f)) == 4.f);
        CHECK(Salix::dot(a, Salix::Vector3(-4.f, 0.f, 0.f)) == -4.f);
    }

    TEST_CASE("vector3 cross product has right handed orientation and anti symmetry") {
        const Salix::Vector3 x{1.f, 0.f, 0.f}, y{0.f, 1.f, 0.f};
        CHECK(Salix::cross(x, y) == Salix::Vector3(0.f, 0.f, 1.f));
        CHECK(Salix::cross(y, x) == Salix::Vector3(0.f, 0.f, -1.f));
        CHECK(Salix::dot(Salix::cross(x, y), x) == 0.f);
        CHECK(Salix::cross(x, x) == Salix::Vector3(0.f, 0.f, 0.f));
    }

    TEST_CASE("vector3 division by zero in one axis does not corrupt other components") {
        const auto v = Salix::Vector3(12.f, 8.f, -9.f) / Salix::Vector3(3.f, 0.f, -3.f);
        CHECK(v == Salix::Vector3(4.f, 0.f, 3.f));
        CHECK(Salix::Vector3(3.f, 6.f, 9.f) / 0.f == Salix::Vector3(0.f, 0.f, 0.f));
        const auto reciprocal = 18.f / Salix::Vector3(3.f, 0.f, 9.f);
        CHECK(reciprocal == Salix::Vector3(6.f, 0.f, 2.f));
    }

    TEST_CASE("vector3 YAML roundtrip and absent coordinate handling preserve schema") {
        const Salix::Vector3 source{1.5f, -2.75f, 0.125f};
        YAML::Emitter out;
        out << source;
        YAML::Node node = YAML::Load(out.c_str());
        Salix::Vector3 restored;
        node >> restored;
        CHECK(restored.x == doctest::Approx(source.x));
        CHECK(restored.y == doctest::Approx(source.y));
        CHECK(restored.z == doctest::Approx(source.z));
        YAML::Node partial = YAML::Load("{x: 4, z: -3}");
        partial >> restored;
        CHECK(restored == Salix::Vector3(4.f, 0.f, -3.f));
    }
}

// Vector-space algebra, normalization, identity and binary archive contracts.
#include <cereal/archives/binary.hpp>
TEST_SUITE("Salix::math::Vector3Additional") {
    TEST_CASE("vector3 additional Pythagorean length holds for nonplanar three four twelve vector") {
        const Salix::Vector3 value(3.f, 4.f, 12.f);
        CHECK(value.length() == doctest::Approx(13.f));
        CHECK(Salix::Vector3(-3.f, -4.f, -12.f).length() == doctest::Approx(13.f));
    }

    TEST_CASE("vector3 additional normalization preserves sign and makes non-axis direction unit length") {
        Salix::Vector3 value(-2.f, 3.f, -6.f);
        value.normalize();
        CHECK(value.length() == doctest::Approx(1.f));
        CHECK(value.x == doctest::Approx(-2.f / 7.f));
        CHECK(value.y == doctest::Approx(3.f / 7.f));
        CHECK(value.z == doctest::Approx(-6.f / 7.f));
    }

    TEST_CASE("vector3 additional lerp quarter-step interpolates each differently signed component") {
        const Salix::Vector3 a(-8.f, 4.f, 12.f);
        const Salix::Vector3 b(8.f, 12.f, -4.f);
        const auto result = Salix::Vector3::lerp(a, b, 0.25f);
        CHECK(result == Salix::Vector3(-4.f, 6.f, 8.f));
    }

    TEST_CASE("vector3 additional lerp reverse interpolation swaps endpoint and complement fraction") {
        const Salix::Vector3 a(0.125f, -0.25f, 1.f);
        const Salix::Vector3 b(0.625f, 0.5f, -1.f);
        const auto forward = Salix::Vector3::lerp(a, b, 0.25f);
        const auto reverse = Salix::Vector3::lerp(b, a, 0.75f);
        CHECK(forward.x == doctest::Approx(reverse.x));
        CHECK(forward.y == doctest::Approx(reverse.y));
        CHECK(forward.z == doctest::Approx(reverse.z));
    }

    TEST_CASE("vector3 additional componentwise multiplication retains independent signs") {
        const Salix::Vector3 a(-2.f, 3.f, -4.f);
        const Salix::Vector3 b(-5.f, 0.f, 7.f);
        CHECK(a * b == Salix::Vector3(10.f, 0.f, -28.f));
        CHECK(a * Salix::Vector3::One == a);
    }

    TEST_CASE("vector3 additional scalar addition commutes and preserves all three axes") {
        const Salix::Vector3 a(-7.f, 2.f, 11.f);
        CHECK(a + 5.f == 5.f + a);
        CHECK(a + 5.f == Salix::Vector3(-2.f, 7.f, 16.f));
    }

    TEST_CASE("vector3 additional scalar subtraction differs between scalar-first and vector-first") {
        const Salix::Vector3 a(2.f, -3.f, 10.f);
        CHECK(a - 5.f == Salix::Vector3(-3.f, -8.f, 5.f));
        CHECK(5.f - a == Salix::Vector3(3.f, 8.f, -5.f));
    }

    TEST_CASE("vector3 additional compound arithmetic keeps object identity and composes reversibly") {
        Salix::Vector3 value(2.f, -4.f, 6.f);
        Salix::Vector3* const before = &value;
        CHECK(&(value += Salix::Vector3(3.f, 2.f, -1.f)) == before);
        CHECK(&(value -= Salix::Vector3(3.f, 2.f, -1.f)) == before);
        CHECK(&(value *= 2.f) == before);
        CHECK(&(value /= 2.f) == before);
        CHECK(value == Salix::Vector3(2.f, -4.f, 6.f));
    }

    TEST_CASE("vector3 additional compound division by zero clears every component explicitly") {
        Salix::Vector3 value(2.f, -4.f, 6.f);
        value /= 0.f;
        CHECK(value == Salix::Vector3(0.f, 0.f, 0.f));
        CHECK(value.length() == 0.f);
    }

    TEST_CASE("vector3 additional binary serialization roundtrip preserves signed fractional vector components") {
        Salix::Vector3 source(-0.125f, 128.5f, -1024.25f);
        std::stringstream bytes;
        { cereal::BinaryOutputArchive archive(bytes); archive(source); }
        Salix::Vector3 loaded;
        { cereal::BinaryInputArchive archive(bytes); archive(loaded); }
        CHECK(loaded == source);
        CHECK(loaded.to_glm().z == source.z);
    }
}
