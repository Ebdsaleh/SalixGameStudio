// Salix/math/Vector3
#pragma once

#include <Salix/core/Core.h>
#include <cmath>  // For the sqrtf function.
#include <cereal/cereal.hpp>
#include <vector>
#include <glm/glm.hpp>

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

        glm::vec3 to_glm() const;


        Vector3& operator+=(const Vector3& other) {
        // Modify this object's members
        x += other.x;
        y += other.y;
        z += other.z;

        // Return a reference to this object
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3& operator/=(float divisor) {
        if (divisor != 0.0f) {
            x /= divisor;
            y /= divisor;
            z /= divisor;
        } else {
            // Handle error, e.g., set to zero or log a warning.
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
        }
        return *this;
    }
    };

    // Operator overloads
    // Adding Vector3's.
    inline Vector3 operator+(const Vector3& a, const Vector3& b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    inline Vector3 operator+(const Vector3& a, float increment) {
        return Vector3 { a.x + increment, a.y + increment, a.x + increment };
    }

    inline Vector3 operator+(float increment, const Vector3& a) {
        return Vector3 { increment + a.x, increment + a.y, increment + a.z };
    }

    // Subtracting Vector3's.
    inline Vector3 operator-(const Vector3& a, const Vector3& b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    inline Vector3 operator-(const Vector3& a, float decrement) {
        return Vector3 { a.x - decrement, a.y - decrement, a.z - decrement };
    }

    inline Vector3 operator-(float decrement, const Vector3& a) {
        return Vector3 { decrement - a.x, decrement - a.y, decrement - a.z };
    }

    // Multiplying Vector3's.
    inline Vector3 operator*(const Vector3& a, const Vector3& b) {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    inline Vector3 operator*(float scalar, const Vector3& a) {
        return Vector3 { scalar * a.x, scalar * a.y, scalar * a.z };
    }

    inline Vector3 operator*(const Vector3& a, float scalar) {
        return Vector3 { a.x * scalar, a.y * scalar, a.z * scalar };
    }

    // Dividing Vector3's.
    inline Vector3 operator/(const Vector3& a, const Vector3& b) {
        // Check each axis individually to handle division by Zero safely.

        float result_x = 0.0f;
        if (b.x != 0.0f) {
            result_x = a.x / b.x;
        } else {
            std::cerr << "Warning: Division by zero on X-axis!" << std::endl;
        }

        float result_y = 0.0f;
        if (b.y !=0) {
            result_y = a.y / b.y;
        } else {
            std::cerr << "Warning: Division by zero on Y-axis!" << std::endl;
        }

        float result_z = 0.0f;
        if (b.z !=0) {
            result_z = a.z / b.z;
        } else {
            std::cerr << "Warning: Division by zero on Z-axis!" << std::endl;
        }
        return { result_x, result_y, result_z };
    }

    // Vector3 / float
    inline Vector3 operator/(const Vector3& a, float divisor) {
        if (divisor == 0.0f) {
            std::cerr << "Warning: Division by zero scalar/divisor!" << std::endl;
            return Vector3 { 0.0f, 0.0f, 0.0f };
        }
        return Vector3 { a.x / divisor, a.y / divisor, a.z / divisor };
    }

    inline Vector3 operator/(float divisor, const Vector3& a) {
        float result_x = 0.0f;
        if (a.x != 0.0f) {
            result_x = divisor / a.x;
        } else {
            std::cerr << "Warning: Division by zero on X-axis!" << std::endl;
        }

        float result_y = 0.0f;
        if (a.y != 0.0f) {
            result_y = divisor / a.y;
        } else {
            std::cerr << "Warning: Division by zero on Y-axis!" << std::endl;
        }

        float result_z = 0.0f;
        if (a.z != 0.0f) {
            result_z = divisor / a.z;
        } else {
            std::cerr << "Warning: Division by zero on Z-axis!" << std::endl;
        }

        return Vector3 { result_x, result_y, result_z };
    }

    /// Calculates the dot product of two vectors.
    inline float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    /// Calculates the cross product of two vectors, returning a new vector
    /// that is perpendicular to both a and b.
    inline Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }     
} // namespace Salix