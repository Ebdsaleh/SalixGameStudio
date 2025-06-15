// Engine.cpp
#include "Engine.h"
#include "rendering/Renderer.h"
// We need to include the full SDL header here to use its functions
#include <SDL.h>
#include <iostream>

// Constants for the window
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Engine::Engine() {
    window = nullptr;
    renderer = nullptr;
    is_running = false;
}

Engine::~Engine() {
    // The shutdown method will handel cleanup, but this is here for completeness.

}

bool Engine::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
        return false;

    }

    window = SDL_CreateWindow(
        "Salix Game Studio", // Window title
        SDL_WINDOWPOS_CENTERED, // x position of the window
        SDL_WINDOWPOS_CENTERED, // y position of the window
        WINDOW_WIDTH, // Width of the window
        WINDOW_HEIGHT, // Height of the window
        SDL_WINDOW_SHOWN // The window's flag, in this case we are causing the window to be visible.
    );

    if (window == nullptr) {
        std::cerr << "Engine::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Create and initialize Renderer
    renderer = new Renderer();
    if (!renderer->initialize(window)) {
        std::cerr << "Engine::initalize - Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        delete renderer;
        return false;
    }

    is_running = true;
    std::cout << "Engine initialized successfully." << std::endl;
    return true;
}

void Engine::run() {
    while (is_running) {
        process_input();
        update();
        render();
    }
}

void Engine::shutdown() {
    std::cout << "Shutting down engine." << std::endl;
    if(renderer) {
        renderer->shutdown();
        delete renderer;
    }

    if (window) {
        SDL_DestroyWindow(window);
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

void Engine::update() {
    // Game logic will go here
}

void Engine::render() {
    // Safer error handling if renderer is null, just stop the engine.
    if(renderer == nullptr) {
        std::cerr << "Engine::render - Renderer is null, shuting down." << std::endl;
        is_running = false;  // Gracefully exit the main loop.
        return;
    }
    
    // Begin drawing the frame
    renderer->begin_frame();

    // Process rendering objects here

    // Present the rendered frame
    renderer->end_frame();
}