// Salix/gui/imgui/ImGuiFontManager.cpp 
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/management/FileManager.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
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
        IFont* active_font = nullptr;
        SDL_GLContext* gl_context = nullptr;
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

        RendererType current_renderer_type = gui_system->get_app_config()->renderer_type;

        if (current_renderer_type == RendererType::SDL) {
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
        }
        else if (current_renderer_type == RendererType::OpenGL) {
            IRenderer* renderer_interface = pimpl->gui_system->get_renderer();
            if (!renderer_interface) {
                std::cerr << "ImGuiFontManager::initialize - IRenderer interface is null from IGui." << std::endl;
                return false;
            }
            OpenGLRenderer* opengl_renderer = dynamic_cast<OpenGLRenderer*>(renderer_interface);
            if (!opengl_renderer) {
                std::cerr << "ImGuiFontManager::initialize - Could not cast IRenderer to OpenGLRenderer." << std::endl;
                return false;
            }
            OpenGLImGui* opengl_imgui = dynamic_cast<OpenGLImGui*>(pimpl->gui_system);
            if (!opengl_imgui) {
                 std::cerr << "ImGuiFontManager::initialize - Could not cast IGui to OpenGLImGui for GL context." << std::endl;
                 return false;
            }
            SDL_GLContext* gl_context = static_cast<SDL_GLContext*>(opengl_renderer->get_native_handle());
            if (!gl_context) {
                std::cerr << "ImGuiFontManager::initialize - Could not cast OpenGLRenderer for GL context." << std::endl;
            }
            pimpl->gl_context = gl_context;
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
                    opengl_imgui->reinitialize_backend();
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
            std::cerr << "ImGuiFontManager::apply_font - Font '" << font_name << "' not found." << std::endl;
            return false;
        }

        ImFont* imgui_font_ptr = it->second->get_imgui_font_ptr();
        if (!imgui_font_ptr) {
            std::cerr << "ImGuiFontManager::apply_font - Font '" << font_name << "' has an invalid internal ImFont pointer." << std::endl;
            return false;
        }

        // 1. Tell ImGui to use this font
        ImGui::GetIO().FontDefault = imgui_font_ptr;

        // 2. Update the manager's state
        set_active_font(font_name);
        
        std::cout << "ImGuiFontManager - Applied and set active font to '" << font_name << "'." << std::endl;
        return true;
    }



    // Gets a list of all registered font names.
    std::vector<std::string> ImGuiFontManager::get_registered_font_names() const {
        std::vector<std::string> names;
        for (const auto& pair : pimpl->font_registry) {
            names.push_back(pair.first);
        }
        return names;
    }

    const std::unordered_map<std::string, std::unique_ptr<IFont>>& ImGuiFontManager::get_font_registry() {
        return pimpl->font_registry;
    }

    // Optional: Get the data for a specific registered font.
    const IFont* ImGuiFontManager::get_font(const std::string& font_name) const {
        auto it = pimpl->font_registry.find(font_name);
        return (it != pimpl->font_registry.end()) ? it->second.get() : nullptr;
    }

    // --- Convenience / Workflow Methods ---


    std::vector<std::string> ImGuiFontManager::get_unique_font_families() const {
        std::set<std::string> unique_families;

        // Iterate through every font in the registry.
        for (const auto& pair : pimpl->font_registry) {
            // 'pair.second' is the std::unique_ptr<IFont>.
            const IFont* font = pair.second.get();

            // Get the font's family name and insert it into the set.
            // std::set automatically handles duplicates for you.
            if (font) {
                unique_families.insert(font->get_family());
            }
        }

        // Create a vector from the contents of the set and return it.
        return std::vector<std::string>(unique_families.begin(), unique_families.end());
    }
    
    
    bool ImGuiFontManager::load_and_register_font(const std::string& path, const std::string& name, const std::string& family, float size, const ImFontConfig& config) {
    
        // 1. Create the font object and set its family
        auto new_font = std::make_unique<ImGuiFont>(path, name, size);
        new_font->set_family(family);

        // 2. Load the font into ImGui's atlas
        ImGuiIO& io = ImGui::GetIO();
        ImFont* loaded_ptr = io.Fonts->AddFontFromFileTTF(path.c_str(), size, &config);

        if (!loaded_ptr) {
            std::cerr << "ImGuiFontManager Error: Failed to load font from file: " << path << std::endl;
            return false;
        }
        
        loaded_ptr->Scale = 1.0f; // Set the scale to prevent crashes
        new_font->set_imgui_font_ptr(loaded_ptr);

        // 3. Register the fully configured font
        return register_font(std::move(new_font));
    }
  
    void ImGuiFontManager::setup_default_fonts() {
        // Roboto-Regular
        std::string font_path = "Assets/Fonts/Roboto-Regular.ttf";
        std::string font_family = "Roboto-Regular";
        float min_size = 8.0f;
        float max_size = 72.0f;
        create_font_batch(font_path, font_family, min_size, max_size);

        // Roboto-Medium
        font_path = "Assets/Fonts/Roboto-Medium.ttf";
        font_family = "Roboto-Medium";
        create_font_batch(font_path, font_family, min_size, max_size);

        // Karla-Regular
        font_path = "Assets/Fonts/Karla-Regular.ttf";
        font_family = "Karla-Regular";
        create_font_batch(font_path, font_family, min_size, max_size);

        // Proggy-Clean
        font_path = "Assets/Fonts/ProggyClean.ttf";
        font_family = "ProggyClean";
        create_font_batch(font_path, font_family, min_size, max_size);

        // Proggy-Tiny
        font_path = "Assets/Fonts/ProggyTiny.ttf";
        font_family = "ProggyTiny";
        create_font_batch(font_path, font_family, min_size, max_size);

        // DroidSans
        font_path = "Assets/Fonts/DroidSans.ttf";
        font_family = "DroidSans";
        create_font_batch(font_path, font_family, min_size, max_size);

        // Cousine-Regular
        font_path = "Assets/Fonts/Cousine-Regular.ttf";
        font_family = "Cousine-Regular";
        create_font_batch(font_path, font_family, min_size, max_size);
    }

    void ImGuiFontManager::create_font_batch(const std::string& font_path, const std::string& font_family, float min_size, float max_size) {
        if (!FileManager::path_exists(font_path)) {
            std::cerr << "ImGuiFontManager::create_font_batch - Failed to create font batch, 'font_path' is invalid!" <<
            std::endl;
            return;
        }

        float dpi_scale = pimpl->gui_system->get_app_config()->gui_settings.global_dpi_scaling;
        
        for (float size = min_size; size <= max_size; size += 2.0f) {
            std::string variant_name = font_family + "_" + std::to_string(static_cast<int>(size)) + "px";

            ImFontConfig font_cfg;
            font_cfg.SizePixels = size;
            load_and_register_font(font_path, variant_name, font_family, size * dpi_scale, font_cfg);
        }

    }
   
    std::vector<float> ImGuiFontManager::get_font_sizes_for_font_family(const std::string& font_family) const {
        std::vector<float> family_font_sizes;

        // Iterate through the font registry
        for (const auto& pair : pimpl->font_registry) {
            // 'pair.second' is the std::unique_ptr<IFont>
            const IFont* font = pair.second.get();

            // Check if the font's family matches the requested family
            if (font && font->get_family() == font_family) {
                // If it matches, get its data and add the size to our vector
                if (IFontData* data = font->get_data()) {
                    family_font_sizes.push_back(data->get_font_size());
                }
            }
        }

        // Sort the sizes for a clean, ordered dropdown in the UI
        std::sort(family_font_sizes.begin(), family_font_sizes.end());

        return family_font_sizes;
    }
   
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
        auto it = pimpl->font_registry.find(font_name);
        if (it == pimpl->font_registry.end()) {
            std::cerr << "ImGuiFontManager::set_active_font - Font '" << font_name << "' not found." << std::endl;
            return false;
        }
        // Just update the manager's internal pointer
        pimpl->active_font = it->second.get();
        return true;
    }
    

    IFont* ImGuiFontManager::get_active_font() {
        return pimpl->active_font;

    }
    // Purges a font by its name, unregisters it, and unloads it if it was active.
    bool ImGuiFontManager::purge_font(const std::string& font_name) {
        std::cerr << "ImGuiFontManager::purge_font - Stubbed." << std::endl;
        if (font_name.empty() ) { return false; }
        return false;
    }
} // namespace Salix