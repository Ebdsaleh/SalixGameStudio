// Salix/ecs/Scene.cpp
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>  // used for test code.
#include <Salix/ecs/Sprite2D.h>  // used for test code.
#include <Salix/assets/AssetManager.h>
#include <Salix/management/FileManager.h> // For file operations
#include <Salix/core/SerializationRegistrations.h>
#include <Salix/rendering/ICamera.h>

#include <filesystem> // For std::filesystem::path
#include <fstream>    // For std::ifstream
#include <algorithm>  // For std::remove_if
#include <memory>     // For std::unique_ptr
#include <iostream>   // For std::cout, std::cerr

// Cereal headers for Scene's internal content loading (entities/elements)
#include <cereal/cereal.hpp>

#include <cereal/types/string.hpp>     // For std::string (used by entity name, element type, texture path etc.)
#include <cereal/types/vector.hpp>     // For std::vector (of entities, elements, scenes)
#include <cereal/types/memory.hpp>     // For std::unique_ptr (to entities, elements)
#include <cereal/types/polymorphic.hpp> // Crucial for unique_ptr<Element> elements
// Include specific element headers if their serialization functions are defined here
#include <cereal/archives/json.hpp>  // For now, loading Default.json
#include <cereal/archives/binary.hpp>  // For future .scene binary files

namespace Salix {
    extern std::filesystem::path g_project_root_path;
    // Scene::Pimpl struct definition (Pimpl holds only entities)
    struct Scene::Pimpl {
        std::vector<std::unique_ptr<Entity>> entities;
        InitContext context;
        ICamera* active_camera = nullptr;
        Pimpl() = default; // Default constructor

        template <class Archive>
        void serialize(Archive& archive) {
            archive(cereal::make_nvp("entities", entities));
        }
    };

    // --- Constructors ---
    // Default constructor for Scene (lightweight shell)
    // Initializes shell members to empty/default.
    Scene::Scene() : name("Untitled"), path("../Scenes"), pimpl(std::make_unique<Pimpl>()) {}

    // Constructor for named Scene shell with default path (e.g., from user input in editor)
    Scene::Scene(const std::string& name_val) // Corresponds to Scene(const std::string& new_scene_name) in .h
        : name(name_val), path("../Scenes"), pimpl(std::make_unique<Pimpl>()) {}

    // Constructor for named Scene shell with file path (for loading from project files)
    Scene::Scene(const std::string& name_val, const std::string& relative_file_path_val)
        : name(name_val), path(relative_file_path_val), pimpl(std::make_unique<Pimpl>()) {}

    // Destructor (defined here for Pimpl's complete type)
    Scene::~Scene() {
        on_unload(); // Clears entities
    }

    void Scene::set_active_camera(ICamera* camera) {
        if (!camera) return;
        pimpl->active_camera = camera;
    }

    ICamera* Scene::get_active_camera() {
        return pimpl->active_camera;
    }
    // --- NEW: Method to load the scene's content (entities/elements) from its file into Pimpl ---
    // This is the core "load on demand" logic, called by on_load().
    bool Scene::load_content_from_file(const std::string& project_root_path) {
        if (!pimpl->entities.empty()) {
            std::cout << "Scene: Content for '" << name << "' already loaded. Re-initializing entities." << std::endl;
            for (auto& entity : pimpl->entities) {
                if (entity) {
                    entity->on_load(pimpl->context);
                }
            }
            return true;
        }

        std::filesystem::path full_path = std::filesystem::path(project_root_path) / path;
        full_path.make_preferred(); // Converts to OS path format.
        if (!FileManager::path_exists(full_path.string())) {
            std::cerr << "Scene Error: File '" << full_path.string() << "' does not exist." << std::endl;
            return false;
        }

        std::ifstream is(full_path.string());
        if (!is.is_open()) {
            std::cerr << "Scene Error: Could not open file '" << full_path.string() << "'." << std::endl;
            return false;
        }

        try {
            cereal::JSONInputArchive archive(is);
            archive(*this);
        } catch (const cereal::Exception& e) {
            std::cerr << "Scene Error: Failed to deserialize scene '" << name << "': " << e.what() << std::endl;
            return false;
        }

        std::cout << "Scene: Content loaded for '" << name << "'. Initializing " << pimpl->entities.size() << " entities..." << std::endl;
        for (auto& entity : pimpl->entities) {
            if (entity) {
                entity->on_load(pimpl->context);
            }
        }

        return true;
    }



    // --- Lifecycle methods ---
    // Corrected: on_load now calls load_content_from_file and includes project_root_path
    void Scene::on_load(const InitContext& new_context, const std::string& project_root_path) {
        Salix::g_project_root_path = project_root_path;
        pimpl->context = new_context;
        std::cout << "Scene '" << name << "' on_load: Triggering content load and entity initializations." << std::endl;

        std::filesystem::path full_path = std::filesystem::path(project_root_path) / path;
        full_path.make_preferred();  // Convert to OS path style.
        if (FileManager::path_exists(full_path.string())) {
            std::ifstream is(full_path.string());
            if (!is.is_open()) {
                std::cerr << "Scene Error: Could not open file '" << full_path.string() << "' for reading." << std::endl;
            } else {
                try {
                    cereal::JSONInputArchive archive(is);
                    archive(*this);

                    std::cout << "Scene '" << name << "' successfully loaded from '" << full_path.string() << "'." << std::endl;

                    for (auto& entity : pimpl->entities) {
                        if (entity) {
                            entity->on_load(pimpl->context);
                        }
                    }
                    // return;
                } catch (const cereal::Exception& e) {
                    std::cerr << "Scene Error: Failed to deserialize scene '" << name << "': " << e.what() << std::endl;
                }
            }
        } else {
            std::cerr << "Scene Error: File '" << full_path.string() << "' does not exist." << std::endl;
        }

        // --- Fallback Injection + Serialization ---
        std::cerr << "Scene Warning: Scene file missing or invalid. Injecting default entity for '" << name << "'..." << std::endl;
        std::cout <<"[SCENE:] name is: " <<  this->name << std::endl;
    
        
    }

