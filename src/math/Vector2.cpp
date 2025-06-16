// Vector2.cpp
#include "Vector2.h"
#include <iostream>
#include <algorithm>

// A static "factory" method for linear interpolation.
    Vector2 Vector2::lerp(const Vector2& start, const Vector2& end, float t) {
        t = std::max(0.0f, std::min(1.0f, t));  // Clamp t between 0 and 1
        return start + (end - start) * t;
    }
// --- Overloading definitions ---
// Addition
Vector2 operator+(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x + b.x, a.y + b.y };
}

Vector2 operator+(const Vector2& a, float increment) {
    return Vector2 { a.x + increment, a.y + increment };
}

Vector2 operator+(float increment, const Vector2& a) {
    return Vector2 { increment + a.x, increment + a.y };
}

// Subtraction
Vector2 operator-(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x-b.x, a.y - b.y };
}

Vector2 operator-(const Vector2& a, float decrement) {
    return Vector2 { a.x - decrement, a.y - decrement };
}

Vector2 operator-(float decrement, const Vector2& a) {
    return Vector2 { decrement - a.x, decrement - a.y };
}

// Multiplication 
// Multiply 2 Vector2's
Vector2 operator*(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x * b.x, a.y * b.y };
}

// Multiply Vector2 and a float 
Vector2 operator*(const Vector2& a, float scalar) {
    return Vector2 { a.x * scalar, a.y * scalar };
}

// Multiply a float and a Vector2
Vector2 operator*(float scalar, const Vector2& a) {
    return Vector2 {  scalar * a.x, scalar * a.y };
}

// Vector2 Division
Vector2 operator/(const Vector2& a, const Vector2& b){
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
Vector2 operator/(const Vector2& a, float divisor) {
    if (divisor == 0.0f) {
        std::cerr << "Warning: Division by zero scalar/divisor!" << std::endl;
        return Vector2 { 0.0f, 0.0f };
    }
    return Vector2 { a.x / divisor, a.y / divisor };
}

// float / Vector 2
Vector2 operator/(float divisor, const Vector2& a) {
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
