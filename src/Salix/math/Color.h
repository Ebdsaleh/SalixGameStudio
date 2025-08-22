// Salix/math/Color.h
#pragma once
#include <Salix/core/Core.h>
#include <algorithm>
#include <cereal/access.hpp>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <ostream>
#include <yaml-cpp/yaml.h> 

namespace Salix {

    struct SALIX_API Color {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;

        // Constructors
        constexpr Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        constexpr Color(float red, float green, float blue, float alpha = 1.0f) 
            : r(red), g(green), b(blue), a(alpha) {}

        // Static factory methods
        static constexpr Color from_rgba_int(int red, int green, int blue, int alpha = 255) {
            return Color(
                static_cast<float>(red) / 255.0f,
                static_cast<float>(green) / 255.0f,
                static_cast<float>(blue) / 255.0f,
                static_cast<float>(alpha) / 255.0f
            );
        }

        static Color from_hsv(float h, float s, float v, float alpha = 1.0f);

        // Color operations
        static constexpr Color lerp(const Color& start, const Color& end, float t) {
            t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
            return {
                start.r * (1.0f - t) + end.r * t,
                start.g * (1.0f - t) + end.g * t,
                start.b * (1.0f - t) + end.b * t,
                start.a * (1.0f - t) + end.a * t
            };
        }

        ImVec4 to_imvec4() const;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive) {
            archive(cereal::make_nvp("r", r), cereal::make_nvp("g", g),
                   cereal::make_nvp("b", b), cereal::make_nvp("a", a));
        }

        // In Salix/math/Color.h, inside the struct
        bool operator==(const Color& other) const {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }
        bool operator!=(const Color& other) const {
            return !(*this == other);
        }
    private:
        friend class cereal::access;
    };

    // --- OSTREASM HELPER FUNCTION ---
    inline std::ostream& operator<<(std::ostream& os, const Color& c) {
        os << "{ r: " << c.r << ", g: " << c.g << ", b: " << c.b << ", a: " << c.a << " }";
        return os;
    }


    // Predefined colors
    inline constexpr Color White   = Color(1.0f, 1.0f, 1.0f);
    inline constexpr Color Black   = Color(0.0f, 0.0f, 0.0f);
    inline constexpr Color Red     = Color(1.0f, 0.0f, 0.0f);
    inline constexpr Color Green   = Color(0.0f, 1.0f, 0.0f);
    inline constexpr Color Blue    = Color(0.0f, 0.0f, 1.0f);
    inline constexpr Color Yellow  = Color(1.0f, 1.0f, 0.0f);
    inline constexpr Color Cyan    = Color(0.0f, 1.0f, 1.0f);
    inline constexpr Color Magenta = Color(1.0f, 0.0f, 1.0f);

    // Operators
    constexpr SALIX_API Color operator+(const Color& a, const Color& b) {
        return {a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a};
    }

    constexpr SALIX_API Color operator-(const Color& a, const Color& b) {
        return {a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a};
    }

    constexpr SALIX_API Color operator*(const Color& a, float scalar) {
        return {a.r * scalar, a.g * scalar, a.b * scalar, a.a * scalar};
    }

    constexpr SALIX_API Color operator*(float scalar, const Color& a) {
        return a * scalar;
    }

    // Non-constexpr operators (YAML)
    SALIX_API YAML::Emitter& operator<<(YAML::Emitter& out, const Color& c);
    SALIX_API void operator>>(const YAML::Node& node, Color& c);
}