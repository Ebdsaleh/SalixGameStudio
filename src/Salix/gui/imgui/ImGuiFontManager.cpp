// Salix/gui/imgui/ImGuiFontManager.cpp 
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/gui/IFont.h> // For IFont interface (if you have one)
#include <Salix/gui/IGui.h> // For IGui interface (to get renderer)
#include <Salix/rendering/IRenderer.h> // For IRenderer::get_native_handle()
#include <filesystem>
#include <iostream>
#include <Salix/core/ApplicationConfig.h>
#include <unordered_map> // For std::unordered_map
#include <imgui.h> // For ImGuiIO, ImFontAtlas, ImFont
#include <backends/imgui_impl_sdlrenderer2.h> // For ImGui_ImplSDLRenderer2_CreateFontsTexture
#include <SDL.h> // For SDL_Renderer* (needed for static_cast)
#include <filesystem> // For std::filesystem::directory_iterator (if loading from directory)

namespace Salix {

    // Pimpl struct definition
    struct ImGuiFontManager::Pimpl {
        IGui* gui_system = nullptr; // Non-owning pointer to the GUI system
        SDL_Renderer* sdl_renderer = nullptr; // Raw pointer to the backend renderer (for font texture creation)
        std::unordered_map<std::string, std::unique_ptr<IFont>> font_registry; // Registry for loaded fonts
        // ImFont* active_imgui_font = nullptr; // Optional: to track currently active ImGui font
    };

    // Constructor
    ImGuiFontManager::ImGuiFontManager() : pimpl(std::make_unique<Pimpl>()) {}

    // Destructor (defined here for unique_ptr Pimpl)
    ImGuiFontManager::~ImGuiFontManager() = default;

    // Initializes the Font Manager.
    // It needs a reference to the IGui instance to apply fonts.
    bool ImGuiFontManager::initialize(IGui* gui_system) {
        if (!gui_system) {
            std::cerr << "ImGuiFontManager::initialize - 'gui_system' is null." << std::endl;
            return false;
        }
        pimpl->gui_system = gui_system;

        // Get the native renderer handle from the IGui system
        IRenderer* renderer_interface = pimpl->gui_system->get_renderer();
        if (!renderer_interface) {
            std::cerr << "ImGuiFontManager::initialize - IRenderer interface is null from IGui." << std::endl;
            return false;
        }
        pimpl->sdl_renderer = static_cast<SDL_Renderer*>(renderer_interface->get_native_handle()); // Cast to concrete renderer type

        if (!pimpl->sdl_renderer) {
            std::cerr << "ImGuiFontManager::initialize - Native SDL_Renderer handle is null." << std::endl;
            return false;
        }

        std::cout << "ImGuiFontManager::initialize - Operation Successful." << std::endl;
        return true;
    }

    // Shuts down the Font Manager.
    void ImGuiFontManager::shutdown() {
        pimpl->font_registry.clear(); // Clear unique_ptrs in registry
        pimpl->gui_system = nullptr;
        pimpl->sdl_renderer = nullptr;
        std::cout << "ImGuiFontManager::shutdown - Operation Successful." << std::endl;
    }

    // --- Core Font Management Operations ---

    // Registers a font definition with the manager.
    bool ImGuiFontManager::register_font(std::unique_ptr<IFont> font) {
        if (!font) {
            std::cerr << "ImGuiFontManager::register_font - Provided font is null." << std::endl;
            return false;
        }
        const std::string& name = font->get_name(); // Assuming IFont has get_name()
        if (pimpl->font_registry.count(name)) {
            std::cerr << "ImGuiFontManager::register_font - Font '" << name << "' already registered." << std::endl;
            return false;
        }
        pimpl->font_registry[name] = std::move(font);
        std::cout << "ImGuiFontManager::register_font - Font '" << name << "' registered." << std::endl;
        return true;
    }

    // Unregisters a font by its name.
    bool ImGuiFontManager::unregister_font(const std::string& font_name) {
        if (pimpl->font_registry.erase(font_name) > 0) {
            std::cout << "ImGuiFontManager::unregister_font - Font '" << font_name << "' unregistered." << std::endl;
            return true;
        }
        std::cerr << "ImGuiFontManager::unregister_font - Font '" << font_name << "' not found." << std::endl;
        return false;
    }


