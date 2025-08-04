// Salix/math/MathUtils.cpp
#include <Salix/math/MathUtils.h>
#include <algorithm> // For std::clamp, std::min, std::max
#include <cmath>     // For std::fmod

namespace Salix {
    // Core constants definitions
    const float PI           = 3.14159265358979323846f;
    const float PI_2         = 1.57079632679489661923f;
    const float PI_4         = 0.785398163397448309616f;
    const float TAU          = 6.28318530717958647692f;
    const float E            = 2.71828182845904523536f;
    const float SQRT2        = 1.41421356237309504880f;
    const float SQRT3        = 1.73205080756887729352f;
    const float GOLDEN_RATIO = 1.61803398874989484820f;

    // Conversion factors
    const float DEG2RAD      = PI / 180.0f;
    const float RAD2DEG      = 180.0f / PI;
    
    // Tiny epsilon values
    const float EPSILON      = 1.0e-6f;
    const float EPSILON_SQRT = 1.0e-12f;
    const float EPSILON_LERP = 1.0e-5f;

    // Function implementations...
    template<typename T>
    T lerp(const T& a, const T& b, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    template<>
    float lerp<float>(const float& a, const float& b, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return std::fma(t, (b - a), a);
    }

    // Inverse linear interpolation
    float inverse_lerp(float a, float b, float value) noexcept {
        return (value - a) / (b - a);
    }

    // Remap a value from one range to another
    float remap(float value, float in_min, float in_max, 
               float out_min, float out_max) noexcept {
        float t = inverse_lerp(in_min, in_max, value);
        return out_min + (out_max - out_min) * t;
    }

    // Smoothstep (improved version with correct derivatives at edges)
    float smooth_step(float edge0, float edge1, float x) noexcept {
        x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    // Smootherstep (even smoother alternative)
    float smoother_step(float edge0, float edge1, float x) noexcept {
        x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
    }

    // Ping-pong a value between 0 and length
    float ping_pong(float value, float length) noexcept {
        float mod = std::fmod(value, length * 2.0f);
        return length - std::abs(mod - length);
    }

    // Cubic interpolation (Catmull-Rom spline)
    float cubic_interpolate(float y0, float y1, float y2, float y3, 
                          float t) noexcept {
        float t2 = t * t;
        float t3 = t2 * t;
        return 0.5f * ((2.0f * y1) + 
                      (-y0 + y2) * t +
                      (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3) * t2 +
                      (-y0 + 3.0f * y1 - 3.0f * y2 + y3) * t3);
    }

    // Wrapping/clamping utilities
    float wrap(float value, float min, float max) noexcept {
        float range = max - min;
        return min + std::fmod(range + std::fmod(value - min, range), range);
    }

    float saturate(float value) noexcept {
        return std::clamp(value, 0.0f, 1.0f);
    }

    // Compare floats with epsilon
    bool nearly_equal(float a, float b, float epsilon) noexcept {
        return std::abs(a - b) <= epsilon;
    }

    // Fast approximate inverse square root (Quake-style)
    float inv_sqrt(float x) noexcept {
        float xhalf = 0.5f * x;
        int i = *(int*)&x;            // Evil floating point bit hack
        i = 0x5f3759df - (i >> 1);    // What the fuck? HAHAHA
        x = *(float*)&i;
        return x * (1.5f - (xhalf * x * x)); // 1-2 Newton iterations
    }
   
} // namespace Salix
