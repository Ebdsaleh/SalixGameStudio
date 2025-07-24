// Salix/gui/imgui/ImGuiFontManager.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IFontManager.h>
#include <string>             // For std::string
#include <vector>             // For std::vector
#include <memory>             // For std::unique_ptr (for internal management in concrete class)
#include <unordered_map>
#include <map>
#include <imgui/imgui.h>

namespace Salix {
// Forward declarations for interfaces managed by this manager
class IGui; // The GUI system this manager interacts with
class IFont;
}

// Forward declare SDL_Renderer if ImGuiFontManager needs to store it in Pimpl
struct SDL_Renderer; // Needed for pimpl->sdl_renderer

namespace Salix {
        
    class SALIX_API ImGuiFontManager : public IFontManager { // Ensure SALIX_API is here
    public:
        
        ImGuiFontManager();
        ~ImGuiFontManager() override;

        // Initializes the Font Manager.
        // It needs a reference to the IGui instance to apply fonts.
        bool initialize(IGui* gui_system) override;


        // Shuts down the Font Manager.
        void shutdown() override;


        // --- Core Font Management Operations ---

        // Registers a font definition with the manager.
        // The font data would typically be loaded from a file or defined programmatically.
        // Returns true if registered successfully, false if name already exists.
        bool register_font(std::unique_ptr<IFont> font) override;


        // Unregisters a font by its name.
        // Returns true if unregistered successfully, false if font not found.
        bool unregister_font(const std::string& font_name) override;


        // Applies a registered font by its name as the default GUI font.
        // Returns true if applied successfully, false if font not found or application failed.
        bool apply_font(const std::string& font_name) override;


        // Gets a list of all registered font names.
        std::vector<std::string> get_registered_font_names() const override;

        const std::unordered_map<std::string, std::unique_ptr<IFont>>& get_font_registry() override;
        
        std::vector<float> get_font_sizes_for_font_family(const std::string& font_family) const override;

        // Optional: Get the data for a specific registered font.
        const IFont* get_font(const std::string& font_name) const override;

        void create_font_batch(const std::string& font_path, const std::string& font_family, float min_size, float max_size) override;
        
        void setup_default_fonts() override;

        std::vector<std::string> get_unique_font_families() const override;
        // --- Convenience / Workflow Methods ---

        // Loads a font from a file, registers it, and optionally applies it.
        bool load_font_from_file(const std::string& file_path, const std::string& font_name, float font_size, bool apply_immediately = false) override;


        // Loads all fonts from a specified directory and registers them.
        // This would typically scan for font files (e.g., .ttf, .otf) and register them.
        bool load_fonts_from_directory(const std::string& directory_path) override;


        // Sets the active font by its registered name.
        bool set_active_font(const std::string& font_name) override;
        IFont* get_active_font() override;

        // Purges a font by its name, unregisters it, and unloads it if it was active.
        bool purge_font(const std::string& font_name) override;


        bool rebuild_font_atlas_texture() override;
        // Remember std::map<const std::string, std::unique_ptr<IFont>> for the font_registry.


    private:

        // Private memeber implementation, enhanced encapsulation.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        bool load_and_register_font(const std::string& path, const std::string& name, const std::string& family, float size, const ImFontConfig& config);
    };

} // namespace Salix