// Salix/math/Vector3
#pragma once

#include <Salix/core/Core.h>
#include <cmath>  // For the sqrtf function.
#include <cereal/cereal.hpp>
namespace Salix {

    struct SALIX_API Vector3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

         // --- CONSTRUCTORS ---
        // Default constructor (creates a zero vector)
        Vector3() = default;

        // Constructor that takes x, y, and z values
        Vector3(float x_in, float y_in, float z_in) : x(x_in), y(y_in), z(z_in) {}

        //  Calculates the magnitude (length) of the vector.
        float length() const;

        // Modifies this vector to make it a unit vector (length of 1).
        void normalize();

        // static lerp method declaration
        static Vector3 lerp(const Vector3& start, const Vector3& end, float t);

        // --- NEW: The Cereal Serialization "Blueprint" ---
        // This one templated function tells Cereal how to both SAVE and LOAD a Vector3.
        // The 'Archive' can be a JSON archive, an XML archive, etc. The Vector3 doesn't care.
        template<class Archive>
        void serialize(Archive& archive)
        {
            // This is the magic. The CEREAL_NVP macro creates a named key-value pair.
            // When saving, it writes the value of 'x' to the key "x".
            // When loading, it finds the key "x" and reads its value into the 'x' member.
            archive(cereal::make_nvp("x", x), cereal::make_nvp("y", y), cereal::make_nvp("z", z));
        }
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