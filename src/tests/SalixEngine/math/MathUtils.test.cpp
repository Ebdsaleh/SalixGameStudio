// =================================================================================
// Filename:    src/Tests/SalixEngine/math/MathUtils.test.cpp
// Description: Contains unit tests for the MathUtils functions.
// =================================================================================

#include <doctest.h>
#include <Salix/math/MathUtils.h>

TEST_SUITE("Salix::math::MathUtils") {
    TEST_CASE("constants are defined correctly") {
        CHECK(Salix::PI == doctest::Approx(3.14159265f));
        CHECK(Salix::DEG2RAD == doctest::Approx(Salix::PI / 180.0f));
        CHECK(Salix::RAD2DEG == doctest::Approx(180.0f / Salix::PI));
    }

    TEST_CASE("lerp interpolates correctly") {
        CHECK(Salix::lerp(0.0f, 100.0f, 0.5f) == doctest::Approx(50.0f));
        CHECK(Salix::lerp(-10.0f, 10.0f, 0.75f) == doctest::Approx(5.0f));
        CHECK(Salix::lerp(0.0f, 10.0f, 0.0f) == doctest::Approx(0.0f)); // Lower bound
        CHECK(Salix::lerp(0.0f, 10.0f, 1.0f) == doctest::Approx(10.0f)); // Upper bound
        CHECK(Salix::lerp(0.0f, 10.0f, 2.0f) == doctest::Approx(10.0f)); // Clamped upper
    }

    TEST_CASE("lerp interpolates values and clamps t") {
        // Test midpoint
        CHECK(Salix::lerp(0.0f, 10.0f, 0.5f) == doctest::Approx(5.0f));
        
        // Test bounds
        CHECK(Salix::lerp(0.0f, 10.0f, 0.0f) == doctest::Approx(0.0f));
        CHECK(Salix::lerp(0.0f, 10.0f, 1.0f) == doctest::Approx(10.0f));

        // Test that t is clamped
        CHECK(Salix::lerp(0.0f, 10.0f, -1.0f) == doctest::Approx(0.0f));
        CHECK(Salix::lerp(0.0f, 10.0f, 2.0f) == doctest::Approx(10.0f));
    }

    TEST_CASE("inverse_lerp calculates t") {
        CHECK(Salix::inverse_lerp(0.0f, 100.0f, 50.0f) == doctest::Approx(0.5f));
        CHECK(Salix::inverse_lerp(10.0f, 20.0f, 10.0f) == doctest::Approx(0.0f));
    }
    
    TEST_CASE("remap converts value between ranges") {
        // Remap 5 from range [0, 10] to [0, 100] -> should be 50
        CHECK(Salix::remap(5.0f, 0.0f, 10.0f, 0.0f, 100.0f) == doctest::Approx(50.0f));
    }

    TEST_CASE("smooth_step provides smooth interpolation") {
        CHECK(Salix::smooth_step(0.0f, 1.0f, 0.0f) == doctest::Approx(0.0f));
        CHECK(Salix::smooth_step(0.0f, 1.0f, 0.5f) == doctest::Approx(0.5f));
        CHECK(Salix::smooth_step(0.0f, 1.0f, 1.0f) == doctest::Approx(1.0f));
    }

    TEST_CASE("smoother_step provides and even smoother interpolation") {
        // ARRANGE
        float edge0 = 0.0f;
        float edge1 = 1.0f;

        // ACT & ASSERT
        // At the start of the range, the result should be 0.
        CHECK(Salix::smoother_step(edge0, edge1, 0.0f) == doctest::Approx(0.0f));

        // Exactly in the middle, the result should be 0.5.
        CHECK(Salix::smoother_step(edge0, edge1, 0.5f) == doctest::Approx(0.5f));

        // At the end of the range, the result should be 1.
        CHECK(Salix::smoother_step(edge0, edge1, 1.0f) == doctest::Approx(1.0f));
    }

    TEST_CASE("ping_pong wraps values correctly") {
        CHECK(Salix::ping_pong(3.0f, 10.0f) == doctest::Approx(3.0f));
        CHECK(Salix::ping_pong(12.0f, 10.0f) == doctest::Approx(8.0f)); // Bounces back
        CHECK(Salix::ping_pong(25.0f, 10.0f) == doctest::Approx(5.0f)); // Completes one cycle and continues
    }

    TEST_CASE("wrap keeps values within range") {
        CHECK(Salix::wrap(12.0f, 0.0f, 10.0f) == doctest::Approx(2.0f)); // Wraps around
        CHECK(Salix::wrap(-2.0f, 0.0f, 10.0f) == doctest::Approx(8.0f)); // Wraps from below
    }

    TEST_CASE("saturate clamps value between 0 and 1") {
        CHECK(Salix::saturate(0.5f) == doctest::Approx(0.5f));
        CHECK(Salix::saturate(-1.0f) == doctest::Approx(0.0f));
        CHECK(Salix::saturate(1.5f) == doctest::Approx(1.0f));
    }

    TEST_CASE("nearly_equal compares floats with tolerance") {
        CHECK(Salix::nearly_equal(1.0f, 1.000001f) == true);
        CHECK(Salix::nearly_equal(1.0f, 1.001f) == false);
        CHECK(Salix::nearly_equal(1.0f, 1.01f, 0.1f) == true); // With custom epsilon
    }

    TEST_CASE("inv_sqrt calculates fast approximate inverse square root") {
        // It's an approximation, so we use a larger epsilon
        CHECK(Salix::inv_sqrt(4.0f) == doctest::Approx(0.5f).epsilon(0.01));
        CHECK(Salix::inv_sqrt(16.0f) == doctest::Approx(0.25f).epsilon(0.01));
    }

    TEST_CASE("cubic_interpolate passes through endpoints") {
        // A simple test for a Catmull-Rom spline is to ensure that
        // at t=0 it returns the second point (y1) and at t=1 it returns
        // the third point (y2).
        float p0 = 0.0f, p1 = 10.0f, p2 = 20.0f, p3 = 30.0f;
        CHECK(Salix::cubic_interpolate(p0, p1, p2, p3, 0.0f) == doctest::Approx(p1));
        CHECK(Salix::cubic_interpolate(p0, p1, p2, p3, 1.0f) == doctest::Approx(p2));
    }
}

