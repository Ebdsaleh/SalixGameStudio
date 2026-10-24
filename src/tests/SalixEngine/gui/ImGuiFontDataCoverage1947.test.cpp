#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <imgui/imgui.h>
#include <cfloat>

TEST_SUITE("Salix::gui::ImGuiFontDataCoverage1947") {
    TEST_CASE("default config conversion preserves boolean sampling defaults") {
        Salix::ImGuiFontData data;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK_FALSE(config.MergeMode);
        CHECK_FALSE(config.PixelSnapH);
        CHECK(config.PixelSnapV);
    }

    TEST_CASE("config conversion preserves font index oversampling and pixel size") {
        Salix::ImGuiFontData data;
        data.config.font_no = 3;
        data.config.oversample_h = 4;
        data.config.oversample_v = 2;
        data.config.size_pixels = 31.5f;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.FontNo == 3);
        CHECK(config.OversampleH == 4);
        CHECK(config.OversampleV == 2);
        CHECK(config.SizePixels == doctest::Approx(31.5f));
    }

    TEST_CASE("config conversion preserves glyph range pointers") {
        static const ImWchar ranges[] = {0x20, 0x7E, 0};
        static const ImWchar excluded[] = {0x30, 0x39, 0};
        Salix::ImGuiFontData data;
        data.config.glyph_ranges = ranges;
        data.config.glyph_exclude_ranges = excluded;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.GlyphRanges == ranges);
        CHECK(config.GlyphExcludeRanges == excluded);
    }

    TEST_CASE("config conversion preserves glyph offset coordinates") {
        Salix::ImGuiFontData data;
        data.config.glyph_offset = ImVec2(-3.25f, 7.5f);
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.GlyphOffset.x == doctest::Approx(-3.25f));
        CHECK(config.GlyphOffset.y == doctest::Approx(7.5f));
    }

    TEST_CASE("config conversion preserves advance constraints and extra advance") {
        Salix::ImGuiFontData data;
        data.config.glyph_min_advance_x = 2.0f;
        data.config.glyph_max_advance_x = 20.0f;
        data.config.glyph_extra_advance_x = 1.25f;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.GlyphMinAdvanceX == doctest::Approx(2.0f));
        CHECK(config.GlyphMaxAdvanceX == doctest::Approx(20.0f));
        CHECK(config.GlyphExtraAdvanceX == doctest::Approx(1.25f));
    }

    TEST_CASE("config conversion preserves loader flags and rasterizer settings") {
        Salix::ImGuiFontData data;
        data.config.font_loader_flags = 0x55AAu;
        data.config.rasterizer_multiply = 1.75f;
        data.config.rasterizer_density = 2.25f;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.FontLoaderFlags == 0x55AAu);
        CHECK(config.RasterizerMultiply == doctest::Approx(1.75f));
        CHECK(config.RasterizerDensity == doctest::Approx(2.25f));
    }

    TEST_CASE("config conversion preserves explicit ellipsis codepoint") {
        Salix::ImGuiFontData data;
        data.config.ellipsis_char = static_cast<ImWchar>(0x2026);
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.EllipsisChar == static_cast<ImWchar>(0x2026));
    }

    TEST_CASE("returned ImFontConfig is an independent value snapshot") {
        Salix::ImGuiFontData data;
        data.config.merge_mode = true;
        data.config.size_pixels = 18.0f;
        ImFontConfig config = data.config.to_imgui_font_config();
        config.MergeMode = false;
        config.SizePixels = 99.0f;
        CHECK(data.config.merge_mode);
        CHECK(data.config.size_pixels == doctest::Approx(18.0f));
    }

    TEST_CASE("custom font constructor size appears in converted config") {
        Salix::ImGuiFontData data("Custom", "custom.ttf", 27.0f);
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.SizePixels == doctest::Approx(27.0f));
        CHECK(data.get_font_size() == doctest::Approx(27.0f));
    }

    TEST_CASE("all mutable config channels can be combined in one converted snapshot") {
        Salix::ImGuiFontData data;
        data.config.merge_mode = true;
        data.config.pixel_snap_h = true;
        data.config.pixel_snap_v = false;
        data.config.size_pixels = 42.0f;
        data.config.glyph_min_advance_x = 4.0f;
        data.config.glyph_max_advance_x = FLT_MAX;
        data.config.rasterizer_multiply = 0.75f;
        ImFontConfig config = data.config.to_imgui_font_config();
        CHECK(config.MergeMode);
        CHECK(config.PixelSnapH);
        CHECK_FALSE(config.PixelSnapV);
        CHECK(config.SizePixels == doctest::Approx(42.0f));
        CHECK(config.GlyphMinAdvanceX == doctest::Approx(4.0f));
        CHECK(config.GlyphMaxAdvanceX == doctest::Approx(FLT_MAX));
        CHECK(config.RasterizerMultiply == doctest::Approx(0.75f));
    }
}
