// Engine.h
#pragma once
#include "../rendering/IRenderer.h"
#include "../ecs/Entity.h"
#include <memory>

// Forwared declare SDL_Window, so we don't have to include the SDL header here.
// This is a good practice to keep the header files clean and reduce compile times.

class AssetManager;
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
    AssetManager* asset_manager;  // The new AssetManager.

    // --- TEST  ENTITIES ---
    // A test entity to display out sprite.
    // We use a unique_ptr to manage its memory automatically.
    std::unique_ptr<Entity> parent_entity;
    std::unique_ptr<Entity> child_entity;
    // --- END TEST ---

};