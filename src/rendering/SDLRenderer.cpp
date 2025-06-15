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

SDL_Window* SDLRenderer::initialize(const WindowConfig& config) {

    window = SDL_CreateWindow(
        config.title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        config.width,
        config.height,
        SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cout << "Renderer::initialize - Window is null!" << std::endl;
        return nullptr;
    }

    // Create the SDL renderer
    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (sdl_renderer == nullptr) {
        std::cout << "Renderer::initalize - Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    std::cout << "Renderer created successfully." << std::endl;
    return window;
}

void SDLRenderer::shutdown() {
    std::cout << "Shutting down renderer." << std::endl;
    SDL_DestroyRenderer(sdl_renderer);
    sdl_renderer = nullptr; // Prevent double-deletion
    window = nullptr;
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