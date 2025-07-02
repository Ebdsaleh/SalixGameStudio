// Salix/management/SceneManager.cpp
#include <Salix/core/InitContext.h>
#include <Salix/management/SceneManager.h>
#include <Salix/management/FileManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Scene.h>
#include <Salix/assets/AssetManager.h>
#include <iostream>
#include <algorithm>
#include <fstream>    // For std::ifstream
#include <sstream>    // For std::stringstream
#include <filesystem> // For std::filesystem::path
// Cereal headers for Scene loading
#include <cereal/archives/json.hpp>    // If scenes are JSON (for debug)
#include <cereal/archives/binary.hpp>  // If scenes are binary (recommended)
#include <cereal/types/string.hpp>     // Scene name, etc.
#include <cereal/types/vector.hpp>     // Vector of entities
#include <cereal/types/memory.hpp>     // Unique_ptr to entities
#include <cereal/types/polymorphic.hpp> // For Element polymorphism within Scene (Scene needs its types registered globally)



namespace Salix {

    struct SceneManager::Pimpl {
        std::vector<std::unique_ptr<Scene>> scene_list;
        Scene* active_scene = nullptr;
        InitContext context;
        std::string project_root_path;
    };
    SceneManager::SceneManager() : pimpl(std::make_unique<Pimpl>()) {}

    SceneManager::~SceneManager() = default;

    void SceneManager::initialize(const InitContext& new_context) {
        pimpl->context = new_context;
        std::cout<< "SceneManager Initialized." << std::endl;
    }

    void SceneManager::shutdown() {
        pimpl->scene_list.clear();
        pimpl->active_scene = nullptr;
    }

    void SceneManager::update(float delta_time) {
        if (pimpl->active_scene) {
            pimpl->active_scene->update(delta_time);
        }
    }

    void SceneManager::render(IRenderer* renderer) {
        if (pimpl->active_scene) {
            pimpl->active_scene->render(renderer);
        }
    }

    // --- API IMPLEMENTATIONS ---

    // This is useful as an Engine API call.
    Scene* SceneManager::create_scene(const std::string& scene_name) {
        if (get_scene(scene_name)) {
            std::cerr << "SceneManager: Scene '" << scene_name << "' already exists." << std::endl;
            return nullptr;
        }

        std::cout << "SceneManager: Creating new scene '" << scene_name << "'" << std::endl;
        auto new_scene_owner = std::make_unique<Scene>(scene_name);
        Scene* new_scene_ptr = new_scene_owner.get();
        pimpl->scene_list.push_back(std::move(new_scene_owner));
        return new_scene_ptr;
    }

    // Used by the Owning Project's pimpl->scene_manager while loading a Project.
    Scene* SceneManager::create_scene(const std::string& scene_name, const std::string& scene_relative_path) {
         if (get_scene(scene_name)) {
            std::cerr << "SceneManager: Scene '" << scene_name << "' already exists." << std::endl;
            return nullptr;
        }
        std::cout << "SceneManager: Creating new scene '" << scene_name << "'" << std::endl;
        auto new_scene_owner = std::make_unique<Scene>(scene_name, scene_relative_path);
        Scene* new_scene_ptr = new_scene_owner.get();
        pimpl->scene_list.push_back(std::move(new_scene_owner));
        return new_scene_ptr;
    }

    // This is useful as a 'user' API call.
    void SceneManager::add_scene(std::unique_ptr<Scene> scene_to_add) {
        if (!scene_to_add) return;
        if (get_scene(scene_to_add->get_name())) {
            std::cerr << "SceneManager: Scene '" << scene_to_add->get_name() << "' already exists." << std::endl;
            return;
        }
        std::cout << "SceneManager: Adding existing scene '" << scene_to_add->get_name() << "'" << std::endl;
        pimpl->scene_list.push_back(std::move(scene_to_add));
    }

    // This uses the find-by-name and remove pattern.
    void SceneManager::remove_scene(const std::string& scene_name) {
        auto it = std::remove_if(pimpl->scene_list.begin(), pimpl->scene_list.end(),
            [&](const std::unique_ptr<Scene>& scene) {
                bool should_remove = scene->get_name() == scene_name;
                if (should_remove && scene.get() == pimpl->active_scene) {
                    pimpl->active_scene = nullptr;
                }
                return should_remove;
            });
        pimpl->scene_list.erase(it, pimpl->scene_list.end());
    }

