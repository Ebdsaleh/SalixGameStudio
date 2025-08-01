// Salix/gui/ITheme.h
#pragma once
#include <Salix/core/Core.h> 
#include <string>

namespace Salix {
    // Forward declaration of IGui, as a theme needs to apply itself to a GUI system
    class IGui;

    class SALIX_API ITheme {
    public:
        virtual ~ITheme() = default;
        // Returns the unique name of this theme.
        virtual const std::string& get_name() const = 0;
        virtual const std::string& get_file_path() const = 0;
        virtual void apply(IGui* gui_system) = 0;
        virtual bool load_from_yaml(const std::string& file_path) = 0; 
        virtual bool save_to_yaml(const std::string& file_path) const = 0 ;
        // Optional: Method to update the theme's properties from new data.
        // This could take a generic data structure or rely on the concrete
        // implementation to handle its own data updates.
        // virtual void update_from_data(const void* data) = 0;
    };

} // namespace Salix