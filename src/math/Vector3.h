// Vector3
#pragma once

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // static lerp method declaration
    static Vector3 lerp(const Vector3& start, const Vector3& end, float t);
};

// Operator overloads
// Adding Vector3's.
Vector3 operator+(const Vector3& a, const Vector3& b);

// Subtracting Vector3's.
Vector3 operator-(const Vector3& a, const Vector3& b); 

// Multiplying Vector3's.
Vector3 operator*(const Vector3& a, const Vector3& b);

// Dividing Vector3's.
Vector3 operator/(const Vector3& a, const Vector3& b);
