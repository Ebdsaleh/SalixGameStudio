// Salix/ecs/Scene.h
#pragma once
#include <Salix/core/Core.h>
#include <vector>
#include <memory>
#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

namespace Salix {

// Forward declarations
class Entity;
class IRenderer;
class AssetManager;

    class SALIX_API Scene {
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

        
        // A method to get an Entity by name.
        Entity* get_entity_by_name(const std::string& entity_name);  // Had to add this for testing in 'Game/GameState.cpp'

        template <class Archive>
        void serialize(Archive& archive) {
            archive(
                CEREAL_NVP(pimpl->scene_name), CEREAL_NVP(pimpl->entities)
            );
        }

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix