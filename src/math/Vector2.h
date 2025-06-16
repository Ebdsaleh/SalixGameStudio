// Vector2.h
#pragma once

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

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



