// Salix/math/Vector2.h
#pragma once
#include <Salix/core/Core.h>
#include <cmath>  // For sqrtf function.
#include <cereal/cereal.hpp>
#include <imgui/imgui.h>
#include <yaml-cpp/yaml.h> 

namespace Salix {
    struct SALIX_API Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        // Default Constructor
        Vector2() = default;
        // Constructor that takes x and y values
        Vector2(float x_in, float y_in) : x(x_in), y(y_in) {}

        // Calculates the magnitude (length) of the vector.
        float length() const;

        // Modifies this vector to make it a unit vector (length of 1).
        void normalize();

        ImVec2 to_im_vec2();

        // A static "factory" method for linear interpolation.
        static Vector2 lerp(const Vector2& start, const Vector2& end, float t);

         // --- The Cereal Serialization "Blueprint" ---
        // This one templated function tells Cereal how to both SAVE and LOAD a Vector2.
        // The 'Archive' can be a JSON archive, an XML archive, etc. The Vector2 doesn't care.
        template<class Archive>
        void serialize(Archive& archive)
        {
            // This is the magic. The CEREAL_NVP macro creates a named key-value pair.
            // When saving, it writes the value of 'x' to the key "x".
            // When loading, it finds the key "x" and reads its value into the 'x' member.
            archive(cereal::make_nvp("x", x), cereal::make_nvp("y", y));
        }

        Vector2& operator+=(const Vector2& other) {
        // Modify this object's members
        x += other.x;
        y += other.y;
        // Return a reference to this object
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(float divisor) {
        if (divisor != 0.0f) {
            x /= divisor;
            y /= divisor;
        } else {
            // Handle error, e.g., set to zero or log a warning.
            x = 0.0f;
            y = 0.0f;
        }
        return *this;
    }

    };

    SALIX_API YAML::Emitter& operator<<(YAML::Emitter& out, const Salix::Vector2& v);
    
    SALIX_API void operator>>(const YAML::Node& node, Salix::Vector2& v);
    // --- Overloading definitions ---
    // Addition
    inline Vector2 operator+(const Vector2& a, const Vector2& b) {
        return Vector2 { a.x + b.x, a.y + b.y };
    }

    inline Vector2 operator+(const Vector2& a, float increment) {
        return Vector2 { a.x + increment, a.y + increment };
    }

    inline Vector2 operator+(float increment, const Vector2& a) {
        return Vector2 { increment + a.x, increment + a.y };
    }

    // Subtraction
    inline Vector2 operator-(const Vector2& a, const Vector2& b) {
        return Vector2 { a.x-b.x, a.y - b.y };
    }

    inline Vector2 operator-(const Vector2& a, float decrement) {
        return Vector2 { a.x - decrement, a.y - decrement };
    }

    inline Vector2 operator-(float decrement, const Vector2& a) {
        return Vector2 { decrement - a.x, decrement - a.y };
    }

    // Multiplication 
    // Multiply 2 Vector2's
    inline Vector2 operator*(const Vector2& a, const Vector2& b) {
        return Vector2 { a.x * b.x, a.y * b.y };
    }

    // Multiply Vector2 and a float 
    inline Vector2 operator*(const Vector2& a, float scalar) {
        return Vector2 { a.x * scalar, a.y * scalar };
    }

    // Multiply a float and a Vector2
    inline Vector2 operator*(float scalar, const Vector2& a) {
        return Vector2 {  scalar * a.x, scalar * a.y };
    }

    // Vector2 Division
    inline Vector2 operator/(const Vector2& a, const Vector2& b){
        // Division by zero check.
        float result_x = 0.0f;
        if (b.x != 0.0f) {
            result_x = a.x / b.x;
        } else {
            std::cerr << "Warning: Division by zero on X-axis!" << std::endl;
        }

        float result_y = 0.0f;
        if (b.y != 0.0f) {
            result_y = a.y / b.y;
        } else {
            std::cerr << "Warning: Division by zero on Y-axis!" << std::endl;
        }
        return Vector2 { result_x, result_y };
    }

    // Vector2 / float
    inline Vector2 operator/(const Vector2& a, float divisor) {
        if (divisor == 0.0f) {
            std::cerr << "Warning: Division by zero scalar/divisor!" << std::endl;
            return Vector2 { 0.0f, 0.0f };
        }
        return Vector2 { a.x / divisor, a.y / divisor };
    }

    // float / Vector 2
    inline Vector2 operator/(float divisor, const Vector2& a) {
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

        return Vector2 { result_x, result_y };
    }
    
    // Inequality operator
    inline bool operator!=(const Vector2& a, const Vector2& b) {
        return !(a.x == b.x && a.y == b.y);
    }

    // Equality operator
    inline bool operator==(const Vector2& a, const Vector2& b) {
        return a.x == b.x && a.y == b.y;
    }

    
    // Calculates the dot product of two 2D vectors.
    inline float dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }
    
    // Calculates the 2D pseudo-cross product, which returns a scalar (float).
    // This is useful for determining winding order and orientation.
    inline float cross(const Vector2& a, const Vector2& b) {
        return a.x * b.y - a.y * b.x;
    }

    /// Returns a new vector that is perpendicular (rotated 90 degrees counter-clockwise) to the input vector.
    inline Vector2 perp(const Vector2& v) {
        return Vector2(-v.y, v.x);
    }

    
} // namespace Salix