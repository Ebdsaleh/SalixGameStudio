#include <doctest.h>
#include <Salix/gui/IconInfo.h>
#include <string>

TEST_SUITE("Salix::gui::IconInfoBatch1247") {
    TEST_CASE("icon info defaults to no texture and empty path") {
        Salix::IconInfo icon;
        CHECK(icon.texture_id == static_cast<ImTextureID>(0));
        CHECK(icon.path.empty());
    }

    TEST_CASE("icon info defaults to sixteen pixel render size") {
        Salix::IconInfo icon;
        CHECK(icon.size.x == doctest::Approx(16.0f));
        CHECK(icon.size.y == doctest::Approx(16.0f));
    }

    TEST_CASE("icon info defaults to full texture UV range") {
        Salix::IconInfo icon;
        CHECK(icon.uv0.x == doctest::Approx(0.0f));
        CHECK(icon.uv0.y == doctest::Approx(0.0f));
        CHECK(icon.uv1.x == doctest::Approx(1.0f));
        CHECK(icon.uv1.y == doctest::Approx(1.0f));
    }

    TEST_CASE("icon info texture assignment preserves all presentation fields") {
        Salix::IconInfo icon;
        icon.path = "icons/player.png";
        icon.texture_id = static_cast<ImTextureID>(42);
        CHECK(icon.texture_id == static_cast<ImTextureID>(42));
        CHECK(icon.path == "icons/player.png");
        CHECK(icon.size.x == doctest::Approx(16.0f));
        CHECK(icon.uv1.x == doctest::Approx(1.0f));
    }

    TEST_CASE("icon info accepts independent non-square render dimensions") {
        Salix::IconInfo icon;
        icon.size = ImVec2(32.0f, 18.0f);
        CHECK(icon.size.x == doctest::Approx(32.0f));
        CHECK(icon.size.y == doctest::Approx(18.0f));
        CHECK(icon.uv0.x == doctest::Approx(0.0f));
    }

    TEST_CASE("icon info sprite sheet UV coordinates preserve arbitrary subregion") {
        Salix::IconInfo icon;
        icon.uv0 = ImVec2(0.25f, 0.5f);
        icon.uv1 = ImVec2(0.75f, 0.875f);
        CHECK(icon.uv0.x == doctest::Approx(0.25f));
        CHECK(icon.uv0.y == doctest::Approx(0.5f));
        CHECK(icon.uv1.x == doctest::Approx(0.75f));
        CHECK(icon.uv1.y == doctest::Approx(0.875f));
    }

    TEST_CASE("icon info does not clamp reversed UV coordinates") {
        Salix::IconInfo icon;
        icon.uv0 = ImVec2(1.0f, 1.0f);
        icon.uv1 = ImVec2(0.0f, 0.0f);
        CHECK(icon.uv0.x == doctest::Approx(1.0f));
        CHECK(icon.uv1.x == doctest::Approx(0.0f));
    }

    TEST_CASE("icon info copy construction preserves the complete descriptor") {
        Salix::IconInfo source;
        source.texture_id = static_cast<ImTextureID>(99);
        source.path = "icons/copied.png";
        source.size = ImVec2(24.0f, 30.0f);
        source.uv0 = ImVec2(0.1f, 0.2f);
        source.uv1 = ImVec2(0.8f, 0.9f);
        Salix::IconInfo copy(source);
        CHECK(copy.texture_id == source.texture_id);
        CHECK(copy.path == source.path);
        CHECK(copy.size.x == doctest::Approx(source.size.x));
        CHECK(copy.uv0.y == doctest::Approx(source.uv0.y));
        CHECK(copy.uv1.x == doctest::Approx(source.uv1.x));
    }

    TEST_CASE("icon info copied string and geometry state remain independent") {
        Salix::IconInfo source;
        source.path = "icons/original.png";
        source.size = ImVec2(16.0f, 16.0f);
        Salix::IconInfo copy = source;
        copy.path = "icons/copy.png";
        copy.size.x = 64.0f;
        CHECK(source.path == "icons/original.png");
        CHECK(source.size.x == doctest::Approx(16.0f));
        CHECK(copy.path == "icons/copy.png");
        CHECK(copy.size.x == doctest::Approx(64.0f));
    }

    TEST_CASE("icon info zero render size remains a representable descriptor state") {
        Salix::IconInfo icon;
        icon.size = ImVec2(0.0f, 0.0f);
        icon.texture_id = static_cast<ImTextureID>(7);
        CHECK(icon.size.x == doctest::Approx(0.0f));
        CHECK(icon.size.y == doctest::Approx(0.0f));
        CHECK(icon.texture_id == static_cast<ImTextureID>(7));
    }
}
