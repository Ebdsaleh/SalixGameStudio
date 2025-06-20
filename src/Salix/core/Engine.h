// =================================================================================
// Filename:    Salix/core/Engine.h
// Author:      SalixGameStudio
// Description: Declares the main Engine class, which orchestrates all subsystems.
// =================================================================================
#pragma once

// Include the headers for types used in the function signatures.
#include <Salix/rendering/IRenderer.h>  // For RendererType
#include <Salix/core/ITimer.h>          // For TimerType
#include <Salix/window/IWindow.h>       // For WindowConfig
#include <Salix/states/IAppState.h>     // This defines AppStateType
#include <memory>                       // For std::unique_ptr
#include <vector>                       // For the state stack

namespace Salix {
    // Forward declarations for types we only hold pointers to.
    class IInputManager;
    class AssetManager;
    class IEventPoller;
    class EventManager;
    
    class Engine {
    public:
        Engine();
        ~Engine();

        // The main phases of the engine's lifecycle.
        // This new signature solves the circular dependency and is very clear.
        bool initialize(const WindowConfig& config, RendererType renderer_type, TimerType timer_type, int target_fps = 60);
        void run();
        void shutdown();

        // Getters for states to access the core systems.
        IRenderer* get_renderer() { return renderer.get(); }
        AssetManager* get_asset_manager() { return asset_manager.get(); }
        IInputManager* get_input_manager() { return input_manager.get(); }
        EventManager* get_event_manager() { return event_manager.get(); }
        // A public method to allow states to request a change.
        void switch_state(AppStateType new_state_type); 

    private:
        void process_input();
        void update(float delta_time);
        void render();

        bool is_running;

        // The Engine now safely owns all its subsystems via smart pointers.
        std::unique_ptr<IRenderer> renderer;
        std::unique_ptr<AssetManager> asset_manager;
        std::unique_ptr<IInputManager> input_manager;
        std::unique_ptr<ITimer> timer;
        std::unique_ptr<IEventPoller> event_poller;
        std::unique_ptr<EventManager> event_manager;
        // State stack logic...
        std::unique_ptr<IAppState> current_state;
    };

} // namespace Salix