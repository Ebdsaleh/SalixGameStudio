// Color.h
#pragma once
#include <algorithm> // For std::min and std::max


struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;


    // Default constructor for white.
    Color() = default;

    // Constructor to easily create a color.
    Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {}

    // --- Static Factory Methods ---
    // A static method to create a Color object from 0-255 integer values
    static Color from_rbga_int(int red, int green, int blue, int alpha = 255){
        return Color(
            static_cast<float>(red) / 255.0f,
            static_cast<float>(green) / 255.0f,
            static_cast<float>(blue) / 255.0f,
            static_cast<float>(alpha) / 255.0f
        );
    }

    // A static method to create a Color object from HSV values.
    // h (hue) is 0-360, s (saturation) and v (value) are 0-1.
    static Color from_hsv(float h, float s, float v, float alpha = 1.0f) {
        if (s <= 0.0f) {  // If saturation is 0, it's a shade of grey
            return Color(v, v, v, alpha);
        }

        h = fmod(h, 360.0f);  // Wrap hue around.
        if (h < 0.0f) h += 360.0f;
        h/= 60.0f;  // Convert hue to a 0-6 range.

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


};