      bool ImGuiFontManager::rebuild_font_atlas_texture() {

        if (!pimpl->gui_system) {
            std::cerr << "ImGuiFontManager::rebuild_font_atlas_texture - IGui reference is NULL." << std::endl;
            return false;
        }

        // Call the public backend method to recreate device objects (including font texture).
        // This is the correct way to trigger the font texture recreation from the font manager.
        if (pimpl->gui_system->get_app_config()->renderer_type == RendererType::SDL) {
            SDLImGui* sdl_imgui = dynamic_cast<SDLImGui*>(pimpl->gui_system);
            if (sdl_imgui) {

                sdl_imgui->reinitialize_backend(); // Call the new re-initialization method
                std::cout << "ImGuiFontManager::rebuild_font_atlas_texture - Operation Successful." << std::endl;
                return true;
            } else {

                std::cerr << "ImGuiFontManager::rebuild_font_atlas_texture - Could not cast IGui* to SDLImGui*. Font texture not rebuilt." << std::endl;
                return false;
            }
        } else if (pimpl->gui_system->get_app_config()->renderer_type == RendererType::OpenGL) {
                OpenGLImGui* opengl_imgui = dynamic_cast<OpenGLImGui*>(pimpl->gui_system);
                if (opengl_imgui) {
                std::cout << "ImGuiFontManager::rebuild_font_atlas_texture - Operation Successful." << std::endl;
                return true;
            } else {

                std::cerr << "ImGuiFontManager::rebuild_font_atlas_texture - Could not cast IGui* to OpenGLImGui*. Font texture not rebuilt." << std::endl;
                return false;
            }
        } else {
            std::cerr << "ImGuiFontManager::rebuild_font_atlas_texture - Unrecognized 'gui_system'!" << std::endl;
            return false;
        }
    }


    // Applies a registered font by its name as the default GUI font.
    bool ImGuiFontManager::apply_font(const std::string& font_name) {
        auto it = pimpl->font_registry.find(font_name);
        if (it == pimpl->font_registry.end()) {
            std::cerr << "ImGuiFontManager::apply_font - Font '" << font_name << "' not found in registry." << std::endl;
            return false;
        }
        std::cout << "DEBUG: ImGuiFontManager - Current Working Directory: " << std::filesystem::current_path() << std::endl;
        IFont* font_interface = it->second.get();
        if (!font_interface) {
            std::cerr << "ImGuiFontManager::apply_font - Registered font '" << font_name << "' has null interface." << std::endl;
            return false;
        }

        // --- ImGui Font Atlas Management ---
        ImGuiIO& io = ImGui::GetIO();

        // CRITICAL: Get the ImFont* directly from the registered IFont object
        // This ImFont* was loaded and stored in ImGuiFont::set_imgui_font_ptr() by load_font_from_file.
        ImFont* imgui_font_ptr_to_apply = font_interface->get_imgui_font_ptr();

        if (!imgui_font_ptr_to_apply) {
            std::cerr << "ImGuiFontManager::apply_font - Registered font '" << font_name << "' has no valid ImGuiFont*. Falling back to default." << std::endl;
            // Fallback: Try to load default font if the registered one is invalid
            imgui_font_ptr_to_apply = io.Fonts->AddFontDefault();
            if (!imgui_font_ptr_to_apply) {
                std::cerr << "ImGuiFontManager::apply_font - Critical: Fallback to default font also failed." << std::endl;
                return false;
            }
            std::cerr << "ImGuiFontManager::apply_font - Using default ImGui font." << std::endl;
        } else {
            std::cout << "ImGuiFontManager::apply_font - Applying font: " << font_interface->get_name() << std::endl;
        }

        // REMOVED: io.Fonts->Build(); // DO NOT BUILD FONT ATLAS HERE ON EVERY APPLY!
        // Font atlas should be built once after all fonts are added (e.g., in FontManager::initialize)
        // or when a new font is loaded by load_font_from_file.

        // Set the default font for ImGui's current context
        if (imgui_font_ptr_to_apply) {
            io.FontDefault = imgui_font_ptr_to_apply;
            ImGui::PushFont(io.FontDefault);
            ImGui::PopFont();
        } else {
            std::cerr << "ImGuiFontManager::apply_font - No valid ImGui font loaded. Cannot set default." << std::endl;
            return false;
        }

        // Rebuild the texture on the GPU
        // This should be called after io.Fonts->Build() (which happens in load_font_from_file)
        // and when a new font is applied.
        if (!rebuild_font_atlas_texture()) {
            std::cerr << "ImGuiFontManager::apply_font - Failed to rebuild font atlas texture." << std::endl;
            return false;
        }

        std::cout << "ImGuiFontManager::apply_font - Font '" << font_name << "' applied." << std::endl;
        return true;
    }



    // Gets a list of all registered font names.
    std::vector<std::string> ImGuiFontManager::get_registered_fonts() const {
        std::vector<std::string> names;
        for (const auto& pair : pimpl->font_registry) {
            names.push_back(pair.first);
        }
        return names;
    }

    // Optional: Get the data for a specific registered font.
    const IFont* ImGuiFontManager::get_font(const std::string& font_name) const {
        auto it = pimpl->font_registry.find(font_name);
        return (it != pimpl->font_registry.end()) ? it->second.get() : nullptr;
    }

