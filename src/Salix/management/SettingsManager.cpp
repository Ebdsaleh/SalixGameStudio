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

    bool write_cache(const std::string& cache_path, const ApplicationConfig& config);
    bool read_cache(const std::string& cache_path, ApplicationConfig& out_config);

    // Helper function to convert YAML node to AppStateType enum
    void parse_app_state_type(const YAML::Node& node, AppStateType& type) {
        if (!node) return; // Do nothing if the node doesn't exist
        std::string val = node.as<std::string>();
        if (val == "Launch") type = AppStateType::Launch;
        else if (val == "Editor") type = AppStateType::Editor;
        else if (val == "Game") type = AppStateType::Game;
        else if (val == "Options") type = AppStateType::Options;
    }
    
    // Helper function to convert YAML node to RendererType enum
    void parse_renderer_type(const YAML::Node& node, RendererType& type) {
        if (!node) return; // Do nothing if the node doesn't exist
        std::string val = node.as<std::string>();
        if (val == "SDL") type = RendererType::SDL;
        // FIX: Add OpenGL renderer type parsing
        else if (val == "OpenGL") type = RendererType::OpenGL; 
        // Add other renderer types here in the future
    }
    
    // Helper function to convert YAML node to GuiType enum
    void parse_gui_type(const YAML::Node& node, GuiType& type) {
        if (!node) return;
        std::string val = node.as<std::string>();
        if (val == "ImGui") type = GuiType::ImGui;
        else if (val == "None") type = GuiType::None;
    }

    // Helper function to convert YAML node to TimerType enum
    void parse_timer_type(const YAML::Node& node, TimerType& type) {
        if (!node) return;
        std::string val = node.as<std::string>();
        if (val == "SDL") type = TimerType::SDL;
        else if (val == "Chrono") type = TimerType::Chrono;
    }

    bool SettingsManager::load_settings(const std::string& yaml_path, ApplicationConfig& out_config) {
        std::string cache_path = get_cache_path(yaml_path);

        if (std::filesystem::exists(cache_path) && std::filesystem::exists(yaml_path) &&
            std::filesystem::last_write_time(cache_path) >= std::filesystem::last_write_time(yaml_path))
        {
            // --- FAST PATH: Load from SAFE binary cache ---
            if (read_cache(cache_path, out_config)) {
                std::cout << "SettingsManager: Loaded settings from fast binary cache '" << cache_path << "'" << std::endl;
                return true;
            }
        }

        // --- SLOW PATH: Load from YAML and create cache ---
        if (!std::filesystem::exists(yaml_path)) {
            std::cerr << "SettingsManager Warning: YAML config file not found at '" << yaml_path << "'. Using default settings." << std::endl;
            return true;
        }

        try {
            YAML::Node root = YAML::LoadFile(yaml_path);

            // --- Full YAML Parsing Logic ---
            if (root["Window"]) {
                if (root["Window"]["title"])  out_config.window_config.title  = root["Window"]["title"].as<std::string>();
                if (root["Window"]["width"])  out_config.window_config.width  = root["Window"]["width"].as<int>();
                if (root["Window"]["height"]) out_config.window_config.height = root["Window"]["height"].as<int>();
            }
            if (root["Engine"]) {
                parse_app_state_type(root["Engine"]["initial_state"], out_config.initial_state);
                if (root["Engine"]["target_fps"]) out_config.target_fps = root["Engine"]["target_fps"].as<int>();
            }
            if (root["Renderer"]) {
                parse_renderer_type(root["Renderer"]["type"], out_config.renderer_type);
            }
            if (root["GUI"]) {
                parse_gui_type(root["GUI"]["type"], out_config.gui_type);
                if (root["GUI"]["dialog_width_ratio"]) {
                    out_config.gui_settings.dialog_width_ratio = root["GUI"]["dialog_width_ratio"].as<float>();
                }
                if (root["GUI"]["dialog_height_ratio"]) {
                    out_config.gui_settings.dialog_height_ratio = root["GUI"]["dialog_height_ratio"].as<float>();
                }
                if (root["GUI"]["font_scaling"])
                {
                    out_config.gui_settings.font_scaling = root["GUI"]["font_scaling"].as<float>();
                }
                if (root["GUI"]["global_dpi_scaling"])
                {
                    out_config.gui_settings.global_dpi_scaling = root["GUI"]["global_dpi_scaling"].as<float>();
                }
            }
            if (root["Timer"]) {
                parse_timer_type(root["Timer"]["type"], out_config.timer_type);
            }
            
            std::cout << "SettingsManager: Loaded settings from YAML file '" << yaml_path << "'" << std::endl;

            // --- Create the SAFE binary cache ---
            if (write_cache(cache_path, out_config)) {
                std::cout << "SettingsManager: Created binary cache '" << cache_path << "'" << std::endl;
            }

        } catch (const YAML::Exception& e) {
            std::cerr << "SettingsManager Error: Failed to parse YAML file '" << yaml_path << "'. Error: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

   bool SettingsManager::save_settings(const std::string& yaml_path, const ApplicationConfig& config)
    {
        if (yaml_path.empty()) { 
            std::cerr << "SettingsManager Error: YAML path is empty for saving settings." << std::endl;
            return false; 
        }

        try {
            YAML::Emitter emitter;
            emitter << YAML::BeginMap;

            // Window Settings
            emitter << YAML::Key << "Window";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "title" << YAML::Value << config.window_config.title;
            emitter << YAML::Key << "width" << YAML::Value << config.window_config.width;
            emitter << YAML::Key << "height" << YAML::Value << config.window_config.height;
            emitter << YAML::EndMap;

            // Engine Settings
            emitter << YAML::Key << "Engine";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "initial_state" << YAML::Value;
            // Convert enum to string for YAML output
            switch (config.initial_state) {
                case AppStateType::Launch:  emitter << "Launch"; break;
                case AppStateType::Editor:  emitter << "Editor"; break;
                case AppStateType::Game:    emitter << "Game"; break;
                case AppStateType::Options: emitter << "Options"; break;
                default: emitter << "Launch"; break; // Default case
            }
            emitter << YAML::Key << "target_fps" << YAML::Value << config.target_fps;
            emitter << YAML::EndMap;

            // Renderer Settings
            emitter << YAML::Key << "Renderer";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "type" << YAML::Value;
            switch (config.renderer_type) {
                case RendererType::SDL:    emitter << "SDL"; break;
                case RendererType::OpenGL: emitter << "OpenGL"; break; // FIX: Added OpenGL type
                default: emitter << "SDL"; break; // Default case
            }
            emitter << YAML::EndMap;

            // GUI Settings
            emitter << YAML::Key << "GUI";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "type" << YAML::Value;
            switch (config.gui_type) {
                case GuiType::ImGui: emitter << "ImGui"; break;
                case GuiType::None:  emitter << "None"; break;
                default: emitter << "ImGui"; break; // Default case
            }
            emitter << YAML::Key << "dialog_width_ratio" << YAML::Value << config.gui_settings.dialog_width_ratio;
            emitter << YAML::Key << "dialog_height_ratio" << YAML::Value << config.gui_settings.dialog_height_ratio;
            emitter << YAML::Key << "font_scaling" << YAML::Value << config.gui_settings.font_scaling;
            emitter << YAML::Key << "global_dpi_scaling" << YAML::Value << config.gui_settings.global_dpi_scaling;
            emitter << YAML::EndMap;

            // Timer Settings
            emitter << YAML::Key << "Timer";
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "type" << YAML::Value;
            switch (config.timer_type) {
                case TimerType::SDL:    emitter << "SDL"; break;
                case TimerType::Chrono: emitter << "Chrono"; break;
                default: emitter << "SDL"; break; // Default case
            }
            emitter << YAML::EndMap;

            emitter << YAML::EndMap; // End root map

            // Write to file
            std::ofstream file(yaml_path);
            if (!file.is_open()) {
                std::cerr << "SettingsManager Error: Could not open file for writing: '" << yaml_path << "'" << std::endl;
                return false;
            }
            file << emitter.c_str();
            file.close();

            std::cout << "SettingsManager: Saved settings to YAML file '" << yaml_path << "'" << std::endl;
            return true;

        } catch (const YAML::Exception& e) {
            std::cerr << "SettingsManager Error: Failed to save YAML file '" << yaml_path << "'. Error: " << e.what() << std::endl;
            return false;
        }
    }
 

    std::string SettingsManager::get_cache_path(const std::string& yaml_path) const
    {
        return std::filesystem::path(yaml_path).replace_extension(".cache").string();
    }

    // --- Private Helper Functions for Safe Caching ---

    // Helper to write a std::string safely to a binary stream
    void write_string(std::ofstream& stream, const std::string& str) {
        size_t len = str.length();
        stream.write(reinterpret_cast<const char*>(&len), sizeof(len));
        stream.write(str.c_str(), len);
    }

    // Helper to read a std::string safely from a binary stream
    void read_string(std::ifstream& stream, std::string& str) {
        size_t len;
        stream.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::vector<char> buffer(len);
        stream.read(buffer.data(), len);
        str.assign(buffer.data(), len);
    }

    // Writes the entire config to a binary cache file
    bool write_cache(const std::string& cache_path, const ApplicationConfig& config) {
        std::ofstream out(cache_path, std::ios::binary);
        if (!out.is_open()) return false;

        // Write simple types directly
        out.write(reinterpret_cast<const char*>(&config.window_config.width), sizeof(config.window_config.width));
        out.write(reinterpret_cast<const char*>(&config.window_config.height), sizeof(config.window_config.height));
        out.write(reinterpret_cast<const char*>(&config.renderer_type), sizeof(config.renderer_type));
        out.write(reinterpret_cast<const char*>(&config.initial_state), sizeof(config.initial_state));
        out.write(reinterpret_cast<const char*>(&config.gui_type), sizeof(config.gui_type));
        out.write(reinterpret_cast<const char*>(&config.timer_type), sizeof(config.timer_type));
        out.write(reinterpret_cast<const char*>(&config.target_fps), sizeof(config.target_fps));
        out.write(reinterpret_cast<const char*>(&config.gui_settings), sizeof(config.gui_settings));

        // Write string safely
        write_string(out, config.window_config.title);

        return true;
    }

    // Reads the entire config from a binary cache file
    bool read_cache(const std::string& cache_path, ApplicationConfig& out_config) {
        std::ifstream in(cache_path, std::ios::binary);
        if (!in.is_open()) return false;

        // Read simple types directly
        in.read(reinterpret_cast<char*>(&out_config.window_config.width), sizeof(out_config.window_config.width));
        in.read(reinterpret_cast<char*>(&out_config.window_config.height), sizeof(out_config.window_config.height));
        in.read(reinterpret_cast<char*>(&out_config.renderer_type), sizeof(out_config.renderer_type));
        in.read(reinterpret_cast<char*>(&out_config.initial_state), sizeof(out_config.initial_state));
        in.read(reinterpret_cast<char*>(&out_config.gui_type), sizeof(out_config.gui_type));
        in.read(reinterpret_cast<char*>(&out_config.timer_type), sizeof(out_config.timer_type));
        in.read(reinterpret_cast<char*>(&out_config.target_fps), sizeof(out_config.target_fps));
        in.read(reinterpret_cast<char*>(&out_config.gui_settings), sizeof(out_config.gui_settings));

        // Read string safely
        read_string(in, out_config.window_config.title);

        return true;
    }

} // namespace Salix