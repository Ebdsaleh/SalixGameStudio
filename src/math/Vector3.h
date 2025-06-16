// Vector3
#pragma once

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
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
