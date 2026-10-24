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


TEST_SUITE("Salix::gui::IconInfoBatch2047") {
    TEST_CASE("copy assignment transfers every descriptor field") {
        Salix::IconInfo source; source.texture_id = static_cast<ImTextureID>(123); source.path = "icons/source.png";
        source.size = ImVec2(21.0f, 34.0f); source.uv0 = ImVec2(0.2f, 0.3f); source.uv1 = ImVec2(0.7f, 0.9f);
        Salix::IconInfo destination; destination = source;
        CHECK(destination.texture_id == source.texture_id); CHECK(destination.path == source.path);
        CHECK(destination.size.x == doctest::Approx(21.0f)); CHECK(destination.size.y == doctest::Approx(34.0f));
        CHECK(destination.uv0.x == doctest::Approx(0.2f)); CHECK(destination.uv1.y == doctest::Approx(0.9f));
    }
    TEST_CASE("self assignment preserves complete icon descriptor") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(17); icon.path = "icons/self.png"; icon.size = ImVec2(3.0f, 4.0f);
        icon = icon; CHECK(icon.texture_id == static_cast<ImTextureID>(17)); CHECK(icon.path == "icons/self.png");
        CHECK(icon.size.x == doctest::Approx(3.0f)); CHECK(icon.size.y == doctest::Approx(4.0f));
    }
    TEST_CASE("icon path preserves embedded null bytes as owned string data") {
        Salix::IconInfo icon; icon.path = std::string("icon\0sheet.png", 14); CHECK(icon.path.size() == 14); CHECK(icon.path[4] == '\0');
    }
    TEST_CASE("icon path preserves UTF8 bytes without changing texture identity") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(88); icon.path = u8"icons/世界/icon.png";
        CHECK(icon.path == u8"icons/世界/icon.png"); CHECK(icon.texture_id == static_cast<ImTextureID>(88));
    }
    TEST_CASE("negative render dimensions remain representable without clamping") {
        Salix::IconInfo icon; icon.size = ImVec2(-16.0f, -32.0f);
        CHECK(icon.size.x == doctest::Approx(-16.0f)); CHECK(icon.size.y == doctest::Approx(-32.0f));
    }
    TEST_CASE("UV coordinates outside normalized range remain representable") {
        Salix::IconInfo icon; icon.uv0 = ImVec2(-0.5f, 1.5f); icon.uv1 = ImVec2(2.0f, -1.0f);
        CHECK(icon.uv0.x == doctest::Approx(-0.5f)); CHECK(icon.uv0.y == doctest::Approx(1.5f));
        CHECK(icon.uv1.x == doctest::Approx(2.0f)); CHECK(icon.uv1.y == doctest::Approx(-1.0f));
    }
    TEST_CASE("mutating lower UV bound does not implicitly alter upper bound") {
        Salix::IconInfo icon; icon.uv0 = ImVec2(0.4f, 0.6f);
        CHECK(icon.uv1.x == doctest::Approx(1.0f)); CHECK(icon.uv1.y == doctest::Approx(1.0f));
    }
    TEST_CASE("texture identity survives independent path size and UV edits") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(999); icon.path = "changed.png";
        icon.size = ImVec2(100.0f, 50.0f); icon.uv0 = ImVec2(0.1f, 0.2f); icon.uv1 = ImVec2(0.3f, 0.4f);
        CHECK(icon.texture_id == static_cast<ImTextureID>(999));
    }
    TEST_CASE("later source mutations do not affect an already copy assigned descriptor") {
        Salix::IconInfo source; source.path = "before.png"; source.size = ImVec2(10.0f, 20.0f);
        Salix::IconInfo destination; destination = source; source.path = "after.png"; source.size.x = 99.0f;
        CHECK(destination.path == "before.png"); CHECK(destination.size.x == doctest::Approx(10.0f)); CHECK(destination.size.y == doctest::Approx(20.0f));
    }
    TEST_CASE("zero texture handle may coexist with nondefault presentation metadata") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(0); icon.path = "metadata-only.png";
        icon.size = ImVec2(48.0f, 24.0f); icon.uv0 = ImVec2(0.25f, 0.25f);
        CHECK(icon.texture_id == static_cast<ImTextureID>(0)); CHECK(icon.path == "metadata-only.png");
        CHECK(icon.size.x == doctest::Approx(48.0f)); CHECK(icon.uv0.x == doctest::Approx(0.25f));
    }
}
