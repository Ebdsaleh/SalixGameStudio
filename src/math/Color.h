// Color.h
#pragma once
#include <algorithm> // For std::min and std::max


struct Color {
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

    // --- NEW: STATIC PRE-DEFINED COLORS ---
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;


};

// --- OPERATOR OVERLOADS for color math ---
Color operator+(const Color& a, const Color& b);

Color operator-(const Color& a, const Color& b); 

Color operator*(const Color& a, float scalar); 

Color operator*(float scalar, const Color& a);  // required for lerp method.