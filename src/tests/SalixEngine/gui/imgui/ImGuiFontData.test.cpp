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
