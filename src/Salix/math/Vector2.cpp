// Salix/math/Vector2.cpp
#include <Salix/math/Vector2.h>
#include <iostream>
#include <algorithm>


namespace Salix {

    // Calculates the magnitude (length) of the vector.
    float Vector2::length() const {
        return sqrtf(x * x + y * y);
    }

    // Modifies this vector to make it a unit vector (length of 1).
    void Vector2::normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
        }
    }

    // A static "factory" method for linear interpolation.
        Vector2 Vector2::lerp(const Vector2& start, const Vector2& end, float t) {
            t = std::max(0.0f, std::min(1.0f, t));  // Clamp t between 0 and 1
            return start + (end - start) * t;
        }

    ImVec2 Vector2::to_im_vec2() {
        return ImVec2(x, y);
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const Salix::Vector2& v) {
        out << YAML::BeginMap;
        out << YAML::Key << "x" << YAML::Value << v.x;
        out << YAML::Key << "y" << YAML::Value << v.y;
        out << YAML::EndMap;
        return out;
    }
    
    void operator>>(const YAML::Node& node, Salix::Vector2& v) {
        v.x = node["x"] ? node["x"].as<float>() : 0.0f; 
        v.y = node["y"] ? node["y"].as<float>() : 0.0f;
    }

} // namespace Salix