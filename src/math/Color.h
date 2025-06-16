// Color.h
#pragma once

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;


    // Default constructor for white.
    Color() = default;

    // Constructor to easily create a color.
    Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {}


};