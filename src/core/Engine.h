// Engine.h
#pragma once
#include "../rendering/IRenderer.h"
// Forwared declare SDL_Window, so we don't have to include the SDL header here.
// This is a good practice to keep the header files clean and reduce compile times.

class IRenderer;
struct SDL_Window;

class Engine {
    public:
    Engine();
    ~Engine();

    // The main phases of the engine's lifecycle
    bool initialize(const WindowConfig& config);
    void run();
    void shutdown();

    private:
    void process_input();
    void update();
    void render();

    bool is_running;

    // The Engine owns the main window
    SDL_Window *window = nullptr;
    // The Engine owns the core subsystems.
    // The Engine holds a pointer to abstract interface, not a complete implmentation.
    IRenderer* renderer;

};