// Tests/SalixEngine/core/TestValidators.h
#pragma once
#include <doctest.h>
#include <Salix/math/Vector3.h>
#include <string>
#include <type_traits> // Required for this new function

// This is a namespace just for our test utility functions
namespace TestValidators {

    // A generic helper to check if a variable is of the expected type.
    template<typename ExpectedType, typename ActualType>
    void CHECK_IS_TYPE(const ActualType& variable) {
        // std::is_same_v compares two types at compile time.
        // std::decay_t removes extras like 'const' and references (&)
        // so we are comparing the pure underlying types (e.g., int vs. int).
        bool is_same_type = std::is_same_v<std::decay_t<ExpectedType>, std::decay_t<ActualType>>;
        CHECK(is_same_type);
    }

    // Checks if two Vector3 objects are almost equal...
    inline void CHECK_VEC3_EQUALS(const Salix::Vector3& a, const Salix::Vector3& b) {
        CHECK(a.x == doctest::Approx(b.x));
        CHECK(a.y == doctest::Approx(b.y));
        CHECK(a.z == doctest::Approx(b.z));
    }

    // A simple validator to check if a string is not empty.
    inline void CHECK_STRING_NOT_EMPTY(const std::string& str) {
        CHECK_FALSE(str.empty());
    }

} // namespace TestValidators