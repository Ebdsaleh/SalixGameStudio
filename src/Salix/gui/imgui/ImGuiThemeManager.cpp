// Salix/gui/imgui/ImGuiThemeManager.cpp

#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/core/ApplicationConfig.h>
#include <Salix/gui/IIconManager.h>
#include <imgui/imgui.h>
#include <fstream>
#include <filesystem>
#include <Salix/management/FileManager.h>

 

namespace Salix {

    // ---------------------- ImGuiThemeManager ----------------------
    struct ImGuiThemeManager::Pimpl {
        IGui* gui_system = nullptr;
        std::unordered_map<std::string, std::unique_ptr<ITheme>> theme_registry;
        std::string active_theme_name;
        float current_scale = 1.0f;
        ITheme* active_theme = nullptr;
        IIconManager* icon_manager = nullptr;
    };

    ImGuiThemeManager::ImGuiThemeManager() : pimpl(std::make_unique<Pimpl>()) {}
    
    ImGuiThemeManager::~ImGuiThemeManager() = default;



    bool ImGuiThemeManager::initialize(IGui* gui_system) {
        pimpl->gui_system = gui_system;
        pimpl->icon_manager = gui_system->get_icon_manager();
        return true;
    }

    void ImGuiThemeManager::shutdown() {
        pimpl->theme_registry.clear();
        pimpl->gui_system = nullptr;
        pimpl->active_theme = nullptr;
    }

    bool ImGuiThemeManager::register_theme(std::unique_ptr<ITheme> theme) {
        const std::string& name = theme->get_name(); // Get name before moving
        if (pimpl->theme_registry.count(name)) {
            std::cerr << "ImGuiThemeManager Error: Theme '" << name << "' already registered." << std::endl;
            return false;
        }
        pimpl->theme_registry[name] = std::move(theme); // CORRECT: Just std::move(theme)
        std::cout << "ImGuiThemeManager: Theme '" << name << "' registered." << std::endl;
        return true;
        }

    bool ImGuiThemeManager::unregister_theme(const std::string& theme_name) {
        return pimpl->theme_registry.erase(theme_name) > 0;
    }

    void ImGuiThemeManager::apply_style_scale(float scale) {
        if (scale <= 0.0f) { return; }
        // First, reset the current scaling to get back to a baseline of 1.0
        ImGui::GetStyle().ScaleAllSizes(1.0f / pimpl->current_scale);
         // Apply the new scaling
        ImGui::GetStyle().ScaleAllSizes(scale);
        pimpl->current_scale = scale;
        
    }




    bool ImGuiThemeManager::apply_theme(const std::string& theme_name) {
        // 1. Find the theme in the registry
        auto it = pimpl->theme_registry.find(theme_name);
        if (it == pimpl->theme_registry.end()) {
            std::cerr << "ImGuiThemeManager Error: Theme '" << theme_name << "' not found." << std::endl;
            return false;
        }

        ITheme* theme_to_apply = it->second.get();

        // 2. Check that the GUI system is valid
        if (!pimpl->gui_system) {
            std::cerr << "ImGuiThemeManager Error: GUI system is not initialized. Cannot apply theme." << std::endl;
            return false;
        }

        // 3. Delegate the work to the theme object
        theme_to_apply->apply(pimpl->gui_system);

        // 4. Apply the icon theme
        ImGuiTheme* imgui_theme = dynamic_cast<ImGuiTheme*>(theme_to_apply);
        if (imgui_theme && pimpl->icon_manager) {
            ImGuiThemeData* data = imgui_theme->get_data();
            if (data) {
                for (const auto& [type_name, path] : data->icon_paths) {
                    pimpl->icon_manager->update_icon(type_name, path);
                }
            }
        }

        // 5. Update the manager's internal state
        pimpl->active_theme = theme_to_apply;
        pimpl->active_theme_name = theme_name;
        
        // 6. Re-apply the current UI scale on top of the new theme's base styles
        apply_style_scale(pimpl->current_scale);

        std::cout << "ImGuiThemeManager: Theme '" << theme_name << "' applied." << std::endl;
        return true;
    }



    std::vector<std::string> ImGuiThemeManager::get_registered_themes() const {
        std::vector<std::string> names;
        for (const auto& [key, _] : pimpl->theme_registry)
            names.push_back(key);
        return names;
    }

    ITheme* ImGuiThemeManager::get_theme(const std::string& theme_name) const {
        auto it = pimpl->theme_registry.find(theme_name);
        return (it != pimpl->theme_registry.end()) ? it->second.get() : nullptr;
    }



