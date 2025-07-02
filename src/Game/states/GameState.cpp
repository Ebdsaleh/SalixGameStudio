// Salix/states/GameState.cpp

#include <Game/states/GameState.h>

// Include all the headers for the systems and components we need to interact with
#include <Salix/core/Core.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Scene.h>
#include <Salix/input/IInputManager.h>
#include <iostream>
#include <filesystem>

namespace Salix {

    GameState::GameState() :
    asset_manager(nullptr),
    renderer(nullptr) {}

    GameState::~GameState() {}

    /* --- NON TEST CODE ---

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
        // --- End of Previous Implementation ---
    }
        --- END NON-TEST CODE ---
    */

    void GameState::on_enter(const InitContext& new_context) {
        std::cout << "Entering GameState..." << std::endl;
        context = new_context;
        asset_manager = context.asset_manager;
        context.engine->set_mode(EngineMode::Game);
        // 1. Get the path to where the executable is being run from.
        std::filesystem::path current_working_dir = std::filesystem::current_path();

        // 2. Define the relative path from that location to your project folder.
        std::filesystem::path relative_path_to_project = "src/Sandbox/TestProject";

        // 3. Combine them to create the true absolute path and set our global variable.
        Salix::g_project_root_path = (current_working_dir / relative_path_to_project).lexically_normal();

        std::cout << "DEBUG: Project root path set to: " << Salix::g_project_root_path.string() << std::endl;
       

        
        // --- SETUP ---
        project_manager = std::make_unique<ProjectManager>();
        project_manager->initialize(context);

        
        // Now, use the absolute path to load the project file
        std::filesystem::path project_file_path = Salix::g_project_root_path / "TestProject.salixproj";
        Project* current_project = project_manager->load_project_from_file(project_file_path.string());

        if (!current_project) {
            std::cerr << "FATAL: Could not load project." << std::endl;
            return;
        }

        SceneManager* scene_manager = current_project->get_scene_manager();
        scene_manager->set_active_scene(current_project->get_starting_scene());

        // Try to load the scene file.
        bool loaded_successfully = scene_manager->load_active_scene();

        // If loading failed...
        if (!loaded_successfully) {
            // ...create the default scene.
            current_project->create_and_save_default_scene();
            // ...and then load the assets for the newly created content.
            scene_manager->get_active_scene()->load_assets(context);
        }

        std::cout << "GameState setup complete. Starting game loop..." << std::endl;
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