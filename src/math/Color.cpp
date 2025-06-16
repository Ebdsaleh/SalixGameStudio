// Color.cpp
#include "Color.h"

// Define the actual values for our static pre-defined colors.
const Color Color::White(1.0f, 1.0f, 1.0f);
const Color Color::Black(0.0f, 0.0f, 0.0f);
const Color Color::Red(1.0f, 0.0f, 0.0f);
const Color Color::Green(0.0f, 1.0f, 0.0f);
const Color Color::Blue(0.0f, 0.0f, 1.0f);
const Color Color::Yellow(1.0f, 1.0f, 0.0f);
const Color Color::Cyan(0.0f, 1.0f, 1.0f);
const Color Color::Magenta(1.0f, 0.0f, 1.0f);

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