// Additional cases preserved from the separate regression suite.
// Numeric regression tests: intervals, easing, wrapping, interpolation and tolerances.
#include <doctest.h>
#include <Salix/math/MathUtils.h>

TEST_SUITE("Salix::math::MathUtilsRegression") {
    TEST_CASE("math utils linear interpolation clamps both sides and preserves midpoint") {
        CHECK(Salix::lerp<float>(8.f, 24.f, -2.f) == doctest::Approx(8.f));
        CHECK(Salix::lerp<float>(8.f, 24.f, 2.f) == doctest::Approx(24.f));
        CHECK(Salix::lerp<float>(8.f, 24.f, 0.5f) == doctest::Approx(16.f));
    }

    TEST_CASE("math utils inverse lerp supports descending intervals") {
        CHECK(Salix::inverse_lerp(10.f, 0.f, 7.5f) == doctest::Approx(0.25f));
        CHECK(Salix::inverse_lerp(-10.f, 10.f, 0.f) == doctest::Approx(0.5f));
    }

    TEST_CASE("math utils remap handles reversed input and output ranges") {
        CHECK(Salix::remap(2.5f, 10.f, 0.f, -1.f, 1.f) == doctest::Approx(0.5f));
        CHECK(Salix::remap(5.f, 0.f, 10.f, 100.f, 0.f) == doctest::Approx(50.f));
    }

    TEST_CASE("math utils smoothstep clamps outside its domain and follows cubic easing") {
        CHECK(Salix::smooth_step(2.f, 6.f, -100.f) == doctest::Approx(0.f));
        CHECK(Salix::smooth_step(2.f, 6.f, 100.f) == doctest::Approx(1.f));
        CHECK(Salix::smooth_step(0.f, 1.f, 0.25f) == doctest::Approx(0.15625f));
    }

    TEST_CASE("math utils smootherstep has quintic easing and clamps endpoints") {
        CHECK(Salix::smoother_step(0.f, 1.f, 0.25f) == doctest::Approx(0.103515625f));
        CHECK(Salix::smoother_step(0.f, 1.f, -1.f) == doctest::Approx(0.f));
        CHECK(Salix::smoother_step(0.f, 1.f, 2.f) == doctest::Approx(1.f));
    }

    TEST_CASE("math utils ping pong reflects across cycle boundaries") {
        CHECK(Salix::ping_pong(0.f, 2.f) == doctest::Approx(0.f));
        CHECK(Salix::ping_pong(1.f, 2.f) == doctest::Approx(1.f));
        CHECK(Salix::ping_pong(2.f, 2.f) == doctest::Approx(2.f));
        CHECK(Salix::ping_pong(3.f, 2.f) == doctest::Approx(1.f));
        CHECK(Salix::ping_pong(4.f, 2.f) == doctest::Approx(0.f));
    }

    TEST_CASE("math utils wrap maps negative positions and upper boundary into half open interval") {
        CHECK(Salix::wrap(-1.f, 0.f, 360.f) == doctest::Approx(359.f));
        CHECK(Salix::wrap(360.f, 0.f, 360.f) == doctest::Approx(0.f));
        CHECK(Salix::wrap(27.f, 10.f, 20.f) == doctest::Approx(17.f));
    }

    TEST_CASE("math utils saturate clamps negative and overbright values while preserving interior") {
        CHECK(Salix::saturate(-0.2f) == doctest::Approx(0.f));
        CHECK(Salix::saturate(0.35f) == doctest::Approx(0.35f));
        CHECK(Salix::saturate(1.2f) == doctest::Approx(1.f));
    }

    TEST_CASE("math utils cubic interpolation preserves both segment endpoints") {
        CHECK(Salix::cubic_interpolate(-4.f, 1.f, 8.f, 10.f, 0.f) == doctest::Approx(1.f));
        CHECK(Salix::cubic_interpolate(-4.f, 1.f, 8.f, 10.f, 1.f) == doctest::Approx(8.f));
        CHECK(Salix::cubic_interpolate(1.f, 3.f, 5.f, 7.f, 0.5f) == doctest::Approx(4.f));
    }

    TEST_CASE("math utils float tolerances and inverse square root stay within expected error") {
        CHECK(Salix::nearly_equal(1.f, 1.0005f, 0.001f));
        CHECK_FALSE(Salix::nearly_equal(1.f, 1.01f, 0.001f));
        CHECK(Salix::inv_sqrt(4.f) == doctest::Approx(0.5f).epsilon(0.02));
        CHECK(Salix::DEG2RAD * 180.f == doctest::Approx(Salix::PI));
        CHECK(Salix::RAD2DEG * Salix::PI == doctest::Approx(180.f));
    }
}

