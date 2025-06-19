// Salix/states/EditorState.cpp
#include <EditorState.h>
#include <Salix/core/Engine.h>
#include <Salix/management/ProjectManager.h>
#include <iostream>

namespace Salix {

    EditorState::EditorState() :
        engine(nullptr),
        asset_manager(nullptr),
        renderer(nullptr) {}

    EditorState::~EditorState() {}

    void EditorState::on_enter(Engine* owner_engine) {
        std::cout << "Entering EditorState..." << std::endl;
        engine = owner_engine;

        // Get the core systems from the engine
        asset_manager = engine->get_asset_manager();
        renderer = engine->get_renderer();

        // The EditorState creates and initializes the ProjectManager.
        project_manager = std::make_unique<ProjectManager>();
        project_manager->initialize(asset_manager);
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