// Engine.h
#pragma once

// Forwared declare SDL_Window, so we don't have to include the SDL header here.
// This is a good practice to keep the header files clean and reduce compile times.

struct SDL_Window;
struct SDL_Renderer;

class Engine {
    public:
    Engine();
    ~Engine();

    // The main phases of the engine's lifecycle
    bool initialize();
    void run();
    void shutdown();

    private:
    void process_input();
    void update();
    void render();

    bool is_running;

    // pointers to the core SDL objects
    SDL_Window *window;
    SDL_Renderer* renderer;

};