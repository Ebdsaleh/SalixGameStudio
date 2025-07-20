// Salix/gui/imgui/ImGuiFontData.cpp
#include <Salix/gui/imgui/ImGuiFontData.h>
#include<imgui/imgui.h>
// ImGui is already included by ImGuiFontData.h now.
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
namespace Salix {

    // Define the nested Config struct's constructor here
    ImGuiFontData::Config::Config() :
        merge_mode                  { false },          // false 
        pixel_snap_h                { false },          // false 
        pixel_snap_v                { true },           // true 
        font_no                     { 0 },              // 0 
        oversample_h                { 2 },              // 0 (2)
        oversample_v                { 1 },              // 0 (1)
        size_pixels                 { NULL },           //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
        glyph_ranges                { NULL },           // NULL  
        glyph_exclude_ranges        { NULL },           // NULL
        glyph_offset                { 0, 0 },           // 0, 0
        glyph_min_advance_x         { 0 },              // 0
        glyph_max_advance_x         { FLT_MAX },        // FLT_MAX = 3.402823466e+38F // Maximum AdvanceX for glyphs
        glyph_extra_advance_x       { 0 },              // 0      
        font_loader_flags           { 0 },              // 0
        rasterizer_multiply         { 1.0f },           // 1.0f 
        rasterizer_density          { 1.0f },           // 1.0f 
        ellipsis_char               { 0 }               // 0
    {
        // Constructor body (empty if all initialized in initializer list)
    }

    // Default constructor for ImGuiFontData
    ImGuiFontData::ImGuiFontData() :
        name("Default Font"),
        file_path(""),
        font_size(20.0f),
        gui_type("ImGui"),
        config(Config())
    {
        // Constructor body (if any)
    }

    // Constructor with name, file_path, font_size
    ImGuiFontData::ImGuiFontData(const std::string& name, const std::string& file_path, float font_size) :
        name(name),
        file_path(file_path),
        font_size(font_size),
        gui_type("ImGui"),
        // Initialize ImFontConfig-related members with defaults
        config(Config())
    {
        // Constructor body (if any)
    }

    // --- IFontData overrides ---
    const std::string& ImGuiFontData::get_name() const { return name; }
    const std::string& ImGuiFontData::get_file_path() const { return file_path; }
    float ImGuiFontData::get_font_size() const { return font_size; }
    const std::string& ImGuiFontData::get_gui_type() const { return gui_type; }
    void ImGuiFontData::set_font_size(float new_font_size) {
        font_size = new_font_size;
    }
} // namespace Salix