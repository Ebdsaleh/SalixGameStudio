// Vector3.cpp
#include "Vector3.h"
#include <iostream>  // Needed for std::cerr

// Operator overloads
// Adding Vector3's.
Vector3 operator+(const Vector3& a, const Vector3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

// Subtracting Vector3's.
Vector3 operator-(const Vector3& a, const Vector3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

// Multiplying Vector3's.
Vector3 operator*(const Vector3& a, const Vector3& b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
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