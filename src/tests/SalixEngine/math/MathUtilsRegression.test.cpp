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
