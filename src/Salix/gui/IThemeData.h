// Salix/gui/IThemeData.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string>             // For std::string

namespace Salix {

    // Abstract base struct for theme data.
    // Concrete theme data structs (e.g., ImGuiThemeData) will inherit from this.
    // This allows for polymorphic handling of theme data.
    struct SALIX_API IThemeData {
        virtual ~IThemeData() = default;

        // Returns the unique name of this theme data.
        virtual const std::string& get_name() const = 0;

        // Optional: A method to get the type of GUI library this theme data is for.
        // This could be used by the ThemeManager to determine which concrete theme object to create.
        virtual const std::string& get_gui_type() const = 0;

        // Note: Serialization methods (e.g., virtual void serialize(Archive&))
        // would be added here if you want polymorphic serialization of the data itself.
        // For now, as per your instruction, no Cereal code.
    };

} // namespace Salix