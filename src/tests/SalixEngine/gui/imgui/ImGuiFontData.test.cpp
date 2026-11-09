#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFontData.h>
#include <cfloat>
#include <imgui/imgui.h>
#include <limits>
#include <cmath>

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
    TEST_CASE("separate font instances keep independent config state") {
        Salix::ImGuiFontData first("First", "first.ttf", 12.0f);
        Salix::ImGuiFontData second("Second", "second.ttf", 24.0f);
        first.config.merge_mode = true;
        first.config.oversample_h = 4;
        first.config.rasterizer_density = 2.0f;

        CHECK(first.config.merge_mode);
        CHECK(first.config.oversample_h == 4);
        CHECK(first.config.rasterizer_density == doctest::Approx(2.0f));
        CHECK_FALSE(second.config.merge_mode);
        CHECK(second.config.oversample_h == 2);
        CHECK(second.config.rasterizer_density == doctest::Approx(1.0f));
    }

    TEST_CASE("copy construction preserves every scalar config channel") {
        Salix::ImGuiFontData original;
        original.config.merge_mode = true;
        original.config.pixel_snap_h = true;
        original.config.pixel_snap_v = false;
        original.config.font_no = 3;
        original.config.oversample_h = 4;
        original.config.oversample_v = 2;
        original.config.size_pixels = 31.5f;
        original.config.glyph_min_advance_x = 2.0f;
        original.config.glyph_max_advance_x = 20.0f;
        original.config.glyph_extra_advance_x = 1.25f;
        original.config.font_loader_flags = 0x55AAu;
        original.config.rasterizer_multiply = 1.75f;
        original.config.rasterizer_density = 2.25f;
        original.config.ellipsis_char = static_cast<ImWchar>(0x2026);

        Salix::ImGuiFontData copy = original;

        CHECK(copy.config.merge_mode);
        CHECK(copy.config.pixel_snap_h);
        CHECK_FALSE(copy.config.pixel_snap_v);
        CHECK(copy.config.font_no == 3);
        CHECK(copy.config.oversample_h == 4);
        CHECK(copy.config.oversample_v == 2);
        CHECK(copy.config.size_pixels == doctest::Approx(31.5f));
        CHECK(copy.config.glyph_min_advance_x == doctest::Approx(2.0f));
        CHECK(copy.config.glyph_max_advance_x == doctest::Approx(20.0f));
        CHECK(copy.config.glyph_extra_advance_x == doctest::Approx(1.25f));
        CHECK(copy.config.font_loader_flags == 0x55AAu);
        CHECK(copy.config.rasterizer_multiply == doctest::Approx(1.75f));
        CHECK(copy.config.rasterizer_density == doctest::Approx(2.25f));
        CHECK(copy.config.ellipsis_char == static_cast<ImWchar>(0x2026));
    }

    TEST_CASE("copy construction preserves glyph pointer identities") {
        static const ImWchar ranges[] = {0x20, 0x7E, 0};
        static const ImWchar excluded[] = {0x30, 0x39, 0};
        Salix::ImGuiFontData original;
        original.config.glyph_ranges = ranges;
        original.config.glyph_exclude_ranges = excluded;

        Salix::ImGuiFontData copy = original;

        CHECK(copy.config.glyph_ranges == ranges);
        CHECK(copy.config.glyph_exclude_ranges == excluded);
    }

    TEST_CASE("copied glyph offset is independent from later source mutation") {
        Salix::ImGuiFontData original;
        original.config.glyph_offset = ImVec2(-3.25f, 7.5f);
        Salix::ImGuiFontData copy = original;

        original.config.glyph_offset = ImVec2(9.0f, 11.0f);

        CHECK(copy.config.glyph_offset.x == doctest::Approx(-3.25f));
        CHECK(copy.config.glyph_offset.y == doctest::Approx(7.5f));
    }

    TEST_CASE("config fields accept zeroed sampling and rasterizer values") {
        Salix::ImGuiFontData data;
        data.config.oversample_h = 0;
        data.config.oversample_v = 0;
        data.config.size_pixels = 0.0f;
        data.config.rasterizer_multiply = 0.0f;
        data.config.rasterizer_density = 0.0f;

        CHECK(data.config.oversample_h == 0);
        CHECK(data.config.oversample_v == 0);
        CHECK(data.config.size_pixels == doctest::Approx(0.0f));
        CHECK(data.config.rasterizer_multiply == doctest::Approx(0.0f));
        CHECK(data.config.rasterizer_density == doctest::Approx(0.0f));
    }

    TEST_CASE("config fields preserve signed font index and glyph offset values") {
        Salix::ImGuiFontData data;
        data.config.font_no = static_cast<ImS8>(-1);
        data.config.glyph_offset = ImVec2(-8.5f, -4.25f);

        CHECK(data.config.font_no == static_cast<ImS8>(-1));
        CHECK(data.config.glyph_offset.x == doctest::Approx(-8.5f));
        CHECK(data.config.glyph_offset.y == doctest::Approx(-4.25f));
    }

    TEST_CASE("glyph range pointers can be assigned and cleared independently") {
        static const ImWchar ranges[] = {0x20, 0x7E, 0};
        static const ImWchar excluded[] = {0x30, 0x39, 0};
        Salix::ImGuiFontData data;
        data.config.glyph_ranges = ranges;
        data.config.glyph_exclude_ranges = excluded;
        data.config.glyph_ranges = nullptr;

        CHECK(data.config.glyph_ranges == nullptr);
        CHECK(data.config.glyph_exclude_ranges == excluded);

        data.config.glyph_exclude_ranges = nullptr;
        CHECK(data.config.glyph_exclude_ranges == nullptr);
    }

    TEST_CASE("font metadata mutation does not overwrite explicit config size") {
        Salix::ImGuiFontData data("Custom", "custom.ttf", 27.0f);
        data.config.size_pixels = 42.0f;
        data.name = "Renamed";
        data.file_path = "renamed.ttf";
        data.set_font_size(33.0f);

        CHECK(data.get_name() == "Renamed");
        CHECK(data.get_file_path() == "renamed.ttf");
        CHECK(data.get_font_size() == doctest::Approx(33.0f));
        CHECK(data.config.size_pixels == doctest::Approx(42.0f));
    }

    TEST_CASE("config supports extreme advance bounds without changing font metadata") {
        Salix::ImGuiFontData data("Bounds", "bounds.ttf", 18.0f);
        data.config.glyph_min_advance_x = -FLT_MAX;
        data.config.glyph_max_advance_x = FLT_MAX;
        data.config.glyph_extra_advance_x = -3.5f;

        CHECK(data.config.glyph_min_advance_x == doctest::Approx(-FLT_MAX));
        CHECK(data.config.glyph_max_advance_x == doctest::Approx(FLT_MAX));
        CHECK(data.config.glyph_extra_advance_x == doctest::Approx(-3.5f));
        CHECK(data.get_name() == "Bounds");
        CHECK(data.get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("combined config mutation remains stable across font data copy") {
        static const ImWchar ranges[] = {0x400, 0x4FF, 0};
        Salix::ImGuiFontData original("Combined", "combined.ttf", 19.0f);
        original.config.merge_mode = true;
        original.config.pixel_snap_h = true;
        original.config.pixel_snap_v = false;
        original.config.glyph_ranges = ranges;
        original.config.glyph_offset = ImVec2(1.5f, -2.5f);
        original.config.font_loader_flags = 0x1234u;
        original.config.ellipsis_char = static_cast<ImWchar>(0x2026);

        Salix::ImGuiFontData copy = original;
        original.config.merge_mode = false;
        original.config.glyph_ranges = nullptr;
        original.config.glyph_offset = ImVec2();

        CHECK(copy.config.merge_mode);
        CHECK(copy.config.pixel_snap_h);
        CHECK_FALSE(copy.config.pixel_snap_v);
        CHECK(copy.config.glyph_ranges == ranges);
        CHECK(copy.config.glyph_offset.x == doctest::Approx(1.5f));
        CHECK(copy.config.glyph_offset.y == doctest::Approx(-2.5f));
        CHECK(copy.config.font_loader_flags == 0x1234u);
        CHECK(copy.config.ellipsis_char == static_cast<ImWchar>(0x2026));
    }
}

TEST_SUITE("Salix::gui::ImGuiFontDataBatch2447") {
    TEST_CASE("batch2447 default config exposes canonical rasterization defaults") {
        Salix::ImGuiFontData data;
        CHECK_FALSE(data.config.merge_mode);
        CHECK_FALSE(data.config.pixel_snap_h);
        CHECK(data.config.pixel_snap_v);
        CHECK(data.config.font_no == 0);
        CHECK(data.config.oversample_h == 2);
        CHECK(data.config.oversample_v == 1);
    }

    TEST_CASE("batch2447 parameterized constructor synchronizes initial config pixel size") {
        Salix::ImGuiFontData data("Body2447", "body2447.ttf", 27.5f);
        CHECK(data.get_font_size() == doctest::Approx(27.5f));
        CHECK(data.config.size_pixels == doctest::Approx(27.5f));
    }

    TEST_CASE("batch2447 font size setter changes metadata without rewriting explicit config size") {
        Salix::ImGuiFontData data("Body2447", "body2447.ttf", 16.0f);
        data.config.size_pixels = 48.0f;
        data.set_font_size(22.0f);
        CHECK(data.get_font_size() == doctest::Approx(22.0f));
        CHECK(data.config.size_pixels == doctest::Approx(48.0f));
    }

    TEST_CASE("batch2447 font metadata preserves negative diagnostic size assignments") {
        Salix::ImGuiFontData data;
        data.set_font_size(-12.5f);
        CHECK(data.get_font_size() == doctest::Approx(-12.5f));
        CHECK(data.config.size_pixels == doctest::Approx(20.0f));
    }

    TEST_CASE("batch2447 GUI identity remains ImGui after public metadata mutation") {
        Salix::ImGuiFontData data;
        data.name = "Renamed2447";
        data.file_path = "renamed2447.otf";
        data.set_font_size(31.0f);
        CHECK(data.get_gui_type() == "ImGui");
        CHECK(data.get_name() == "Renamed2447");
        CHECK(data.get_file_path() == "renamed2447.otf");
    }

    TEST_CASE("batch2447 copied config scalar state becomes independently mutable") {
        Salix::ImGuiFontData original("Original2447", "o.ttf", 18.0f);
        original.config.rasterizer_multiply = 1.5f;
        original.config.rasterizer_density = 2.0f;
        Salix::ImGuiFontData copy = original;
        copy.config.rasterizer_multiply = 0.5f;
        copy.config.rasterizer_density = 3.0f;
        CHECK(original.config.rasterizer_multiply == doctest::Approx(1.5f));
        CHECK(original.config.rasterizer_density == doctest::Approx(2.0f));
        CHECK(copy.config.rasterizer_multiply == doctest::Approx(0.5f));
        CHECK(copy.config.rasterizer_density == doctest::Approx(3.0f));
    }

    TEST_CASE("batch2447 copied config preserves caller-owned glyph range pointers") {
        static const ImWchar ranges[] = {0x20, 0x7E, 0};
        static const ImWchar excluded[] = {0x30, 0x39, 0};
        Salix::ImGuiFontData original;
        original.config.glyph_ranges = ranges;
        original.config.glyph_exclude_ranges = excluded;
        Salix::ImGuiFontData copy = original;
        CHECK(copy.config.glyph_ranges == ranges);
        CHECK(copy.config.glyph_exclude_ranges == excluded);
    }

    TEST_CASE("batch2447 loader flags and ellipsis codepoint remain independently assignable") {
        Salix::ImGuiFontData data;
        data.config.font_loader_flags = 0xA5A5u;
        data.config.ellipsis_char = static_cast<ImWchar>(0x2026);
        CHECK(data.config.font_loader_flags == 0xA5A5u);
        CHECK(data.config.ellipsis_char == static_cast<ImWchar>(0x2026));
        CHECK(data.config.glyph_ranges == nullptr);
    }

    TEST_CASE("batch2447 IFontData dispatch updates concrete font size") {
        Salix::ImGuiFontData data("Interface2447", "interface.ttf", 14.0f);
        Salix::IFontData* base = &data;
        base->set_font_size(26.0f);
        CHECK(base->get_font_size() == doctest::Approx(26.0f));
        CHECK(base->get_name() == "Interface2447");
        CHECK(base->get_gui_type() == "ImGui");
    }

    TEST_CASE("batch2447 separate font data objects keep independent config and metadata") {
        Salix::ImGuiFontData first("First2447", "first.ttf", 12.0f);
        Salix::ImGuiFontData second("Second2447", "second.ttf", 24.0f);
        first.config.merge_mode = true;
        second.config.pixel_snap_h = true;
        CHECK(first.get_name() == "First2447");
        CHECK(second.get_name() == "Second2447");
        CHECK(first.config.merge_mode);
        CHECK_FALSE(second.config.merge_mode);
        CHECK_FALSE(first.config.pixel_snap_h);
        CHECK(second.config.pixel_snap_h);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontDataBatch3347") {
    TEST_CASE("batch3347 default config converts canonical sampling booleans to ImGui") {
        Salix::ImGuiFontData data; const auto cfg=data.config.to_imgui_font_config(); CHECK_FALSE(cfg.MergeMode); CHECK_FALSE(cfg.PixelSnapH); CHECK(cfg.PixelSnapV);
    }
    TEST_CASE("batch3347 custom sampling booleans convert exactly to ImGui") {
        Salix::ImGuiFontData data; data.config.merge_mode=true; data.config.pixel_snap_h=true; data.config.pixel_snap_v=false; const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.MergeMode); CHECK(cfg.PixelSnapH); CHECK_FALSE(cfg.PixelSnapV);
    }
    TEST_CASE("batch3347 font index and oversampling channels convert exactly") {
        Salix::ImGuiFontData data; data.config.font_no=3; data.config.oversample_h=4; data.config.oversample_v=2; const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.FontNo==3); CHECK(cfg.OversampleH==4); CHECK(cfg.OversampleV==2);
    }
    TEST_CASE("batch3347 configured pixel size converts independently from metadata size") {
        Salix::ImGuiFontData data("Font3347","font3347.ttf",18.f); data.config.size_pixels=31.5f; const auto cfg=data.config.to_imgui_font_config();
        CHECK(data.get_font_size()==doctest::Approx(18.f)); CHECK(cfg.SizePixels==doctest::Approx(31.5f));
    }
    TEST_CASE("batch3347 glyph range pointers convert without ownership changes") {
        static const ImWchar ranges[]={0x20,0x7e,0}; static const ImWchar exclude[]={0x30,0x39,0}; Salix::ImGuiFontData data; data.config.glyph_ranges=ranges; data.config.glyph_exclude_ranges=exclude; const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.GlyphRanges==ranges); CHECK(cfg.GlyphExcludeRanges==exclude);
    }
    TEST_CASE("batch3347 glyph offset converts both signed coordinates") {
        Salix::ImGuiFontData data; data.config.glyph_offset=ImVec2(-3.25f,7.5f); const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.GlyphOffset.x==doctest::Approx(-3.25f)); CHECK(cfg.GlyphOffset.y==doctest::Approx(7.5f));
    }
    TEST_CASE("batch3347 glyph advance bounds and extra advance convert exactly") {
        Salix::ImGuiFontData data; data.config.glyph_min_advance_x=2.f; data.config.glyph_max_advance_x=19.f; data.config.glyph_extra_advance_x=1.25f; const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.GlyphMinAdvanceX==doctest::Approx(2.f)); CHECK(cfg.GlyphMaxAdvanceX==doctest::Approx(19.f)); CHECK(cfg.GlyphExtraAdvanceX==doctest::Approx(1.25f));
    }
    TEST_CASE("batch3347 loader flags and rasterizer controls convert exactly") {
        Salix::ImGuiFontData data; data.config.font_loader_flags=0x3347u; data.config.rasterizer_multiply=1.5f; data.config.rasterizer_density=2.25f; const auto cfg=data.config.to_imgui_font_config();
        CHECK(cfg.FontLoaderFlags==0x3347u); CHECK(cfg.RasterizerMultiply==doctest::Approx(1.5f)); CHECK(cfg.RasterizerDensity==doctest::Approx(2.25f));
    }
    TEST_CASE("batch3347 ellipsis codepoint converts exactly") {
        Salix::ImGuiFontData data; data.config.ellipsis_char=static_cast<ImWchar>(0x2026); const auto cfg=data.config.to_imgui_font_config(); CHECK(cfg.EllipsisChar==static_cast<ImWchar>(0x2026));
    }
    TEST_CASE("batch3347 metadata size setter leaves explicitly configured rasterizer size unchanged") {
        Salix::ImGuiFontData data("Font3347","font3347.ttf",20.f); data.config.size_pixels=42.f; data.set_font_size(12.f); const auto cfg=data.config.to_imgui_font_config();
        CHECK(data.get_font_size()==doctest::Approx(12.f)); CHECK(cfg.SizePixels==doctest::Approx(42.f));
    }
}
TEST_SUITE("Salix::gui::ImGuiFontDataBatch3747") {
    TEST_CASE("batch3747 default config conversion preserves advance rasterizer and ellipsis defaults") {
        Salix::ImGuiFontData data;
        const auto cfg = data.config.to_imgui_font_config();
        CHECK(cfg.GlyphMinAdvanceX == doctest::Approx(0.0f));
        CHECK(cfg.GlyphMaxAdvanceX == doctest::Approx(FLT_MAX));
        CHECK(cfg.GlyphExtraAdvanceX == doctest::Approx(0.0f));
        CHECK(cfg.FontLoaderFlags == 0u);
        CHECK(cfg.RasterizerMultiply == doctest::Approx(1.0f));
        CHECK(cfg.RasterizerDensity == doctest::Approx(1.0f));
        CHECK(cfg.EllipsisChar == 0);
    }

    TEST_CASE("batch3747 mutating converted ImFontConfig never changes source config") {
        Salix::ImGuiFontData data;
        data.config.merge_mode = true;
        data.config.size_pixels = 37.47f;
        auto cfg = data.config.to_imgui_font_config();
        cfg.MergeMode = false;
        cfg.SizePixels = 99.0f;
        CHECK(data.config.merge_mode);
        CHECK(data.config.size_pixels == doctest::Approx(37.47f));
    }

    TEST_CASE("batch3747 repeated config conversions return independent value snapshots") {
        Salix::ImGuiFontData data;
        data.config.glyph_offset = ImVec2(3.0f, 7.0f);
        auto first = data.config.to_imgui_font_config();
        first.GlyphOffset = ImVec2(-1.0f, -2.0f);
        const auto second = data.config.to_imgui_font_config();
        CHECK(second.GlyphOffset.x == doctest::Approx(3.0f));
        CHECK(second.GlyphOffset.y == doctest::Approx(7.0f));
    }

    TEST_CASE("batch3747 metadata font size accepts positive infinity without rewriting config size") {
        Salix::ImGuiFontData data("Infinite3747", "infinite.ttf", 18.0f);
        const float config_size = data.config.size_pixels;
        data.set_font_size(std::numeric_limits<float>::infinity());
        CHECK(std::isinf(data.get_font_size()));
        CHECK(data.get_font_size() > 0.0f);
        CHECK(data.config.size_pixels == doctest::Approx(config_size));
    }

    TEST_CASE("batch3747 metadata font size accepts NaN without rewriting config size") {
        Salix::ImGuiFontData data("NaN3747", "nan.ttf", 19.0f);
        const float config_size = data.config.size_pixels;
        data.set_font_size(std::numeric_limits<float>::quiet_NaN());
        CHECK(std::isnan(data.get_font_size()));
        CHECK(data.config.size_pixels == doctest::Approx(config_size));
    }

    TEST_CASE("batch3747 public name and path preserve embedded null bytes through getters") {
        Salix::ImGuiFontData data;
        const std::string name("Font\0Name3747", 13);
        const std::string path("path\0font.ttf", 13);
        data.name = name;
        data.file_path = path;
        CHECK(data.get_name() == name);
        CHECK(data.get_file_path() == path);
        CHECK(data.get_name().size() == name.size());
        CHECK(data.get_file_path().size() == path.size());
    }

    TEST_CASE("batch3747 public GUI type mutation is reflected by interface getter") {
        Salix::ImGuiFontData data;
        Salix::IFontData& base = data;
        data.gui_type = "CustomGui3747";
        CHECK(base.get_gui_type() == "CustomGui3747");
        data.gui_type.clear();
        CHECK(base.get_gui_type().empty());
    }

    TEST_CASE("batch3747 signed ImS8 config boundaries convert without normalization") {
        Salix::ImGuiFontData data;
        data.config.font_no = (std::numeric_limits<ImS8>::min)();
        data.config.oversample_h = (std::numeric_limits<ImS8>::max)();
        data.config.oversample_v = (std::numeric_limits<ImS8>::min)();
        const auto cfg = data.config.to_imgui_font_config();
        CHECK(cfg.FontNo == (std::numeric_limits<ImS8>::min)());
        CHECK(cfg.OversampleH == (std::numeric_limits<ImS8>::max)());
        CHECK(cfg.OversampleV == (std::numeric_limits<ImS8>::min)());
    }

    TEST_CASE("batch3747 negative zero config pixel size preserves sign through conversion") {
        Salix::ImGuiFontData data;
        data.config.size_pixels = -0.0f;
        const auto cfg = data.config.to_imgui_font_config();
        CHECK(cfg.SizePixels == 0.0f);
        CHECK(std::signbit(cfg.SizePixels));
    }

    TEST_CASE("batch3747 copy assignment replaces metadata and config while remaining independent") {
        Salix::ImGuiFontData source("Source3747", "source3747.ttf", 27.0f);
        source.config.merge_mode = true;
        source.config.rasterizer_density = 2.5f;
        Salix::ImGuiFontData destination("Destination", "dest.ttf", 11.0f);
        destination = source;
        CHECK(destination.get_name() == "Source3747");
        CHECK(destination.get_file_path() == "source3747.ttf");
        CHECK(destination.get_font_size() == doctest::Approx(27.0f));
        CHECK(destination.config.merge_mode);
        CHECK(destination.config.rasterizer_density == doctest::Approx(2.5f));
        source.name = "MutatedSource";
        source.config.rasterizer_density = 9.0f;
        CHECK(destination.get_name() == "Source3747");
        CHECK(destination.config.rasterizer_density == doctest::Approx(2.5f));
    }
}