    // This uses a find-by-name and remove pattern but by passing in the Scene pointer.
    void SceneManager::remove_scene(Scene* scene_to_remove) {
        if (scene_to_remove) {
            remove_scene(scene_to_remove->get_name());
        }
    }

    // This uses a find-by-index and remove pattern.
    void SceneManager::remove_scene_at(int index) {
        if (index >= 0 && index < pimpl->scene_list.size()) {
            if (pimpl->scene_list[index].get() == pimpl->active_scene) {
                pimpl->active_scene = nullptr;
            }
            pimpl->scene_list.erase(pimpl->scene_list.begin() + index);
        } else {
            std::cerr << "SceneManager: Invalid index for remove_scene_at." << std::endl;
        }
    }

    bool SceneManager::set_active_scene(const std::string& scene_name) {
        Scene* scene = get_scene(scene_name);
        if (scene) {
            
            pimpl->active_scene = scene;
            // pimpl->active_scene->on_load(pimpl->asset_manager, pimpl->project_root_path);
            std::cout << "SceneManager: Set active scene to '" << scene_name << "'" << std::endl;
            return true;
        } else {
            std::cerr << "SceneManager: Could not find scene '" << scene_name << "' to set as active." << std::endl;
            return false;
        }
    }

    Scene* SceneManager::get_active_scene() const {
        return pimpl->active_scene;
    }

    // This uses as find-by-name and retrieve pattern.
    Scene* SceneManager::get_scene(const std::string& scene_name) const {
        auto it = std::find_if(pimpl->scene_list.begin(), pimpl->scene_list.end(), 
            [&](const auto& scene) { return scene->get_name() == scene_name; });
        return (it != pimpl->scene_list.end()) ? it->get() : nullptr;
    }

    // This uses a find-by-index and retrieve pattern.
    Scene* SceneManager::get_scene_at(int index) const {
        if (index >= 0 && index < pimpl->scene_list.size()) {
            return pimpl->scene_list[index].get();
        }
        return nullptr;
    }

    size_t SceneManager::get_scene_count() const {
        return pimpl->scene_list.size();
    }

    void SceneManager::set_project_root_path(const std::string& path) {
        pimpl->project_root_path = path;
    }
    
    const std::string& SceneManager::get_project_root_path() const {
        return pimpl->project_root_path;
    }
    
    // --- SCENE LOADING METHOD (load_scene) ---
    bool SceneManager::load_scene(const std::string& relative_scene_path, const std::string& project_root_path_str) {
        // --- Removed: Unload current active scene and clear scene_list here ---
        // These responsibilities belong to set_active_scene() or explicit management.
        // This method's job is to LOAD a scene from file and add it to the list.

        // Form the absolute path to the scene file
        std::filesystem::path full_scene_path = std::filesystem::path(project_root_path_str) / relative_scene_path;

        // Check if the scene file actually exists on disk.
        if (!FileManager::path_exists(full_scene_path.string())) {
            std::cerr << "SceneManager Error: Failed to load scene '" << relative_scene_path
                    << "'. Full path '" << full_scene_path.string() << "' does not exist." << std::endl;
            return false;
        }

        // NEW: Check if a scene with this name is ALREADY loaded.
        // If scene names are unique and match the file they load, this prevents re-loading.
        // Assuming Scene::get_name() returns the unique name for the scene.
        // We will get the scene's name from the file after deserialization.
        // For now, let's just use its path as a rough check for "already loaded".
        for (const auto& scene_ptr : pimpl->scene_list) {
            if (scene_ptr && scene_ptr->get_file_path() == relative_scene_path) { // Compare by file path
                std::cout << "SceneManager: Scene from path '" << relative_scene_path << "' is already loaded. Skipping re-load." << std::endl;
                return true; // Already loaded, consider it successful
            }
        }


        std::cout << "SceneManager: Loading scene from '" << full_scene_path.string() << "'..." << std::endl;

        // Create a new, empty Scene object in memory, which Cereal will populate.
        // This will be a lightweight shell, which Cereal will fill with name, path, and entities.
        std::unique_ptr<Scene> new_scene_owner = std::make_unique<Scene>();

        // --- Cereal Deserialization of Scene ---
        // Scene template is currently a JSON file (Default.json), but will eventually be binary (.scene).
        // For now, we'll open it for JSON reading.
        std::ifstream scene_file_is(full_scene_path.string()); // Open for JSON read (no std::ios::binary)

        if (!scene_file_is.is_open()) {
            std::cerr << "SceneManager Error: Could not open scene file '" << full_scene_path.string() << "' for reading." << std::endl;
            return false;
        }

        try {
            cereal::JSONInputArchive archive(scene_file_is); // Use JSONInputArchive for current .json scene files
            archive(*new_scene_owner); // Deserialize data from file into the new Scene object
        } catch (const cereal::Exception& e) {
            std::cerr << "SceneManager Error: Failed to deserialize scene from '" << full_scene_path.string() << "': " << e.what() << std::endl;
            scene_file_is.close();
            return false;
        }
        scene_file_is.close(); // Close the file stream

        // Add the newly loaded scene to the list of managed scenes.
        // It should NOT be set active or have on_load called here. That's set_active_scene's job.
        std::cout << "SceneManager: Successfully loaded scene '" << new_scene_owner->get_name() << "' and added to list." << std::endl;
        
        // Transfer ownership to scene_list.
        pimpl->scene_list.push_back(std::move(new_scene_owner)); 

        return true; // Successfully loaded and added to list.
    }

