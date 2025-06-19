// Salix/math/Vector2.h
#pragma once
#include <cmath>  // For sqrtf function.

namespace Salix {
    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        // Calculates the magnitude (length) of the vector.
        float length() const;

        // Modifies this vector to make it a unit vector (length of 1).
        void normalize();

        // A static "factory" method for linear interpolation.
        static Vector2 lerp(const Vector2& start, const Vector2& end, float t);
    };

    // --- Overloading declarations ---
    // Addition
    Vector2 operator+(const Vector2& a, const Vector2& b);

    Vector2 operator+(const Vector2& a, float increment);

    Vector2 operator+(float increment, const Vector2& a);

    // Subtraction
    Vector2 operator-(const Vector2& a, const Vector2& b);

    Vector2 operator-(const Vector2& a, float decrement);

    Vector2 operator-(float decrement, const Vector2& a);

    // Multiplication
    Vector2 operator*(const Vector2& a, const Vector2& b);

    Vector2 operator*(const Vector2& a, float scalar);

    Vector2 operator*(float scalar, const Vector2& a);

    // Division
    Vector2 operator/(const Vector2& a, const Vector2& b);

    Vector2 operator/(const Vector2& a, float divisor);

    Vector2 operator/(float divisor, const Vector2& a);
} // namespace Salix