// Salix/gui/IFont.h
#pragma once
#include <Salix/core/Core.h>
#include <string>             
#include <Salix/gui/IFontData.h>

struct IFontData;
struct ImFont;
namespace Salix {

    class SALIX_API IFont {
    public:

        virtual ~IFont() = default;
        // Returns the unique name of this font (from its data).

        virtual const std::string& get_name() const = 0; // RE-ADDED: Needed for font_registry map key

        // Provides access to the mutable font data.
        virtual IFontData* get_data() = 0;

        // Provides access to the raw ImGuiFont* (concrete ImGui font object).
        virtual ImFont* get_imgui_font_ptr() const = 0;

        
    };

} // namespace Salix