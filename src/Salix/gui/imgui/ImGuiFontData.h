// Salix/gui/imgui/ImGuiFontData.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IFontData.h> // Inherit from abstract base
#include<imgui/imgui.h>
#include <string>
#include <vector> // For optional GlyphRanges

// CRITICAL: Include imgui.h for ImFontConfig and related types
#include <imgui/imgui.h>

namespace Salix {

    struct SALIX_API ImGuiFontData : public IFontData {
        // --- Constructors ---
        ImGuiFontData();
        ImGuiFontData(const std::string& name, const std::string& file_path, float font_size);

        // --- IFontData overrides ---
        const std::string& get_name() const override;
        const std::string& get_file_path() const override;
        float get_font_size() const override;
        const std::string& get_gui_type() const override;
        void set_font_size(float new_font_size) override;
        // --- ImGui-specific properties (mirroring ImFontConfig as closely as possible) ---
        // These are the core properties passed to io.Fonts->AddFontFromFileTTF
        std::string name; // This is the font's display name
        std::string file_path;
        float font_size; // size_pixels argument
        std::string gui_type = "ImGui";

        /* NOTES:
        struct ImFontConfig
        {
            // Data Source
            char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
            void*           FontData;               //          // TTF/OTF data
            int             FontDataSize;           //          // TTF/OTF data size
            bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the container ImFontAtlas (will delete memory itself).

            // Options
            bool            MergeMode;              // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
            bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
            bool            PixelSnapV;             // true     // Align Scaled GlyphOffset.y to pixel boundaries.
            ImS8            FontNo;                 // 0        // Index of font within TTF/OTF file
            ImS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
            ImS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
            float           SizePixels;             //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
            const ImWchar*  GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
            const ImWchar*  GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
            //ImVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
            ImVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
            float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
            float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
            float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
            unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
            //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Ue FontLoaderFlags.
            float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
            float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when ImGuiBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
            ImWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.

            // [Internal]
            ImFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
            ImFont*         DstFont;                // Target font (as we merging fonts, multiple ImFontConfig may target the same font)
            const ImFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in ImFontAtlas)
            void*           FontLoaderData;         // Font loader opaque storage (per font config)

            IMGUI_API ImFontConfig();
        };*/
        struct Config {
            bool            merge_mode;             // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
            bool            pixel_snap_h;             // false    // Align every glyph AdvanceX to pixel boundaries. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
            bool            pixel_snap_v;             // true     // Align Scaled GlyphOffset.y to pixel boundaries.
            ImS8            font_no;                 // 0        // Index of font within TTF/OTF file
            ImS8            oversample_h;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
            ImS8            oversample_v;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
            float           size_pixels;             //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
            const ImWchar*  glyph_ranges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
            const ImWchar*  glyph_exclude_ranges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
            //ImVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
            ImVec2          glyph_offset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
            float           glyph_min_advance_x;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
            float           glyph_max_advance_x;       // FLT_MAX  // Maximum AdvanceX for glyphs
            float           glyph_extra_advance_x;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
            unsigned int    font_loader_flags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
            //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Ue FontLoaderFlags.
            float           rasterizer_multiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
            float           rasterizer_density;      // 1.0f     // [LEGACY: this only makes sense when ImGuiBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
            ImWchar         ellipsis_char;           // 0        // Explicitly specify Unicode codepoint of ellipsi

            // Constructor for nested Config struct (defined below)
            Config();
            ImFontConfig to_imgui_font_config() const;
        };
        // Optional: If you want to store custom glyph ranges defined by the user
        // std::vector<ImWchar> custom_glyph_ranges_data; // Store actual data if not using static ranges
        Config config;
    };

} // namespace Salix