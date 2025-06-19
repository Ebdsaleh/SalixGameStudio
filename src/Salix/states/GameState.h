// Salix/states/GameState.h
#pragma once

#include <IAppState.h>
#include <memory>

namespace Salix {

    // Forward declarations
    class ProjectManager;
    class AssetManager;

    class GameState : public IAppState{
        public:
            GameState();
            virtual ~GameState();

            void on_enter(Engine* engine) override;
            void on_exit() override;
            void update(float delta_time) override;
            void render(IRenderer* renderer) override;
        
        private:
            // The GameState now OWNS the ProjectManager
            std::unique_ptr<ProjectManager> project_manager;

            // It holds non-owning pointers to the engine systems it needs.
            Engine* engine;
            AssetManager* asset_manager;
            IRenderer* renderer;
    };
} // namespace Salix