    bool ImGuiThemeManager::load_internal_theme_from_file(const std::string& file_path, bool apply_immediately) {
        if (!FileManager::path_exists(file_path)) {
            std::cerr << "ImGuiThemeManager Error: Theme file not found at '" << file_path << "'" << std::endl;
            return false; // File doesn't exist
        }

        // Create a new ImGuiTheme instance.
        // Its constructor will create its ImGuiThemeData with default values.
        std::unique_ptr<ImGuiTheme> new_theme = std::make_unique<ImGuiTheme>(); 

        // Delegate the actual loading of data from YAML to the ImGuiTheme object.
        if (!new_theme->load_from_yaml(file_path)) {
            std::cerr << "ImGuiThemeManager Error: Failed to load theme data from YAML file '" << file_path << "'" << std::endl;
            return false;
        }

        // The theme's name is now loaded from the YAML file, so we can use it for registration.
        const std::string& theme_name = new_theme->get_name();

        // If a theme with this name already exists, unregister it first (optional, but good practice)
        if (pimpl->theme_registry.count(theme_name)) {
            std::cout << "ImGuiThemeManager: Overwriting existing theme '" << theme_name << "'." << std::endl;
            unregister_theme(theme_name); // Unregister the old one
        }

        // Register the newly loaded theme with the manager.
        if (!register_theme(std::move(new_theme))) {
            std::cerr << "ImGuiThemeManager Error: Failed to register loaded theme '" << theme_name << "'." << std::endl;
            return false;
        }

        std::cout << "ImGuiThemeManager: Loaded theme '" << theme_name << "' from file '" << file_path << "'" << std::endl;

        // Apply the theme immediately if requested.
        if (apply_immediately) {
            return apply_theme(theme_name); // This will apply the newly loaded theme's properties
        }

        return true;
    }

    bool ImGuiThemeManager::load_theme_from_file(const std::string& theme_name, const std::string& file_path, bool apply_immediately) {
        if (!FileManager::path_exists(file_path)) {
            std::cerr << "ImGuiThemeManager Error: Theme file not found at '" << file_path << "'" << std::endl;
            return false; // File doesn't exist
        }

        // Create a new ImGuiTheme instance.
        // Its constructor will create its ImGuiThemeData with default values.
        std::unique_ptr<ImGuiTheme> new_theme = std::make_unique<ImGuiTheme>(theme_name, file_path); 

        // Delegate the actual loading of data from YAML to the ImGuiTheme object.
        if (!new_theme->load_from_yaml(file_path)) {
            std::cerr << "ImGuiThemeManager Error: Failed to load theme data from YAML file '" << file_path << "'" << std::endl;
            new_theme.reset();
            return false;
        }


        // If a theme with this name already exists, unregister it first (optional, but good practice)
        if (pimpl->theme_registry.count(theme_name)) {
            std::cout << "ImGuiThemeManager: Overwriting existing theme '" << theme_name << "'." << std::endl;
            unregister_theme(theme_name); // Unregister the old one
        }

        // Register the newly loaded theme with the manager.
        if (!register_theme(std::move(new_theme))) {
            std::cerr << "ImGuiThemeManager Error: Failed to register loaded theme '" << theme_name << "'." << std::endl;
            new_theme.reset();
            return false;
        }

        std::cout << "ImGuiThemeManager: Loaded theme '" << theme_name << "' from file '" << file_path << "'" << std::endl;

        // Apply the theme immediately if requested.
        if (apply_immediately) {
            return apply_theme(theme_name); // This will apply the newly loaded theme's properties
        }

        return true;
    }





    bool ImGuiThemeManager::load_themes_from_directory(const std::string& directory_path) {
        if (!FileManager::path_exists(directory_path)) return false;

        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".theme") {
                load_internal_theme_from_file(entry.path().string());
            }
        }
        return true;
    }

    bool ImGuiThemeManager::set_active_theme(const std::string& theme_name) {
        ITheme* active_theme = get_theme(theme_name);
        if (!active_theme) { return false; }
        pimpl->active_theme = active_theme;
        pimpl->active_theme_name = theme_name;
        return true;
        
    }

    ITheme* ImGuiThemeManager::get_active_theme() {
        return pimpl->active_theme;
    }

    bool ImGuiThemeManager::purge_theme(const std::string& theme_name) {
        if (pimpl->active_theme_name == theme_name)
            pimpl->active_theme_name.clear();
        return unregister_theme(theme_name);
    }


    bool ImGuiThemeManager::save_theme_to_file(const std::string& theme_name, const std::string& file_path) {
   
        if (file_path.empty()) {
            std::cerr << "ImGuiThemeManager Error: YAML path is empty for saving theme." << std::endl;
            return false;
        }

        // Find the theme by name in the registry.
        // This method's purpose is to save the theme IDENTIFIED BY theme_name.
        auto it = pimpl->theme_registry.find(theme_name);
        if (it == pimpl->theme_registry.end()) {
            std::cerr << "ImGuiThemeManager Error: Theme '" << theme_name << "' not found in registry for saving." << std::endl;
            return false;
        }

        // Cast to ImGuiTheme to access its save_to_yaml method.
        // (it->second.get() returns an ITheme*, so we dynamic_cast to ImGuiTheme*)
        ImGuiTheme* theme_to_save = dynamic_cast<ImGuiTheme*>(it->second.get());
        if (!theme_to_save) {
            std::cerr << "ImGuiThemeManager Error: Registered theme '" << theme_name << "' is not an ImGuiTheme. Cannot save." << std::endl;
            return false;
        }

        // Delegate the actual saving to the ImGuiTheme object.
        if (!theme_to_save->save_to_yaml(file_path)) {
            std::cerr << "ImGuiThemeManager Error: Failed to save theme '" << theme_name << "' to file '" << file_path << "'." << std::endl;
            return false;
        }

        std::cout << "ImGuiThemeManager: Saved theme '" << theme_name << "' to file '" << file_path << "'" << std::endl;
        return true;
    }


} // namespace Salix
