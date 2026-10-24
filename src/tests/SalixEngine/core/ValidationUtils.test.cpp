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

// Additional cases preserved from the separate regression suite.
// Validation boundary, finite-number and type-trait regression checks.
#include <doctest.h>
#include <Salix/core/ValidationUtils.h>
#include <cstdint>
#include <complex>
#include <limits>

TEST_SUITE("Salix::core::ValidationUtilsRegression") {
    TEST_CASE("validation empty strings are accepted only with explicit allow empty flag") {
        using Salix::ValidationUtils::is_valid_string;
        CHECK_FALSE(is_valid_string(""));
        CHECK(is_valid_string("", true));
        CHECK_FALSE(is_valid_string("", false, true));
        CHECK(is_valid_string("data"));
    }

    TEST_CASE("validation whitespace only strings need explicit allowance but mixed text is valid") {
        using Salix::ValidationUtils::is_valid_string;
        CHECK_FALSE(is_valid_string(" \t\n"));
        CHECK(is_valid_string(" \t\n", false, true));
        CHECK(is_valid_string("\t data\n"));
        CHECK(is_valid_string("x y"));
    }

    TEST_CASE("validation C string validation checks pointer identity not string length") {
        CHECK_FALSE(Salix::ValidationUtils::is_valid_c_str(nullptr));
        CHECK(Salix::ValidationUtils::is_valid_c_str(""));
        const char text[] = "salix";
        CHECK(Salix::ValidationUtils::is_valid_c_str(text));
    }

    TEST_CASE("validation signed validity rejects both extreme sentinels and negative values") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_int(0));
        CHECK(is_valid_int(42));
        CHECK_FALSE(is_valid_int(-1));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::min()));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::max()));
    }

    TEST_CASE("validation unsigned validity treats maximum as invalid sentinel and zero as valid") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_unsigned_int(0U));
        CHECK(is_valid_uint8_t(static_cast<std::uint8_t>(254)));
        CHECK_FALSE(is_valid_uint8_t(std::numeric_limits<std::uint8_t>::max()));
        CHECK_FALSE(is_valid_uint64_t(std::numeric_limits<std::uint64_t>::max()));
    }

    TEST_CASE("validation least and fast width validator families apply the same boundary policy") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_intleast16_t(static_cast<int_least16_t>(8)));
        CHECK_FALSE(is_valid_intleast16_t(static_cast<int_least16_t>(-4)));
        CHECK(is_valid_int_fast32_t(static_cast<int_fast32_t>(12)));
        CHECK_FALSE(is_valid_uint_fast64_t(std::numeric_limits<uint_fast64_t>::max()));
    }

    TEST_CASE("validation compile time category and type helpers discriminate numeric domains") {
        using namespace Salix::ValidationUtils;
        CHECK(is_integral<const int&>());
        CHECK_FALSE(is_integral<float>());
        CHECK(is_floating_point<double>());
        CHECK_FALSE(is_floating_point<int>());
        CHECK(is_signed<long>());
        CHECK(is_unsigned<unsigned long>());
        const long value = 7;
        CHECK(is_type<long>(value));
        CHECK_FALSE(is_type<int>(value));
    }

    TEST_CASE("validation bound predicates differentiate signed minimum maximum and zero") {
        using namespace Salix::ValidationUtils;
        CHECK(is_within_bounds(0));
        CHECK_FALSE(is_within_bounds(std::numeric_limits<int>::min()));
        CHECK_FALSE(is_within_bounds(std::numeric_limits<int>::max()));
        CHECK(is_not_negative(0));
        CHECK_FALSE(is_not_negative(-1));
        CHECK(is_within_bounds_unsigned(0U));
        CHECK_FALSE(is_within_bounds_unsigned(std::numeric_limits<unsigned>::max()));
    }

    TEST_CASE("validation finite floating validation excludes both infinities and NaN") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_float(0.f));
        CHECK(is_valid_double(-100.5));
        CHECK(is_valid_long_double(0.0L));
        CHECK_FALSE(is_valid_float(std::numeric_limits<float>::infinity()));
        CHECK_FALSE(is_valid_double(-std::numeric_limits<double>::infinity()));
        CHECK_FALSE(is_valid_double(std::numeric_limits<double>::quiet_NaN()));
    }

    TEST_CASE("validation complex components must both be finite and null pointers fail") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_complex_float(std::complex<float>{1.f, -2.f}));
        CHECK_FALSE(is_valid_complex_double(std::complex<double>{1., std::numeric_limits<double>::infinity()}));
        CHECK_FALSE(is_valid_complex_long_double(std::complex<long double>{std::numeric_limits<long double>::quiet_NaN(), 0.L}));
        int number = 42;
        CHECK(is_valid_ptr(&number));
        CHECK_FALSE(is_valid_ptr(static_cast<int*>(nullptr)));
    }
}

