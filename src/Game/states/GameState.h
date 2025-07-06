// =================================================================================
// Filename:    Game/states/GameState.h
// =================================================================================
#pragma once

#include <Salix/core/InitContext.h>
#include <Salix/states/IAppState.h>  
#include <Salix/core/EngineInterface.h>
#include <memory>

namespace Salix {

    // Forward declarations for the pointers we will hold
    class ProjectManager;
    class AssetManager;

    class GameState : public IAppState{
        public:
            GameState();
            virtual ~GameState();

            void on_enter(const InitContext& new_context) override;
            void on_exit() override;
            void update(float delta_time) override;
            void render(IRenderer* renderer) override;
        
        private:
            // The GameState now OWNS the ProjectManager
            std::unique_ptr<ProjectManager> project_manager;
            InitContext context;
            // It holds non-owning pointers to the engine systems it needs.
            AssetManager* asset_manager;
            IRenderer* renderer;
            EngineInterface* engine;
    };
} // namespace Salix