    // --- Convenience / Workflow Methods ---

   
   
   
    // Loads a font from a file, registers it, and optionally applies it.
    bool ImGuiFontManager::load_font_from_file(
        const std::string& file_path, const std::string& font_name,
        float font_size, bool apply_immediately) {

        if (font_size < 6.0f) { // Check for minimum font size
            std::cerr << "ImGuiFontManager::load_font_from_file - Font size too small: " << font_size << std::endl;
            return false;
        }
        if (file_path.empty()) {
            std::cerr << "ImGuiFontManager::load_font_from_file - File path is empty. Cannot load font." << std::endl;
            return false;
        }
        std::cout << "DEBUG: ImGuiFontManager::load_font_from_file - Current Working Directory: " << std::filesystem::current_path() << std::endl;
        // 1. Create a concrete ImGuiFont object to hold the data
        auto new_font = std::make_unique<ImGuiFont>(file_path, font_name, font_size);
        if (!new_font) {
            std::cerr << "ImGuiFontManager::load_font_from_file - Failed to create ImGuiFont object for '" << font_name << "'." << std::endl;
            return false;
        }

        // 2. Load the ImFont* into the ImGuiFont object using ImGui's API
        // This is done here, not in apply_font, so apply_font just activates it.
        ImGuiIO& io = ImGui::GetIO();
        

        // Populate font_cfg from new_font->get_data()->config
        // FIX: Dynamic cast IFontData* to ImGuiFontData*
        ImGuiFontData* font_data = dynamic_cast<ImGuiFontData*>(new_font->get_data());
        if (!font_data) { // Check if the cast was successful
            std::cerr << "ImGuiFontManager::load_font_from_file - ImGuiFont object has incompatible font data." << std::endl;
            return false;
        }
        ImFontConfig font_cfg;
        font_cfg.MergeMode = font_data->config.merge_mode;
        font_cfg.PixelSnapH = font_data->config.pixel_snap_h;
        font_cfg.PixelSnapV = font_data->config.pixel_snap_v;
        font_cfg.FontNo = font_data->config.font_no;
        font_cfg.OversampleH = font_data->config.oversample_h;
        font_cfg.OversampleV = font_data->config.oversample_v;
        font_cfg.SizePixels = font_data->get_font_size(); // Use font_data's size
        font_cfg.GlyphRanges = font_data->config.glyph_ranges;
        font_cfg.GlyphExcludeRanges = font_data->config.glyph_exclude_ranges;
        font_cfg.GlyphOffset = font_data->config.glyph_offset;
        font_cfg.GlyphMinAdvanceX = font_data->config.glyph_min_advance_x;
        font_cfg.GlyphMaxAdvanceX = font_data->config.glyph_max_advance_x;
        font_cfg.GlyphExtraAdvanceX = font_data->config.glyph_extra_advance_x;
        font_cfg.FontLoaderFlags = font_data->config.font_loader_flags;
        font_cfg.RasterizerMultiply = font_data->config.rasterizer_multiply;
        font_cfg.RasterizerDensity = font_data->config.rasterizer_density;
        font_cfg.EllipsisChar = font_data->config.ellipsis_char;

        // Copy font name for debugger
        strncpy_s(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), font_data->get_name().c_str(), IM_ARRAYSIZE(font_cfg.Name) - 1);
        font_cfg.Name[IM_ARRAYSIZE(font_cfg.Name) - 1] = '\0';

        ImFont* loaded_imgui_font_ptr = io.Fonts->AddFontFromFileTTF(
            file_path.c_str(), // Use the file_path parameter here
            font_size,         // Use the font_size parameter here
            &font_cfg,
            NULL
        );

        if (!loaded_imgui_font_ptr) {
            std::cerr << "ImGuiFontManager::load_font_from_file - ImGui failed to load font from file: " << file_path << std::endl;
            return false; // Critical failure if ImGui can't load it
        }
        new_font->set_imgui_font_ptr(loaded_imgui_font_ptr); // Store the loaded ImFont*
    

        // 3. Register the font with the manager
        if (!register_font(std::move(new_font))) {
            std::cerr << "ImGuiFontManager::load_font_from_file - Failed to register font '" << font_name << "'." << std::endl;
            return false;
        }

        std::cout << "ImGuiFontManager::load_font_from_file - Font '" << font_name << "' loaded and registered." << std::endl;

        if (apply_immediately) {
            return apply_font(font_name);
        }
        return true;
    }


    // Loads all fonts from a specified directory and registers them.
    bool ImGuiFontManager::load_fonts_from_directory(const std::string& directory_path) {
        std::cerr << "ImGuiFontManager::load_fonts_from_directory - Stubbed." << std::endl;
        if (directory_path.empty()) { return false; }
        return false;
    }

    // Sets the active font by its registered name.
    bool ImGuiFontManager::set_active_font(const std::string& font_name) {
        if (font_name.empty() ) { return false; }
        return apply_font(font_name); // Delegates to apply_font
    }

    // Purges a font by its name, unregisters it, and unloads it if it was active.
    bool ImGuiFontManager::purge_font(const std::string& font_name) {
        std::cerr << "ImGuiFontManager::purge_font - Stubbed." << std::endl;
        if (font_name.empty() ) { return false; }
        return false;
    }
} // namespace Salix