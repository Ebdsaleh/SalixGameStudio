#include <doctest.h>
#include <Salix/math/Color.h>

namespace {
void check_color1747(const Salix::Color& value, float r, float g, float b, float a = 1.0f) {
    CHECK(value.r == doctest::Approx(r));
    CHECK(value.g == doctest::Approx(g));
    CHECK(value.b == doctest::Approx(b));
    CHECK(value.a == doctest::Approx(a));
}
}

TEST_SUITE("Salix::math::ColorClosure1747") {
    TEST_CASE("batch1747 color HSV sector zero interpolates red toward yellow") {
        check_color1747(Salix::Color::from_hsv(30.0f, 1.0f, 1.0f), 1.0f, 0.5f, 0.0f);
    }

    TEST_CASE("batch1747 color HSV sector one interpolates yellow toward green") {
        check_color1747(Salix::Color::from_hsv(90.0f, 1.0f, 1.0f), 0.5f, 1.0f, 0.0f);
    }

    TEST_CASE("batch1747 color HSV sector two interpolates green toward cyan") {
        check_color1747(Salix::Color::from_hsv(150.0f, 1.0f, 1.0f), 0.0f, 1.0f, 0.5f);
    }

    TEST_CASE("batch1747 color HSV sector three interpolates cyan toward blue") {
        check_color1747(Salix::Color::from_hsv(210.0f, 1.0f, 1.0f), 0.0f, 0.5f, 1.0f);
    }

    TEST_CASE("batch1747 color HSV sector four interpolates blue toward magenta") {
        check_color1747(Salix::Color::from_hsv(270.0f, 1.0f, 1.0f), 0.5f, 0.0f, 1.0f);
    }

    TEST_CASE("batch1747 color HSV sector five interpolates magenta toward red") {
        check_color1747(Salix::Color::from_hsv(330.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.5f);
    }

    TEST_CASE("batch1747 color HSV negative hue wraps into equivalent positive sector") {
        const auto negative = Salix::Color::from_hsv(-30.0f, 1.0f, 1.0f, 0.25f);
        const auto wrapped = Salix::Color::from_hsv(330.0f, 1.0f, 1.0f, 0.25f);
        CHECK(negative == wrapped);
    }

    TEST_CASE("batch1747 color HSV multiple full turns reduce to equivalent hue") {
        const auto long_hue = Salix::Color::from_hsv(750.0f, 1.0f, 1.0f);
        const auto base_hue = Salix::Color::from_hsv(30.0f, 1.0f, 1.0f);
        CHECK(long_hue.r == doctest::Approx(base_hue.r));
        CHECK(long_hue.g == doctest::Approx(base_hue.g));
        CHECK(long_hue.b == doctest::Approx(base_hue.b));
    }

    TEST_CASE("batch1747 color HSV nonpositive saturation returns grayscale and preserves alpha") {
        check_color1747(Salix::Color::from_hsv(123.0f, 0.0f, 0.375f, 0.625f),
                        0.375f, 0.375f, 0.375f, 0.625f);
        check_color1747(Salix::Color::from_hsv(123.0f, -1.0f, 0.375f, 0.625f),
                        0.375f, 0.375f, 0.375f, 0.625f);
    }

    TEST_CASE("batch1747 color integer factory intentionally does not clamp out of byte range inputs") {
        const auto color = Salix::Color::from_rgba_int(510, -255, 0, 255);
        check_color1747(color, 2.0f, -1.0f, 0.0f, 1.0f);
    }
}
