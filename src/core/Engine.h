// Engine.h
#pragma once
#include "../rendering/IRenderer.h"
#include "../ecs/Entity.h"
#include <memory>

// Forward declarations
class AssetManager;
class IAppState;

class Engine {
    public:
    Engine();
    ~Engine();

    // The main phases of the engine's lifecycle
    bool initialize(const WindowConfig& config);
    void run();
    void shutdown();
    AssetManager* get_asset_manager();
    IRenderer* get_renderer();

    private:
    void process_input();
    void update(float delta_time);
    void render();

    // A method to change states.
    void switch_state(std::unique_ptr<IAppState> new_state);
    bool is_running;

    // Engine owns the low-level systems.
    AssetManager* asset_manager;
    IRenderer* renderer;

    // The Engine now owns the current state.
    std::unique_ptr<IAppState> current_state;
};