// Salix/core/Engine.h
#pragma once
#include <Salix/rendering/IRenderer.h>
#include <ITimer.h>
#include <Salix/input/IInputManager.h>
#include <Salix/ecs/Entity.h>
#include <memory>

namespace Salix {
    // A type-safe enum to indentify the application states.
    enum class AppStateType {
        None,
        Launch,
        Editor,
        Game,
        Options
    };

    // Forward declarations
    class AssetManager;
    class IAppState;
    class ITimer;

    class Engine {
        public:
        Engine();
        ~Engine();

        // The main phases of the engine's lifecycle
        // Added target fps to Engine's initialize method, to implement framerate limtting.
        bool initialize(const WindowConfig& config, TimerType timer_type = TimerType::SDL, int target_fps = 60);
        void run();
        void shutdown();
        
        // getters for the states to access the core systems.
        IRenderer* get_renderer() const { return renderer; }
        AssetManager* get_asset_manager() const { return asset_manager;}
        IInputManager* get_input_manager() const;
        
        // A public method to allow states to request a change.
        void switch_state(AppStateType new_state_type); 

        private:
        void process_input(float delta_time);
        void update(float delta_time);
        void render();

        bool is_running;

        // Engine owns the low-level systems.
        AssetManager* asset_manager;
        IRenderer* renderer;
        std::unique_ptr<IInputManager> input_manager;

        // The Engine now owns the current state.
        std::unique_ptr<IAppState> current_state;

        // The Engine now owns the master clock.
        std::unique_ptr<ITimer> timer;
    };
} // namespace Salix