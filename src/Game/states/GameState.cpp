// Salix/states/GameState.cpp

#include <Game/states/GameState.h>

// Include all the headers for the systems and components we need to interact with
#include <Salix/core/Core.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/RealmManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Realm.h>
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

        // Capture the context
        context = new_context;

        // --- VALIDATE CONTEXT FIRST ---
        if (!context.engine) { std::cerr << "[GameState] FATAL: Engine is null in InitContext\n"; return; }
        if (!context.asset_manager) { std::cerr << "[GameState] FATAL: AssetManager is null in InitContext\n"; return; }

        asset_manager = context.asset_manager;
        renderer      = context.renderer;
        engine        = context.engine;

        std::cout << "[GameState] Context pointers set. EngineMode = "
                << static_cast<int>(context.engine_mode) << std::endl;

        // --- Set the project root ---
        std::filesystem::path cwd = std::filesystem::current_path();
        std::filesystem::path project_path = cwd / "src/Sandbox/TestProject";
        g_project_root_path = project_path.lexically_normal();

        std::cout << "DEBUG: Project root path set to: " << g_project_root_path << std::endl;

        try {
            // --- Allocate and initialize the ProjectManager ---
            std::cout << "[GameState] Creating ProjectManager..." << std::endl;
            project_manager = std::make_unique<ProjectManager>();
            project_manager->initialize(context);  // Pass full InitContext instead of just asset_manager
            std::cout << "ProjectManager Initialized." << std::endl;

            std::filesystem::path project_file_path = g_project_root_path / "TestProject.salixproj";
            std::cout << "[GameState] Loading project from " << project_file_path << std::endl;

            Project* current_project = project_manager->load_project_from_file(project_file_path.string());

            if (!current_project) {
                std::cerr << "[GameState] FATAL: Could not load project from file.\n";
                return;
            }

            RealmManager* realm_manager = current_project->get_realm_manager();
            realm_manager->set_active_realm(current_project->get_starting_realm());

            std::cout << "[GameState] Loading active realm..." << std::endl;
            bool loaded = realm_manager->load_active_realm();

            if (!loaded) {
                std::cerr << "[GameState] Warning: Realm load failed. Creating default realm." << std::endl;
                current_project->create_and_save_default_realm();
                realm_manager->get_active_realm()->load_assets(context);
            }

            std::cout << "GameState setup complete. Starting game loop..." << std::endl;

        } catch (const std::exception& ex) {
            std::cerr << "[GameState] EXCEPTION: " << ex.what() << std::endl;
        } catch (...) {
            std::cerr << "[GameState] UNKNOWN ERROR during on_enter()\n";
        }
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