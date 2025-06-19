// Salix/states/GameState.cpp

#include <GameState.h>
#include <Salix/core/Engine.h>
#include <Salix/management/ProjectManager.h>
#include <iostream>

namespace Salix {

    GameState::GameState() :
    engine(nullptr),
    asset_manager(nullptr),
    renderer(nullptr) {}

    GameState::~GameState() {}

    void GameState::on_enter(Engine* owner_engine) {
        std::cout << "Entering GameState..." << std::endl;
        engine = owner_engine;

        // We need to get the asset manager and renderer from the engine.
        // This is a bit of a temporary hack. A better system would pass
        // a "context" object with all the systems. But for now, this works.
        // We will add getters to the Engine class for this.
        asset_manager = engine->get_asset_manager();
        renderer = engine->get_renderer();

        // The GameState now creates and intializes the project manager.
        project_manager = std::make_unique<ProjectManager>();
        project_manager->initialize(asset_manager);
    }

    void GameState::on_exit() {
        std::cout << "Exiting GameState..." << std::endl;
        if (project_manager) {
            project_manager->shutdown();
            project_manager.reset();
        }
    }

    void GameState::update(float delta_time) {
        if (project_manager) {
            project_manager->update(delta_time);
        }
    }

    void GameState::render(IRenderer* renderer_param) {
        if (project_manager) {
            project_manager->render(renderer_param);
        }

    }
} // namespace Salix