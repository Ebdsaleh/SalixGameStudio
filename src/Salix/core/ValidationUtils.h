// Salix/core/ValidationUtils.h
#pragma once
#include <Salix/core/Core.h>
#include <cstdint>
#include <cmath>
#include <complex>
#include <limits>
#include <type_traits>
#include <string>


// In C++, it's more common to group free-standing utility functions
// in a namespace rather than a class with only static methods.
namespace Salix::ValidationUtils {

    // --- Function DECLARATIONS ---
    

    SALIX_API bool is_valid_string(const std::string& test_string, bool allow_empty = false, bool allow_whitespace = false);
    SALIX_API bool is_valid_c_str(const char* test_c_str);

    // Signed ints
    SALIX_API bool is_valid_int(int test_int);
    SALIX_API bool is_valid_int8_t(int8_t test_int8_t);
    SALIX_API bool is_valid_int16_t(int16_t test_int16_t);
    SALIX_API bool is_valid_int32_t(int32_t test_int32_t);
    SALIX_API bool is_valid_int64_t(int64_t test_int64_t);

    // Unsigned ints
    SALIX_API bool is_valid_unsigned_int(unsigned int test_unsigned_int);
    SALIX_API bool is_valid_uint8_t(uint8_t test_uint8_t);
    SALIX_API bool is_valid_uint16_t(uint16_t test_uint16_t);
    SALIX_API bool is_valid_uint32_t(uint32_t test_uint32_t);
    SALIX_API bool is_valid_uint64_t(uint64_t test_uint64_t);

    // Least signed ints
    SALIX_API bool is_valid_intleast8_t(int_least8_t test_int_least8_t);
    SALIX_API bool is_valid_intleast16_t(int_least16_t test_int_least16_t);
    SALIX_API bool is_valid_intleast32_t(int_least32_t test_int_least32_t);
    SALIX_API bool is_valid_intleast64_t(int_least64_t test_int_least64_t);

    // Least unsigned ints
    SALIX_API bool is_valid_uint_least8_t(uint_least8_t test_uint_least8_t);
    SALIX_API bool is_valid_uint_least16_t(uint_least16_t test_uint_least16_t);
    SALIX_API bool is_valid_uint_least32_t(uint_least32_t test_uint_least32_t);
    SALIX_API bool is_valid_uint_least64_t(uint_least64_t test_uint_least64_t);

    // Fast signed ints
    SALIX_API bool is_valid_int_fast8_t(int_fast8_t test_int_fast8_t);
    SALIX_API bool is_valid_int_fast16_t(int_fast16_t test_int_fast16_t);
    SALIX_API bool is_valid_int_fast32_t(int_fast32_t test_int_fast32_t);
    SALIX_API bool is_valid_int_fast64_t(int_fast64_t test_int_fast64_t);

    // Fast unsigned ints
    SALIX_API bool is_valid_uint_fast8_t(uint_fast8_t test_uint_fast8_t);
    SALIX_API bool is_valid_uint_fast16_t(uint_fast16_t test_uint_fast16_t);
    SALIX_API bool is_valid_uint_fast32_t(uint_fast32_t test_uint_fast32_t);
    SALIX_API bool is_valid_uint_fast64_t(uint_fast64_t test_uint_fast64_t);

    // Float
    SALIX_API bool is_valid_float(float test_float);
    // Double
    SALIX_API bool is_valid_double(double test_double);
    // Long Double
    SALIX_API bool is_valid_long_double(long double test_long_double);

    // Maximum signed int
    SALIX_API bool is_valid_intmax_t(intmax_t test_intmax_t);

    // Maximum unsigned int
    SALIX_API bool is_valid_uintmax_t(uintmax_t test_uintmax_t);
    
    // Complex numbers
    SALIX_API bool is_valid_complex_float(const std::complex<float>& test_complex_float);
    SALIX_API bool is_valid_complex_double(const std::complex<double>& test_complex_double);
    SALIX_API bool is_valid_complex_long_double(const std::complex<long double>& test_complex_long_double);