// Numeric sentinels and validation-template semantics used by reflection.
TEST_SUITE("Salix::core::ValidationUtilsAdditional20261019") {
    TEST_CASE("validation extended signed template validity only excludes the maximum sentinel") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_integral<int>(0));
        CHECK(is_valid_integral<int>(-9));
        CHECK(is_valid_integral<int>(std::numeric_limits<int>::min()));
        CHECK_FALSE(is_valid_integral<int>(std::numeric_limits<int>::max()));
    }

    TEST_CASE("validation extended unsigned generic validation accepts zero and rejects maximum") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_integral<std::uint16_t>(0));
        CHECK(is_valid_integral<std::uint16_t>(65534));
        CHECK_FALSE(is_valid_integral<std::uint16_t>(65535));
    }

    TEST_CASE("validation extended floating template excludes NaN and both signed infinities") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_floating_point<float>(0.f));
        CHECK(is_valid_floating_point<double>(-12.5));
        CHECK_FALSE(is_valid_floating_point<double>(std::numeric_limits<double>::quiet_NaN()));
        CHECK_FALSE(is_valid_floating_point<float>(std::numeric_limits<float>::infinity()));
        CHECK_FALSE(is_valid_floating_point<float>(-std::numeric_limits<float>::infinity()));
    }

    TEST_CASE("validation extended zero and small finite floats do not become false negatives") {
        using namespace Salix::ValidationUtils;
        CHECK(is_finite(0.f));
        CHECK(is_finite(-0.f));
        CHECK(is_finite(std::numeric_limits<float>::min()));
        CHECK(is_finite(std::numeric_limits<float>::denorm_min()));
    }

    TEST_CASE("validation extended complex validation checks real and imaginary channels separately") {
        using namespace Salix::ValidationUtils;
        const double inf = std::numeric_limits<double>::infinity();
        CHECK(is_valid_complex_double(std::complex<double>(1., -3.)));
        CHECK_FALSE(is_valid_complex_double(std::complex<double>(inf, 0.)));
        CHECK_FALSE(is_valid_complex_double(std::complex<double>(0., -inf)));
    }

    TEST_CASE("validation extended pointer validity handles const and polymorphic addresses without ownership") {
        using namespace Salix::ValidationUtils;
        const int value = 6;
        const int* existing = &value;
        const int* missing = nullptr;
        CHECK(is_valid_ptr(existing));
        CHECK_FALSE(is_valid_ptr(missing));
        CHECK(is_valid_c_str("value"));
        CHECK_FALSE(is_valid_c_str(nullptr));
    }

    TEST_CASE("validation extended decayed compile-time type equality distinguishes float and double") {
        using namespace Salix::ValidationUtils;
        const float f = 0.f;
        const double d = 0.;
        CHECK(is_type<float>(f));
        CHECK(is_type<const float&>(f));
        CHECK_FALSE(is_type<double>(f));
        CHECK(is_type<double>(d));
        CHECK_FALSE(is_type<int>(d));
    }

    TEST_CASE("validation extended numeric trait classification treats bool differently from floating point") {
        using namespace Salix::ValidationUtils;
        CHECK(is_integral<bool>());
        CHECK(is_integral<const long&>());
        CHECK_FALSE(is_floating_point<bool>());
        CHECK(is_floating_point<const long double&>());
        CHECK_FALSE(is_integral<double>());
    }

    TEST_CASE("validation extended not-negative helper discriminates minus one and negative zero") {
        using namespace Salix::ValidationUtils;
        CHECK(is_not_negative(0));
        CHECK(is_not_negative(200LL));
        CHECK_FALSE(is_not_negative(-1));
        CHECK_FALSE(is_not_negative(-922LL));
    }

    TEST_CASE("validation extended whitespace policy distinguishes spaces from text with trimmed padding") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_string("\r\t "));
        CHECK(is_valid_string("\r\t ", false, true));
        CHECK(is_valid_string("\tname\r"));
        CHECK(is_valid_string("value\n"));
        CHECK_FALSE(is_valid_string("", false, true));
        CHECK(is_valid_string("", true, false));
    }
}

