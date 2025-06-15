// Renderer.cpp
#include "SDLRenderer.h"
#include <SDL.h>
#include <iostream>

SDLRenderer::SDLRenderer() {
    sdl_renderer = nullptr;

}

SDLRenderer::~SDLRenderer() {
    // Shutdown should be called explicitly but this is just a safeguard.
    if (sdl_renderer) {
        SDL_DestroyRenderer(sdl_renderer);
    }
}

bool SDLRenderer::initialize(SDL_Window* window) {
    if (!window) {
        std::cout << "Renderer::initialize - Window is null!" << std::endl;
        return false;
    }

    // Create the SDL renderer
    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (sdl_renderer == nullptr) {
        std::cout << "Renderer::initalize - Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "Renderer created successfully." << std::endl;
    return true;
}

void SDLRenderer::shutdown() {
    std::cout << "Shutting down renderer." << std::endl;
    SDL_DestroyRenderer(sdl_renderer);
    sdl_renderer = nullptr; // Prevent double-deletion
}

void SDLRenderer::begin_frame() {
    // Set the draw color and clear the screen
    SDL_SetRenderDrawColor(sdl_renderer, 15, 20, 40, 255); // A dark-blue color.
    SDL_RenderClear(sdl_renderer);
}

void SDLRenderer::end_frame() {
    // Present the back buffer to the screen.
    SDL_RenderPresent(sdl_renderer);
}