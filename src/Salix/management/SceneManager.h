// Salix/management/SceneManager.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
#include <vector>
#include <memory>

namespace Salix {

    // Forward declarations
    class Scene;
    class IRenderer;
    class AssetManager;

    class SALIX_API SceneManager {
    public:
        SceneManager();
        ~SceneManager();

        void initialize(AssetManager* asset_manager);
        void shutdown();

        void update(float delta_time);
        void render(IRenderer* renderer);

        // --- NEW, FULLY-FEATURED API ---

        // For the engine/editor: Creates a new, empty scene managed by the SceneManager.
        Scene* create_scene(const std::string& scene_name);

        // For user scripts: Takes ownership of an externally created scene.
        void add_scene(std::unique_ptr<Scene> scene_to_add);

        // Removes scenes by different identifiers.
        void remove_scene(const std::string& scene_name);
        void remove_scene(Scene* scene_to_remove);
        void remove_scene_at(int index);

        // Manages the active scene.
        void set_active_scene(const std::string& scene_name);
        Scene* get_active_scene() const;
        
        // Getters for querying the list of scenes.
        Scene* get_scene(const std::string& scene_name) const;
        Scene* get_scene_at(int index) const;
        size_t get_scene_count() const;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix