// Salix/management/ProjectConfig.h
#pragma once

#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/access.hpp> // Needed for friend cereal::access

namespace Salix {

    // Helper struct for realms within the project config (RealmInfo remains the same)
    struct RealmInfo {
        std::string name;
        std::string path; // Relative path to the realm file

        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("path", path)
            );
        }

        RealmInfo() = default;
        RealmInfo(const std::string& n, const std::string& p) : name(n), path(p) {}
    };

    // Helper struct for build settings (BuildSettings remains the same)
    struct BuildSettings {
        std::string engine_version = "0.1.0-Alpha"; // Hardcoded as agreed
        std::string game_dll_name;

        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                cereal::make_nvp("engine_version", engine_version),
                cereal::make_nvp("game_dll_name", game_dll_name)
            );
        }

        BuildSettings() = default;
        // The custom constructor was removed for simplicity and to avoid issues.
        // BuildSettings(const std::string& ev, const std::string& gdn) : engine_version(ev), game_dll_name(gdn) {}
    };

    // --- NEW STRUCT: Mirrors the "project_data" object in your JSON ---
    struct ProjectData {
        // These members will now be nested under "project_data" in the JSON
        std::string project_path;
        std::string project_file_name; // This will map to "project_file" in JSON
        std::string project_name;
        std::vector<RealmInfo> realms; // List of all realms in the project
        std::string starting_realm;    // Name of the realm to load on startup

        template<class Archive>
        void serialize(Archive & archive) {
            // Use cereal::make_nvp with an alias if the C++ member name differs from JSON key
            archive( cereal::make_nvp("project_path", project_path) );
            archive( cereal::make_nvp("project_file", project_file_name) ); // <-- ALIAS for "project_file" in JSON
            archive( cereal::make_nvp("project_name", project_name) );
            archive( cereal::make_nvp("realms", realms) );
            archive( cereal::make_nvp("starting_realm", starting_realm) );
        }
        ProjectData() = default;
    };


    class ProjectConfig {
    public:
        // --- Core Project Data (now within a nested struct) ---
        ProjectData project_data; // <--- NEW: Member of the new struct type

        // --- Build Settings ---
        BuildSettings build_settings; // Remains the same

        // Required for deserialization
        ProjectConfig() = default;

    private:
        friend class cereal::access;

        template<class Archive>
        void serialize(Archive & archive) {
            // Serialize the nested structs, matching the JSON keys
            archive(
                cereal::make_nvp("project_data", project_data),
            
                cereal::make_nvp("build_settings", build_settings) 
            );
        }
    };

} // namespace Salix