    bool SceneManager::load_active_scene() {
        // 1. Check if there is an active scene to load into.
        if (!pimpl->active_scene) {
            std::cerr << "SceneManager Error: Cannot load content, no active scene is set." << std::endl;
            return false;
        }

        // 2. Get the file path from the active scene object itself.
        std::filesystem::path full_scene_path = std::filesystem::path(pimpl->project_root_path) / pimpl->active_scene->get_file_path();

        if (!FileManager::path_exists(full_scene_path.string())) {
            std::cerr << "SceneManager Info: Scene file does not exist for '" << pimpl->active_scene->get_name() << "'." << std::endl;
            return false; // Return false so the orchestrator knows to create a default.
        }

        // 3. Open the file and deserialize directly into the active scene.
        std::ifstream scene_file_is(full_scene_path.string());
        if (!scene_file_is.is_open()) {
            std::cerr << "SceneManager Error: Could not open scene file for reading: " << full_scene_path.string() << std::endl;
            return false;
        }

        try {
            cereal::JSONInputArchive archive(scene_file_is);
            // This is the key: we deserialize INTO the existing active scene object.
            archive(*pimpl->active_scene);

            // --- THIS IS THE MISSING STEP ---
            // Now that the scene has entities, iterate through them and call their
            // on_load method so they can load their assets (textures, sounds, etc.).
            std::cout << "SceneManager: Initializing loaded entities..." << std::endl;
            for (const auto& entity : pimpl->active_scene->get_entities()) {
                if (entity) {
                    entity->on_load(pimpl->context);
                }
            }
        // --------------------------------
            std::cout << "SceneManager: Successfully loaded content for active scene '" << pimpl->active_scene->get_name() << "'." << std::endl;
            return true;
        } catch (const cereal::Exception& e) {
            std::cerr << "SceneManager Error: Failed to deserialize active scene: " << e.what() << std::endl;
            return false;
        }
    }

    bool SceneManager::save_active_scene() {
        // 1. Check if there is an active scene to save.
        if (!pimpl->active_scene) {
            std::cerr << "SceneManager Error: Cannot save, no active scene is set." << std::endl;
            return false;
        }

        // 2. Get the file path from the active scene object.
        std::filesystem::path full_scene_path = std::filesystem::path(pimpl->project_root_path) / pimpl->active_scene->get_file_path();

        // 3. Open the file for writing and serialize the active scene's data.
        std::ofstream scene_file_os(full_scene_path.string());
        if (!scene_file_os.is_open()) {
            std::cerr << "SceneManager Error: Could not open scene file for writing: " << full_scene_path.string() << std::endl;
            return false;
        }

        try {
            cereal::JSONOutputArchive archive(scene_file_os);
            // This is the key: we serialize FROM the existing active scene object.
            archive(*pimpl->active_scene);
            std::cout << "SceneManager: Successfully saved active scene '" << pimpl->active_scene->get_name() << "'." << std::endl;
            return true;
        } catch (const cereal::Exception& e) {
            std::cerr << "SceneManager Error: Failed to serialize active scene: " << e.what() << std::endl;
            return false;
        }
    }
}  // namespace Salix
