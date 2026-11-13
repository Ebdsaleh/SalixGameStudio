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
#include <glm/glm.hpp>

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

// Cross-product identities, GLM interoperability and archive preservation.
#include <cereal/types/vector.hpp>
#include <vector>
TEST_SUITE("Salix::math::Vector3Additional20261019") {
    TEST_CASE("vector3 extended three-dimensional magnitude agrees with dot of a vector with itself") {
        const Salix::Vector3 v(-2.f,3.f,6.f);
        CHECK(v.length() == doctest::Approx(7.f));
        CHECK(Salix::dot(v,v) == doctest::Approx(v.length()*v.length()));
    }

    TEST_CASE("vector3 extended cross product is orthogonal to both arbitrary nonparallel inputs") {
        const Salix::Vector3 a(2.f,-3.f,5.f), b(7.f,11.f,-1.f);
        const auto n = Salix::cross(a,b);
        CHECK(Salix::dot(n,a) == doctest::Approx(0.f));
        CHECK(Salix::dot(n,b) == doctest::Approx(0.f));
    }

    TEST_CASE("vector3 extended cyclic unit axes preserve right-handed cross-product orientation") {
        const Salix::Vector3 x(1.f,0.f,0.f), y(0.f,1.f,0.f), z(0.f,0.f,1.f);
        CHECK(Salix::cross(x,y) == z);
        CHECK(Salix::cross(y,z) == x);
        CHECK(Salix::cross(z,x) == y);
    }

    TEST_CASE("vector3 extended cross product flips direction when operands exchange order") {
        const Salix::Vector3 a(1.f,2.f,3.f), b(4.f,-5.f,6.f);
        const auto ab=Salix::cross(a,b), ba=Salix::cross(b,a);
        CHECK(ab == ba * -1.f);
    }

    TEST_CASE("vector3 extended linear interpolation maintains collinearity over opposite extremes") {
        const Salix::Vector3 a(-10.f,20.f,5.f), b(10.f,-20.f,-5.f);
        const auto mid=Salix::Vector3::lerp(a,b,0.5f);
        CHECK(mid == Salix::Vector3(0.f,0.f,0.f));
        CHECK(Salix::Vector3::lerp(a,b,0.25f) == a * 0.5f);
    }

    TEST_CASE("vector3 extended normalizing a negative Z unit direction never changes its orientation") {
        Salix::Vector3 v(0.f,0.f,-500.f);
        v.normalize();
        CHECK(v == Salix::Vector3(0.f,0.f,-1.f));
        CHECK(v.length() == doctest::Approx(1.f));
    }

    TEST_CASE("vector3 extended GLM interoperation roundtrips signed fractions") {
        const Salix::Vector3 value(0.125f,-40.5f,7.25f);
        const auto as_glm = value.to_glm();
        CHECK(as_glm.x == doctest::Approx(value.x));
        CHECK(as_glm.y == doctest::Approx(value.y));
        CHECK(Salix::Vector3::from_glm(as_glm) == value);
    }

    TEST_CASE("vector3 extended roundtrip arithmetic preserves every coordinate under component division") {
        const Salix::Vector3 base(3.f,-8.f,2.f), multiplier(-2.f,0.5f,4.f);
        CHECK((base * multiplier) / multiplier == base);
        CHECK((base + multiplier) - multiplier == base);
    }

    TEST_CASE("vector3 extended binary archive roundtrip preserves a unit axis and its negative") {
        std::vector<Salix::Vector3> source{Salix::Vector3(0.f,0.f,1.f),Salix::Vector3(0.f,0.f,-1.f)};
        std::stringstream data;
        { cereal::BinaryOutputArchive output(data); output(source); }
        std::vector<Salix::Vector3> restored;
        { cereal::BinaryInputArchive input(data); input(restored); }
        REQUIRE(restored.size()==2);
        CHECK(restored[0]==source[0]);
        CHECK(restored[1]==source[1]);
    }

    TEST_CASE("vector3 extended YAML decode replaces old values with a negative three-axis vector") {
        Salix::Vector3 value(20.f,30.f,40.f);
        const YAML::Node yaml=YAML::Load("{x: -2.5, y: -8, z: -0.125}");
        yaml >> value;
        CHECK(value.x == doctest::Approx(-2.5f));
        CHECK(value.y == doctest::Approx(-8.f));
        CHECK(value.z == doctest::Approx(-0.125f));
    }
}

// Deterministic geometry and persistence boundary scenarios.
TEST_SUITE("Salix::Batch747::Vector3Batch747") {
    TEST_CASE("vector3 cross of a vector with itself has no normal component") {
        const Salix::Vector3 v(-4.f,9.f,2.f);CHECK(Salix::cross(v,v)==Salix::Vector3(0.f,0.f,0.f));
    }

    TEST_CASE("vector3 signed scalar triple product agrees with right handed volume") {
        const Salix::Vector3 x(2.f,0.f,0.f),y(0.f,3.f,0.f),z(0.f,0.f,4.f);CHECK(Salix::dot(x,Salix::cross(y,z))==doctest::Approx(24.f));CHECK(Salix::dot(y,Salix::cross(x,z))==doctest::Approx(-24.f));
    }

    TEST_CASE("vector3 cross of parallel signed multiples has zero length") {
        const Salix::Vector3 a(2.f,-3.f,4.f),b(-6.f,9.f,-12.f);CHECK(Salix::cross(a,b).length()==0.f);
    }

    TEST_CASE("vector3 component-wise division by zero only clears the affected middle axis") {
        const auto q=Salix::Vector3(20.f,3.f,-8.f)/Salix::Vector3(-5.f,0.f,2.f);CHECK(q==Salix::Vector3(-4.f,0.f,-4.f));
    }

    TEST_CASE("vector3 reciprocal division handles a missing Z while preserving X and Y") {
        const auto v=12.f/Salix::Vector3(-3.f,4.f,0.f);CHECK(v==Salix::Vector3(-4.f,3.f,0.f));
    }

    TEST_CASE("vector3 normalization of zero vector leaves all coordinates finite") {
        Salix::Vector3 v(0.f,0.f,0.f);v.normalize();CHECK(v==Salix::Vector3(0.f,0.f,0.f));CHECK(std::isfinite(v.x));CHECK(std::isfinite(v.y));CHECK(std::isfinite(v.z));
    }

    TEST_CASE("vector3 scalar broadcast constructor does not change static unit basis") {
        const Salix::Vector3 scalar(7);CHECK(scalar==Salix::Vector3(7.f,7.f,7.f));CHECK(Salix::Vector3::One==Salix::Vector3(1.f,1.f,1.f));
    }

    TEST_CASE("vector3 GLM constructor roundtrips without interchanging Y and Z") {
        const glm::vec3 source(-1.25f,3.5f,-7.75f);const Salix::Vector3 result(source);CHECK(result.x==source.x);CHECK(result.y==source.y);CHECK(result.z==source.z);CHECK(result.to_glm()==source);
    }

    TEST_CASE("vector3 YAML map with absent Z clears old third coordinate") {
        Salix::Vector3 v(100.f,200.f,300.f);const YAML::Node n=YAML::Load("{x: -1.5, y: 2.25}");n >> v;CHECK(v==Salix::Vector3(-1.5f,2.25f,0.f));
    }

    TEST_CASE("vector3 JSON output records named axes separately for stable persistence") {
        Salix::Vector3 v(-0.5f,2.25f,4.f);std::stringstream ss;{cereal::JSONOutputArchive out(ss);out(v);}const auto encoded=ss.str();CHECK(encoded.find("\"x\"")!=std::string::npos);CHECK(encoded.find("\"y\"")!=std::string::npos);CHECK(encoded.find("\"z\"")!=std::string::npos);
    }
}

