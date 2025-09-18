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

TEST_SUITE("Salix::core::ValidationUtils") {
    TEST_CASE("string validation") {
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

    TEST_CASE("pointer validation") {
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

    
    TEST_CASE("signed integer validation") {
        SUBCASE("int8_t") {
            using T = int8_t;
            CHECK(is_valid_int8_t(static_cast<T>(100)));
            CHECK_FALSE(is_valid_int8_t(static_cast<T>(-100)));

            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int16_t") {
            using T = int16_t;
            CHECK(is_valid_int16_t(static_cast<T>(100)));
            CHECK_FALSE(is_valid_int16_t(static_cast<T>(-100)));

            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int32_t") {
            using T = int32_t;
            CHECK(is_valid_int32_t(static_cast<T>(100)));
            CHECK_FALSE(is_valid_int32_t(static_cast<T>(-100)));

            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
        SUBCASE("int64_t") {
            using T = int64_t;
            CHECK(is_valid_int64_t(static_cast<T>(100)));
            CHECK_FALSE(is_valid_int64_t(static_cast<T>(-100)));

            CHECK(is_within_bounds(static_cast<T>(100)));
            CHECK_FALSE(is_not_negative(static_cast<T>(-100)));
            CHECK_FALSE(is_within_bounds(std::numeric_limits<T>::max()));
        }
    }

    TEST_CASE("high-level signed integer validators")
    {
        CHECK(is_valid_int(5));
        CHECK_FALSE(is_valid_int(-5));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::max()));
    }

    
    TEST_CASE("unsigned integer validation") {
        SUBCASE("uint8_t") {
            using T = uint8_t;
            CHECK(is_valid_uint8_t(static_cast<T>(100)));
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint16_t") {
            using T = uint16_t;
            CHECK(is_valid_uint16_t(static_cast<T>(100)));
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint32_t") {
            using T = uint32_t;
            CHECK(is_valid_uint32_t(static_cast<T>(100)));
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
        SUBCASE("uint64_t") {
            using T = uint64_t;
            CHECK(is_valid_uint64_t(static_cast<T>(100)));
            CHECK(is_within_bounds_unsigned(static_cast<T>(100)));
            CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<T>::max()));
        }
    }
    
    TEST_CASE("high-level unsigned integer validators") {
        CHECK(is_valid_unsigned_int(5));
        CHECK_FALSE(is_valid_unsigned_int(std::numeric_limits<unsigned int>::max()));
    }

    TEST_CASE("floating point validation") {
        SUBCASE("float") {
            using T = float;
            CHECK(is_valid_floating_point(static_cast<T>(3.14f)));
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
        SUBCASE("double") {
            using T = double;
            CHECK(is_valid_double(static_cast<T>(3.14f)));
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
        SUBCASE("long double") {
            using T = long double;
            CHECK(is_valid_long_double(static_cast<T>(3.14f)));
            CHECK(is_finite(static_cast<T>(3.14)));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::quiet_NaN()));
            CHECK_FALSE(is_finite(std::numeric_limits<T>::infinity()));
        }
    }

    TEST_CASE("high-level floating point validation") {
        CHECK(is_valid_float(3.14f));
        CHECK_FALSE(is_valid_float(std::numeric_limits<float>::quiet_NaN()));
    }

    TEST_CASE("complex number validation") {
        SUBCASE("complex Floating Point Validation") {
            using T = float;
            const auto nan_val = std::numeric_limits<T>::quiet_NaN();
            const auto inf_val = std::numeric_limits<T>::infinity();

            std::complex<T> value = {2.0f, -1.0f};
            CHECK(is_valid_complex_float(value));
            CHECK(is_valid_complex_float({1.0f, -2.5f}));
            CHECK_FALSE(is_valid_complex_float({nan_val, 1.0f}));
            CHECK_FALSE(is_valid_complex_float({1.0f, nan_val}));
            CHECK_FALSE(is_valid_complex_float({inf_val, 1.0f}));
            CHECK_FALSE(is_valid_complex_float({1.0f, inf_val}));
        }

        SUBCASE("complex double validation") {
            using T = double;
            const auto nan_val = std::numeric_limits<T>::quiet_NaN();
            const auto inf_val = std::numeric_limits<T>::infinity();

            CHECK(is_valid_complex_double({1.0, -2.5}));
            CHECK_FALSE(is_valid_complex_double({nan_val, 1.0}));
            CHECK_FALSE(is_valid_complex_double({1.0, nan_val}));
            CHECK_FALSE(is_valid_complex_double({inf_val, 1.0}));
            CHECK_FALSE(is_valid_complex_double({1.0, inf_val}));
        }

        SUBCASE("complex Long double validation") {
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

    TEST_CASE("least-width integer validation") {
        // --- Signed Least Integers ---
        SUBCASE("int_least8_t") {
            CHECK(is_valid_intleast8_t(static_cast<int_least8_t>(100)));
            CHECK_FALSE(is_valid_intleast8_t(static_cast<int_least8_t>(-100)));
        }
        SUBCASE("int_least16_t") {
            CHECK(is_valid_intleast16_t(static_cast<int_least16_t>(30000)));
            CHECK_FALSE(is_valid_intleast16_t(static_cast<int_least16_t>(-30000)));
        }
        SUBCASE("int_least32_t") {
            CHECK(is_valid_intleast32_t(static_cast<int_least32_t>(2000000)));
            CHECK_FALSE(is_valid_intleast32_t(static_cast<int_least32_t>(-2000000)));
        }
        SUBCASE("int_least64_t") {
            CHECK(is_valid_intleast64_t(static_cast<int_least64_t>(1000000000)));
            CHECK_FALSE(is_valid_intleast64_t(static_cast<int_least64_t>(-1000000000)));
        }

        // --- Unsigned Least Integers ---
        SUBCASE("uint_least8_t") {
            CHECK(is_valid_uint_least8_t(static_cast<uint_least8_t>(200)));
            CHECK_FALSE(is_valid_uint_least8_t(std::numeric_limits<uint_least8_t>::max()));
        }
        SUBCASE("uint_least16_t") {
            CHECK(is_valid_uint_least16_t(static_cast<uint_least16_t>(60000)));
            CHECK_FALSE(is_valid_uint_least16_t(std::numeric_limits<uint_least16_t>::max()));
        }
        SUBCASE("uint_least32_t") {
            CHECK(is_valid_uint_least32_t(static_cast<uint_least32_t>(4000000000)));
            CHECK_FALSE(is_valid_uint_least32_t(std::numeric_limits<uint_least32_t>::max()));
        }
        SUBCASE("uint_least64_t") {
            // This subcase was calling the wrong function (16_t instead of 64_t)
            CHECK(is_valid_uint_least64_t(static_cast<uint_least64_t>(1000000000000)));
            CHECK_FALSE(is_valid_uint_least64_t(std::numeric_limits<uint_least64_t>::max()));
        }
    }

    TEST_CASE("fast-width integer validation") {
        // --- Signed Fast Integers ---
        SUBCASE("int_fast8_t") {
            CHECK(is_valid_int_fast8_t(static_cast<int_fast8_t>(100)));
            CHECK_FALSE(is_valid_int_fast8_t(static_cast<int_fast8_t>(-100)));
        }
        SUBCASE("int_fast16_t") {
            CHECK(is_valid_int_fast16_t(static_cast<int_fast16_t>(30000)));
            CHECK_FALSE(is_valid_int_fast16_t(static_cast<int_fast16_t>(-30000)));
        }
        SUBCASE("int_fast32_t") {
            CHECK(is_valid_int_fast32_t(static_cast<int_fast32_t>(2000000)));
            CHECK_FALSE(is_valid_int_fast32_t(static_cast<int_fast32_t>(-2000000)));
        }
        SUBCASE("int_fast64_t") {
            CHECK(is_valid_int_fast64_t(static_cast<int_fast64_t>(1000000000)));
            CHECK_FALSE(is_valid_int_fast64_t(static_cast<int_fast64_t>(-1000000000)));
        }

        // --- Unsigned Fast Integers ---
        SUBCASE("uint_fast8_t") {
            CHECK(is_valid_uint_fast8_t(static_cast<uint_fast8_t>(200)));
            CHECK_FALSE(is_valid_uint_fast8_t(std::numeric_limits<uint_fast8_t>::max()));
        }
        SUBCASE("uint_fast16_t") {
            CHECK(is_valid_uint_fast16_t(static_cast<uint_fast16_t>(60000)));
            CHECK_FALSE(is_valid_uint_fast16_t(std::numeric_limits<uint_fast16_t>::max()));
        }
        SUBCASE("uint_fast32_t") {
            CHECK(is_valid_uint_fast32_t(static_cast<uint_fast32_t>(4000000000)));
            CHECK_FALSE(is_valid_uint_fast32_t(std::numeric_limits<uint_fast32_t>::max()));
        }
        SUBCASE("uint_fast64_t") {
            CHECK(is_valid_uint_fast64_t(static_cast<uint_fast64_t>(1000000000000)));
            CHECK_FALSE(is_valid_uint_fast64_t(std::numeric_limits<uint_fast64_t>::max()));
        }
    }

    TEST_CASE("maximum-width integer validation") {
        SUBCASE("intmax_t") {
            CHECK(is_valid_intmax_t(static_cast<intmax_t>(999999)));
            CHECK_FALSE(is_valid_intmax_t(static_cast<intmax_t>(-999999)));
            // Add a direct check for the invalid boundary condition
            CHECK_FALSE(is_valid_intmax_t(std::numeric_limits<intmax_t>::max()));
        }
        SUBCASE("uintmax_t") {
            CHECK(is_valid_uintmax_t(static_cast<uintmax_t>(999999)));
            // Test the public function directly for better practice
            CHECK_FALSE(is_valid_uintmax_t(std::numeric_limits<uintmax_t>::max()));
        }
    }

    TEST_CASE("template helper edge cases") {
        SUBCASE("is_finite with non-float type returns false") {
            // This test covers the final branch of the is_finite template.
            CHECK_FALSE(is_finite(10)); // Call with an integer
        }
    }
}