// Interpolation algebra, periodic invariants and epsilon symmetry.
#include <initializer_list>
TEST_SUITE("Salix::math::MathUtilsExtended") {
    TEST_CASE("math invariant core angle constants satisfy a full-turn identity") {
        CHECK(Salix::TAU == doctest::Approx(2.f * Salix::PI));
        CHECK(Salix::PI_2 == doctest::Approx(Salix::PI / 2.f));
        CHECK(Salix::PI_4 == doctest::Approx(Salix::PI / 4.f));
        CHECK(Salix::DEG2RAD * Salix::RAD2DEG == doctest::Approx(1.f));
    }

    TEST_CASE("math invariant inverse lerp preserves extrapolation outside original interval") {
        CHECK(Salix::inverse_lerp(0.f, 10.f, -5.f) == doctest::Approx(-0.5f));
        CHECK(Salix::inverse_lerp(0.f, 10.f, 15.f) == doctest::Approx(1.5f));
        CHECK(Salix::inverse_lerp(10.f, 0.f, -5.f) == doctest::Approx(1.5f));
    }

    TEST_CASE("math invariant remapping between two ranges and back restores original point") {
        for (float value : { -4.f, 0.f, 2.f, 4.f, 8.f }) {
            const float mapped = Salix::remap(value, -4.f, 8.f, 30.f, -6.f);
            const float restored = Salix::remap(mapped, 30.f, -6.f, -4.f, 8.f);
            CHECK(restored == doctest::Approx(value));
        }
    }

    TEST_CASE("math invariant smoothstep is symmetric around its midpoint") {
        for (float t : { 0.1f, 0.25f, 0.5f, 0.75f, 0.9f }) {
            const float left = Salix::smooth_step(0.f, 1.f, t);
            const float right = Salix::smooth_step(0.f, 1.f, 1.f - t);
            CHECK(left + right == doctest::Approx(1.f));
        }
    }

    TEST_CASE("math invariant smootherstep is symmetric around midpoint for a shifted domain") {
        for (float t : { 0.1f, 0.3f, 0.5f, 0.7f, 0.9f }) {
            const float left = Salix::smoother_step(10.f, 20.f, 10.f + t * 10.f);
            const float right = Salix::smoother_step(10.f, 20.f, 20.f - t * 10.f);
            CHECK(left + right == doctest::Approx(1.f));
        }
    }

    TEST_CASE("math invariant both easing functions are monotone on an increasing interval") {
        float previous_smooth = -1.f, previous_smoother = -1.f;
        for (int i = 0; i <= 10; ++i) {
            const float x = static_cast<float>(i) / 10.f;
            const float smooth = Salix::smooth_step(0.f, 1.f, x);
            const float smoother = Salix::smoother_step(0.f, 1.f, x);
            CHECK(smooth >= previous_smooth);
            CHECK(smoother >= previous_smoother);
            previous_smooth = smooth;
            previous_smoother = smoother;
        }
    }

    TEST_CASE("math invariant wrapping positive integer periods preserves cyclic coordinates") {
        for (float value : { 2.f, 7.f, 11.f, 19.f }) {
            const float initial = Salix::wrap(value, 2.f, 12.f);
            CHECK(Salix::wrap(value + 30.f, 2.f, 12.f) == doctest::Approx(initial));
            CHECK(initial >= 2.f);
            CHECK(initial < 12.f);
        }
    }

    TEST_CASE("math invariant ping pong reflection repeats after two complete lengths") {
        for (float t : { 0.f, 1.f, 2.f, 4.f, 6.f }) {
            CHECK(Salix::ping_pong(t, 5.f) == doctest::Approx(Salix::ping_pong(t + 10.f, 5.f)));
            CHECK(Salix::ping_pong(t, 5.f) >= 0.f);
            CHECK(Salix::ping_pong(t, 5.f) <= 5.f);
        }
    }

    TEST_CASE("math invariant uniformly spaced Catmull Rom control points interpolate linearly") {
        CHECK(Salix::cubic_interpolate(0.f, 10.f, 20.f, 30.f, 0.25f) == doctest::Approx(12.5f));
        CHECK(Salix::cubic_interpolate(0.f, 10.f, 20.f, 30.f, 0.75f) == doctest::Approx(17.5f));
        CHECK(Salix::cubic_interpolate(-10.f, -5.f, 0.f, 5.f, 0.5f) == doctest::Approx(-2.5f));
    }

    TEST_CASE("math invariant epsilon equality remains symmetric and excludes separated samples") {
        const float a = 1.f, b = 1.002f;
        CHECK(Salix::nearly_equal(a, b, 0.005f));
        CHECK(Salix::nearly_equal(b, a, 0.005f));
        CHECK_FALSE(Salix::nearly_equal(a, b, 0.001f));
        CHECK(Salix::nearly_equal(a, a, 0.f));
        CHECK(Salix::inv_sqrt(9.f) == doctest::Approx(1.f / 3.f).epsilon(0.02));
    }
}

