// Salix/states/EditorState.h
#pragma once

#include <IAppState.h>
#include <memory>


namespace Salix {

    // Forward declarations
    class ProjectManager;
    class AssetManager;
    class IRenderer;

    class EditorState : public IAppState{
        public:
            EditorState();
            virtual ~EditorState();

            // Implement the IAppState interface
            void on_enter(class Engine* engine) override;
            void on_exit()override;
            void update(float delta_time) override;
            void render(IRenderer* renderer) override;
        
        private:
            // The EditorState OWNS the ProjectManager for the currently open project.
            std::unique_ptr<ProjectManager> project_manager;

            // Non-owning pointers to the engine and its systems.
            Engine* engine;
            AssetManager* asset_manager;
            IRenderer* renderer;
    };
} // namespace Salix