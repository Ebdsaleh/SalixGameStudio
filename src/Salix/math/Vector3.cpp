// Vector3.cpp
#include <Vector3.h>
#include <iostream>  // Needed for std::cerr
#include <algorithm>

namespace Salix {

    // Calculates the magnitude (length) of the vector.
    float Vector3::length() const {
        return sqrtf(x * x + y * y + z * z);
    }

    // Modifies this vector to make it a unit vector (length of 1).
    void Vector3::normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    // Lerp method definition
    Vector3 Vector3::lerp(const Vector3& start, const Vector3& end, float t) {
        t = std::max(0.0f, std::min(1.0f, t));  // Clamp t between 0 and 1.
        // Note: We use the '+' operator for vectors, not '*' like we did for color.
        return start + (end - start) * t;
    }

    // Operator overloads
    // Adding Vector3's.
    Vector3 operator+(const Vector3& a, const Vector3& b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    Vector3 operator+(const Vector3& a, float increment) {
        return Vector3 { a.x + increment, a.y + increment, a.x + increment };
    }

    Vector3 operator+(float increment, const Vector3& a) {
        return Vector3 { increment + a.x, increment + a.y, increment + a.z };
    }

    // Subtracting Vector3's.
    Vector3 operator-(const Vector3& a, const Vector3& b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    Vector3 operator-(const Vector3& a, float decrement) {
        return Vector3 { a.x - decrement, a.y - decrement, a.z - decrement };
    }

    Vector3 operator-(float decrement, const Vector3& a) {
        return Vector3 { decrement - a.x, decrement - a.y, decrement - a.z };
    }

    // Multiplying Vector3's.
    Vector3 operator*(const Vector3& a, const Vector3& b) {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    Vector3 operator*(float scalar, const Vector3& a) {
        return Vector3 { scalar * a.x, scalar * a.y, scalar * a.z };
    }

    Vector3 operator*(const Vector3& a, float scalar) {
        return Vector3 { a.x * scalar, a.y * scalar, a.z * scalar };
    }

    // Dividing Vector3's.
    Vector3 operator/(const Vector3& a, const Vector3& b) {
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
    Vector3 operator/(const Vector3& a, float divisor) {
        if (divisor == 0.0f) {
            std::cerr << "Warning: Division by zero scalar/divisor!" << std::endl;
            return Vector3 { 0.0f, 0.0f, 0.0f };
        }
        return Vector3 { a.x / divisor, a.y / divisor, a.z / divisor };
    }

    Vector3 operator/(float divisor, const Vector3& a) {
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
} // namespace Salix