    // --- TEMPLATE FUNCTIONS ---
    // Template functions are defined entirely in the header file.
    // This one function can check ANY type of pointer for null.

    // --- TYPE TRAIT HELPERS (For auto-generation) ---
    // These are the C++ equivalent of Python's isinstance(). They check the fundamental
    // category of a type at compile-time.

    template<typename T>
    inline constexpr bool is_integral() {
        return std::is_integral_v<std::decay_t<T>>;
    }

    template<typename T>
    inline constexpr bool is_floating_point() {
        return std::is_floating_point_v<std::decay_t<T>>;
    }

    template<typename T>
    inline constexpr bool is_signed() {
        return std::is_signed_v<std::decay_t<T>>;
    }

    template<typename T>
    inline constexpr bool is_unsigned() {
        return std::is_unsigned_v<std::decay_t<T>>;
    }


    // Generic validation helpers.

    // This is the generic type-checker for ByteMirror.
    template<typename ExpectedType, typename ActualType>
    inline constexpr bool is_type(const ActualType& variable) {
        (void)variable;
        // This checks if the core types are the same. It will return
        // true for is_type<int>(my_int_variable)
        return std::is_same_v<std::decay_t<ExpectedType>, std::decay_t<ActualType>>;
    }


    // Checks if a signed type is within its bounds (not min or max).
    template<typename T>
    inline bool is_within_bounds(const T& value) {
        static_assert(is_signed<T>(), "is_within_bounds is for signed types.");
        return value != std::numeric_limits<T>::max() && value != std::numeric_limits<T>::min();
    }

    // Checks if an unsigned type is within its bounds (not max).
    template<typename T>
    inline bool is_within_bounds_unsigned(const T& value) {
        static_assert(is_unsigned<T>(), "is_within_bounds_unsigned is for unsigned types.");
        return value != std::numeric_limits<T>::max();
    }

    // Generic function to check if any integral type (int, uint8_t, uint64_t, etc.)
    // has a "valid" value (i.e., it hasn't overflowed to its maximum).
    template<typename T>
    inline bool is_valid_integral(const T& value) {
        // This static_assert ensures this function can ONLY be compiled with integer types.
        static_assert(is_integral<T>(), "is_valid_integral can only be used with integer types.");
        return value != std::numeric_limits<T>::max();
    }

    // Generic function to check if any floating-point type (float, double, long double).
    // is a normal, finite number.
    template<typename T>
    inline bool is_valid_floating_point(const T& value) {
        // This static_assert ensures this function can ONLY be used with floating-point types.
        static_assert(is_floating_point<T>(), "is_valid_floating_point can only be used with float, double, or long double.");
        return !std::isnan(value) && !std::isinf(value);
    }

    
    template<typename T>
    inline bool is_valid_ptr(T* test_ptr) {
        return test_ptr != nullptr;
    }

     // Checks if any signed number (int, short, long, etc.) is not negative.
    template<typename T>
    inline bool is_not_negative(const T& value) {
        // This check only works for signed types.
        static_assert(is_signed<T>(), "is_not_negative can only be used with signed types.");
        return value >= 0;
    }

    // Checks if a float, double, or long double is a normal, finite number.
    template<typename T>
    inline bool is_finite(const T& value) {
        // Use 'if constexpr' to perform the check at COMPILE TIME.
        if constexpr (is_floating_point<T>()) {
            // This code block will only be compiled if T is a float, double, or long double.
            return !std::isnan(value) && !std::isinf(value);
        } else {
            // This code block will be compiled for all other types (like int).
            return false;
        }
    }

    // A dedicated template for complex types.
    template<typename T>
    inline bool is_finite_complex(const std::complex<T>& value) {
        // A complex number is finite if both its real and imaginary parts are finite.
        // This correctly calls the is_finite template above for the .real() and .imag() parts.
        return is_finite(value.real()) && is_finite(value.imag());
    }


} // namespace Salix::ValidationUtils