// Stable interpolation and interval laws used by transforms and animations.
TEST_SUITE("Salix::math::MathUtilsAdditional20261019") {
    TEST_CASE("math extended lerp clamps negative interpolation before evaluating far endpoints") {
        CHECK(Salix::lerp<float>(-100.f, 100.f, -0.01f) == doctest::Approx(-100.f));
        CHECK(Salix::lerp<float>(-100.f, 100.f, 1.01f) == doctest::Approx(100.f));
    }

    TEST_CASE("math extended lerp restores endpoints under reversed ordering") {
        for (float t : {0.f, 0.25f, 0.5f, 0.75f, 1.f}) {
            const float forward = Salix::lerp<float>(-8.f, 32.f, t);
            const float backward = Salix::lerp<float>(32.f, -8.f, 1.f - t);
            CHECK(forward == doctest::Approx(backward));
        }
    }

    TEST_CASE("math extended inverse lerp interprets the midpoint of a negative interval") {
        CHECK(Salix::inverse_lerp(-100.f, -20.f, -60.f) == doctest::Approx(0.5f));
        CHECK(Salix::inverse_lerp(-20.f, -100.f, -60.f) == doctest::Approx(0.5f));
    }

    TEST_CASE("math extended remapping with equal-width ranges is translation") {
        for (float v : {-20.f, -5.f, 0.f, 20.f}) {
            CHECK(Salix::remap(v, -20.f, 20.f, 80.f, 120.f) == doctest::Approx(v + 100.f));
        }
    }

    TEST_CASE("math extended smooth step across a non-unit range reaches exact endpoints") {
        CHECK(Salix::smooth_step(-7.f, 3.f, -7.f) == doctest::Approx(0.f));
        CHECK(Salix::smooth_step(-7.f, 3.f, 3.f) == doctest::Approx(1.f));
        CHECK(Salix::smooth_step(-7.f, 3.f, -2.f) == doctest::Approx(0.5f));
    }

    TEST_CASE("math extended smoother step clamps a negative domain before and after boundaries") {
        CHECK(Salix::smoother_step(-20.f, -10.f, -25.f) == doctest::Approx(0.f));
        CHECK(Salix::smoother_step(-20.f, -10.f, -5.f) == doctest::Approx(1.f));
        CHECK(Salix::smoother_step(-20.f, -10.f, -15.f) == doctest::Approx(0.5f));
    }

    TEST_CASE("math extended cubic interpolation preserves constant control points at many times") {
        for (float t : {0.f, 0.1f, 0.3f, 0.5f, 0.9f, 1.f}) {
            CHECK(Salix::cubic_interpolate(14.f,14.f,14.f,14.f,t) == doctest::Approx(14.f));
        }
    }

    TEST_CASE("math extended cubic interpolation is affine under translating every control point") {
        const float original = Salix::cubic_interpolate(-5.f,0.f,15.f,25.f,0.3f);
        const float shifted = Salix::cubic_interpolate(95.f,100.f,115.f,125.f,0.3f);
        CHECK(shifted == doctest::Approx(original + 100.f));
    }

    TEST_CASE("math extended wrap respects a shifted positive interval and its upper edge") {
        CHECK(Salix::wrap(15.f, 5.f, 15.f) == doctest::Approx(5.f));
        CHECK(Salix::wrap(4.f, 5.f, 15.f) == doctest::Approx(14.f));
        CHECK(Salix::wrap(45.f, 5.f, 15.f) == doctest::Approx(5.f));
    }

    TEST_CASE("math extended saturate is monotonic and does not change values in unit interval") {
        float previous = -1.f;
        for (float x : {-3.f,-1.f,0.f,0.125f,0.5f,0.875f,1.f,2.f}) {
            const float y = Salix::saturate(x);
            CHECK(y >= previous);
            CHECK(y >= 0.f);
            CHECK(y <= 1.f);
            previous = y;
        }
    }
}

