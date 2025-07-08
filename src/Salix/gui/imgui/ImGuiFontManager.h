// Salix/gui/imgui/ImGuiFontManager.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IFontManager.h>
#include <string>             // For std::string
#include <vector>             // For std::vector
#include <memory>             // For std::unique_ptr (for internal management in concrete class)

// Forward declarations for interfaces managed by this manager
class IGui; // The GUI system this manager interacts with
class IFont;



namespace Salix {
        
    class SALIX_API ImGuiFontManager : IFontManager { // Ensure SALIX_API is here
    public:
        
        ImGuiFontManager();
        ~ImGuiFontManager() override;

        // Initializes the Font Manager.
        // It needs a reference to the IGui instance to apply fonts.
        bool initialize(IGui* gui_system);


        // Shuts down the Font Manager.
        void shutdown();


        // --- Core Font Management Operations ---

        // Registers a font definition with the manager.
        // The font data would typically be loaded from a file or defined programmatically.
        // Returns true if registered successfully, false if name already exists.
        bool register_font(std::unique_ptr<IFont> font);


        // Unregisters a font by its name.
        // Returns true if unregistered successfully, false if font not found.
        bool unregister_font(const std::string& font_name);


        // Applies a registered font by its name as the default GUI font.
        // Returns true if applied successfully, false if font not found or application failed.
        bool apply_font(const std::string& font_name);


        // Rebuilds the Font Atlas Texture, required after applying a font to a theme.
        bool rebuild_font_atlas_texture();


        // Gets a list of all registered font names.
        std::vector<std::string> get_registered_fonts() const;


        // Optional: Get the data for a specific registered font.
        const IFont* get_font(const std::string& font_name) const;

    
        // --- Convenience / Workflow Methods ---

        // Loads a font from a file, registers it, and optionally applies it.
        bool load_font_from_file(const std::string& file_path, const std::string& font_name, float font_size, bool apply_immediately = false);


        // Loads all fonts from a specified directory and registers them.
        // This would typically scan for font files (e.g., .ttf, .otf) and register them.
        bool load_fonts_from_directory(const std::string& directory_path);


        // Sets the active font by its registered name.
        bool set_active_font(const std::string& font_name);


        // Purges a font by its name, unregisters it, and unloads it if it was active.
        bool purge_font(const std::string& font_name);


        // Remember std::map<const std::const&, std::unique_ptr> for the font_registry.

    private:

        // Private memeber implementation, enhanced encapsulation.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix