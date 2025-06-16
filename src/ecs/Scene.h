// Scene.h
#pragma once

#include <vector>
#include <memory>
#include <string>

// Forward declarations
class Entity;
class IRenderer;

class Scene {
    public:
    Scene();
    ~Scene();

    // Lifecyle methods.
    void on_load();
    void update(float delta_time);
    void render(IRenderer* renderer);
    void on_unload();

    // A method to create new Entity within this scene.
    Entity* create_entity(const std::string& name = "Entity");

    private:
    // The Scene OWNS all the entities.
    // When the vector of unique_ptr is cleared, all entities are destroyed.
    std::vector<std::unique_ptr<Entity>> entities;

    // We can add a name for debugging or for the editor later.
    std::string scene_name;

};