// Boundary behavior pinned against the implementation and public API.
TEST_SUITE("Salix::Batch747::MathUtilsBatch747") {
    TEST_CASE("math signed half-turn converts to radians and back without losing sign") {
        const float degrees=-180.f; const float radians=degrees*Salix::DEG2RAD; CHECK(radians==doctest::Approx(-Salix::PI)); CHECK(radians*Salix::RAD2DEG==doctest::Approx(degrees));
    }

    TEST_CASE("math lerp clamps both extrapolation signs for negative targets") {
        CHECK(Salix::lerp(25.f,-75.f,-0.5f)==doctest::Approx(25.f)); CHECK(Salix::lerp(25.f,-75.f,1.5f)==doctest::Approx(-75.f)); CHECK(Salix::lerp(25.f,-75.f,0.25f)==doctest::Approx(0.f));
    }

    TEST_CASE("math inverse lerp exposes ordered positions beyond both boundaries") {
        CHECK(Salix::inverse_lerp(-20.f,20.f,-40.f)==doctest::Approx(-0.5f)); CHECK(Salix::inverse_lerp(-20.f,20.f,40.f)==doctest::Approx(1.5f));
    }

    TEST_CASE("math remap sends both source endpoints to reversed destination endpoints") {
        CHECK(Salix::remap(-2.f,-2.f,6.f,100.f,-100.f)==doctest::Approx(100.f)); CHECK(Salix::remap(6.f,-2.f,6.f,100.f,-100.f)==doctest::Approx(-100.f)); CHECK(Salix::remap(2.f,-2.f,6.f,100.f,-100.f)==doctest::Approx(0.f));
    }

    TEST_CASE("math cubic and quintic smoothstep evaluate known quarter positions") {
        CHECK(Salix::smooth_step(0.f,1.f,0.25f)==doctest::Approx(0.15625f)); CHECK(Salix::smoother_step(0.f,1.f,0.25f)==doctest::Approx(0.103515625f));
    }

    TEST_CASE("math easing returns exact zeros before an offset interval and ones after") {
        for(float x:{-12.f,-5.f,2.f}) {CHECK(Salix::smooth_step(3.f,9.f,x)==0.f);CHECK(Salix::smoother_step(3.f,9.f,x)==0.f);} for(float x:{9.f,12.f,100.f}) {CHECK(Salix::smooth_step(3.f,9.f,x)==1.f);CHECK(Salix::smoother_step(3.f,9.f,x)==1.f);}
    }

    TEST_CASE("math Catmull Rom interpolates a uniformly spaced negative segment") {
        CHECK(Salix::cubic_interpolate(-20.f,-10.f,0.f,10.f,0.25f)==doctest::Approx(-7.5f)); CHECK(Salix::cubic_interpolate(-20.f,-10.f,0.f,10.f,0.75f)==doctest::Approx(-2.5f));
    }

    TEST_CASE("math wrap period preserves fractional positions over many cycles") {
        for(float offset:{-12.f,-6.f,0.f,6.f,12.f}) CHECK(Salix::wrap(-2.25f+offset,-3.f,3.f)==doctest::Approx(-2.25f));
    }

    TEST_CASE("math ping pong samples both slopes of a noninteger cycle") {
        CHECK(Salix::ping_pong(0.75f,2.f)==doctest::Approx(0.75f));CHECK(Salix::ping_pong(2.75f,2.f)==doctest::Approx(1.25f));CHECK(Salix::ping_pong(4.75f,2.f)==doctest::Approx(0.75f));
    }

    TEST_CASE("math custom epsilon is inclusive and rejects values just outside it") {
        CHECK(Salix::nearly_equal(1.f,1.125f,0.125f));CHECK_FALSE(Salix::nearly_equal(1.f,1.25f,0.125f));CHECK(Salix::inv_sqrt(4.f)==doctest::Approx(0.5f).epsilon(0.01));
    }
}

