// Salix/math/Color.cpp
#include <Salix/math/Color.h>
#include <cmath>
#include <iostream> // For std::cerr, though not used in this version
#include <algorithm> // For std::min/max

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

    // Required for the lerp method.
    Color operator*(float scalar, const Color& a) {
        return a * scalar; // Just reuse the other operator
    }
} // namespace Salix