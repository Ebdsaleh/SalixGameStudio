#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFontData.h>
#include <cfloat>

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
