// Salix/math/Color.h
#pragma once
#include <Salix/core/Core.h>
#include <algorithm> // For std::min and std::max
#include <cereal/access.hpp>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <yaml-cpp/yaml.h> 

namespace Salix {

    struct SALIX_API Color {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;


        // Default constructor for white.
        Color();

        // Constructor to easily create a color.
        Color(float red, float green, float blue, float alpha = 1.0f);

        // --- Static Factory Methods ---
        // A static method to create a Color object from 0-255 integer values
        static Color from_rgba_int(int red, int green, int blue, int alpha = 255);

        // A static method to create a Color object from HSV values.
        // h (hue) is 0-360, s (saturation) and v (value) are 0-1.
        static Color from_hsv(float h, float s, float v, float alpha = 1.0f);

        // Linearly interpolates between two colors.
        // t=0.0 returns 'start', t=1.0 returns 'end'.
        static Color lerp(const Color& start, const Color& end, float t); 

        ImVec4 to_imvec4();
        
        

    private:
        friend class cereal::access;

        template<class Archive>
        void serialize(Archive& archive);
        
    };
    // --- NEW: STATIC PRE-DEFINED COLORS ---
    // By making these 'inline', we can define them directly in the header.
    // This is the modern C++17 way to handle static const members in a DLL.
    inline static const Color White   = Color(1.0f, 1.0f, 1.0f); 
    inline static const Color Black   = Color(0.0f, 0.0f, 0.0f);
    inline static const Color Red     = Color(1.0f, 0.0f, 0.0f);
    inline static const Color Green   = Color(0.0f, 1.0f, 0.0f);
    inline static const Color Blue    = Color(0.0f, 0.0f, 1.0f);
    inline static const Color Yellow  = Color(1.0f, 1.0f, 0.0f);
    inline static const Color Cyan    = Color(0.0f, 1.0f, 1.0f);
    inline static const Color Magenta = Color(1.0f, 0.0f, 1.0f);

    // --- OPERATOR OVERLOADS for color math ---
    SALIX_API Color operator+(const Color& a, const Color& b);

    SALIX_API Color operator-(const Color& a, const Color& b); 

    SALIX_API Color operator*(const Color& a, float scalar); 

    SALIX_API Color operator*(float scalar, const Color& a);  // required for lerp method.

    SALIX_API YAML::Emitter& operator<<(YAML::Emitter& out, const Salix::Color& c);
    
    SALIX_API void operator>>(const YAML::Node& node, Salix::Color& c);
     
} // namespace Salix