#include <doctest.h>
#include <Salix/core/ValidationUtils.h>
#include <complex>
#include <cstdint>
#include <limits>
#include <type_traits>

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