TEST_SUITE("Salix::math::MathUtilsBatch947") {
    TEST_CASE("square-root constants reconstruct their defining integers within float precision") {
        CHECK(Salix::SQRT2 * Salix::SQRT2 == doctest::Approx(2.0f).epsilon(0.00001));
        CHECK(Salix::SQRT3 * Salix::SQRT3 == doctest::Approx(3.0f).epsilon(0.00001));
    }

    TEST_CASE("golden ratio satisfies phi squared equals phi plus one") {
        CHECK(Salix::GOLDEN_RATIO * Salix::GOLDEN_RATIO ==
              doctest::Approx(Salix::GOLDEN_RATIO + 1.0f).epsilon(0.00001));
    }

    TEST_CASE("degree and radian conversion factors are reciprocal") {
        CHECK(Salix::DEG2RAD * Salix::RAD2DEG == doctest::Approx(1.0f).epsilon(0.00001));
        CHECK(360.0f * Salix::DEG2RAD == doctest::Approx(Salix::TAU).epsilon(0.00001));
    }

    TEST_CASE("lerp of identical endpoints is invariant across clamped interpolation range") {
        for (float t : {-10.0f, -1.0f, 0.0f, 0.25f, 1.0f, 2.0f, 10.0f}) {
            CHECK(Salix::lerp(7.25f, 7.25f, t) == doctest::Approx(7.25f));
        }
    }

    TEST_CASE("inverse lerp is unchanged when interval and value receive equal translation") {
        const float base = Salix::inverse_lerp(-8.0f, 12.0f, 3.0f);
        const float shifted = Salix::inverse_lerp(92.0f, 112.0f, 103.0f);
        CHECK(base == doctest::Approx(0.55f));
        CHECK(shifted == doctest::Approx(base));
    }

    TEST_CASE("remap preserves relative position across independently shifted scaled ranges") {
        CHECK(Salix::remap(25.0f, 0.0f, 100.0f, -20.0f, 20.0f) == doctest::Approx(-10.0f));
        CHECK(Salix::remap(75.0f, 0.0f, 100.0f, 1000.0f, 2000.0f) == doctest::Approx(1750.0f));
    }

    TEST_CASE("cubic and quintic easing both pass exactly through arbitrary interval midpoint") {
        CHECK(Salix::smooth_step(-4.0f, 10.0f, 3.0f) == doctest::Approx(0.5f));
        CHECK(Salix::smoother_step(-4.0f, 10.0f, 3.0f) == doctest::Approx(0.5f));
    }

    TEST_CASE("wrap is invariant when adding or subtracting complete interval widths") {
        const float expected = Salix::wrap(2.75f, -5.0f, 7.0f);
        for (float cycles : {-120.0f, -24.0f, 0.0f, 24.0f, 120.0f}) {
            CHECK(Salix::wrap(2.75f + cycles, -5.0f, 7.0f) == doctest::Approx(expected));
        }
    }

    TEST_CASE("ping pong repeats at twice its length for positive sample positions") {
        for (float value : {0.25f, 1.0f, 2.75f, 5.5f}) {
            const float base = Salix::ping_pong(value, 3.0f);
            CHECK(Salix::ping_pong(value + 6.0f, 3.0f) == doctest::Approx(base));
            CHECK(Salix::ping_pong(value + 12.0f, 3.0f) == doctest::Approx(base));
        }
    }

    TEST_CASE("inverse square root tracks mathematical reciprocal root across several positive magnitudes") {
        for (float value : {1.0f, 4.0f, 9.0f, 16.0f, 100.0f}) {
            const float expected = 1.0f / std::sqrt(value);
            CHECK(Salix::inv_sqrt(value) == doctest::Approx(expected).epsilon(0.02));
        }
    }
}

