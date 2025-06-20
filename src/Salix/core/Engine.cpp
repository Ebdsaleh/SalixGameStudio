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

    // --- FIX #1: Updated method signature ---
    // The signature now matches the one in our new Engine.h.
    bool Engine::initialize(const WindowConfig& config, RendererType renderer_type, TimerType timer_type, int target_fps) {
        // --- 1: CONTEXT INITIALIZATION (SDL) ---
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
            std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // --- 2: RENDERER INITIALIZATION ---
        // --- FIX #2: Use the renderer_type parameter and std::make_unique ---
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
        // --- FIX #3: Use std::make_unique for safe memory management ---
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

        // --- 6: STATE MACHINE INITIALIZATION ---
        // TODO: This logic for state switching should be moved into a dedicated StateManager class
        // For now, setting the initial state directly is okay.
        current_state = std::make_unique<LaunchState>();
        current_state->on_enter(this);

        // --- 7: START THE ENGINE ---
        is_running = true;
        std::cout << "Engine initialized successfully." << std::endl;
        return true;
    }

    // --- The Corrected Main Loop ---
    void Engine::run() {
        while (is_running) {
            timer->tick_start();
            float delta_time = timer->get_delta_time();

            // The order is now: Process Events, then Update based on new state.
            process_input();
            update(delta_time);
            render();

            timer->tick_end();
        }
    }

    void Engine::shutdown() {
        std::cout << "Shutting down engine." << std::endl;

        // --- FIX #4: Correct shutdown order using smart pointers ---
        // Shutdown subsystems in the reverse order of creation (LIFO).
        // The .reset() call will destroy the object and call its destructor.
        current_state.reset();
        input_manager.reset();
        timer.reset();
        asset_manager.reset();
        renderer.reset(); // This will destroy the renderer and the window it owns.

        // Finally, quit the underlying SDL system.
        SDL_Quit();
    }

    // --- THE NEW, CORRECTED process_input METHOD ---
    void Engine::process_input() {
        // This is the logic from our old EventHandler, now correctly placed in the Engine.
        // It polls for raw SDL events, translates them into our abstract Salix::IEvent objects,
        // and then sends them to the InputManager.

        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) { // This "answers the phone" and prevents the freeze!
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                {
                    WindowCloseEvent event;
                    input_manager->process_event(event);
                    break;
                }
                case SDL_KEYDOWN:
                {
                    // Ignore key repeats for the process_event part of our logic
                    if (sdl_event.key.repeat == 0) {
                        KeyPressedEvent event(sdl_event.key.keysym.sym);
                        input_manager->process_event(event);
                    }
                    break;
                }
                case SDL_KEYUP:
                {
                    KeyReleasedEvent event(sdl_event.key.keysym.sym);
                    input_manager->process_event(event);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    MouseButtonPressedEvent event(sdl_event.button.button);
                    input_manager->process_event(event);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                {
                    MouseButtonReleasedEvent event(sdl_event.button.button);
                    input_manager->process_event(event);
                    break;
                }
                case SDL_MOUSEMOTION:
                {
                    MouseMovedEvent event((float)sdl_event.motion.x, (float)sdl_event.motion.y);
                    input_manager->process_event(event);
                    break;
                }
                 // Add other events like MouseScrolled here if needed
            }
        }

        // After processing all events, check if the Input Manager received a quit event.
        if (input_manager->wants_to_quit()) {
            is_running = false;
        }
    }

    // TODO: This state switching logic should be moved to a StateManager class.
    void Engine::switch_state(AppStateType /*new_state_type*/) {
        // ... (Your state switching logic is good, but belongs in a manager) ...
    }

    void Engine::update(float delta_time) {
        // --- THE FIX IS HERE ---
        // We must update the game logic FIRST, so it can see the single-frame
        // "Down" and "Released" events from the most recent process_input() call.

        // 1. Update the current game state.
        if (current_state) {
            current_state->update(delta_time);
        }

        // 2. THEN, update the input manager. This transitions its internal states
        // (Down -> Held, Released -> Up) in preparation for the NEXT frame.
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