TEST_SUITE("Salix::math::Vector3Closure1747") {
    TEST_CASE("batch1747 vector3 normalization converts two three six vector to unit length") {
        Salix::Vector3 value(2.0f, 3.0f, 6.0f);
        CHECK(value.length() == doctest::Approx(7.0f));
        value.normalize();
        CHECK(value.x == doctest::Approx(2.0f / 7.0f));
        CHECK(value.y == doctest::Approx(3.0f / 7.0f));
        CHECK(value.z == doctest::Approx(6.0f / 7.0f));
        CHECK(value.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch1747 vector3 normalization leaves exact zero vector unchanged") {
        Salix::Vector3 value = Salix::Vector3::Zero;
        value.normalize();
        CHECK(value == Salix::Vector3::Zero);
    }

    TEST_CASE("batch1747 vector3 lerp clamps fractions outside zero to one interval") {
        const Salix::Vector3 start(-1.0f, 2.0f, 3.0f);
        const Salix::Vector3 end(9.0f, -4.0f, 7.0f);
        CHECK(Salix::Vector3::lerp(start, end, -1.0f) == start);
        CHECK(Salix::Vector3::lerp(start, end, 2.0f) == end);
    }

    TEST_CASE("batch1747 vector3 GLM conversion roundtrip preserves all components") {
        const Salix::Vector3 source(-1.25f, 2.5f, -3.75f);
        const glm::vec3 glm_value = source.to_glm();
        const Salix::Vector3 restored = Salix::Vector3::from_glm(glm_value);
        CHECK(restored == source);
    }

    TEST_CASE("batch1747 vector3 component division zeroes only axes with zero divisors") {
        const auto result = Salix::Vector3(12.0f, 15.0f, 20.0f) /
                            Salix::Vector3(3.0f, 0.0f, 5.0f);
        CHECK(result.x == doctest::Approx(4.0f));
        CHECK(result.y == 0.0f);
        CHECK(result.z == doctest::Approx(4.0f));
    }

    TEST_CASE("batch1747 scalar divided by vector zeroes every zero component independently") {
        const auto result = 18.0f / Salix::Vector3(0.0f, 3.0f, 0.0f);
        CHECK(result.x == 0.0f);
        CHECK(result.y == doctest::Approx(6.0f));
        CHECK(result.z == 0.0f);
    }

    TEST_CASE("batch1747 vector3 compound division by zero resets all components and returns self") {
        Salix::Vector3 value(5.0f, -7.0f, 9.0f);
        Salix::Vector3& returned = (value /= 0.0f);
        CHECK(&returned == &value);
        CHECK(value == Salix::Vector3::Zero);
    }

    TEST_CASE("batch1747 vector3 dot product identifies orthogonal basis vectors") {
        const Salix::Vector3 x(1.0f, 0.0f, 0.0f);
        const Salix::Vector3 y(0.0f, 1.0f, 0.0f);
        const Salix::Vector3 z(0.0f, 0.0f, 1.0f);
        CHECK(Salix::dot(x, y) == 0.0f);
        CHECK(Salix::dot(y, z) == 0.0f);
        CHECK(Salix::dot(z, x) == 0.0f);
    }

    TEST_CASE("batch1747 vector3 cross product follows right handed basis orientation") {
        const Salix::Vector3 x(1.0f, 0.0f, 0.0f);
        const Salix::Vector3 y(0.0f, 1.0f, 0.0f);
        CHECK(Salix::cross(x, y) == Salix::Vector3(0.0f, 0.0f, 1.0f));
        CHECK(Salix::cross(y, x) == Salix::Vector3(0.0f, 0.0f, -1.0f));
    }

    TEST_CASE("batch1747 vector3 cross result is perpendicular to both source vectors") {
        const Salix::Vector3 a(2.0f, -3.0f, 4.0f);
        const Salix::Vector3 b(-5.0f, 7.0f, 11.0f);
        const auto c = Salix::cross(a, b);
        CHECK(Salix::dot(c, a) == doctest::Approx(0.0f).epsilon(0.0001));
        CHECK(Salix::dot(c, b) == doctest::Approx(0.0f).epsilon(0.0001));
    }
}

TEST_SUITE("Salix::math::Vector3Batch2147") {
    TEST_CASE("batch2147 vector3 length follows three-dimensional Pythagorean magnitude") {
        CHECK(Salix::Vector3(2.0f, 3.0f, 6.0f).length() == doctest::Approx(7.0f));
    }
    TEST_CASE("batch2147 vector3 normalization produces a unit vector") {
        Salix::Vector3 v(0.0f, 3.0f, 4.0f); v.normalize();
        CHECK(v.x == 0.0f); CHECK(v.y == doctest::Approx(0.6f)); CHECK(v.z == doctest::Approx(0.8f)); CHECK(v.length() == doctest::Approx(1.0f));
    }
    TEST_CASE("batch2147 vector3 normalization leaves zero vector unchanged") {
        Salix::Vector3 v; v.normalize(); CHECK(v == Salix::Vector3::Zero);
    }
    TEST_CASE("batch2147 vector3 lerp clamps negative interpolation to start") {
        CHECK(Salix::Vector3::lerp({1.0f,2.0f,3.0f},{4.0f,5.0f,6.0f},-2.0f) == Salix::Vector3(1.0f,2.0f,3.0f));
    }
    TEST_CASE("batch2147 vector3 lerp clamps oversized interpolation to end") {
        CHECK(Salix::Vector3::lerp({1.0f,2.0f,3.0f},{4.0f,5.0f,6.0f},2.0f) == Salix::Vector3(4.0f,5.0f,6.0f));
    }
    TEST_CASE("batch2147 vector3 lerp computes midpoint componentwise") {
        CHECK(Salix::Vector3::lerp({-2.0f,0.0f,8.0f},{2.0f,10.0f,12.0f},0.5f) == Salix::Vector3(0.0f,5.0f,10.0f));
    }
    TEST_CASE("batch2147 vector3 to glm preserves each coordinate") {
        const Salix::Vector3 v(-1.25f,2.5f,9.0f); const auto g=v.to_glm();
        CHECK(g.x==v.x); CHECK(g.y==v.y); CHECK(g.z==v.z);
    }
    TEST_CASE("batch2147 vector3 from glm preserves each coordinate") {
        const glm::vec3 g(7.0f,-8.0f,0.125f); const auto v=Salix::Vector3::from_glm(g);
        CHECK(v.x==g.x); CHECK(v.y==g.y); CHECK(v.z==g.z);
    }
    TEST_CASE("batch2147 vector3 YAML input defaults omitted coordinates to zero") {
        const YAML::Node node=YAML::Load("{y: 5.5}"); Salix::Vector3 v(1.0f,2.0f,3.0f); node >> v;
        CHECK(v.x==0.0f); CHECK(v.y==doctest::Approx(5.5f)); CHECK(v.z==0.0f);
    }
    TEST_CASE("batch2147 vector3 exported zero and one constants retain canonical values") {
        CHECK(Salix::Vector3::Zero == Salix::Vector3(0.0f,0.0f,0.0f));
        CHECK(Salix::Vector3::One == Salix::Vector3(1.0f,1.0f,1.0f));
    }
}

TEST_SUITE("Salix::math::Vector3Batch3147") {
    TEST_CASE("batch3147 vector3 six eight zero triangle has magnitude ten") {
        CHECK(Salix::Vector3(6.0f, 8.0f, 0.0f).length() == doctest::Approx(10.0f));
    }

    TEST_CASE("batch3147 vector3 normalization preserves signed direction and unit magnitude") {
        Salix::Vector3 v(-2.0f, 0.0f, 0.0f);
        v.normalize();
        CHECK(v == Salix::Vector3(-1.0f, 0.0f, 0.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch3147 vector3 zero normalization remains canonical Zero constant") {
        Salix::Vector3 v;
        v.normalize();
        CHECK(v == Salix::Vector3::Zero);
    }

    TEST_CASE("batch3147 vector3 lerp clamps negative interpolation to start") {
        const Salix::Vector3 a(-1.0f, 2.0f, -3.0f), b(4.0f, -5.0f, 6.0f);
        CHECK(Salix::Vector3::lerp(a, b, -0.25f) == a);
    }

    TEST_CASE("batch3147 vector3 lerp clamps oversized interpolation to end") {
        const Salix::Vector3 a(-1.0f, 2.0f, -3.0f), b(4.0f, -5.0f, 6.0f);
        CHECK(Salix::Vector3::lerp(a, b, 1.25f) == b);
    }

    TEST_CASE("batch3147 vector3 interpolation computes independent quarter point axes") {
        const auto v = Salix::Vector3::lerp({0.0f, 4.0f, 8.0f}, {8.0f, -4.0f, 0.0f}, 0.25f);
        CHECK(v.x == doctest::Approx(2.0f)); CHECK(v.y == doctest::Approx(2.0f)); CHECK(v.z == doctest::Approx(6.0f));
    }

    TEST_CASE("batch3147 vector3 GLM roundtrip preserves diagnostic signed coordinates") {
        const Salix::Vector3 original(-7.25f, 0.125f, 1024.5f);
        const auto restored = Salix::Vector3::from_glm(original.to_glm());
        CHECK(restored == original);
    }

    TEST_CASE("batch3147 vector3 YAML partial decode clears both omitted axes") {
        Salix::Vector3 v(1.0f, 2.0f, 3.0f);
        const YAML::Node node = YAML::Load("{z: -9.5}");
        node >> v;
        CHECK(v.x == 0.0f); CHECK(v.y == 0.0f); CHECK(v.z == doctest::Approx(-9.5f));
    }

    TEST_CASE("batch3147 vector3 YAML emitter roundtrips all signed finite coordinates") {
        const Salix::Vector3 original(-0.5f, 2.75f, -8.125f);
        YAML::Emitter out; out << original;
        Salix::Vector3 restored;
        YAML::Load(out.c_str()) >> restored;
        CHECK(restored.x == doctest::Approx(original.x));
        CHECK(restored.y == doctest::Approx(original.y));
        CHECK(restored.z == doctest::Approx(original.z));
    }

    TEST_CASE("batch3147 vector3 compound scale and divide return self and restore original value") {
        Salix::Vector3 v(1.5f, -2.0f, 4.0f);
        const Salix::Vector3 original = v;
        Salix::Vector3& mul_ref = (v *= 4.0f);
        CHECK(&mul_ref == &v);
        Salix::Vector3& div_ref = (v /= 4.0f);
        CHECK(&div_ref == &v);
        CHECK(v == original);
    }
}
TEST_SUITE("Salix::math::Vector3Batch3547") {
    TEST_CASE("batch3547 float scalar constructor broadcasts to all axes") {
        const Salix::Vector3 v(3.5f);
        CHECK(v == Salix::Vector3(3.5f, 3.5f, 3.5f));
    }

    TEST_CASE("batch3547 integer scalar constructor broadcasts exact float representation") {
        const Salix::Vector3 v(-7);
        CHECK(v == Salix::Vector3(-7.0f, -7.0f, -7.0f));
    }

    TEST_CASE("batch3547 vector3 dot with itself equals squared length for two three six vector") {
        const Salix::Vector3 v(2.0f, 3.0f, 6.0f);
        CHECK(Salix::dot(v, v) == doctest::Approx(49.0f));
        CHECK(Salix::dot(v, v) == doctest::Approx(v.length() * v.length()));
    }

    TEST_CASE("batch3547 vector3 cross with itself is canonical zero") {
        const Salix::Vector3 v(2.0f, -3.0f, 5.0f);
        CHECK(Salix::cross(v, v) == Salix::Vector3::Zero);
    }

    TEST_CASE("batch3547 vector3 cross product is antisymmetric componentwise") {
        const Salix::Vector3 a(2.0f, -3.0f, 5.0f);
        const Salix::Vector3 b(7.0f, 11.0f, -13.0f);
        const auto ab = Salix::cross(a, b);
        const auto ba = Salix::cross(b, a);
        CHECK(ab == ba * -1.0f);
    }

    TEST_CASE("batch3547 scalar addition overloads are symmetric") {
        const Salix::Vector3 v(-1.0f, 2.0f, -3.0f);
        CHECK(v + 4.0f == 4.0f + v);
        CHECK(v + 4.0f == Salix::Vector3(3.0f, 6.0f, 1.0f));
    }

    TEST_CASE("batch3547 scalar subtraction overloads preserve operand order") {
        const Salix::Vector3 v(1.0f, 2.0f, 3.0f);
        CHECK(v - 2.0f == Salix::Vector3(-1.0f, 0.0f, 1.0f));
        CHECK(10.0f - v == Salix::Vector3(9.0f, 8.0f, 7.0f));
    }

    TEST_CASE("batch3547 GLM converting constructor preserves signed coordinates") {
        const glm::vec3 source(-1.25f, 2.5f, -3.75f);
        const Salix::Vector3 converted(source);
        CHECK(converted.x == source.x);
        CHECK(converted.y == source.y);
        CHECK(converted.z == source.z);
    }

    TEST_CASE("batch3547 compound division by zero returns self and resets to canonical zero") {
        Salix::Vector3 v(4.0f, -5.0f, 6.0f);
        Salix::Vector3& ref = (v /= 0.0f);
        CHECK(&ref == &v);
        CHECK(v == Salix::Vector3::Zero);
    }

    TEST_CASE("batch3547 YAML empty mapping clears all previously populated axes") {
        Salix::Vector3 v(9.0f, -8.0f, 7.0f);
        const YAML::Node node = YAML::Load("{}");
        node >> v;
        CHECK(v == Salix::Vector3::Zero);
    }
}

TEST_SUITE("Salix::math::Vector3Batch3947") {
    TEST_CASE("batch3947 default vector remains canonical zero constant") {
        const Salix::Vector3 v;
        CHECK(v == Salix::Vector3::Zero);
    }

    TEST_CASE("batch3947 one two two vector has magnitude three") {
        CHECK(Salix::Vector3(1.0f, 2.0f, 2.0f).length() == doctest::Approx(3.0f));
    }

    TEST_CASE("batch3947 normalization preserves signed zero three four direction") {
        Salix::Vector3 v(0.0f, -3.0f, 4.0f);
        v.normalize();
        CHECK(v.x == doctest::Approx(0.0f));
        CHECK(v.y == doctest::Approx(-0.6f));
        CHECK(v.z == doctest::Approx(0.8f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch3947 three quarter interpolation advances all axes independently") {
        const auto v = Salix::Vector3::lerp({0.0f, 4.0f, -8.0f}, {8.0f, -4.0f, 8.0f}, 0.75f);
        CHECK(v.x == doctest::Approx(6.0f));
        CHECK(v.y == doctest::Approx(-2.0f));
        CHECK(v.z == doctest::Approx(4.0f));
    }

    TEST_CASE("batch3947 component multiplication preserves three independent products") {
        const auto v = Salix::Vector3(-2.0f, 3.0f, -4.0f) * Salix::Vector3(5.0f, -6.0f, -7.0f);
        CHECK(v == Salix::Vector3(-10.0f, -18.0f, 28.0f));
    }

    TEST_CASE("batch3947 scalar multiplication overloads remain symmetric") {
        const Salix::Vector3 v(-1.0f, 2.0f, -3.0f);
        CHECK(v * 4.0f == 4.0f * v);
        CHECK(v * 4.0f == Salix::Vector3(-4.0f, 8.0f, -12.0f));
    }

    TEST_CASE("batch3947 component division zeroes only zero divisor axis") {
        const auto v = Salix::Vector3(8.0f, 9.0f, 10.0f) / Salix::Vector3(2.0f, 3.0f, 0.0f);
        CHECK(v.x == doctest::Approx(4.0f));
        CHECK(v.y == doctest::Approx(3.0f));
        CHECK(v.z == 0.0f);
    }

    TEST_CASE("batch3947 GLM roundtrip preserves representative coordinates exactly") {
        const Salix::Vector3 original(-0.125f, 16.5f, -2048.0f);
        const auto restored = Salix::Vector3::from_glm(original.to_glm());
        CHECK(restored == original);
    }

    TEST_CASE("batch3947 YAML x only decode clears both omitted axes") {
        Salix::Vector3 v(9.0f, 8.0f, 7.0f);
        const YAML::Node node = YAML::Load("{x: 1.25}");
        node >> v;
        CHECK(v.x == doctest::Approx(1.25f));
        CHECK(v.y == 0.0f);
        CHECK(v.z == 0.0f);
    }

    TEST_CASE("batch3947 vector3 ostream preserves signed component order") {
        const Salix::Vector3 v(-1.5f, 2.25f, -3.75f);
        std::stringstream stream;
        stream << v;
        CHECK(stream.str() == "{ x: -1.5, y: 2.25, z: -3.75 }");
    }
}

TEST_SUITE("Salix::math::Vector3Batch4247") {
    TEST_CASE("batch4247 two three six vector has magnitude seven") {
        CHECK(Salix::Vector3(2.0f, 3.0f, 6.0f).length() == doctest::Approx(7.0f));
    }

    TEST_CASE("batch4247 normalization preserves negative Z axis direction") {
        Salix::Vector3 v(0.0f, 0.0f, -5.0f);
        v.normalize();
        CHECK(v == Salix::Vector3(0.0f, 0.0f, -1.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch4247 zero vector normalization remains canonical zero") {
        Salix::Vector3 v;
        v.normalize();
        CHECK(v == Salix::Vector3::Zero);
    }

    TEST_CASE("batch4247 midpoint interpolation advances all three axes independently") {
        const auto v = Salix::Vector3::lerp({-4.0f, 8.0f, 12.0f}, {4.0f, -8.0f, -4.0f}, 0.5f);
        CHECK(v == Salix::Vector3(0.0f, 0.0f, 4.0f));
    }

    TEST_CASE("batch4247 orthogonal basis vectors have zero dot product") {
        CHECK(Salix::dot(Salix::Vector3(1.0f, 0.0f, 0.0f),
                         Salix::Vector3(0.0f, 1.0f, 0.0f)) == doctest::Approx(0.0f));
    }

    TEST_CASE("batch4247 canonical basis cross product produces positive Z") {
        CHECK(Salix::cross(Salix::Vector3(1.0f, 0.0f, 0.0f),
                           Salix::Vector3(0.0f, 1.0f, 0.0f)) ==
              Salix::Vector3(0.0f, 0.0f, 1.0f));
    }

    TEST_CASE("batch4247 GLM roundtrip preserves mixed signed coordinates") {
        const Salix::Vector3 original(-42.0f, 4.247f, 17.0f);
        CHECK(Salix::Vector3::from_glm(original.to_glm()) == original);
    }

    TEST_CASE("batch4247 component division zeroes only zero divisor components") {
        const auto v = Salix::Vector3(12.0f, -21.0f, 8.0f) /
                       Salix::Vector3(3.0f, 0.0f, -2.0f);
        CHECK(v.x == doctest::Approx(4.0f));
        CHECK(v.y == 0.0f);
        CHECK(v.z == doctest::Approx(-4.0f));
    }

    TEST_CASE("batch4247 YAML y only decode clears omitted x and z") {
        Salix::Vector3 v(1.0f, 2.0f, 3.0f);
        YAML::Load("{y: -4.247}") >> v;
        CHECK(v.x == 0.0f);
        CHECK(v.y == doctest::Approx(-4.247f));
        CHECK(v.z == 0.0f);
    }

    TEST_CASE("batch4247 compound add and subtract restore original vector") {
        Salix::Vector3 v(-1.0f, 2.0f, -3.0f);
        const Salix::Vector3 original = v;
        v += Salix::Vector3(4.0f, 5.0f, 6.0f);
        v -= Salix::Vector3(4.0f, 5.0f, 6.0f);
        CHECK(v == original);
    }
}

TEST_SUITE("Salix::math::Vector3Batch4647") {
    TEST_CASE("batch4647 floating scalar constructor broadcasts identical value to every axis") {
        const Salix::Vector3 v(-4.647f);
        CHECK(v == Salix::Vector3(-4.647f, -4.647f, -4.647f));
    }

    TEST_CASE("batch4647 normalization preserves signed direction across three nonzero axes") {
        Salix::Vector3 v(-2.0f, 3.0f, -6.0f);
        REQUIRE(v.length() == doctest::Approx(7.0f));
        v.normalize();
        CHECK(v.x == doctest::Approx(-2.0f / 7.0f));
        CHECK(v.y == doctest::Approx(3.0f / 7.0f));
        CHECK(v.z == doctest::Approx(-6.0f / 7.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch4647 scalar addition overloads are symmetric across all axes") {
        const Salix::Vector3 v(-1.0f, 2.0f, -3.0f);
        CHECK(v + 4.0f == 4.0f + v);
        CHECK(v + 4.0f == Salix::Vector3(3.0f, 6.0f, 1.0f));
    }

    TEST_CASE("batch4647 scalar subtraction overloads preserve operand order") {
        const Salix::Vector3 v(1.0f, 2.0f, 3.0f);
        CHECK(v - 1.0f == Salix::Vector3(0.0f, 1.0f, 2.0f));
        CHECK(10.0f - v == Salix::Vector3(9.0f, 8.0f, 7.0f));
    }

    TEST_CASE("batch4647 component multiplication preserves independent signed products") {
        const auto product = Salix::Vector3(-2.0f, 3.0f, -4.0f) *
                             Salix::Vector3(5.0f, -6.0f, -7.0f);
        CHECK(product == Salix::Vector3(-10.0f, -18.0f, 28.0f));
    }

    TEST_CASE("batch4647 component division zeroes only zero divisor axes") {
        std::stringstream ignored;
        CerrRedirect redirect(ignored);
        const auto result = Salix::Vector3(12.0f, 15.0f, -20.0f) /
                            Salix::Vector3(0.0f, 3.0f, 0.0f);
        CHECK(result == Salix::Vector3(0.0f, 5.0f, 0.0f));
    }

    TEST_CASE("batch4647 reciprocal scalar division preserves nonzero axes and zeroes missing axis") {
        std::stringstream ignored;
        CerrRedirect redirect(ignored);
        const auto result = 24.0f / Salix::Vector3(-6.0f, 0.0f, 8.0f);
        CHECK(result == Salix::Vector3(-4.0f, 0.0f, 3.0f));
    }

    TEST_CASE("batch4647 compound multiply then divide returns self and restores original value") {
        Salix::Vector3 v(1.25f, -2.5f, 5.0f);
        const Salix::Vector3 original = v;
        Salix::Vector3& mul_ref = (v *= 8.0f);
        CHECK(&mul_ref == &v);
        Salix::Vector3& div_ref = (v /= 8.0f);
        CHECK(&div_ref == &v);
        CHECK(v == original);
    }

    TEST_CASE("batch4647 dot self equals squared length and cross self remains zero") {
        const Salix::Vector3 v(2.0f, -3.0f, 6.0f);
        CHECK(Salix::dot(v, v) == doctest::Approx(v.length() * v.length()));
        CHECK(Salix::dot(v, v) == doctest::Approx(49.0f));
        CHECK(Salix::cross(v, v) == Salix::Vector3::Zero);
    }

    TEST_CASE("batch4647 binary archive roundtrip preserves signed finite coordinates") {
        const Salix::Vector3 original(-4.647f, 7.464f, -0.125f);
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(original); }
        Salix::Vector3 restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored == original);
    }
}

TEST_SUITE("Salix::math::Vector3Batch5147") {
    TEST_CASE("batch5147 integer scalar constructor broadcasts exact value to every axis") {
        const Salix::Vector3 v(-5147);
        CHECK(v == Salix::Vector3(-5147.0f, -5147.0f, -5147.0f));
    }

    TEST_CASE("batch5147 normalizing zero vector is an exact no op") {
        Salix::Vector3 v = Salix::Vector3::Zero;
        v.normalize();
        CHECK(v == Salix::Vector3::Zero);
        CHECK(v.length() == 0.0f);
    }

    TEST_CASE("batch5147 positive Z axis normalization produces canonical unit axis") {
        Salix::Vector3 v(0.0f, 0.0f, 9.0f);
        v.normalize();
        CHECK(v == Salix::Vector3(0.0f, 0.0f, 1.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch5147 interpolation clamps both outside endpoints") {
        const Salix::Vector3 a(-5.0f, 1.0f, 4.0f);
        const Salix::Vector3 b(7.0f, -3.0f, 2.0f);
        CHECK(Salix::Vector3::lerp(a, b, -1.0f) == a);
        CHECK(Salix::Vector3::lerp(a, b, 2.0f) == b);
    }

    TEST_CASE("batch5147 scalar multiplication is symmetric across operand order") {
        const Salix::Vector3 v(-5.0f, 1.0f, 4.0f);
        CHECK(v * 3.0f == 3.0f * v);
        CHECK(v * 3.0f == Salix::Vector3(-15.0f, 3.0f, 12.0f));
    }

    TEST_CASE("batch5147 vector addition followed by subtraction restores source exactly") {
        const Salix::Vector3 source(-5.0f, 1.0f, 4.0f);
        const Salix::Vector3 delta(7.0f, -3.0f, 2.0f);
        CHECK((source + delta) - delta == source);
    }

    TEST_CASE("batch5147 dot product is commutative for signed vectors") {
        const Salix::Vector3 a(-2.0f, 3.0f, 5.0f);
        const Salix::Vector3 b(7.0f, -11.0f, 13.0f);
        CHECK(Salix::dot(a, b) == doctest::Approx(Salix::dot(b, a)));
        CHECK(Salix::dot(a, b) == doctest::Approx(18.0f));
    }

    TEST_CASE("batch5147 cross product is antisymmetric for nonparallel vectors") {
        const Salix::Vector3 a(1.0f, 2.0f, 3.0f);
        const Salix::Vector3 b(-4.0f, 5.0f, -6.0f);
        CHECK(Salix::cross(a, b) == Salix::cross(b, a) * -1.0f);
    }

    TEST_CASE("batch5147 GLM roundtrip preserves representative finite coordinates") {
        const Salix::Vector3 original(-5.147f, 14.0f, 7.5f);
        const glm::vec3 glm_value = original.to_glm();
        CHECK(Salix::Vector3::from_glm(glm_value) == original);
        CHECK(Salix::Vector3(glm_value) == original);
    }

    TEST_CASE("batch5147 partial YAML x and z decode clears omitted y coordinate") {
        Salix::Vector3 v(9.0f, 8.0f, 7.0f);
        YAML::Load("{x: -5.147, z: 14.7}") >> v;
        CHECK(v.x == doctest::Approx(-5.147f));
        CHECK(v.y == 0.0f);
        CHECK(v.z == doctest::Approx(14.7f));
    }
}

TEST_SUITE("Salix::math::Vector3Batch5547") {
    TEST_CASE("batch5547 negative X axis normalization produces canonical unit axis") {
        Salix::Vector3 v(-5.0f, 0.0f, 0.0f);
        v.normalize();
        CHECK(v == Salix::Vector3(-1.0f, 0.0f, 0.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch5547 signed diagonal normalization preserves direction and unit length") {
        Salix::Vector3 v(-2.0f, 3.0f, -6.0f);
        const auto before = v;
        v.normalize();
        CHECK(v.length() == doctest::Approx(1.0f));
        CHECK(Salix::dot(v, before) > 0.0f);
    }

    TEST_CASE("batch5547 midpoint interpolation averages all three components") {
        const Salix::Vector3 a(-5.0f, 4.0f, -7.0f);
        const Salix::Vector3 b(7.0f, -6.0f, 9.0f);
        CHECK(Salix::Vector3::lerp(a, b, 0.5f) == Salix::Vector3(1.0f, -1.0f, 1.0f));
    }

    TEST_CASE("batch5547 component multiplication is commutative") {
        const Salix::Vector3 a(-5.0f, 4.0f, -7.0f);
        const Salix::Vector3 b(7.0f, -6.0f, 9.0f);
        CHECK(a * b == b * a);
        CHECK(a * b == Salix::Vector3(-35.0f, -24.0f, -63.0f));
    }

    TEST_CASE("batch5547 scalar subtraction preserves operand orientation") {
        const Salix::Vector3 v(-5.0f, 4.0f, -7.0f);
        CHECK(v - 2.0f == Salix::Vector3(-7.0f, 2.0f, -9.0f));
        CHECK(2.0f - v == Salix::Vector3(7.0f, -2.0f, 9.0f));
    }

    TEST_CASE("batch5547 dot product with perpendicular cross product is zero") {
        const Salix::Vector3 a(1.0f, 2.0f, 3.0f);
        const Salix::Vector3 b(-4.0f, 5.0f, -6.0f);
        const auto c = Salix::cross(a, b);
        CHECK(Salix::dot(a, c) == doctest::Approx(0.0f));
        CHECK(Salix::dot(b, c) == doctest::Approx(0.0f));
    }

    TEST_CASE("batch5547 GLM conversion preserves signed components in both directions") {
        const Salix::Vector3 source(-5.547f, 7.445f, -0.25f);
        const glm::vec3 raw = source.to_glm();
        CHECK(raw.x == source.x);
        CHECK(raw.y == source.y);
        CHECK(raw.z == source.z);
        CHECK(Salix::Vector3::from_glm(raw) == source);
    }

    TEST_CASE("batch5547 scalar multiply divide roundtrip restores finite vector") {
        const Salix::Vector3 source(-5.0f, 4.0f, -7.0f);
        CHECK((source * 4.0f) / 4.0f == source);
    }

    TEST_CASE("batch5547 YAML decode with y only clears omitted x and z") {
        Salix::Vector3 v(9.0f, 8.0f, 7.0f);
        YAML::Load("{y: -5.547}") >> v;
        CHECK(v.x == 0.0f);
        CHECK(v.y == doctest::Approx(-5.547f));
        CHECK(v.z == 0.0f);
    }

    TEST_CASE("batch5547 JSON roundtrip preserves signed coordinates") {
        const Salix::Vector3 source(-5.547f, 7.445f, -0.125f);
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::Vector3 restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored == source);
    }
}

TEST_SUITE("Salix::math::Vector3Batch5947") {
    TEST_CASE("batch5947 zero vector length is zero and normalization is stable") {
        Salix::Vector3 v = Salix::Vector3::Zero;
        CHECK(v.length() == 0.0f);
        v.normalize();
        CHECK(v == Salix::Vector3::Zero);
    }

    TEST_CASE("batch5947 negative Z axis normalizes to canonical unit axis") {
        Salix::Vector3 v(0.0f, 0.0f, -5.0f);
        v.normalize();
        CHECK(v == Salix::Vector3(0.0f, 0.0f, -1.0f));
        CHECK(v.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch5947 lerp clamps below zero to start vector") {
        const Salix::Vector3 start(-5.0f, 9.0f, -47.0f);
        const Salix::Vector3 end(7.0f, -4.0f, 8.0f);
        CHECK(Salix::Vector3::lerp(start, end, -5.947f) == start);
    }

    TEST_CASE("batch5947 lerp clamps above one to end vector") {
        const Salix::Vector3 start(-5.0f, 9.0f, -47.0f);
        const Salix::Vector3 end(7.0f, -4.0f, 8.0f);
        CHECK(Salix::Vector3::lerp(start, end, 5.947f) == end);
    }

    TEST_CASE("batch5947 midpoint lerp averages each component independently") {
        const Salix::Vector3 a(-6.0f, 2.0f, -4.0f);
        const Salix::Vector3 b(8.0f, 6.0f, 10.0f);
        CHECK(Salix::Vector3::lerp(a, b, 0.5f) == Salix::Vector3(1.0f, 4.0f, 3.0f));
    }

    TEST_CASE("batch5947 scalar subtraction preserves operand orientation") {
        const Salix::Vector3 v(-5.0f, 9.0f, -47.0f);
        CHECK(v - 2.0f == Salix::Vector3(-7.0f, 7.0f, -49.0f));
        CHECK(2.0f - v == Salix::Vector3(7.0f, -7.0f, 49.0f));
    }

    TEST_CASE("batch5947 component multiplication is commutative") {
        const Salix::Vector3 a(-5.0f, 9.0f, -47.0f);
        const Salix::Vector3 b(7.0f, -4.0f, 8.0f);
        CHECK(a * b == b * a);
        CHECK(a * b == Salix::Vector3(-35.0f, -36.0f, -376.0f));
    }

    TEST_CASE("batch5947 dot product remains commutative for signed vectors") {
        const Salix::Vector3 a(-2.0f, 3.0f, 5.0f);
        const Salix::Vector3 b(7.0f, -11.0f, 13.0f);
        CHECK(Salix::dot(a, b) == doctest::Approx(Salix::dot(b, a)));
        CHECK(Salix::dot(a, b) == doctest::Approx(18.0f));
    }

    TEST_CASE("batch5947 cross product remains antisymmetric") {
        const Salix::Vector3 a(1.0f, 2.0f, 3.0f);
        const Salix::Vector3 b(-4.0f, 5.0f, -6.0f);
        CHECK(Salix::cross(a, b) == Salix::cross(b, a) * -1.0f);
    }

    TEST_CASE("batch5947 GLM and YAML conversions preserve documented component semantics") {
        const Salix::Vector3 source(-5.947f, 7.495f, -0.25f);
        const glm::vec3 raw = source.to_glm();
        CHECK(Salix::Vector3::from_glm(raw) == source);
        Salix::Vector3 partial(1.0f, 2.0f, 3.0f);
        YAML::Load("{x: -5.947, z: 7.495}") >> partial;
        CHECK(partial.x == doctest::Approx(-5.947f));
        CHECK(partial.y == 0.0f);
        CHECK(partial.z == doctest::Approx(7.495f));
    }
}

TEST_SUITE("Salix::math::Vector3Batch6047") {
    TEST_CASE("batch6047 vector addition is commutative for mixed signed components") {
        const Salix::Vector3 a(-6.0f, 4.0f, 7.0f);
        const Salix::Vector3 b(3.0f, -9.0f, 5.0f);
        CHECK(a + b == b + a);
        CHECK(a + b == Salix::Vector3(-3.0f, -5.0f, 12.0f));
    }

    TEST_CASE("batch6047 subtraction followed by addition restores original vector") {
        const Salix::Vector3 a(-6.0f, 4.0f, 7.0f);
        const Salix::Vector3 b(3.0f, -9.0f, 5.0f);
        CHECK((a - b) + b == a);
    }

    TEST_CASE("batch6047 multiplication by zero produces canonical zero vector") {
        const Salix::Vector3 source(-6.0f, 4.0f, 7.0f);
        CHECK(source * 0.0f == Salix::Vector3::Zero);
        CHECK(0.0f * source == Salix::Vector3::Zero);
    }

    TEST_CASE("batch6047 negative scalar multiply divide roundtrip restores finite vector") {
        const Salix::Vector3 source(-6.0f, 4.0f, 9.0f);
        CHECK((source * -3.0f) / -3.0f == source);
    }

    TEST_CASE("batch6047 dot product with self equals squared vector length") {
        const Salix::Vector3 source(3.0f, 4.0f, 12.0f);
        CHECK(Salix::dot(source, source) == doctest::Approx(source.length() * source.length()));
        CHECK(source.length() == doctest::Approx(13.0f));
    }

    TEST_CASE("batch6047 canonical X cross Y produces canonical positive Z") {
        CHECK(Salix::cross(
            Salix::Vector3(1.0f, 0.0f, 0.0f),
            Salix::Vector3(0.0f, 1.0f, 0.0f)) == Salix::Vector3(0.0f, 0.0f, 1.0f));
    }

    TEST_CASE("batch6047 cross product of vector with itself is zero") {
        const Salix::Vector3 source(-6.0f, 4.0f, 7.0f);
        CHECK(Salix::cross(source, source) == Salix::Vector3::Zero);
    }

    TEST_CASE("batch6047 quarter interpolation computes each component independently") {
        const Salix::Vector3 start(0.0f, -8.0f, 4.0f);
        const Salix::Vector3 end(8.0f, 8.0f, 12.0f);
        CHECK(Salix::Vector3::lerp(start, end, 0.25f) == Salix::Vector3(2.0f, -4.0f, 6.0f));
    }

    TEST_CASE("batch6047 GLM roundtrip preserves zero and signed coordinates") {
        const Salix::Vector3 source(0.0f, -60.47f, 47.06f);
        const glm::vec3 raw = source.to_glm();
        CHECK(raw.x == 0.0f);
        CHECK(raw.y == doctest::Approx(-60.47f));
        CHECK(raw.z == doctest::Approx(47.06f));
        CHECK(Salix::Vector3::from_glm(raw) == source);
    }

    TEST_CASE("batch6047 YAML full mapping replaces every prior component") {
        Salix::Vector3 value(9.0f, 8.0f, 7.0f);
        YAML::Load("{x: -6.047, y: 0.0, z: 47.06}") >> value;
        CHECK(value.x == doctest::Approx(-6.047f));
        CHECK(value.y == 0.0f);
        CHECK(value.z == doctest::Approx(47.06f));
    }
}

TEST_SUITE("Salix::math::Vector3Batch6147") {
    TEST_CASE("batch6147 compound addition mutates each component and returns self") {
        Salix::Vector3 value(-6.0f, 14.0f, 7.0f);
        Salix::Vector3* address = &value;
        Salix::Vector3& result = (value += Salix::Vector3(1.0f, -4.0f, 3.0f));
        CHECK(&result == address);
        CHECK(value == Salix::Vector3(-5.0f, 10.0f, 10.0f));
    }

    TEST_CASE("batch6147 compound subtraction mutates each component and returns self") {
        Salix::Vector3 value(-6.0f, 14.0f, 7.0f);
        Salix::Vector3* address = &value;
        Salix::Vector3& result = (value -= Salix::Vector3(1.0f, -4.0f, 3.0f));
        CHECK(&result == address);
        CHECK(value == Salix::Vector3(-7.0f, 18.0f, 4.0f));
    }

    TEST_CASE("batch6147 compound negative scalar multiplication preserves signs exactly") {
        Salix::Vector3 value(-2.0f, 3.0f, -4.0f);
        value *= -2.0f;
        CHECK(value == Salix::Vector3(4.0f, -6.0f, 8.0f));
    }

    TEST_CASE("batch6147 compound negative scalar division restores expected components") {
        Salix::Vector3 value(4.0f, -6.0f, 8.0f);
        value /= -2.0f;
        CHECK(value == Salix::Vector3(-2.0f, 3.0f, -4.0f));
    }

    TEST_CASE("batch6147 compound division by zero resets every component to zero") {
        Salix::Vector3 value(6.0f, -14.0f, 7.0f);
        value /= 0.0f;
        CHECK(value == Salix::Vector3::Zero);
    }

    TEST_CASE("batch6147 component division zeroes only axes with zero divisors") {
        const Salix::Vector3 numerator(8.0f, 9.0f, 10.0f);
        const Salix::Vector3 divisor(2.0f, 0.0f, 5.0f);
        CHECK(numerator / divisor == Salix::Vector3(4.0f, 0.0f, 2.0f));
    }

    TEST_CASE("batch6147 scalar divided by vector zeroes only zero component axes") {
        const Salix::Vector3 divisor(2.0f, 0.0f, -4.0f);
        CHECK(8.0f / divisor == Salix::Vector3(4.0f, 0.0f, -2.0f));
    }

    TEST_CASE("batch6147 lerp exact endpoints return exact source values") {
        const Salix::Vector3 start(-6.0f, 14.0f, 7.0f);
        const Salix::Vector3 end(1.0f, -4.0f, 3.0f);
        CHECK(Salix::Vector3::lerp(start, end, 0.0f) == start);
        CHECK(Salix::Vector3::lerp(start, end, 1.0f) == end);
    }

    TEST_CASE("batch6147 normalizing an already unit axis remains unchanged") {
        Salix::Vector3 value(0.0f, -1.0f, 0.0f);
        value.normalize();
        CHECK(value == Salix::Vector3(0.0f, -1.0f, 0.0f));
        CHECK(value.length() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch6147 zero vector is neutral for dot and cross products") {
        const Salix::Vector3 value(-6.0f, 14.0f, 7.0f);
        CHECK(Salix::dot(value, Salix::Vector3::Zero) == 0.0f);
        CHECK(Salix::cross(value, Salix::Vector3::Zero) == Salix::Vector3::Zero);
        CHECK(Salix::cross(Salix::Vector3::Zero, value) == Salix::Vector3::Zero);
    }
}
