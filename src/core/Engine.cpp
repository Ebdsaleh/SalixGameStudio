// Engine.cpp
#include "Engine.h"
// --- NOTE ---
// All RendererTypes will be included here to allow the Engine to use a singular set of methods for rendering.
#include "../rendering/SDLRenderer.h"
// --- END NOTE ---
#include "../management/ProjectManager.h"
#include "../assets/AssetManager.h"
// We need to include the full SDL header here to use its functions
#include <SDL.h>
#include <iostream>


Engine::Engine() {
    window = nullptr;
    renderer = nullptr;
    is_running = false;
    asset_manager = nullptr;
}

Engine::~Engine() {
    // The shutdown method will handel cleanup, but this is here for completeness.

}

bool Engine::initialize(const WindowConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }  
    
    // Create and initialize Renderer
    // --- Renderer Factory ---
    switch (config.renderer_type) {
        case RendererType::SDL:
            renderer = new SDLRenderer();         
            break;
        
        // --- Future renderer types would be handled here ---
        case RendererType::OpenGl:
            // renderer = new OpenGLRenderer(); // Example for the future
            std::cerr << "Engine::initialize - OpenGL renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup
        case RendererType::Vulkan:
            // renderer = new VulkanRenderer(); // Example for the future
            std::cerr << "Engine::initialize - Vulkan renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup
        case RendererType::DirectX:
            // renderer = new DirectXRenderer(); // Example for the future
            std::cerr << "Engine::initialize - DirectX renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup

        default:
        // render_type is null, abort renderer creation and exit main loop.
        // Only call SDL_Quit() as the window doesn't exist yet.
        std::cerr << "Engine::initialize - Invalid or unsupported renderer type requested!" << std::endl;
        SDL_Quit();
        return false;       
    }

    // --- Delegate  initialization --
    // Ask the renderer to initialize passing the configuration it needs to create the window.
    window = renderer->initialize(config);

    if (window == nullptr) {
        std::cerr << "Engine::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        delete renderer;
        SDL_Quit();
        return false;
    }

    // --- AssetManager initialization ---
    asset_manager = new AssetManager();
    asset_manager->initialize(renderer);

    // --- Project Initialization ---
    // Create and load the active_scene via the project_manager.
    project_manager = std::make_unique<ProjectManager>();
    project_manager->initialize(asset_manager);


    is_running = true;
    std::cout << "Engine initialized successfully." << std::endl;
    return true;
}

void Engine::run() {
    // In the future we could calculate delete_time here.
    float delta_time = 1.0f / 60.0f ; // pseudo code for delta_time (not an acutal implementation).
    while (is_running) {
        process_input();
        update(delta_time);
        render();
    }
}

void Engine::shutdown() {
    std::cout << "Shutting down engine." << std::endl;

    // Shutdown subsystems in reverse order of creatation (LIFO).
    
    // Shutdown Managers first.
    if (project_manager) {
        project_manager->shutdown();
        project_manager.reset();
    }
        
    if (asset_manager) {
        asset_manager->shutdown();
        delete asset_manager;
    }
    if (renderer) {
        renderer->shutdown();
        delete renderer;
    }
    SDL_Quit();
}

void Engine::process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running = false;
        }
    }
}

void Engine::update(float delta_time) {
    // The Engine now delegates the update call to the active scene.
    project_manager->update(delta_time);
    // Game logic will go here
}

void Engine::render() {
    // Safer error handling if renderer is null, just stop the engine.
    if (renderer == nullptr) {
        std::cerr << "Engine::render - Renderer is null, shuting down." << std::endl;
        is_running = false;  // Gracefully exit the main loop.
        return;
    }
    // Begin drawing the frame
    renderer->begin_frame();

    // Process rendering objects here

    // --- TEST: Only Render the through the project_manager ---
    if (project_manager) {
        project_manager->render(renderer);
    }

    // --- END TEST ---

    // Present the rendered frame
    renderer->end_frame();   
}