TEST_SUITE("Salix::core::ValidationUtilsClosure1747") {
    TEST_CASE("batch1747 validation empty string policy follows allow empty independently of whitespace flag") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_string("", false, false));
        CHECK_FALSE(is_valid_string("", false, true));
        CHECK(is_valid_string("", true, false));
        CHECK(is_valid_string("", true, true));
    }

    TEST_CASE("batch1747 validation whitespace only policy distinguishes explicit whitespace allowance") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_string(" \t\n\r", false, false));
        CHECK(is_valid_string(" \t\n\r", false, true));
    }

    TEST_CASE("batch1747 validation mixed text and whitespace is valid even when whitespace only strings are forbidden") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_string("  Salix  ", false, false));
        CHECK(is_valid_string("\tEngine\n", false, false));
    }

    TEST_CASE("batch1747 validation C string checks pointer presence rather than string length") {
        using namespace Salix::ValidationUtils;
        const char empty[] = "";
        CHECK(is_valid_c_str(empty));
        CHECK_FALSE(is_valid_c_str(nullptr));
    }

    TEST_CASE("batch1747 validation signed integer zero is valid while both sentinel extrema are invalid") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_int(0));
        CHECK_FALSE(is_valid_int(-1));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::min()));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::max()));
    }

    TEST_CASE("batch1747 validation unsigned zero is valid and maximum sentinel is invalid across widths") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_uint8_t(0));
        CHECK_FALSE(is_valid_uint8_t(std::numeric_limits<uint8_t>::max()));
        CHECK(is_valid_uint64_t(0));
        CHECK_FALSE(is_valid_uint64_t(std::numeric_limits<uint64_t>::max()));
    }

    TEST_CASE("batch1747 validation finite helper accepts signed zero and subnormal finite values") {
        using namespace Salix::ValidationUtils;
        CHECK(is_finite(-0.0f));
        CHECK(is_finite(std::numeric_limits<float>::denorm_min()));
        CHECK(is_finite(std::numeric_limits<double>::denorm_min()));
    }

    TEST_CASE("batch1747 validation finite helper rejects nonfloating inputs at compile selected branch") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_finite(0));
        CHECK_FALSE(is_finite(42LL));
    }

    TEST_CASE("batch1747 validation complex finiteness rejects nonfinite real or imaginary channel independently") {
        using namespace Salix::ValidationUtils;
        const auto inf = std::numeric_limits<double>::infinity();
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        CHECK(is_valid_complex_double({1.0, -2.0}));
        CHECK_FALSE(is_valid_complex_double({inf, 0.0}));
        CHECK_FALSE(is_valid_complex_double({0.0, nan}));
    }

    TEST_CASE("batch1747 validation type traits decay const references before category comparison") {
        using namespace Salix::ValidationUtils;
        const int value = 5;
        CHECK(is_integral<decltype((value))>());
        CHECK(is_signed<decltype((value))>());
        CHECK_FALSE(is_unsigned<decltype((value))>());
        CHECK(is_type<int>(value));
        CHECK_FALSE(is_type<float>(value));
    }
}

