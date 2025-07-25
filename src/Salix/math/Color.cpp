// Salix/math/Color.cpp
#include <Salix/math/Color.h>
#include <Salix/core/SerializationRegistrations.h>
#include <cmath>
#include <iostream> // For std::cerr, though not used in this version
#include <algorithm> // For std::min/max
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <imgui/imgui.h>

namespace Salix {

    // --- DEFINED CONSTRUCTORS ---
    Color::Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}

    // In the definition, we list all parameters without default values.
    Color::Color(float red, float green, float blue, float alpha)
        : r(red), g(green), b(blue), a(alpha) {}

    // --- Static Factory Methods ---
    // The default argument for 'alpha' is removed from the definition here.
    Color Color::from_rgba_int(int red, int green, int blue, int alpha){
        return Color(
            static_cast<float>(red) / 255.0f,
            static_cast<float>(green) / 255.0f,
            static_cast<float>(blue) / 255.0f,
            static_cast<float>(alpha) / 255.0f
        );
    }

    // The default argument for 'alpha' is removed from the definition here.
    Color Color::from_hsv(float h, float s, float v, float alpha) {
        if (s <= 0.0f) {  // If saturation is 0, it's a shade of grey
            return Color(v, v, v, alpha);
        }

        h = fmod(h, 360.0f);  // Wrap hue around.
        if (h < 0.0f) h += 360.0f;
        h /= 60.0f;  // Convert hue to a 0-6 range.

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

    Color Color::lerp(const Color& start, const Color& end, float t) {
        t = std::max(0.0f, std::min(1.0f, t)); // Clamp t between 0 and 1
        return start * (1.0f - t) + end * t;
    }

    ImVec4 Color::to_imvec4() {
      
        return ImVec4(r, g, b, a);

    }
    

    // --- OPERATOR OVERLOADS for color math ---
    Color operator+(const Color& a, const Color& b) {
        return { a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a };
    }

    Color operator-(const Color& a, const Color& b) {
        return { a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a };
    }

    Color operator*(const Color& a, float scalar) {
        return { a.r * scalar, a.g * scalar, a.b * scalar, a.a * scalar };
    }

    // Required for the lerp method.
    Color operator*(float scalar, const Color& a) {
        return a * scalar; // Just reuse the other operator
    }


    // --- YAML PARSING COMPLIANCE ---
    YAML::Emitter& operator<<(YAML::Emitter& out, const Salix::Color& c) {
        out << YAML::BeginMap;
        out << YAML::Key << "r" << YAML::Value << c.r;
        out << YAML::Key << "g" << YAML::Value << c.g;
        out << YAML::Key << "b" << YAML::Value << c.b;
        out << YAML::Key << "a" << YAML::Value << c.a;
        out << YAML::EndMap;
        return out;
    }


    void operator>>(const YAML::Node& node, Salix::Color& c) {
        c.r = node["r"] ? node["r"].as<float>() : 0.0f; 
        c.g = node["g"] ? node["g"].as<float>() : 0.0f;
        c.b = node["b"] ? node["b"].as<float>() : 0.0f;
        c.a = node["a"] ? node["a"].as<float>() : 0.0f;
    }



    template<class Archive>
    void Color::serialize(Archive& archive) {
        archive(
            cereal::make_nvp("r", r), cereal::make_nvp("g", g),
            cereal::make_nvp("b", b), cereal::make_nvp("a", a)
        );
    }

    template void Color::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Color::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    template void Color::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Color::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);
    
} // namespace Salix