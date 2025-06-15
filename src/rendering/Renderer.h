// Renderer.h
#pragma once

// Forward delcarations to keep the headers clean
struct SDL_Window;
struct SDL_Renderer;

class Renderer {
    public:
    Renderer();
    ~Renderer();

    // The Renderer's lifecycle methods
    bool initialize(SDL_Window*);
    void shutdown();

    // The core rendering commands
    void begin_frame();
    void end_frame();

    private:
    // The Renderer's own SDL_Renderer object
    SDL_Renderer* sdl_renderer = nullptr;
};
