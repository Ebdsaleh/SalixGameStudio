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
    struct ApplicationConfig;
    class ThemeReloadEvent;
    class SALIX_API Engine : public EngineInterface {
        public:
            Engine();
            ~Engine();

            // The main phases of the engine's lifecycle.
            // This new signature solves the circular dependency and is very clear.
            bool initialize(const ApplicationConfig& config);
            void run();
            void shutdown();

            // Getters for states to access the core systems.
            IRenderer* get_renderer();
            AssetManager* get_asset_manager();
            IInputManager* get_input_manager();
            EventManager* get_event_manager();
            bool is_running() const override;
            bool const is_running(bool keep_running) override;
            void push_state(IAppState* state);
            void pop_state();
            void change_state(IAppState* state);
            // A public method to allow states to request a change.
            void switch_state(AppStateType new_state_type) override; 
            void set_mode(EngineMode engine_mode) override;
            EngineMode get_mode() const override;
            InitContext make_context() const;
            float get_time_scale() const override;
            void set_time_scale(float new_time_scale) override;
            #ifdef SALIX_TESTS_ENABLED
            static void set_force_ttf_init_failure_for_testing(bool enabled);
            static void set_force_application_listener_init_failure_for_testing(bool enabled);
            static void set_force_gui_init_failure_for_testing(bool enabled);
            static void set_force_sdl_gui_cast_failure_for_testing(bool enabled);
            static void set_force_opengl_gui_cast_failure_for_testing(bool enabled);
            static void set_force_gui_event_polling_renderer_mismatch_for_testing(bool enabled);
            void run_theme_manager_init_failure_for_testing();
            void run_font_manager_init_failure_for_testing();
            void run_null_icon_manager_setup_for_testing();
            #endif
        private:
            void process_input();
            void update(float delta_time);
            void render();

            struct Pimpl; // Forward-declare the private implementation struct.
            std::unique_ptr<Pimpl> pimpl;

            // The Engine now safely owns all its subsystems via smart pointers.
    };

} // namespace Salix