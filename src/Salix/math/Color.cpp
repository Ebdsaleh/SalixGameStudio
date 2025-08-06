// Salix/math/Color.cpp
#include <Salix/math/Color.h>
#include <Salix/core/SerializationRegistrations.h>
#include <cmath>
#include <algorithm>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <imgui/imgui.h>




namespace Salix {

    Color Color::from_hsv(float h, float s, float v, float alpha) {
        if (s <= 0.0f) {
            return Color(v, v, v, alpha);
        }

        h = fmod(h, 360.0f);
        if (h < 0.0f) h += 360.0f;
        h /= 60.0f;

        int i = static_cast<int>(h);
        float f = h - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - (s * f));
        float t = v * (1.0f - (s * (1.0f - f)));

        switch(i) {
            case 0: return Color(v, t, p, alpha);
            case 1: return Color(q, v, p, alpha);
            case 2: return Color(p, v, t, alpha);
            case 3: return Color(p, q, v, alpha);
            case 4: return Color(t, p, v, alpha);
            default: return Color(v, p, q, alpha);
        }
    }

    ImVec4 Color::to_imvec4() const {
        return ImVec4(r, g, b, a);
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const Color& c) {
        out << YAML::BeginMap;
        out << YAML::Key << "r" << YAML::Value << c.r;
        out << YAML::Key << "g" << YAML::Value << c.g;
        out << YAML::Key << "b" << YAML::Value << c.b;
        out << YAML::Key << "a" << YAML::Value << c.a;
        out << YAML::EndMap;
        return out;
    }

    void operator>>(const YAML::Node& node, Color& c) {
        c.r = node["r"] ? node["r"].as<float>() : 0.0f; 
        c.g = node["g"] ? node["g"].as<float>() : 0.0f;
        c.b = node["b"] ? node["b"].as<float>() : 0.0f;
        c.a = node["a"] ? node["a"].as<float>() : 0.0f;
    }

    // Explicit template instantiation
    template void Color::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);
    template void Color::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Color::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
    template void Color::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
}