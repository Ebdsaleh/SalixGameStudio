// =================================================================================
// Filename:    Salix/mangement/SettingsManager.h
// Author:      SalixGameStudio
// Description: Manages loading and saving of application settings from a file.
//              Uses a YAML file for user-facing configuration and creates a
//              binary cache for fast runtime loading.
// =================================================================================
#pragma once
#include <Salix/core/Core.h>
#include <string>

// Forward declare the ApplicationConfig struct
namespace Salix {
    struct ApplicationConfig;
}

namespace Salix {

    class SALIX_API SettingsManager
    {
    public:
        SettingsManager() = default;

        // Loads settings for the application.
        // It first tries to load from a fast binary cache. If the cache is
        // missing or outdated, it loads from the user-friendly YAML file
        // and creates a new cache.
        // @param yaml_path: The path to the user-facing .yaml config file.
        // @param out_config: The config struct to be populated.
        // @return: True if settings were successfully loaded, false otherwise.
        bool load_settings(const std::string& yaml_path, ApplicationConfig& out_config);

        // Saves the given configuration to the YAML file.
        // This would be used by an in-engine settings editor.
        // @param yaml_path: The path to the user-facing .yaml config file.
        // @param config: The config struct to save.
        // @return: True if settings were successfully saved, false otherwise.
        bool save_settings(const std::string& yaml_path, const ApplicationConfig& config);

    private:
        // Generates the path for the binary cache file from the YAML path.
        std::string get_cache_path(const std::string& yaml_path) const;
    };

} // namespace Salix