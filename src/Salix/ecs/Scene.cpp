// Salix/ecs/Scene.cpp
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <algorithm>
#include <memory>
#include <iostream>

namespace Salix {
    struct Scene::Pimpl {
        std::vector<std::unique_ptr<Entity>> entities;
        std::string scene_name;
    };

    Scene::Scene(const std::string& name) : pimpl(std::make_unique<Pimpl>()) {
        pimpl->scene_name = name;
    }

    Scene::~Scene() {
        on_unload();
    }

    void Scene::on_load(AssetManager* /*asset_manager*/) {
        // This is where we will deserialize scene data from a file in the future.
        // For now, it just signals that the scene is ready.
        std::cout << "Scene '" << pimpl->scene_name << "' on_load." << std::endl;
    }

    void Scene::on_unload() {
        // Clearing the vector of unique_ptrs automatically deletes all owned entities.
        pimpl->entities.clear();
        std::cout << "Scene '" << pimpl->scene_name << "' unloaded." << std::endl;
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

    Entity* Scene::create_entity(const std::string& name) {
        auto new_entity_owner = std::make_unique<Entity>();
        Entity* new_entity = new_entity_owner.get();

        new_entity->set_name(name);
        pimpl->entities.push_back(std::move(new_entity_owner));
        return new_entity;
    }

    const std::string& Scene::get_name() const {
        return pimpl->scene_name;
    }
    
    Entity* Scene::get_entity_by_name(const std::string& entity_name) {
    for (const auto& entity : pimpl->entities) {
        if (entity->get_name() == entity_name) {
            return entity.get();
        }
    }
    return nullptr; // Not found
    }
} // namespace Salix