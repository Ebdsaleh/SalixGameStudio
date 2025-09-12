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