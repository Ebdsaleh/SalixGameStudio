// =================================================================================
// Filename:    Salix/core/Engine.h
// Author:      SalixGameStudio
// Description: Declares the main Engine class, which orchestrates all subsystems.
// =================================================================================
#pragma once

// Include the headers for types used in the function signatures.
#include <Salix/core/EngineInterface.h>
#include <Salix/rendering/IRenderer.h>  // For RendererType.
#include <Salix/core/ITimer.h>          // For TimerType.
#include <Salix/window/IWindow.h>       // For WindowConfig.
#include <Salix/states/IAppState.h>     // This defines AppStateType.
#include <Salix/core/EngineMode.h>
#include <memory>                       // For std::unique_ptr.
#include <vector>                       // For the state stack.

namespace Salix {
    // Forward declarations for types we only hold pointers to.
    class IInputManager;
    class AssetManager;
    class IEventPoller;
    class EventManager;
    struct InitContext;
    
    class SALIX_API Engine : public EngineInterface {
        public:
            Engine();
            ~Engine();

            // The main phases of the engine's lifecycle.
            // This new signature solves the circular dependency and is very clear.
            bool initialize(const WindowConfig& config, RendererType renderer_type, TimerType timer_type, int target_fps = 60);
            void run();
            void shutdown();

            // Getters for states to access the core systems.
            IRenderer* get_renderer();
            AssetManager* get_asset_manager();
            IInputManager* get_input_manager();
            EventManager* get_event_manager();
            bool is_running() const;
            void push_state(IAppState* state);
            void pop_state();
            void change_state(IAppState* state);
            // A public method to allow states to request a change.
            void switch_state(AppStateType new_state_type) override; 
            void set_mode(EngineMode engine_mode) override;
            EngineMode get_mode() const override;
            InitContext make_context() const;
        private:
            void process_input();
            void update(float delta_time);
            void render();

            struct Pimpl; // Forward-declare the private implementation struct.
            std::unique_ptr<Pimpl> pimpl;

            // The Engine now safely owns all its subsystems via smart pointers.
    };

} // namespace Salix