TEST_SUITE("Salix::core::ValidationUtilsBatch2147") {
    TEST_CASE("batch2147 empty string validity follows allow empty independently of whitespace policy") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_string("", false, false));
        CHECK(is_valid_string("", true, false));
        CHECK(is_valid_string("", true, true));
    }
    TEST_CASE("batch2147 whitespace-only text follows allow whitespace when nonempty") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_string(" \t\n\r", false, false));
        CHECK(is_valid_string(" \t\n\r", false, true));
    }
    TEST_CASE("batch2147 c string validation checks pointer identity rather than content length") {
        using namespace Salix::ValidationUtils;
        CHECK_FALSE(is_valid_c_str(nullptr));
        CHECK(is_valid_c_str(""));
        CHECK(is_valid_c_str("Salix"));
    }
    TEST_CASE("batch2147 signed integer validation accepts zero and rejects negatives and sentinels") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_int(0));
        CHECK(is_valid_int(42));
        CHECK_FALSE(is_valid_int(-1));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::min()));
        CHECK_FALSE(is_valid_int(std::numeric_limits<int>::max()));
    }
    TEST_CASE("batch2147 fixed-width signed validators share nonnegative bounded contract") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_int8_t(static_cast<int8_t>(0)));
        CHECK(is_valid_int16_t(static_cast<int16_t>(1)));
        CHECK(is_valid_int32_t(static_cast<int32_t>(2)));
        CHECK(is_valid_int64_t(static_cast<int64_t>(3)));
        CHECK_FALSE(is_valid_int8_t(static_cast<int8_t>(-1)));
        CHECK_FALSE(is_valid_int64_t(static_cast<int64_t>(-1)));
    }
    TEST_CASE("batch2147 unsigned validators accept zero but reject their maximum sentinels") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_unsigned_int(0u));
        CHECK(is_valid_uint8_t(static_cast<uint8_t>(0)));
        CHECK(is_valid_uint16_t(static_cast<uint16_t>(1)));
        CHECK(is_valid_uint32_t(static_cast<uint32_t>(2)));
        CHECK(is_valid_uint64_t(static_cast<uint64_t>(3)));
        CHECK_FALSE(is_valid_unsigned_int(std::numeric_limits<unsigned int>::max()));
        CHECK_FALSE(is_valid_uint64_t(std::numeric_limits<uint64_t>::max()));
    }
    TEST_CASE("batch2147 least-width signed validators reject negative values consistently") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_intleast8_t(static_cast<int_least8_t>(0)));
        CHECK(is_valid_intleast16_t(static_cast<int_least16_t>(5)));
        CHECK(is_valid_intleast32_t(static_cast<int_least32_t>(6)));
        CHECK(is_valid_intleast64_t(static_cast<int_least64_t>(7)));
        CHECK_FALSE(is_valid_intleast32_t(static_cast<int_least32_t>(-1)));
    }
    TEST_CASE("batch2147 fast unsigned validators reject only maximum sentinel values") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_uint_fast8_t(static_cast<uint_fast8_t>(0)));
        CHECK(is_valid_uint_fast16_t(static_cast<uint_fast16_t>(9)));
        CHECK(is_valid_uint_fast32_t(static_cast<uint_fast32_t>(10)));
        CHECK(is_valid_uint_fast64_t(static_cast<uint_fast64_t>(11)));
        CHECK_FALSE(is_valid_uint_fast64_t(std::numeric_limits<uint_fast64_t>::max()));
    }
    TEST_CASE("batch2147 floating validators accept finite extremes and reject nan and infinity") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_float(std::numeric_limits<float>::lowest()));
        CHECK(is_valid_double(std::numeric_limits<double>::max()));
        CHECK(is_valid_long_double(0.0L));
        CHECK_FALSE(is_valid_float(std::numeric_limits<float>::infinity()));
        CHECK_FALSE(is_valid_double(std::numeric_limits<double>::quiet_NaN()));
    }
    TEST_CASE("batch2147 complex validators require both components to be finite") {
        using namespace Salix::ValidationUtils;
        CHECK(is_valid_complex_float({1.0f, -2.0f}));
        CHECK(is_valid_complex_double({0.0, 0.0}));
        CHECK_FALSE(is_valid_complex_float({std::numeric_limits<float>::infinity(), 0.0f}));
        CHECK_FALSE(is_valid_complex_double({0.0, std::numeric_limits<double>::quiet_NaN()}));
    }
}

