// =================================================================================
// Filename:    Salix/core/Engine.cpp
// Author:      SalixGameStudio
// Description: Implements the main Engine class, which orchestrates all subsystems.
// =================================================================================
#include <Salix/core/Engine.h>

// Include all required subsystem headers
#include <Salix/core/SDLTimer.h>
#include <Salix/core/ChronoTimer.h>
#include <Salix/input/SDLInputManager.h>
#include <Salix/rendering/SDLRenderer.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/states/IAppState.h> // Include this for the state logic
#include <Salix/states/LaunchState.h>
#include <Salix/states/GameState.h>
#include <Salix/states/EditorState.h>
#include <Salix/states/OptionsMenuState.h>
#include <Salix/events/SDLEvent.h> // <<< CRITICAL: Needed to create our events
#include <Salix/events/SDLEventPoller.h>
#include <Salix/events/EventManager.h>
#include <SDL.h>
#include <iostream>

namespace Salix {

    Engine::Engine() : is_running(false) {
        // Constructor: The unique_ptrs are automatically initialized to nullptr.
    }

    Engine::~Engine() {
        // The unique_ptrs' destructors will automatically clean up any remaining
        // resources, but our explicit shutdown() is the correct place for this.
    }

    // The signature now matches the one in our new Engine.h.
    bool Engine::initialize(const WindowConfig& config, RendererType renderer_type, TimerType timer_type, int target_fps) {
        // --- 1: CONTEXT INITIALIZATION (SDL) ---
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
            std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // --- 2: RENDERER INITIALIZATION ---
        // --- Use the renderer_type parameter and std::make_unique ---
        switch (renderer_type) {
            case RendererType::SDL:
                renderer = std::make_unique<SDLRenderer>();
                break;
            // Add other cases for Vulkan, OpenGL etc. here in the future
            default:
                std::cerr << "Engine::initialize - Invalid or unsupported renderer type requested!" << std::endl;
                SDL_Quit();
                return false;
        }

        // Now, initialize the renderer we just created.
        if (!renderer->initialize(config)) {
            std::cerr << "Engine::initialize - Renderer subsystem failed to initialize." << std::endl;
            // The unique_ptr will automatically be cleaned up, so we just return.
            return false;
        }

        // --- 3: ASSET MANAGER INITIALIZATION ---
        // --- Use std::make_unique for safe memory management ---
        asset_manager = std::make_unique<AssetManager>();
        asset_manager->initialize(renderer.get()); // Pass the raw pointer

        // --- 4: TIMER INITIALIZATION ---
        switch (timer_type) {
            case TimerType::SDL:
                timer = std::make_unique<SDLTimer>();
                break;
            case TimerType::Chrono:
                timer = std::make_unique<ChronoTimer>();
                break;
            default:
                std::cerr << "Engine::initialize - Invalid or unsupported timer type requested." << std::endl;
                return false;
        }
        timer->set_target_fps(target_fps);

        // --- 5: INPUT MANAGER INITIALIZATION ---
        input_manager = std::make_unique<SDLInputManager>();

        // --- 6: EVENT MANAGER INITIALIZATION ---
        event_manager = std::make_unique<EventManager>();

        // --- 7: EVENT POLLER INITIALIZATION ---
        event_poller = std::make_unique<SDLEventPoller>();

        // --- 8: STATE MACHINE INITIALIZATION ---
        // TODO: This logic for state switching should be moved into a dedicated StateManager class
        // For now, setting the initial state directly is okay.
        current_state = std::make_unique<LaunchState>();
        current_state->on_enter(this);

        

        // --- 8: START THE ENGINE ---
        is_running = true;
        std::cout << "Engine initialized successfully." << std::endl;
        return true;
    }

    void Engine::run() {
        while (is_running) {
            timer->tick_start();
            float delta_time = timer->get_delta_time();

            // This order is now correct and will remain correct.
            process_input();
            update(delta_time);
            render();

            timer->tick_end();
        }
    }

    void Engine::shutdown() {
        std::cout << "Shutting down engine." << std::endl;

        // --- Correct shutdown order using smart pointers ---
        // Shutdown subsystems in the reverse order of creation (LIFO).
        // The .reset() call will destroy the object and call its destructor.
        event_poller.reset();
        event_manager.reset();
        current_state.reset();
        input_manager.reset();
        timer.reset();
        asset_manager.reset();
        renderer.reset(); // This will destroy the renderer and the window it owns.

        // Finally, quit the underlying SDL system.
        SDL_Quit();
    }

    // THIS METHOD IS NOW 100% DECOUPLED FROM SDL
    void Engine::process_input() {
        // --- THE NEW, DUAL-DISPATCH SYSTEM ---
        // The engine polls for events and sends them to both the InputManager (for state polling)
        // and the EventManager (for event listening).
        event_poller->poll_events([&](IEvent& event) {
            // 1. Send to the InputManager to update its internal state maps.
            input_manager->process_event(event);

            // 2. Dispatch to the EventManager to notify any listeners (like UI).
            event_manager->dispatch(event);

            // Note: If the event was handled (e.g., by a UI element),
            // the dispatch function will stop propagation, and subsequent listeners
            // might not receive the event, which is correct behavior for UI.
        });

        if (input_manager->wants_to_quit()) {
            is_running = false;
        }
    }


    // TODO: This state switching logic should be moved to a StateManager class.
    void Engine::switch_state(AppStateType /*new_state_type*/) {
        // ... (Your state switching logic is good, but belongs in a manager) ...
    }

    void Engine::update(float delta_time) {
        // This order is correct and final.
        // 1. Update game logic.
        if (current_state) {
            current_state->update(delta_time);
        }
        // 2. Update input manager's internal state machine.
        if (input_manager) {
            input_manager->update(delta_time);
        }
    }

    void Engine::render() {
        if (!renderer) return;

        renderer->begin_frame();

        if (current_state) {
            current_state->render(renderer.get());
        }

        renderer->end_frame();
    }

} // namespace Salix