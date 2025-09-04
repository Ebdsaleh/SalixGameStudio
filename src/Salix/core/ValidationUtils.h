// Salix/core/ValidationUtils.h
#pragma once
#include <Salix/core/Core.h>
#include <type_traits>
#include <string>

// In C++, it's more common to group free-standing utility functions
// in a namespace rather than a class with only static methods.
namespace Salix::ValidationUtils {

    // --- Function DECLARATIONS ---
    

    SALIX_API bool is_valid_string(const std::string& test_string, bool allow_empty = false, bool allow_whitespace);
    SALIX_API bool is_valid_c_str(const char* test_c_str);
    // Signed ints
    SALIX_API bool is_valid_int(int test_int);

    // Unsigned ints
    SALIX_API bool is_valid_unsigned_int(unsigned int test_unsigned_int);
    SALIX_API bool is_valid_uint8_t(uint8_t test_uint8_t);
    SALIX_API bool is_valid_uint16_t(uint16_t test_uint16_t);
    SALIX_API bool is_valid_uint32_t(uint32_t test_uint32_t);
    SALIX_API bool is_valid_uint64_t(uint64_t test_uint64_t);
    // Float
    SALIX_API bool is_valid_float(float test_float);
    // Double
    SALIX_API bool is_valid_double(double test_double);

    

    // --- TEMPLATE FUNCTIONS ---
    // Template functions are defined entirely in the header file.
    // This one function can check ANY type of pointer for null.

    // This is the generic type-checker for ByteMirror.
    template<typename ExpectedType, typename ActualType>
    bool is_type(const ActualType& variable) {
        // This checks if the core types are the same. It will return
        // true for is_type<int>(my_int_variable)
        return std::is_same_v<std::decay_t<ExpectedType>, std::decay_t<ActualType>>;
    }

    template<typename T>
    bool is_valid_ptr(T* test_ptr) {
        return test_ptr != nullptr;
    }

     // Checks if any signed number (int, short, long, etc.) is not negative.
    template<typename T>
    bool is_not_negative(const T& value) {
        // This check only works for signed types.
        static_assert(std::is_signed_v<T>, "is_not_negative can only be used with signed types.");
        return value >= 0;
    }

    // Checks if a float or double is a normal, finite number.
    template<typename T>
    bool is_finite(const T& value) {
        // This check only works for floating-point types.
        static_assert(std::is_floating_point_v<T>, "is_finite can only be used with float or double.");
        return !std::isnan(value) && !std::isinf(value);
    }


    // --- TYPE TRAIT HELPERS (For auto-generation) ---
    // These are the C++ equivalent of Python's isinstance(). They check the fundamental
    // category of a type at compile-time.

    template<typename T>
    constexpr bool is_integral() {
        return std::is_integral_v<std::decay_t<T>>;
    }

    template<typename T>
    constexpr bool is_floating_point() {
        return std::is_floating_point_v<std::decay_t<T>>;
    }

    template<typename T>
    constexpr bool is_signed() {
        return std::is_signed_v<std::decay_t<T>>;
    }

} // namespace Salix::ValidationUtils