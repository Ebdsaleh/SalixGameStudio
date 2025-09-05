// Salix/core/ValidationUtils.cpp
#include <Salix/core/ValidationUtils.h>
#include <type_traits>
#include <string>
#include <cmath>
#include <limits>
#include <cstdint>


namespace Salix::ValidationUtils {

    bool is_valid_string(const std::string& str, bool allow_empty, bool allow_whitespace) {
        // 1. Handle the empty case first.
        if (str.empty()) {
            return allow_empty; // Returns true if empty is allowed, false otherwise.
        }
        // 2. If whitespace is not allowed, check for whitespace-only strings.
        if (!allow_whitespace) {
            if (str.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
                return false;
            }
        }
        // 3. If we've passed the checks, the string is valid.
        return true;
    }

    bool is_valid_c_str(const char* test_c_str) {
        bool is_valid = (test_c_str != nullptr);
        return is_valid;
    }

    // Signed ints
    // A default "valid" int is one that is within bounds AND not negative.
    bool is_valid_int(int value) {
        // We are calling the low-level "toolbox" functions from the header.
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int8_t(int8_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int16_t(int16_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int32_t(int32_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int64_t(int64_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }


    // Unsigned ints
    // A default "valid" unsigned int just needs to be within its bounds.
    bool is_valid_unsigned_int(unsigned int value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint8_t(uint8_t value) {

        return is_within_bounds_unsigned(value);
    }
    bool is_valid_uint16_t(uint16_t value) {

        return is_within_bounds_unsigned(value);
    }
    bool is_valid_uint32_t(uint32_t value) {

        return is_within_bounds_unsigned(value);
    }
    bool is_valid_uint64_t(uint64_t value) {

        return is_within_bounds_unsigned(value);
    }

    // Least signed ints
    bool is_valid_intleast8_t(int_least8_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_intleast16_t(int_least16_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_intleast32_t(int_least32_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_intleast64_t(int_least64_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    // Least unsigned ints
    bool is_valid_uint_least8_t(uint_least8_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_least16_t(uint_least16_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_least32_t(uint_least32_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_least64_t(uint_least64_t value) {
        return is_within_bounds_unsigned(value);
    }


    // Fast signed ints
    bool is_valid_int_fast8_t(int_fast8_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int_fast16_t(int_fast16_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int_fast32_t(int_fast32_t value) {
    return is_within_bounds(value) && is_not_negative(value);
    }

    bool is_valid_int_fast64_t(int_fast64_t value) {
    return is_within_bounds(value) && is_not_negative(value);
    }


    // Fast unsigned ints
    bool is_valid_uint_fast8_t(uint_fast8_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_fast16_t(uint_fast16_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_fast32_t(uint_fast32_t value) {
        return is_within_bounds_unsigned(value);
    }

    bool is_valid_uint_fast64_t(uint_fast64_t value) {
        return is_within_bounds_unsigned(value);
    }


    // Maximum signed int
    bool is_valid_intmax_t(intmax_t value) {
        return is_within_bounds(value) && is_not_negative(value);
    }

    // Maximum unsigned int
    bool is_valid_uintmax_t(uintmax_t value) {
        return is_within_bounds_unsigned(value);
    }

    // A default "valid" float is one that is a finite number.
    bool is_valid_float(float value) {
        return is_finite(value);
    }


    bool is_valid_double(double value) {
        return is_finite(value);
    }

    bool is_valid_long_double(long double value) {
        return is_finite(value);
    }

    // Complex numbers
    bool is_valid_complex_float(const std::complex<float>& value) {
        return is_finite_complex(value);
    }

    bool is_valid_complex_double(const std::complex<double>& value) {
        return is_finite_complex(value);
    }

    bool is_valid_complex_long_double(const std::complex<long double>& value) {
        return is_finite_complex(value);
    }

} // namespace Salix::ValidationUtils