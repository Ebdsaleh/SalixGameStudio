// Salix/ecs/Scene.h
#pragma once
#include <vector>
#include <memory>
#include <string>

namespace Salix {

// Forward declarations
class Entity;
class IRenderer;
class AssetManager;

    class Scene {
    public:
        // The constructor now just takes a name.
        Scene(const std::string& name);
        ~Scene();

        // Lifecycle methods
        void on_load(AssetManager* asset_manager);
        void update(float delta_time);
        void render(IRenderer* renderer);
        void on_unload();

        // A method to create a new entity within this scene.
        Entity* create_entity(const std::string& name = "Entity");
        const std::string& get_name() const;

    private:
        std::vector<std::unique_ptr<Entity>> entities;
        std::string scene_name;
    };
} // namespace Salix