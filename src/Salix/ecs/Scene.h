// Salix/ecs/Scene.h
#pragma once
#include <Salix/core/Core.h>
#include <vector>
#include <memory>
#include <string>
#include <cereal/access.hpp>
#include <Salix/rendering/ICamera.h>

namespace Salix {

// Forward declarations
class Entity;
class IRenderer;
class AssetManager;
struct InitContext;
class SimpleGuid;

    class SALIX_API Scene {
    public:
        // The constructor now just takes a name.
        Scene();
        Scene(const std::string& new_scene_name);
        Scene(const std::string& scene_name, const std::string& relative_file_path);
        ~Scene();


        // Lifecycle methods
        void on_load(const InitContext& new_context, const std::string& project_root_path); // need for AssetManager
        void update(float delta_time);
        void render(IRenderer* renderer);
        void on_unload();
        bool load_content_from_file(const std::string& project_root_path);
        void maintain();
        void set_context(const InitContext& context);

        // Tries to deserialize content from this scene's file path.
        bool load_from_file();
        // Loops through entities and tells them to load their assets (textures, etc.).
        void load_assets(const InitContext& new_context);

        // A method to create a new entity within this scene.
        Entity* create_entity(const std::string& new_entity_name = "Entity");
        Entity* create_entity(SimpleGuid id, const std::string& new_entity_name);
        // A method to get an Entity by name.
        Entity* get_entity_by_name(const std::string& entity_name);  // Had to add this for testing in 'Game/GameState.cpp'
        Entity* get_entity_by_id(SimpleGuid id);
        std::vector<Entity*> get_entities(); 
        // --- Public Getters for Shell Info ---
        const std::string& get_name() const;
        const std::string& get_file_path() const; 
        void clear_all_entities();
        void set_active_camera(ICamera* camera);
        ICamera* get_active_camera();

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        friend class cereal::access;
        template <class Archive>
        void serialize(Archive& archive);
        std::string name;
        std::string path;
    };
} // namespace Salix