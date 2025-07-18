// Salix/math/Vector3.cpp
#include <Salix/math/Vector3.h>
#include <iostream>  // Needed for std::cerr
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

    glm::vec3 Vector3::to_glm() const{
        return glm::vec3(x, y, z);
    }
    
} // namespace Salix