TEST_SUITE("Salix::math::MathUtilsBatch2147") {
    TEST_CASE("batch2147 exported constants retain core mathematical relationships") {
        CHECK(Salix::TAU == doctest::Approx(Salix::PI * 2.0f));
        CHECK(Salix::PI_2 == doctest::Approx(Salix::PI * 0.5f));
        CHECK(Salix::PI_4 == doctest::Approx(Salix::PI * 0.25f));
        CHECK(Salix::DEG2RAD * Salix::RAD2DEG == doctest::Approx(1.0f));
    }
    TEST_CASE("batch2147 float lerp clamps interpolation below zero to the start") {
        CHECK(Salix::lerp<float>(10.0f, 20.0f, -5.0f) == doctest::Approx(10.0f));
    }
    TEST_CASE("batch2147 float lerp clamps interpolation above one to the end") {
        CHECK(Salix::lerp<float>(10.0f, 20.0f, 5.0f) == doctest::Approx(20.0f));
    }
    TEST_CASE("batch2147 inverse lerp supports descending input ranges") {
        CHECK(Salix::inverse_lerp(10.0f, 0.0f, 7.5f) == doctest::Approx(0.25f));
    }
    TEST_CASE("batch2147 remap supports reversing the output range") {
        CHECK(Salix::remap(2.5f, 0.0f, 10.0f, 100.0f, 0.0f) == doctest::Approx(75.0f));
    }
    TEST_CASE("batch2147 smooth step clamps outside edges and reaches midpoint") {
        CHECK(Salix::smooth_step(0.0f, 10.0f, -1.0f) == doctest::Approx(0.0f));
        CHECK(Salix::smooth_step(0.0f, 10.0f, 5.0f) == doctest::Approx(0.5f));
        CHECK(Salix::smooth_step(0.0f, 10.0f, 11.0f) == doctest::Approx(1.0f));
    }
    TEST_CASE("batch2147 smoother step clamps outside edges and reaches midpoint") {
        CHECK(Salix::smoother_step(0.0f, 8.0f, -1.0f) == doctest::Approx(0.0f));
        CHECK(Salix::smoother_step(0.0f, 8.0f, 4.0f) == doctest::Approx(0.5f));
        CHECK(Salix::smoother_step(0.0f, 8.0f, 9.0f) == doctest::Approx(1.0f));
    }
    TEST_CASE("batch2147 ping pong repeats across a complete double-length period") {
        CHECK(Salix::ping_pong(1.0f, 4.0f) == doctest::Approx(1.0f));
        CHECK(Salix::ping_pong(5.0f, 4.0f) == doctest::Approx(3.0f));
        CHECK(Salix::ping_pong(9.0f, 4.0f) == doctest::Approx(1.0f));
    }
    TEST_CASE("batch2147 cubic interpolation returns the two central samples at endpoints") {
        CHECK(Salix::cubic_interpolate(0.0f, 10.0f, 20.0f, 30.0f, 0.0f) == doctest::Approx(10.0f));
        CHECK(Salix::cubic_interpolate(0.0f, 10.0f, 20.0f, 30.0f, 1.0f) == doctest::Approx(20.0f));
    }
    TEST_CASE("batch2147 wrap saturate comparison and inverse square root retain expected contracts") {
        CHECK(Salix::wrap(-1.0f, 0.0f, 10.0f) == doctest::Approx(9.0f));
        CHECK(Salix::wrap(11.0f, 0.0f, 10.0f) == doctest::Approx(1.0f));
        CHECK(Salix::saturate(-2.0f) == 0.0f);
        CHECK(Salix::saturate(2.0f) == 1.0f);
        CHECK(Salix::nearly_equal(1.0f, 1.0000005f));
        CHECK(Salix::inv_sqrt(4.0f) == doctest::Approx(0.5f).epsilon(0.01));
    }
}

