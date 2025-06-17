// Engine.cpp
#include "Engine.h"
// --- NOTE ---
// All RendererTypes will be included here to allow the Engine to use a singular set of methods for rendering.
#include "../rendering/SDLRenderer.h"
// --- END NOTE ---
#include "../assets/AssetManager.h"
// --- Include the States.
#include "../states/LaunchState.h"
#include "../states/GameState.h"
#include "../states/EditorState.h"
#include "../states/OptionsMenuState.h"
// We need to include the full SDL header here to use its functions
#include <SDL.h>
#include <iostream>


Engine::Engine() {
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

    if(!renderer->initialize(config)) return false;
    // --- AssetManager initialization ---
    asset_manager = new AssetManager();
    asset_manager->initialize(renderer);

    // --- STATE MACHINE SETUP ---
    // loads directly into LaunchState.
    switch_state(AppStateType::Launch);
    
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
    
    // Shutdown current state first.
    if (current_state) {
        current_state->on_exit();
        current_state.reset();
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
    // We could pass events to the current state here in the future.
}

void Engine::switch_state(AppStateType new_state_type) {
    if (current_state) {
        current_state->on_exit();
    }

    // Create a new state based on the type requested.
    std::unique_ptr<IAppState> new_state = nullptr;
    
    switch (new_state_type)
    {
    case AppStateType::Launch:
        new_state = std::make_unique<LaunchState>();
        break;
    case AppStateType::Editor:
        new_state = std::make_unique<EditorState>();
        break;
    case AppStateType::Game:
        new_state = std::make_unique<GameState>();
        break;
    case AppStateType::Options:
        new_state = std::make_unique<OptionsMenuState>();
    
    default:
        std::cerr << "Engine::switch_state - Unknown state requested!" << std::endl;
        return;
    }
    current_state = std::move(new_state);
    if (current_state) {
        current_state->on_enter(this);  // pass the engine to the current state.
    }
}

void Engine::update(float delta_time) {
    // The Engine now delegates the update call to the active state.
    current_state->update(delta_time);
}

void Engine::render() {
    if (renderer == nullptr) {
        std::cerr << "Engine::render - Renderer is null, shuting down." << std::endl;
        is_running = false;  // Gracefully exit the main loop.
        return;
    }
    // Begin drawing the frame
    renderer->begin_frame();

    // The Engine now delegates the render call to the active state.
    if (current_state) {
        current_state->render(renderer);
    }

    // Present the rendered frame
    renderer->end_frame();   
}