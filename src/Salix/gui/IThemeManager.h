// Salix/gui/IThemeManager.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string>             // For std::string
#include <vector>             // For std::vector
#include <memory>             // For std::unique_ptr (for internal management in concrete class)

// Forward declarations for interfaces managed by this manager
namespace Salix {
    class IGui;   // The GUI system this manager interacts with
    class ITheme; // The abstract interface for a theme object
}

namespace Salix {

    class SALIX_API IThemeManager {
    public:
        virtual ~IThemeManager() = default;

        // Initializes the Theme Manager.
        // It needs a reference to the IGui instance to apply themes.
        virtual bool initialize(IGui* gui_system) = 0;

        // Shuts down the Theme Manager.
        virtual void shutdown() = 0;

        // --- Core Theme Management Operations ---

        // Registers a theme object with the manager.
        // The manager takes ownership of the theme object.
        // Returns true if registered successfully, false if name already exists.
        virtual bool register_theme(const std::unique_ptr<ITheme> theme) = 0;

        // Unregisters a theme by its name.
        // Returns true if unregistered successfully, false if theme not found.
        virtual bool unregister_theme(const std::string& theme_name) = 0;

        // Applies a registered theme by its name to the active GUI system.
        // Returns true if applied successfully, false if theme not found or application failed.
        virtual bool apply_theme(const std::string& theme_name) = 0;

        // Gets a list of all registered theme names.
        virtual std::vector<std::string> get_registered_themes() const = 0;

        // Gets a pointer to a specific registered theme object.
        // Returns nullptr if theme not found.
        virtual ITheme* get_theme(const std::string& theme_name) const = 0;

        // --- Convenience / Workflow Methods ---

        // Loads a theme from a file, registers it, and optionally applies it.
        // The concrete implementation will determine the theme type from the file.
        // Returns true if loaded and registered successfully.
        virtual bool load_theme_from_file(const std::string& file_path, bool apply_immediately = false) = 0;

        // Loads all themes from a specified directory and registers them.
        virtual bool load_themes_from_directory(const std::string& directory_path) = 0;

        // Sets the active theme by its registered name.
        // This is a high-level "set current theme" operation for the user, assuming it's already loaded.
        virtual bool set_active_theme(const std::string& theme_name) = 0;

        // Purges a theme by its name.
        // This unregisters it, unloads it if it was active, and deletes the ITheme object from memory/registry.
        // Returns true if purged successfully, false if theme not found.
        virtual bool purge_theme(const std::string& theme_name) = 0;

        virtual void apply_style_scale(float scale) = 0;
    };

} // namespace Salix