    void Scene::on_unload() {
        pimpl->entities.clear(); // Clears entities, releasing their memory.
        std::cout << "Scene '" << name << "' unloaded." << std::endl;
    }

    void Scene::update(float delta_time) {
        
        // --- The Purge Phase ---
        auto it = std::remove_if(pimpl->entities.begin(), pimpl->entities.end(), [](const auto& entity) {
            return entity->is_purged();
        });
        pimpl->entities.erase(it, pimpl->entities.end());
        // --- The Update Phase ---
        
        for (auto& entity : pimpl->entities) {
            
            entity->update(delta_time);
        }
        
    }

    void Scene::render(IRenderer* renderer) {
        // --- The Render Phase ---
        for (auto& entity : pimpl->entities) {
            entity->render(renderer);
        }
    }

    bool Salix::Scene::load_from_file() {
        // 1. Get the scene's relative path and combine it with the project root.
        std::filesystem::path full_path = Salix::g_project_root_path / path;

        // 2. Check if the file exists. If not, we can't load it.
        if (!FileManager::path_exists(full_path.string())) {
            std::cout << "Scene Info: File '" << full_path.string() << "' does not exist." << std::endl;
            return false;
        }

        // 3. Open the file for reading.
        std::ifstream is(full_path.string());
        if (!is.is_open()) {
            std::cerr << "Scene Error: Could not open file '" << full_path.string() << "' for reading." << std::endl;
            return false;
        }

        std::cout << "Scene: Loading content from '" << full_path.string() << "'..." << std::endl;

        // 4. Try to deserialize the data into this object (`*this`).
        try {
            cereal::JSONInputArchive archive(is);
            archive(*this); // Populates this Scene object with data from the file
            return true;  // Success!
        } catch (const cereal::Exception& e) {
            std::cerr << "Scene Error: Failed to deserialize '" << name << "': " << e.what() << std::endl;
            return false; // Deserialization failed.
        }
    }

    // Loops through entities and tells them to load their assets (textures, etc.).
    void Scene::load_assets(const InitContext& new_context) {
        std::cout << "Scene '" << get_name() << "': Loading assets for entities..." << std::endl;
        // check against the asset_manager so we know if the pimpl->context has been assigned correctly.
        if (!pimpl->context.asset_manager) {
            pimpl->context = new_context;
        }

        // Loop through all entities currently in this scene
        for (Entity* entity : get_entities()) {
            if (entity) {
                // Tell each entity to perform its on_load logic,
                // which will then tell its components to load their assets.
                entity->on_load(pimpl->context);
            }
        }
    }

    Entity* Scene::create_entity(const std::string& name_val) { // Renamed parameter for clarity
        auto new_entity_owner = std::make_unique<Entity>();
        Entity* new_entity = new_entity_owner.get();

        new_entity->set_name(name_val);
        pimpl->entities.push_back(std::move(new_entity_owner));
        return new_entity;
    }

    // Public Getters for Scene Shell Info
    const std::string& Scene::get_name() const {
        return name; // Returns Scene's own 'name' member
    }

    const std::string& Scene::get_file_path() const {
        return path; // Returns Scene's own 'path' member
    }
    
    Entity* Scene::get_entity_by_name(const std::string& entity_name) {
    for (const auto& entity : pimpl->entities) {
        if (entity && entity->get_name() == entity_name) {
            return entity.get();
        }
    }
    return nullptr; // Not found
    }
    
    std::vector<Entity*> Scene::get_entities() {
        std::vector<Entity*> raw_pointers;
        // Reserve space for efficiency (optional but good practice).
        raw_pointers.reserve(pimpl->entities.size());
        
        // Loop through the vector of unique_ptrs.
        for (const auto& entity_ptr : pimpl->entities) {
        
        // Get the raw pointer from the unique_ptr and add it to our new vector.
        raw_pointers.push_back(entity_ptr.get());
        }

        return raw_pointers;
    }

    // --- CEREAL IMPLEMENTATION ---
    template <class Archive>
    void Scene::serialize(Archive& archive) {
        // Serialize scene shell identity and its path.
        archive(cereal::make_nvp("name", name)); // Serialize Scene's own 'name' member
        archive(cereal::make_nvp("path", path)); // Serialize Scene's own 'path' member

        // When deserializing (loading a shell), pimpl->entities might be empty.
        // When deserializing a FULL scene (e.g., in SceneManager::load_scene, which loads shell+content),
        // or when saving a loaded scene, this will serialize/deserialize the Pimpl's entities.
        archive(cereal::make_nvp("entities", pimpl->entities)); // Corrected key name for Pimpl's content
    }

    template void Scene::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Scene::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    template void Scene::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Scene::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);
 
} // namespace Salix