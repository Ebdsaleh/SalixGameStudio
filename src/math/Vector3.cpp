// Vector3.cpp
#include "Vector3.h"

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