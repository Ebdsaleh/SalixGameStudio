// Scene.cpp
#include "Scene.h"
#include "Entity.h"
#include <algorithm>
#include <memory>
#include <iostream>

Scene::Scene(const std::string& name) : scene_name(name) {}

Scene::~Scene() {
    on_unload();
}

void Scene::on_load(AssetManager* asset_manager) {
    // This is where we will deserialize scene data from a file in the future.
    // For now, it just signals that the scene is ready.
    std::cout << "Scene '" << scene_name << "' on_load." << std::endl;
}

void Scene::on_unload() {
    // Clearing the vector of unique_ptrs automatically deletes all owned entities.
    entities.clear();
    std::cout << "Scene '" << scene_name << "' unloaded." << std::endl;
}

void Scene::update(float delta_time) {
    // --- The Purge Phase ---
    auto it = std::remove_if(entities.begin(), entities.end(), [](const auto& entity) {
        return entity->is_purged();
    });
    entities.erase(it, entities.end());

    // --- The Update Phase ---
    for (auto& entity : entities) {
        entity->update(delta_time);
    }
}

void Scene::render(IRenderer* renderer) {
    // --- The Render Phase ---
    for (auto& entity : entities) {
        entity->render(renderer);
    }
}

Entity* Scene::create_entity(const std::string& name) {
    auto new_entity_owner = std::make_unique<Entity>();
    Entity* new_entity = new_entity_owner.get();
    // We can give the entity a name component later.
    entities.push_back(std::move(new_entity_owner));
    return new_entity;
}

const std::string& Scene::get_name() const {
    return scene_name;
}