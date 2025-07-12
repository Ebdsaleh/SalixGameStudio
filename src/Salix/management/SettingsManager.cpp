// =================================================================================
// Filename:    Salix/management/SettingsManager.cpp
// Author:      SalixGameStudio
// Description: Implementation of the SettingsManager class.
// =================================================================================
#include <Salix/management/SettingsManager.h>
#include <Salix/core/ApplicationConfig.h>


// For file I/O and checking timestamps
#include <fstream>
#include <filesystem> // Requires C++17
#include <iostream>

// YAML parsing library
#include <yaml-cpp/yaml.h>


namespace Salix {

    // Helper function to convert YAML node to AppStateType enum
    void parseAppStateType(const YAML::Node& node, AppStateType& type) {
        if (!node) return; // Do nothing if the node doesn't exist
        std::string val = node.as<std::string>();
        if (val == "Launch") type = AppStateType::Launch;
        else if (val == "Editor") type = AppStateType::Editor;
        else if (val == "Game") type = AppStateType::Game;
        else if (val == "Options") type = AppStateType::Options;
    }
    
    // Helper function to convert YAML node to RendererType enum
    void parseRendererType(const YAML::Node& node, RendererType& type) {
        if (!node) return; // Do nothing if the node doesn't exist
        std::string val = node.as<std::string>();
        if (val == "SDL") type = RendererType::SDL;
        // Add other renderer types here in the future
    }
    
    // Helper function to convert YAML node to GuiType enum
    void parseGuiType(const YAML::Node& node, GuiType& type) {
        if (!node) return;
        std::string val = node.as<std::string>();
        if (val == "ImGui") type = GuiType::ImGui;
        else if (val == "None") type = GuiType::None;
    }

    // Helper function to convert YAML node to TimerType enum
    void parseTimerType(const YAML::Node& node, TimerType& type) {
        if (!node) return;
        std::string val = node.as<std::string>();
        if (val == "SDL") type = TimerType::SDL;
        else if (val == "Chrono") type = TimerType::Chrono;
    }

    bool SettingsManager::loadSettings(const std::string& yaml_path, ApplicationConfig& out_config)
    {
        std::string cache_path = getCachePath(yaml_path);

        // Check if cache is valid and up-to-date
        if (std::filesystem::exists(cache_path) && std::filesystem::exists(yaml_path) &&
            std::filesystem::last_write_time(cache_path) >= std::filesystem::last_write_time(yaml_path))
        {
            // --- FAST PATH: Load from binary cache ---
            std::ifstream in(cache_path, std::ios::binary);
            if (in.is_open()) {
                in.read(reinterpret_cast<char*>(&out_config), sizeof(ApplicationConfig));
                in.close();
                std::cout << "SettingsManager: Loaded settings from fast binary cache '" << cache_path << "'" << std::endl;
                return true;
            }
        }

        // --- SLOW PATH: Load from YAML and create cache ---
        if (!std::filesystem::exists(yaml_path)) {
            std::cerr << "SettingsManager Warning: YAML config file not found at '" << yaml_path << "'. Using default settings." << std::endl;
            // We can still succeed here by using the default-constructed out_config
            return true;
        }

        try {
            YAML::Node root = YAML::LoadFile(yaml_path);

            // --- CORRECTED PARSING LOGIC ---
            if (root["Window"]) {
                if (root["Window"]["title"])  out_config.window_config.title  = root["Window"]["title"].as<std::string>();
                if (root["Window"]["width"])  out_config.window_config.width  = root["Window"]["width"].as<int>();
                if (root["Window"]["height"]) out_config.window_config.height = root["Window"]["height"].as<int>();
            }
            if (root["Engine"]) {
                parseAppStateType(root["Engine"]["initial_state"], out_config.initial_state);
                if (root["Engine"]["target_fps"]) out_config.target_fps = root["Engine"]["target_fps"].as<int>();
            }
            if (root["Renderer"]) {
                parseRendererType(root["Renderer"]["type"], out_config.renderer_type);
            }
            if (root["GUI"]) {
                parseGuiType(root["GUI"]["type"], out_config.gui_type);
            }
            if (root["Timer"]) {
                parseTimerType(root["Timer"]["type"], out_config.timer_type);
            }
            
            std::cout << "SettingsManager: Loaded settings from YAML file '" << yaml_path << "'" << std::endl;

            // Now, create the binary cache for next time
            std::ofstream out(cache_path, std::ios::binary);
            if (out.is_open()) {
                out.write(reinterpret_cast<const char*>(&out_config), sizeof(ApplicationConfig));
                out.close();
                std::cout << "SettingsManager: Created binary cache '" << cache_path << "'" << std::endl;
            }

        } catch (const YAML::Exception& e) {
            std::cerr << "SettingsManager Error: Failed to parse YAML file '" << yaml_path << "'. Error: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    bool SettingsManager::saveSettings(const std::string& yaml_path, const ApplicationConfig& config)
    {
        // (Implementation omitted for brevity)
        std::cout << "SettingsManager: saveSettings() is not yet implemented." << std::endl;
        return false;
    }

    std::string SettingsManager::getCachePath(const std::string& yaml_path) const
    {
        return std::filesystem::path(yaml_path).replace_extension(".cache").string();
    }

} // namespace Salix