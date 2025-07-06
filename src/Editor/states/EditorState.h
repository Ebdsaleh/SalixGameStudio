// Editor/states/EditorState.h
#pragma once

#include <Salix/states/IAppState.h>
#include <Salix/core/InitContext.h>
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
            void on_enter(const InitContext& new_context) override;
            void on_exit()override;
            void update(float delta_time) override;
            void render(IRenderer* renderer) override;
        
        private:
            // The EditorState OWNS the ProjectManager for the currently open project.
            std::unique_ptr<ProjectManager> project_manager;
            InitContext context;
            // Non-owning pointers to the engine and its systems.
            AssetManager* asset_manager;
            IRenderer* renderer;
    };
} // namespace Salix