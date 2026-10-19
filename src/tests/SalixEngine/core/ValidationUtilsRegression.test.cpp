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
