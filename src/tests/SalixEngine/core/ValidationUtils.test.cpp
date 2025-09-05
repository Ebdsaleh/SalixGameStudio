// =================================================================================
// Filename:    src/Tests/SalixEngine/core/ValidationUtils.test.cpp
// Description: Contains unit tests for the ValidationUtils functions.
// =================================================================================
#include <doctest.h>
#include <Salix/core/ValidationUtils.h> // The functions we are testing
#include <string>
#include <limits>
#include <type_traits>
#include <cstdint>
#include <cmath> // For std::nan and std::isinf

// Use the namespace for convenience
using namespace Salix::ValidationUtils;

TEST_SUITE("ValidationUtils") {
    TEST_CASE("String Validation") {
        SUBCASE("is_valid_string") {
            // Default behavior: empty is not allowed, whitespace-only is not allowed
            CHECK_FALSE(is_valid_string(""));
            CHECK_FALSE(is_valid_string("   "));
            CHECK(is_valid_string("hello"));
            CHECK(is_valid_string("  hello  "));

            // Allow empty strings
            CHECK(is_valid_string("", true, false));
            CHECK_FALSE(is_valid_string("   ", true, false));
            CHECK(is_valid_string("hello", true, false));

            // Allow whitespace-only strings
            CHECK_FALSE(is_valid_string("", false, true));
            CHECK(is_valid_string("   ", false, true));
            CHECK(is_valid_string("hello", false, true));

            // Allow both
            CHECK(is_valid_string("", true, true));
            CHECK(is_valid_string("   ", true, true));
            CHECK(is_valid_string("hello", true, true));
        }

        SUBCASE("is_valid_c_str") {
            const char* valid_str = "I am a valid C-string";
            const char* null_str = nullptr;

            CHECK(is_valid_c_str(valid_str));
            CHECK_FALSE(is_valid_c_str(null_str));
        }
    }

    TEST_CASE("Pointer Validation") {
        SUBCASE("is_valid_ptr (template)") {
            int x = 10;
            int* valid_ptr = &x;
            int* null_ptr = nullptr;

            CHECK(is_valid_ptr(valid_ptr));
            CHECK_FALSE(is_valid_ptr(null_ptr));

            std::string s = "test";
            std::string* valid_str_ptr = &s;
            std::string* null_str_ptr = nullptr;

            CHECK(is_valid_ptr(valid_str_ptr));
            CHECK_FALSE(is_valid_ptr(null_str_ptr));
        }
    }

    
    TEST_CASE("Signed Integer Validation") {
        SUBCASE("int8_t") {
            using T = int8_t;
            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int16_t") {
            using T = int16_t;
            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int32_t") {
            using T = int32_t;
            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int64_t") {
            using T = int64_t;
            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
    }

    TEST_CASE("High-Level Signed Integer Validators")
    {
        CHECK(is_valid_int(5));
        CHECK_FALSE(is_valid_int(-5));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::max()));
    }

    
    TEST_CASE("Unsigned Integer Validation") {
        SUBCASE("uint8_t") {
            using T = uint8_t;
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint16_t") {
            using T = uint16_t;
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint32_t") {
            using T = uint32_t;
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint64_t") {
            using T = uint64_t;
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
    }
    
    TEST_CASE("High-Level Unsigned Integer Validators") {
        CHECK(is_valid_unsigned_int(5));
        CHECK_FALSE(is_valid_unsigned_int(std::numeric_limits<unsigned int>::max()));
    }

    // REFACTORED: Use SUBCASEs instead of TEMPLATE_TEST_CASE
    TEST_CASE("Floating Point Validation") {
        SUBCASE("float") {
            using T = float;
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
        SUBCASE("double") {
            using T = double;
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
        SUBCASE("long double") {
            using T = long double;
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
    }

    TEST_CASE("High-Level Floating Point Validation") {
        CHECK(is_valid_float(3.14f));
        CHECK_FALSE(is_valid_float(std::numeric_limits<float>::quiet_NaN()));
    }

    TEST_CASE("Complex Numbers Validation") {
        SUBCASE("Complex Floating Point Validation") {
            using T = float;
            const auto nan_val = std::numeric_limits<T>::quiet_NaN();
            const auto inf_val = std::numeric_limits<T>::infinity();

            CHECK(is_valid_complex_float({1.0f, -2.5f}));
            CHECK_FALSE(is_valid_complex_float({nan_val, 1.0f}));
            CHECK_FALSE(is_valid_complex_float({1.0f, nan_val}));
            CHECK_FALSE(is_valid_complex_float({inf_val, 1.0f}));
            CHECK_FALSE(is_valid_complex_float({1.0f, inf_val}));
        }

        SUBCASE("Complex Double Validation") {
            using T = double;
            const auto nan_val = std::numeric_limits<T>::quiet_NaN();
            const auto inf_val = std::numeric_limits<T>::infinity();

            CHECK(is_valid_complex_double({1.0, -2.5}));
            CHECK_FALSE(is_valid_complex_double({nan_val, 1.0}));
            CHECK_FALSE(is_valid_complex_double({1.0, nan_val}));
            CHECK_FALSE(is_valid_complex_double({inf_val, 1.0}));
            CHECK_FALSE(is_valid_complex_double({1.0, inf_val}));
        }

        SUBCASE("Complex Long Double Validation") {
            using T = long double;
            const auto nan_val = std::numeric_limits<T>::quiet_NaN();
            const auto inf_val = std::numeric_limits<T>::infinity();

            CHECK(is_valid_complex_long_double({1.0L, -2.5L}));
            CHECK_FALSE(is_valid_complex_long_double({nan_val, 1.0L}));
            CHECK_FALSE(is_valid_complex_long_double({1.0L, nan_val}));
            CHECK_FALSE(is_valid_complex_long_double({inf_val, 1.0L}));
            CHECK_FALSE(is_valid_complex_long_double({1.0L, inf_val}));
        }
    }
}