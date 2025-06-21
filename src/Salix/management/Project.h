// Salix/management/Project.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
#include <memory>

namespace Salix {

    // Forward declarations
    class SceneManager;
    class IRenderer;
    class AssetManager;

    class SALIX_API Project {
    public:
        Project();
        ~Project();

        void initialize(AssetManager* asset_manager);
        void shutdown();
        void update(float delta_time);
        void render(IRenderer* renderer);
        
        SceneManager* get_scene_manager() const;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        
    };
} // namespace Salix