// Vector3
#pragma once

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

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
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}
