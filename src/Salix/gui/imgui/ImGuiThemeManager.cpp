// Salix/gui/imgui/ImGuiThemeManager.cpp

#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <Salix/management/FileManager.h>

 

namespace Salix {

    // ---------------------- ImGuiThemeManager ----------------------
    struct ImGuiThemeManager::Pimpl {
        IGui* gui_ref = nullptr;
        std::unordered_map<std::string, std::unique_ptr<ITheme>> theme_registry;
        std::string active_theme_name;
    };

    ImGuiThemeManager::ImGuiThemeManager() : pimpl(std::make_unique<Pimpl>()) {}
    ImGuiThemeManager::~ImGuiThemeManager() = default;
    bool ImGuiThemeManager::initialize(IGui* gui_system) {
        pimpl->gui_ref = gui_system;
        return true;
    }

    void ImGuiThemeManager::shutdown() {
        pimpl->theme_registry.clear();
        pimpl->gui_ref = nullptr;
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

    bool ImGuiThemeManager::apply_theme(const std::string& theme_name) {
        auto it = pimpl->theme_registry.find(theme_name);
        if (it == pimpl->theme_registry.end())
            return  false;
        if (it->second->apply(pimpl->gui_ref)) {
            pimpl->active_theme_name = theme_name;
            return true;
        }
        return false;
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

    // Not implemented yet as serialization format is undecided (e.g., JSON, YAML, custom, etc.).
    bool ImGuiThemeManager::load_theme_from_file(const std::string& file_path, bool apply_immediately) {
        if (!FileManager::path_exists(file_path)) {
            return false; // File doesn't exist
        }
        if(apply_immediately) {}


        // TODO: Implement theme deserialization when format is chosen
        // For now, this stub always fails.
        return false;
    }

    bool ImGuiThemeManager::load_themes_from_directory(const std::string& directory_path) {
        if (!FileManager::path_exists(directory_path)) return false;

        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".theme") {
                load_theme_from_file(entry.path().string());
            }
        }
        return true;
    }

    bool ImGuiThemeManager::set_active_theme(const std::string& theme_name) {
        return apply_theme(theme_name);
    }

    bool ImGuiThemeManager::purge_theme(const std::string& theme_name) {
        if (pimpl->active_theme_name == theme_name)
            pimpl->active_theme_name.clear();
        return unregister_theme(theme_name);
    }

} // namespace Salix