TEST_SUITE("Salix::math::MathUtilsBatch2747") {
    TEST_CASE("batch2747 lerp returns exact start at zero interpolation") {
        CHECK(Salix::lerp<float>(-3.5f,8.25f,0.0f)==doctest::Approx(-3.5f));
    }
    TEST_CASE("batch2747 lerp returns exact end at unit interpolation") {
        CHECK(Salix::lerp<float>(-3.5f,8.25f,1.0f)==doctest::Approx(8.25f));
    }
    TEST_CASE("batch2747 inverse lerp maps interval endpoints to zero and one") {
        CHECK(Salix::inverse_lerp(-4.f,6.f,-4.f)==doctest::Approx(0.f));
        CHECK(Salix::inverse_lerp(-4.f,6.f,6.f)==doctest::Approx(1.f));
    }
    TEST_CASE("batch2747 remap maps input endpoints to output endpoints") {
        CHECK(Salix::remap(-2.f,-2.f,2.f,10.f,20.f)==doctest::Approx(10.f));
        CHECK(Salix::remap(2.f,-2.f,2.f,10.f,20.f)==doctest::Approx(20.f));
    }
    TEST_CASE("batch2747 smooth step is monotonic across representative interior samples") {
        const float a=Salix::smooth_step(0.f,1.f,0.25f);
        const float b=Salix::smooth_step(0.f,1.f,0.5f);
        const float c=Salix::smooth_step(0.f,1.f,0.75f);
        CHECK(a<b); CHECK(b<c);
    }
    TEST_CASE("batch2747 smoother step is monotonic across representative interior samples") {
        const float a=Salix::smoother_step(0.f,1.f,0.25f);
        const float b=Salix::smoother_step(0.f,1.f,0.5f);
        const float c=Salix::smoother_step(0.f,1.f,0.75f);
        CHECK(a<b); CHECK(b<c);
    }
    TEST_CASE("batch2747 cubic interpolation preserves a constant sequence for any interpolation") {
        for(float t:{0.f,0.25f,0.5f,0.75f,1.f})
            CHECK(Salix::cubic_interpolate(7.f,7.f,7.f,7.f,t)==doctest::Approx(7.f));
    }
    TEST_CASE("batch2747 saturate leaves values already inside unit interval unchanged") {
        CHECK(Salix::saturate(0.25f)==doctest::Approx(0.25f));
        CHECK(Salix::saturate(0.75f)==doctest::Approx(0.75f));
    }
    TEST_CASE("batch2747 nearly equal uses inclusive custom epsilon boundary") {
        CHECK(Salix::nearly_equal(5.f,5.5f,0.5f));
        CHECK_FALSE(Salix::nearly_equal(5.f,5.5001f,0.5f));
    }
    TEST_CASE("batch2747 inverse square root remains positive for positive finite inputs") {
        for(float v:{0.25f,1.f,4.f,16.f}) CHECK(Salix::inv_sqrt(v)>0.f);
    }
}

