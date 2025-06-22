// Salix/management/Project.cpp
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>
#include <Salix/management/FileManager.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace Salix {
    struct Project::Pimpl {
        std::unique_ptr<SceneManager> scene_manager;
        std::string name;
        std::string root_path;
        std::string starting_scene;
        std::vector<std::string> scene_paths;
    };
    Project::Project() : pimpl(std::make_unique<Pimpl>()) {}
    Project::~Project() = default;

    void Project::initialize(AssetManager* asset_manager) {
        std::cout << "Project Initializing..." << std::endl;
        pimpl->scene_manager = std::make_unique<SceneManager>();
        pimpl->scene_manager->initialize(asset_manager);
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
    
    void Project::add_scene_path(const std::string& path_to_scene_file) {
        pimpl->scene_paths.push_back(path_to_scene_file);
    }

    bool Project::remove_scene_path(const std::string& path_to_scene_file) {
        if(!FileManager::path_exists(path_to_scene_file)) {
            std::cerr << "Project::remove_scene_path - Failed to remove scene path: '" <<
             path_to_scene_file << "', does not exist!" << std::endl;
            return false;
        }
        auto& scene_paths = pimpl->scene_paths;
        // Check the size before and after to see if an element was removed.
        const auto original_size = scene_paths.size();
        
        // The "erase-remove idiom":
        // 1. std::remove shuffles all elements that are NOT the one we want to remove
        //    to the beginning of the vector. It returns an iterator to the new "end".
        // 2. .erase() then chops off the garbage elements at the end.
        scene_paths.erase(
            std::remove(scene_paths.begin(), scene_paths.end(), path_to_scene_file),
            scene_paths.end()
        );
        // If the new size is less than the original, it means we successfully removed something.
        return scene_paths.size() < original_size;

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
    const std::vector<std::string>& Project::get_scene_paths() const {
        return pimpl->scene_paths;
    }
    } // namespace Salix