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

        // Applies the theme's visual properties to the given GUI system.
        // The concrete implementation will know how to cast 'gui_system'
        // to its specific type (e.g., ImGuiGui*) if needed, or use generic
        // methods on IGui.
        virtual bool apply(IGui* gui_system) = 0;

        // Optional: Method to update the theme's properties from new data.
        // This could take a generic data structure or rely on the concrete
        // implementation to handle its own data updates.
        // virtual void update_from_data(const void* data) = 0;
    };

} // namespace Salix