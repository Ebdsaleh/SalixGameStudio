// Salix/math/Vector3
#pragma once
#include <cmath>  // For the sqrtf function.

namespace Salix {

    struct Vector3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        //  Calculates the magnitude (length) of the vector.
        float length() const;

        // Modifies this vector to make it a unit vector (length of 1).
        void normalize();

        // static lerp method declaration
        static Vector3 lerp(const Vector3& start, const Vector3& end, float t);
    };

    // Operator overloads
    // Adding Vector3's.
    Vector3 operator+(const Vector3& a, const Vector3& b);

    Vector3 operator+(const Vector3& a, float increment);

    Vector3 operator+(float increment, const Vector3& a);

    // Subtracting Vector3's

    Vector3 operator-(const Vector3& a, const Vector3& b); 

    Vector3 operator-(const Vector3& a, float decrement);

    Vector3 operator-(float decrement, Vector3& a);

    // Multiplying Vector3's.
    Vector3 operator*(const Vector3& a, const Vector3& b);

    Vector3 operator*(float scalar, const Vector3& a);

    Vector3 operator*(const Vector3& a, float scalar);

    // Dividing Vector3's.
    Vector3 operator/(const Vector3& a, const Vector3& b);

    Vector3 operator/(const Vector3& a, float divisor);

    Vector3 operator/(float divisor, const Vector3& a);
} // namespace Salix