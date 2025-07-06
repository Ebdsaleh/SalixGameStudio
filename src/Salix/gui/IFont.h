// Salix/gui/IFont.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string>             // For std::string

namespace Salix {

    class SALIX_API IFont { // Ensure SALIX_API is here
    public:
        virtual ~IFont() = default;

        // Returns the unique name of this font.
        virtual const std::string& get_name() const = 0;

        // Returns the file path from which this font was loaded.
        virtual const std::string& get_file_path() const = 0;

        // Returns the size of the font in pixels.
        virtual float get_font_size() const = 0;

        virtual void set_font_size(float font_size) = 0;
        // Applies this font to the active GUI system.
        // The concrete implementation will know how to make this font current.
        // This method will likely interact with the IGui interface or ImGui's context.
        virtual void apply() = 0; // No IGui* needed here if the IGui implementation gets its font from IFontManager
    };

} // namespace Salix