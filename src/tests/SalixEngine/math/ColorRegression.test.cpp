// Color channel conversion, hue sectors, alpha and YAML regression coverage.
#include <doctest.h>
#include <Salix/math/Color.h>
#include <yaml-cpp/yaml.h>

TEST_SUITE("Salix::math::ColorRegression") {
    TEST_CASE("color integer RGBA normalization preserves channel endpoints") {
        const Salix::Color color = Salix::Color::from_rgba_int(255, 0, 128, 255);
        CHECK(color.r == 1.f);
        CHECK(color.g == 0.f);
        CHECK(color.b == doctest::Approx(128.f / 255.f));
        CHECK(color.a == 1.f);
    }

    TEST_CASE("color integer RGBA default alpha is opaque and partial alpha scales") {
        const auto opaque = Salix::Color::from_rgba_int(10, 20, 30);
        const auto translucent = Salix::Color::from_rgba_int(10, 20, 30, 64);
        CHECK(opaque.a == 1.f);
        CHECK(translucent.a == doctest::Approx(64.f / 255.f));
        CHECK(translucent.r == opaque.r);
    }

    TEST_CASE("color zero saturation creates grayscale independent of hue") {
        const auto gray1 = Salix::Color::from_hsv(10.f, 0.f, 0.35f, 0.4f);
        const auto gray2 = Salix::Color::from_hsv(271.f, 0.f, 0.35f, 0.4f);
        CHECK(gray1 == gray2);
        CHECK(gray1.r == doctest::Approx(0.35f));
        CHECK(gray1.g == doctest::Approx(0.35f));
        CHECK(gray1.b == doctest::Approx(0.35f));
        CHECK(gray1.a == doctest::Approx(0.4f));
    }

    TEST_CASE("color HSV primary red green and blue retain channel order") {
        CHECK(Salix::Color::from_hsv(0.f, 1.f, 1.f) == Salix::Red);
        CHECK(Salix::Color::from_hsv(120.f, 1.f, 1.f) == Salix::Green);
        CHECK(Salix::Color::from_hsv(240.f, 1.f, 1.f) == Salix::Blue);
    }

    TEST_CASE("color HSV secondary colors cover all hue sectors") {
        CHECK(Salix::Color::from_hsv(60.f, 1.f, 1.f) == Salix::Yellow);
        CHECK(Salix::Color::from_hsv(180.f, 1.f, 1.f) == Salix::Cyan);
        CHECK(Salix::Color::from_hsv(300.f, 1.f, 1.f) == Salix::Magenta);
    }

    TEST_CASE("color negative hue wraps correctly while preserving alpha") {
        const auto wrapped = Salix::Color::from_hsv(-60.f, 1.f, 1.f, 0.25f);
        CHECK(wrapped.r == doctest::Approx(1.f));
        CHECK(wrapped.g == doctest::Approx(0.f));
        CHECK(wrapped.b == doctest::Approx(1.f));
        CHECK(wrapped.a == doctest::Approx(0.25f));
    }

    TEST_CASE("color multi revolution positive hue matches equivalent hue") {
        const auto reference = Salix::Color::from_hsv(30.f, 0.5f, 0.8f);
        const auto wrapped = Salix::Color::from_hsv(750.f, 0.5f, 0.8f);
        CHECK(wrapped.r == doctest::Approx(reference.r));
        CHECK(wrapped.g == doctest::Approx(reference.g));
        CHECK(wrapped.b == doctest::Approx(reference.b));
    }

    TEST_CASE("color color interpolation clamps outside both endpoints") {
        const Salix::Color a{0.1f, 0.2f, 0.3f, 0.4f};
        const Salix::Color b{0.5f, 0.6f, 0.7f, 0.8f};
        CHECK(Salix::Color::lerp(a, b, -1.f) == a);
        CHECK(Salix::Color::lerp(a, b, 2.f) == b);
    }

    TEST_CASE("color color interpolation treats opacity as its own fourth channel") {
        const Salix::Color a{1.f, 0.f, 0.f, 0.f};
        const Salix::Color b{0.f, 0.f, 1.f, 1.f};
        const auto halfway = Salix::Color::lerp(a, b, 0.5f);
        CHECK(halfway.r == doctest::Approx(0.5f));
        CHECK(halfway.g == 0.f);
        CHECK(halfway.b == doctest::Approx(0.5f));
        CHECK(halfway.a == doctest::Approx(0.5f));
    }

    TEST_CASE("color YAML color representation restores every channel and defaults omissions") {
        const Salix::Color original{0.2f, 0.4f, 0.6f, 0.8f};
        YAML::Emitter out;
        out << original;
        Salix::Color restored;
        YAML::Node node = YAML::Load(out.c_str());
        node >> restored;
        CHECK(restored.r == doctest::Approx(original.r));
        CHECK(restored.g == doctest::Approx(original.g));
        CHECK(restored.b == doctest::Approx(original.b));
        CHECK(restored.a == doctest::Approx(original.a));
        YAML::Node partial = YAML::Load("{r: 1, b: 0.25}");
        partial >> restored;
        CHECK(restored.g == 0.f);
        CHECK(restored.a == 0.f);
    }
}
