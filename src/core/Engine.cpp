// Engine.cpp
#include "Engine.h"

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
        std::cout << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
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
        std::cout << "Engine::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "Engine::initalize - Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
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
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Engine::process_input() {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) {
            is_running = false;
        }
    }
}

void Engine::update() {
    // Game logic will go here
}

void Engine::render() {
    SDL_SetRenderDrawColor(renderer, 15, 20, 40, 255);
    SDL_RenderClear(renderer);

    // All future drawing will go here

    SDL_RenderPresent(renderer);
}