// Salix/gui/IFontData.h
#pragma once
#include <Salix/core/Core.h>
#include <string>

namespace Salix {

    // Abstract base struct for font data.
    // Concrete font data structs (e.g., ImGuiFontData) will inherit from this.
    // This allows for polymorphic handling of font data.
    struct SALIX_API IFontData {
        virtual ~IFontData() = default;
        
        // Returns the unique name of this font data.
        virtual const std::string& get_name() const = 0;

        // Returns the file path from which this font was loaded.
        virtual const std::string& get_file_path() const = 0;

        // Returns the size of the font in pixels.
        virtual float get_font_size() const = 0;

        // Optional: A method to get the type of GUI library this font data is for.
        virtual const std::string& get_gui_type() const = 0;

        virtual void set_font_size(float font_size) = 0;
        // Note: Serialization methods would be added here if you want polymorphic serialization.
        // For now, as per your instruction, no Cereal code.
    };

} // namespace Salix