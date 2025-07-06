// Salix/states/EditorState.cpp
#include <Editor/states/EditorState.h>
#include <Salix/core/EngineMode.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/core/InitContext.h>
#include <iostream>

namespace Salix {

    EditorState::EditorState() :
        asset_manager(nullptr),
        renderer(nullptr) {}

    EditorState::~EditorState() {}

    void EditorState::on_enter(const InitContext& new_context) {
        std::cout << "Entering EditorState..." << std::endl;
        context = new_context;
        if(context.asset_manager == nullptr) {
            std::cerr << "EditorState::on_enter - ERROR: context pointer members are nullptr! " <<
                "Tested against 'context.asset_manager" << std::endl;
        }
        context.engine->set_mode(EngineMode::Editor);

        // Get the core systems from the engine
        asset_manager = context.asset_manager;
        renderer = context.renderer;

        // The EditorState creates and initializes the ProjectManager.
        project_manager = std::make_unique<ProjectManager>();
        project_manager->initialize(context);
    }

    void EditorState::on_exit() {
        std::cout << "Exiting EditorState..." << std::endl;
        if (project_manager) {
            project_manager->shutdown();
            project_manager.reset();
        }
    }

    void EditorState::update(float delta_time) {
        if (project_manager) {
            project_manager->update(delta_time);
        }
    }

    void EditorState::render(IRenderer* renderer_param) {
        if (project_manager) {
            project_manager->render(renderer_param);
        }
        // In the future, we would also render the editor's UI.
    }
} // namespace Salix