// Vector2.cpp
#pragma once

#include "Vector2.h"
#include <iostream>
#include <algorithm>

// A static "factory" method for linear interpolation.
    Vector2 Vector2::lerp(const Vector2& start, const Vector2& end, float t) {
        t = std::max(0.0f, std::min(1.0f, t));  // Clamp t between 0 and 1
        return start * (1.0f - t) + end * t;
    }
// --- Overloading definitions ---
// Addition
Vector2 operator+(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x + b.x, a.y + b.y };
}

// Subtraction
Vector2 operator-(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x-b.x, a.y - b.y };
}

// Multiplication 
// Multiply 2 Vector2's
Vector2 operator*(const Vector2& a, const Vector2& b) {
    return Vector2 { a.x * b.x, a.y * b.y };
}

// --- Required for Linear interpolation ---
// Multiply Vector2 and a float 
Vector2 operator*(const Vector2& a, float scalar) {
    return a * scalar; 
}

// Multiply a float and a Vector2 // required for lerp
Vector2 operator*(float scalar, const Vector2& a) {
    return a * scalar;  
}

// Division
Vector2 operator/(const Vector2& a, const Vector2& b){
    // Division by zero check.
    float result_x = 0.0f;
    if (b.x != 0.0f) {
        result_x = a.x / b.x;
    } else {
        std::cerr << "Warning: Division by zero on Y-axis!" << std::endl;
    }

    float result_y = 0.0f;
    if (b.y != 0.0f) {
        result_y = a.y / b.y;
    } else {
        std::cerr << "Warning: Division by zero on Y-axis!" << std::endl;
    }
    return Vector2 { result_x, result_y };
}

