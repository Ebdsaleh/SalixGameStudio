#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFontData.h>
#include <cfloat>
#include <imgui/imgui.h>

TEST_SUITE("Salix::gui::ImGuiFontDataBatch1047") {
    TEST_CASE("default font data exposes stable identity and size defaults") {
        Salix::ImGuiFontData data;
        CHECK(data.get_name() == "Default Font");
        CHECK(data.get_file_path().empty());
        CHECK(data.get_font_size() == doctest::Approx(20.0f));
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("default config mirrors expected ImGui rasterization defaults") {
        Salix::ImGuiFontData data;
        CHECK_FALSE(data.config.merge_mode);
        CHECK_FALSE(data.config.pixel_snap_h);
        CHECK(data.config.pixel_snap_v);
        CHECK(data.config.font_no == 0);
        CHECK(data.config.oversample_h == 2);
        CHECK(data.config.oversample_v == 1);
        CHECK(data.config.size_pixels == doctest::Approx(20.0f));
        CHECK(data.config.glyph_ranges == nullptr);
        CHECK(data.config.glyph_exclude_ranges == nullptr);
        CHECK(data.config.glyph_max_advance_x == doctest::Approx(FLT_MAX));
    }

    TEST_CASE("custom constructor preserves name path size and GUI type") {
        Salix::ImGuiFontData data("Editor Mono", "Assets/Fonts/Editor Mono.ttf", 17.5f);
        CHECK(data.get_name() == "Editor Mono");
        CHECK(data.get_file_path() == "Assets/Fonts/Editor Mono.ttf");
        CHECK(data.get_font_size() == doctest::Approx(17.5f));
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("custom constructor synchronizes initial config size with requested font size") {
        Salix::ImGuiFontData small("Small", "small.ttf", 8.0f);
        Salix::ImGuiFontData large("Large", "large.ttf", 64.0f);
        CHECK(small.config.size_pixels == doctest::Approx(8.0f));
        CHECK(large.config.size_pixels == doctest::Approx(64.0f));
    }

    TEST_CASE("font size setter updates the public font-size contract for zero negative and positive values") {
        Salix::ImGuiFontData data;
        data.set_font_size(0.0f);
        CHECK(data.get_font_size() == doctest::Approx(0.0f));
        data.set_font_size(-12.5f);
        CHECK(data.get_font_size() == doctest::Approx(-12.5f));
        data.set_font_size(33.25f);
        CHECK(data.get_font_size() == doctest::Approx(33.25f));
    }

    TEST_CASE("IFontData interface dispatches all ImGui font metadata virtually") {
        Salix::ImGuiFontData data("Virtual", "virtual.ttf", 14.0f);
        Salix::IFontData* base = &data;
        CHECK(base->get_name() == "Virtual");
        CHECK(base->get_file_path() == "virtual.ttf");
        CHECK(base->get_font_size() == doctest::Approx(14.0f));
        CHECK(base->get_gui_type() == "ImGui");
        base->set_font_size(18.0f);
        CHECK(data.get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("font instance config stores boolean sampling and size settings independently of metadata") {
        Salix::ImGuiFontData data("Configured", "configured.ttf", 16.0f);
        data.config.merge_mode = true;
        data.config.pixel_snap_h = true;
        data.config.pixel_snap_v = false;
        data.config.font_no = 3;
        data.config.oversample_h = 4;
        data.config.oversample_v = 2;
        data.config.size_pixels = 27.0f;

        CHECK(data.config.merge_mode);
        CHECK(data.config.pixel_snap_h);
        CHECK_FALSE(data.config.pixel_snap_v);
        CHECK(data.config.font_no == 3);
        CHECK(data.config.oversample_h == 4);
        CHECK(data.config.oversample_v == 2);
        CHECK(data.config.size_pixels == doctest::Approx(27.0f));
        CHECK(data.get_name() == "Configured");
        CHECK(data.get_file_path() == "configured.ttf");
        CHECK(data.get_font_size() == doctest::Approx(16.0f));
    }

    TEST_CASE("font instance config preserves glyph range pointers and offset coordinates") {
        static const ImWchar ranges[] = {0x20, 0x7E, 0};
        static const ImWchar excluded[] = {0x30, 0x39, 0};
        Salix::ImGuiFontData data;
        data.config.glyph_ranges = ranges;
        data.config.glyph_exclude_ranges = excluded;
        data.config.glyph_offset = ImVec2(-2.5f, 4.25f);

        CHECK(data.config.glyph_ranges == ranges);
        CHECK(data.config.glyph_exclude_ranges == excluded);
        CHECK(data.config.glyph_offset.x == doctest::Approx(-2.5f));
        CHECK(data.config.glyph_offset.y == doctest::Approx(4.25f));
    }

    TEST_CASE("font instance config preserves advance rasterizer flag density and ellipsis settings") {
        Salix::ImGuiFontData data;
        data.config.glyph_min_advance_x = 2.0f;
        data.config.glyph_max_advance_x = 18.0f;
        data.config.glyph_extra_advance_x = 1.5f;
        data.config.font_loader_flags = 0x1234u;
        data.config.rasterizer_multiply = 1.25f;
        data.config.rasterizer_density = 2.0f;
        data.config.ellipsis_char = static_cast<ImWchar>(0x2026);

        CHECK(data.config.glyph_min_advance_x == doctest::Approx(2.0f));
        CHECK(data.config.glyph_max_advance_x == doctest::Approx(18.0f));
        CHECK(data.config.glyph_extra_advance_x == doctest::Approx(1.5f));
        CHECK(data.config.font_loader_flags == 0x1234u);
        CHECK(data.config.rasterizer_multiply == doctest::Approx(1.25f));
        CHECK(data.config.rasterizer_density == doctest::Approx(2.0f));
        CHECK(data.config.ellipsis_char == static_cast<ImWchar>(0x2026));
    }

    TEST_CASE("copied font data owns independent strings and config values") {
        Salix::ImGuiFontData original("Original", "original.ttf", 21.0f);
        original.config.merge_mode = true;
        Salix::ImGuiFontData copy = original;
        original.name = "Changed";
        original.file_path = "changed.ttf";
        original.config.merge_mode = false;
        original.config.size_pixels = 99.0f;
        CHECK(copy.get_name() == "Original");
        CHECK(copy.get_file_path() == "original.ttf");
        CHECK(copy.config.merge_mode);
        CHECK(copy.config.size_pixels == doctest::Approx(21.0f));
    }
}

TEST_SUITE("Salix::gui::ImGuiFontDataBatch1947") {
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
