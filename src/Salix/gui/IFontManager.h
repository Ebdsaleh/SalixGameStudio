// Salix/gui/IFontManager.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string>             // For std::string
#include <vector>             // For std::vector
#include <memory>             // For std::unique_ptr (for internal management in concrete class)

// Forward declarations for interfaces managed by this manager
namespace Salix {
    class IGui; // The GUI system this manager interacts with
    // You might have an IFont interface here if fonts have behavior beyond just data
    // For now, we'll assume fonts are managed by path/size/name.
}

namespace Salix {

    class IFont;
        
    class SALIX_API IFontManager { // Ensure SALIX_API is here
    public:
        virtual ~IFontManager() = default;


        // Initializes the Font Manager.
        // It needs a reference to the IGui instance to apply fonts.
        virtual bool initialize(IGui* gui_system) = 0;


        // Shuts down the Font Manager.
        virtual void shutdown() = 0;


        // --- Core Font Management Operations ---

        // Registers a font definition with the manager.
        // The font data would typically be loaded from a file or defined programmatically.
        // Returns true if registered successfully, false if name already exists.
        virtual bool register_font(std::unique_ptr<IFont> font) = 0;


        // Unregisters a font by its name.
        // Returns true if unregistered successfully, false if font not found.
        virtual bool unregister_font(const std::string& font_name) = 0;


        // Applies a registered font by its name as the default GUI font.
        // Returns true if applied successfully, false if font not found or application failed.
        virtual bool apply_font(const std::string& font_name) = 0;

        
        // Rebuilds the Font Atlas Texture, required after applying a font to a theme.
        virtual bool rebuild_font_atlas_texture() = 0;


        // Gets a list of all registered font names.
        virtual std::vector<std::string> get_registered_fonts() const = 0;


        // Optional: Get the data for a specific registered font.
        virtual const IFont* get_font(const std::string& font_name) const = 0;


        // --- Convenience / Workflow Methods ---

        // Loads a font from a file, registers it, and optionally applies it.
        virtual bool load_font_from_file(const std::string& file_path, const std::string& font_name, float font_size, bool apply_immediately = false) = 0;


        // Loads all fonts from a specified directory and registers them.
        // This would typically scan for font files (e.g., .ttf, .otf) and register them.
        virtual bool load_fonts_from_directory(const std::string& directory_path) = 0;


        // Sets the active font by its registered name.
        virtual bool set_active_font(const std::string& font_name) = 0;


        // Purges a font by its name, unregisters it, and unloads it if it was active.
        virtual bool purge_font(const std::string& font_name) = 0;


        // NOTE: The concrete implementation of this class will require an std::map<const std::const&, std::unique_ptr>
    };

} // namespace Salix