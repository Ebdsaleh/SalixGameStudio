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
    
    // For constexpr compatibility (C++17)
    const Vector3 Vector3::Zero {0.0f, 0.0f, 0.0f};
    

    YAML::Emitter& operator<<(YAML::Emitter& out, const Salix::Vector3& v) {
        out << YAML::BeginMap;
        out << YAML::Key << "x" << YAML::Value << v.x;
        out << YAML::Key << "y" << YAML::Value << v.y;
        out << YAML::Key << "z" << YAML::Value << v.z;
        out << YAML::EndMap;
        return out;
    }
    
    void operator>>(const YAML::Node& node, Salix::Vector3& v) {
        v.x = node["x"] ? node["x"].as<float>() : 0.0f; 
        v.y = node["y"] ? node["y"].as<float>() : 0.0f;
        v.z = node["z"] ? node["z"].as<float>() : 0.0f;
    }
    
} // namespace Salix