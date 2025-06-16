// Scene.cpp

#include "Scene.h"
#include "Entity.h"

// We will need to include these to create our test entities for now.
#include "Sprite2D.h"
#include "../rendering/IRenderer.h"
#include "../assets/AssetManager.h"  // AssetManager is need by the sprite.
#include <algorithm>
// Constructor
Scene::Scene() {}

// Destructor
Scene::~Scene() {
    // The unique_ptr in the vector will handle cleanup automatically,
    // but we can call on_unload for any other custom cleanup.
    on_unload();
}

void Scene::on_load() {
    // This is where we will eventually deserialize scene data from a file.
    // For now, we will create our test entities here.
    
    // NOTE: This test code requires an AssetManager. For a real engine,
    // we would pass necessary systems (like the asset manager) to the scene.
    // This is a simplification for now.

    // This section will be removed once we can load from files.
    std::cout << "Scene::on_load - Creating temporary test entities." << std::endl;
}

void Scene::on_unload(){
    entities.clear();
}

void Scene::update(float delta_time){
    // --- The Purge Phase ---
    // First remove any entities that have been marked for purging.
    // This uses the 'erase-remove idiom', a standard C++ pattern.
    auto entity_iterator = std::remove_if(entities.begin(), entities.end(), [](const auto& entity) {
        return entity->is_purged();
    });
    entities.erase(entity_iterator, entities.end());

    // --- The Update Phase ---
    // Now update all the remaining entities.
    for (auto& entity : entities) {
        entity->update(delta_time);
    }
}

void Scene::render(IRenderer* renderer) {
    // --- The Render Phase ---
    // Render all entities. In the future, we would sort them first.
    for (auto& entity : entities) {
        entity->render(renderer);
    }
}

Entity* Scene::create_entity(const std::string& name) {
    // Create a new Entity owned by a unique_ptr.
    auto new_entity_owner = std::make_unique<Entity>();
    
    // Get the raw_ptr to return to the user.
    Entity* new_entity = new_entity_owner.get();

    // We can add a name property later, if we want.
    // For now, we just add the entity to our master list.
    entities.push_back(std::move(new_entity_owner));
    
    return new_entity;
}