TEST_SUITE("Salix::gui::ImGuiFontDataBatch4447") {
    TEST_CASE("batch4447 default constructor synchronizes initial metadata and config pixel sizes") {
        Salix::ImGuiFontData data;
        CHECK(data.get_font_size() == doctest::Approx(20.0f));
        CHECK(data.config.size_pixels == doctest::Approx(20.0f));
    }

    TEST_CASE("batch4447 custom constructor synchronizes initial config size with requested metadata size") {
        Salix::ImGuiFontData data("Font4447", "font4447.ttf", 44.47f);
        CHECK(data.get_font_size() == doctest::Approx(44.47f));
        CHECK(data.config.size_pixels == doctest::Approx(44.47f));
    }

    TEST_CASE("batch4447 zero size constructor preserves zero in both initial size channels") {
        Salix::ImGuiFontData data("Zero4447", "zero4447.ttf", 0.0f);
        CHECK(data.get_font_size() == doctest::Approx(0.0f));
        CHECK(data.config.size_pixels == doctest::Approx(0.0f));
    }

    TEST_CASE("batch4447 negative size constructor preserves diagnostic value in both initial channels") {
        Salix::ImGuiFontData data("Negative4447", "negative4447.ttf", -4.447f);
        CHECK(data.get_font_size() == doctest::Approx(-4.447f));
        CHECK(data.config.size_pixels == doctest::Approx(-4.447f));
    }

    TEST_CASE("batch4447 metadata setter preserves name path and gui type") {
        Salix::ImGuiFontData data("Stable4447", "stable4447.ttf", 12.0f);
        data.gui_type = "Custom4447";
        data.set_font_size(27.0f);
        CHECK(data.get_name() == "Stable4447");
        CHECK(data.get_file_path() == "stable4447.ttf");
        CHECK(data.get_gui_type() == "Custom4447");
        CHECK(data.get_font_size() == doctest::Approx(27.0f));
    }

    TEST_CASE("batch4447 default glyph range pointers remain null through config conversion") {
        Salix::ImGuiFontData data;
        const auto cfg = data.config.to_imgui_font_config();
        CHECK(cfg.GlyphRanges == nullptr);
        CHECK(cfg.GlyphExcludeRanges == nullptr);
    }

    TEST_CASE("batch4447 copied config remains independent after source mutation") {
        Salix::ImGuiFontData::Config source;
        source.merge_mode = true;
        source.rasterizer_density = 2.0f;
        auto copy = source;
        source.merge_mode = false;
        source.rasterizer_density = 9.0f;
        CHECK(copy.merge_mode);
        CHECK(copy.rasterizer_density == doctest::Approx(2.0f));
    }

    TEST_CASE("batch4447 config assignment preserves glyph range pointer identity") {
        static const ImWchar ranges[] = {0x20, 0x7e, 0};
        Salix::ImGuiFontData::Config source;
        source.glyph_ranges = ranges;
        Salix::ImGuiFontData::Config copy;
        copy = source;
        CHECK(copy.glyph_ranges == ranges);
    }

    TEST_CASE("batch4447 copy construction keeps metadata and config independent") {
        Salix::ImGuiFontData source("Source4447", "source4447.ttf", 18.0f);
        source.gui_type = "Gui4447";
        source.config.merge_mode = true;
        Salix::ImGuiFontData copy(source);
        source.name = "Mutated4447";
        source.config.merge_mode = false;
        CHECK(copy.get_name() == "Source4447");
        CHECK(copy.get_file_path() == "source4447.ttf");
        CHECK(copy.get_gui_type() == "Gui4447");
        CHECK(copy.config.merge_mode);
    }

    TEST_CASE("batch4447 ellipsis and loader flags convert together without normalization") {
        Salix::ImGuiFontData data;
        data.config.ellipsis_char = static_cast<ImWchar>(0x2026);
        data.config.font_loader_flags = 0x4447u;
        const auto cfg = data.config.to_imgui_font_config();
        CHECK(cfg.EllipsisChar == static_cast<ImWchar>(0x2026));
        CHECK(cfg.FontLoaderFlags == 0x4447u);
    }
}
