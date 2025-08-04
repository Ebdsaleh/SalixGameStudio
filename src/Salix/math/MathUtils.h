// Salix/math/MathUtils.h
#pragma once
#include <Salix/core/Core.h>
#include <algorithm> // For std::clamp, std::min, std::max
#include <cmath>     // For std::fmod

namespace Salix {

    // Core constants - must be extern constexpr for DLL export
    SALIX_API extern const float PI;
    SALIX_API extern const float PI_2;         // π/2
    SALIX_API extern const float PI_4;         // π/4
    SALIX_API extern const float TAU;          // 2π
    SALIX_API extern const float E;            // Euler's number
    SALIX_API extern const float SQRT2;        // √2
    SALIX_API extern const float SQRT3;        // √3
    SALIX_API extern const float GOLDEN_RATIO; // (1+√5)/2

    // Conversion factors
    SALIX_API extern const float DEG2RAD;
    SALIX_API extern const float RAD2DEG;
    
    // Tiny epsilon values
    SALIX_API extern const float EPSILON;      // General purpose
    SALIX_API extern const float EPSILON_SQRT; // For squared magnitude comparisons
    SALIX_API extern const float EPSILON_LERP; // For interpolation thresholds

    // Linear interpolation (generic and float specialized)
    template<typename T>
    SALIX_API T lerp(const T& a, const T& b, float t) noexcept;

    template<>
    SALIX_API float lerp<float>(const float& a, const float& b, float t) noexcept;

    // Inverse linear interpolation
    SALIX_API float inverse_lerp(float a, float b, float value) noexcept;

    // Remap a value from one range to another
    SALIX_API float remap(float value, 
                        float in_min, float in_max, 
                        float out_min, float out_max) noexcept;

    // Smoothstep (improved version with correct derivatives at edges)
    SALIX_API float smooth_step(float edge0, float edge1, float x) noexcept;

    // Smootherstep (even smoother alternative)
    SALIX_API float smoother_step(float edge0, float edge1, float x) noexcept;

    // Ping-pong a value between 0 and length
    SALIX_API float ping_pong(float value, float length) noexcept;

    // Cubic interpolation (Catmull-Rom spline)
    SALIX_API float cubic_interpolate(float y0, float y1, 
                                   float y2, float y3, 
                                   float t) noexcept;

    // Wrapping/clamping utilities
    SALIX_API float wrap(float value, float min, float max) noexcept;

    SALIX_API float saturate(float value) noexcept;

    // Compare floats with epsilon
    SALIX_API bool nearly_equal(float a, float b, float epsilon = EPSILON) noexcept;

    // Fast approximate inverse square root (Quake-style)
    SALIX_API float inv_sqrt(float x) noexcept;

} // namespace Salix