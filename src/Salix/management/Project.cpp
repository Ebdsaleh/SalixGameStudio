// Salix/management/Project.cpp
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>
#include <Salix/management/FileManager.h>
#include <Salix/management/ProjectConfig.h>
#include <Salix/core/SerializationRegistrations.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

namespace Salix {
    struct Project::Pimpl {
        std::unique_ptr<SceneManager> scene_manager; // runtime, does not get serialized.
        std::string name;
        std::string root_path;
        std::string project_file_name;
        std::string starting_scene;
        std::map<std::string, std::string> scene_paths;

        // --- Build Settings (to match BuildSettings in ProjectConfig) ---
        std::string engine_version;
        std::string game_dll_name;

        Pimpl() = default;
        // define the Pimpl constructor
        // Pimpl's serialization function (already here or in .h, make sure it's correct)
        template<class Archive>
        void serialize(Archive & archive) {
            // Serialize all persistent data members of Pimpl
            archive(cereal::make_nvp("name", name));
            archive(cereal::make_nvp("root_path", root_path));
            archive(cereal::make_nvp("project_file_name", project_file_name)); 
            archive(cereal::make_nvp("scene_paths", scene_paths));
            archive(cereal::make_nvp("starting_scene", starting_scene));
            archive(cereal::make_nvp("engine_version", engine_version));
            archive(cereal::make_nvp("game_dll_name", game_dll_name));
            // Don't serialize AssetManager* (raw pointer to external object)
            // Don't serialize scene_manager directly here in Pimpl,
            // as it's typically managed/created at runtime (as per the flow).
            // If you did want to serialize scene_manager itself, it would also need
            // its full type visible.
        }
            
    };

    // Constructor and Destructor
    Project::Project() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = "TestProject";
        pimpl->root_path = "Sandbox/TestProject";
        pimpl->project_file_name = pimpl->name + ".salixproj"; // <--- ADD THIS (derive from name)
        pimpl->engine_version = "0.1.0-Alpha"; // <--- ADD THIS (default, will be overwritten by loaded_config in PM)
        pimpl->game_dll_name = pimpl->name + ".dll"; // <--- ADD THIS (derive, will be overwritten)
    }
    
    Project::Project(const std::string& project_name, const std::string& project_root_path) 
        : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = project_name;
        pimpl->root_path = project_root_path;
        pimpl->project_file_name = project_name + ".salixproj"; // <--- ADD THIS (derive from name)
        pimpl->engine_version = "0.1.0-Alpha"; // <--- ADD THIS (default, will be overwritten by loaded_config in PM)
        pimpl->game_dll_name = project_name + ".dll"; // <--- ADD THIS (derive, will be overwritten)
    }

    Project::~Project() = default;

    void Project::initialize(AssetManager* asset_manager) {
        std::cout << "Project Initializing..." << std::endl;
        pimpl->scene_manager = std::make_unique<SceneManager>();
        pimpl->scene_manager->initialize(asset_manager);
        pimpl->scene_manager->set_project_root_path(pimpl->root_path);

        // --- NEW: POPULATE SCENEMANAGER WITH LIGHTWEIGHT SCENE SHELLS ---
        // Iterate through all scene paths known by this Project object.
        for (const auto& pair : pimpl->scene_paths) { // pair.first is name, pair.second is path
            const std::string& scene_name = pair.first;
            const std::string& scene_relative_path = pair.second;
            // This automatically adds the new Scene to the SceneManager's scene_list
            pimpl->scene_manager->create_scene(scene_name, scene_relative_path);
        }
        // Check that 'starting_scene' is not an empty string.
        if (!pimpl->starting_scene.empty() ) {
                // Call SceneManager::set_active_scene. This method will:
                // 1. Find the scene shell in its scene_list using its name (e.g., "Default").
                // 2. Set it as active.
                // 3. Crucially, call Scene::on_load() for the active scene,
                // which will then trigger Scene::load_content_from_file().
                pimpl->scene_manager->set_active_scene(pimpl->starting_scene);
       } else {
        std::cerr << "Project Error: No starting scene specified in project config!" << std::endl;
        // Handle error: perhaps set a default active scene, or display a critical error.
}
    }

    void Project::shutdown() {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->shutdown();
            pimpl->scene_manager.reset();
        }
    }

    void Project::update(float delta_time) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->update(delta_time);
        }
    }

    void Project::render(IRenderer* renderer) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->render(renderer);
        }
    }

    void Project::set_starting_scene(const std::string& scene_name) {
        pimpl->starting_scene = scene_name;
    }
    
    void Project::add_scene_path(const std::string& scene_name, const std::string& path_to_scene_file) {
        pimpl->scene_paths[scene_name] = path_to_scene_file; // Store in the map
    }

    bool Project::remove_scene_path(const std::string& path_to_scene_file) {
        if(!FileManager::path_exists(path_to_scene_file)) { // This check is for the file itself, not the map entry
            std::cerr << "Project::remove_scene_path - Failed to remove scene path: '" <<
            path_to_scene_file << "', file does not exist on disk." << std::endl;
            // Optionally, still attempt to remove from map even if file doesn't exist on disk,
            // if this method means "remove from project list".
        }

        // Iterate the map to find the entry by value (path) and remove by key (name)
        for (auto it = pimpl->scene_paths.begin(); it != pimpl->scene_paths.end(); ++it) {
            if (it->second == path_to_scene_file) {
                pimpl->scene_paths.erase(it); // Erase by iterator
                std::cout << "Project: Removed scene path '" << path_to_scene_file << "' from project." << std::endl;
                return true; // Scene path found and removed from map
            }
        }
        std::cerr << "Project::remove_scene_path - Scene path '" << path_to_scene_file << "' not found in project map." << std::endl;
        return false; // Scene path not found in the map
    }
    
    // --- Public Getter Methods ---
    SceneManager* Project::get_scene_manager() const {
        return pimpl->scene_manager.get();
    }

    const std::string& Project::get_name() const {

        return pimpl->name;
    }
    const std::string& Project::get_path() const {
        return pimpl->root_path;
    }
    const std::string& Project::get_starting_scene() const {
        return pimpl->starting_scene;
    }
    const std::map<std::string, std::string>& Project::get_scene_paths() const {
        return pimpl->scene_paths;
    }

    const std::string& Project::get_engine_version() const {
        return pimpl->engine_version;
    }
    
    const std::string& Project::get_game_dll_name() const {
        return pimpl->game_dll_name;
    }
    
    const std::string& Project::get_project_file_name() const {
        return pimpl->project_file_name;
    }

    template<class Archive>
    void Project::serialize(Archive & archive) {
        archive (
            cereal::make_nvp("PimplData", pimpl)
        );
    }

    template void Project::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& );
    template void Project::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Project::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Project::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);

} // namespace Salix