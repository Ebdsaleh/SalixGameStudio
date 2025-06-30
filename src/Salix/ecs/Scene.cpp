// Salix/ecs/Scene.cpp
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>  // used for test code.
#include <Salix/ecs/Sprite2D.h>  // used for test code.
#include <Salix/assets/AssetManager.h>
#include <Salix/management/FileManager.h> // For file operations
#include <Salix/core/SerializationRegistrations.h>
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

    // --- NEW: Method to load the scene's content (entities/elements) from its file into Pimpl ---
    // This is the core "load on demand" logic, called by on_load().
    bool Scene::load_content_from_file(const std::string& project_root_path, AssetManager* asset_manager) {
        if (!pimpl->entities.empty()) {
            std::cout << "Scene: Content for '" << name << "' already loaded. Re-initializing entities." << std::endl;
            for (auto& entity : pimpl->entities) {
                if (entity) {
                    entity->on_load(asset_manager);
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
                entity->on_load(asset_manager);
            }
        }

        return true;
    }



    // --- Lifecycle methods ---
    // Corrected: on_load now calls load_content_from_file and includes project_root_path
    void Scene::on_load(AssetManager* asset_manager, const std::string& project_root_path) {
        Salix::g_project_root_path = project_root_path;
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
                            entity->on_load(asset_manager);
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
        Entity* player = create_entity("Player");
        
        if (player) {
            std::cout << "[SCENE:] Player Entity Created..." << std::endl;
        }

        Transform* transform = player->get_transform();
        if (transform) {
            transform->position = { 640.0f, 360.0f, 0.0f };
            transform->rotation = { 0.0f, 0.0f, 0.0f };
            transform->scale    = { 1.0f, 1.0f, 1.0f };
            std::cout << "[SCENE:] player->transform exists and attributes updated. " << std::endl;
        }
        
        Sprite2D* player_sprite = player->add_element<Sprite2D>();

        if (player_sprite != nullptr) {
            std::cout << "[SCENE:] player_sprite successfully created." << std::endl;

            player_sprite->texture_path = "Assets/Images/test.png";
            std::cout << "[SCENE:] Assigned texture path to Sprite2D." << std::endl;
             // try to load texture after serialization
            player_sprite->load_texture(asset_manager, player_sprite->get_texture_path()); // crashing here
            std::cout << "[SCENE:] Loaded texture: " << player_sprite->get_texture_path() << std::endl;
        }

        // --- Ensure the scene directory exists ---
        std::error_code ec;
        std::filesystem::create_directories(full_path.parent_path(), ec);
        if (ec) {
            std::cerr << "[ERROR] Failed to create scene directory '" << full_path.parent_path().string()
                    << "': " << ec.message() << std::endl;
        } else {
            std::cout << "[DEBUG] Scene directory ready: " << full_path.parent_path().string() << std::endl;
        }
        /* --- TEST DEBUG CODE ---
        if (!std::filesystem::exists(full_path.parent_path())) {
            std::cerr << "[DEBUG] Directory does NOT exist at time of open()." << std::endl;
        }
        std::cout << "[DEBUG] Full path: '" << full_path.string() << "'" << std::endl;
        std::ofstream dummy("Assets/Scenes/test_file.txt");
            dummy << "test";
        // --- END TEST DEBUG CODE --- 
        */
        // --- Prepare to write the scene file ---
        std::cout << "[DEBUG] Attempting to write scene file: " << full_path.string() << std::endl;

        std::ofstream os(full_path.string(), std::ios::out | std::ios::trunc);
        if (!os.is_open()) {
            std::cerr << "[ERROR] Could not open scene file for writing: " << full_path.string() << std::endl;
        } else {
            try {
                cereal::JSONOutputArchive archive(os);
                archive(*this);  // Serialize entire scene object
                std::cout << "[SUCCESS] Scene '" << name << "' saved to: " << full_path.string() << std::endl;
            } catch (const cereal::Exception& e) {
                std::cerr << "[ERROR] Failed to serialize scene: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] std::exception while serializing: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[ERROR] Unknown exception during serialization." << std::endl;
            }
        }
       
        
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

    // --- CEREAL IMPLEMENTATION ---
    template <class Archive>
    void Scene::serialize(Archive& archive) {
        // Serialize scene shell identity and its path.
        archive(cereal::make_nvp("name", name)); // Serialize Scene's own 'name' member
        archive(cereal::make_nvp("path", path)); // Serialize Scene's own 'path' member

        // When deserializing (loading a shell), pimpl->entities might be empty.
        // When deserializing a FULL scene (e.g., in SceneManager::load_scene, which loads shell+content),
        // or when saving a loaded scene, this will serialize/deserialize the Pimpl's entities.
        archive(cereal::make_nvp("entities", pimpl)); // Corrected key name for Pimpl's content
    }

    // Explicit instantiations for Scene
    template void Scene::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Scene::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    // Add Binary archives if you plan to use them (recommended for scenes)
    template void Scene::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Scene::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);

} // namespace Salix