// Engine.h
#pragma once
#include "../rendering/IRenderer.h"
#include "../ecs/Entity.h"
#include <memory>

// Forward declarations
class AssetManager;
class ProjectManager; 
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
    void update(float delta_time);
    void render();

    bool is_running;

    // The Engine owns the main window
    SDL_Window *window = nullptr;
    // The Engine owns the core subsystems.
    // The Engine holds a pointer to abstract interface, not a complete implmentation.
    AssetManager* asset_manager;
    IRenderer* renderer;
    std::unique_ptr<ProjectManager> project_manager;
};