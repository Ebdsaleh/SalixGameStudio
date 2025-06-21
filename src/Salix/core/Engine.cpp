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
#include <Salix/states/IAppState.h>
#include <Salix/states/LaunchState.h>
#include <Salix/states/EditorState.h>
#include <Salix/states/OptionsMenuState.h>
#include <Salix/events/IEventPoller.h> // Needed for PIMPL
#include <Salix/events/SDLEventPoller.h>
#include <Salix/events/EventManager.h>

#include <SDL.h>
#include <iostream>

namespace Salix {
    // --- The PIMPL Struct ---
    // Define the actual implementation details here, hidden from the world.
    struct Engine::Pimpl {
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

    // --- The Constructor and Destructor ---
    // These MUST be defined in the .cpp file where Pimpl is fully defined.
    Engine::Engine() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->is_running = false;
    }

    Engine::~Engine() = default;

    bool Engine::initialize(const WindowConfig& config, RendererType renderer_type, TimerType timer_type, int target_fps) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
            std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        switch (renderer_type) {
            case RendererType::SDL:
                pimpl->renderer = std::make_unique<SDLRenderer>();
                break;
            default:
                std::cerr << "Engine::initialize - Invalid or unsupported renderer type requested!" << std::endl;
                SDL_Quit();
                return false;
        }

        // <<< FIX: Must use pimpl-> to access the renderer
        if (!pimpl->renderer->initialize(config)) {
            std::cerr << "Engine::initialize - Renderer subsystem failed to initialize." << std::endl;
            return false;
        }

        pimpl->asset_manager = std::make_unique<AssetManager>();
        // <<< FIX: Must use pimpl-> to access both managers and the renderer
        pimpl->asset_manager->initialize(pimpl->renderer.get());

        switch (timer_type) {
            case TimerType::SDL:
                pimpl->timer = std::make_unique<SDLTimer>();
                break;
            case TimerType::Chrono:
                pimpl->timer = std::make_unique<ChronoTimer>();
                break;
            default:
                std::cerr << "Engine::initialize - Invalid or unsupported timer type requested." << std::endl;
                return false;
        }
        // <<< FIX: Must use pimpl-> to access the timer
        pimpl->timer->set_target_fps(target_fps);

        pimpl->input_manager = std::make_unique<SDLInputManager>();
        pimpl->event_manager = std::make_unique<EventManager>();
        pimpl->event_poller = std::make_unique<SDLEventPoller>();

        // <<< FIX: Must use pimpl-> to access the current_state
        pimpl->current_state = std::make_unique<LaunchState>();
        pimpl->current_state->on_enter(this);

        pimpl->is_running = true;
        std::cout << "Engine initialized successfully." << std::endl;
        return true;
    }

    void Engine::run() {
        // <<< FIX: Must use pimpl-> to access is_running and timer
        while (pimpl->is_running) {
            pimpl->timer->tick_start();
            float delta_time = pimpl->timer->get_delta_time();

            process_input();
            update(delta_time);
            render();

            pimpl->timer->tick_end();
        }
    }

    void Engine::shutdown() {
        std::cout << "Shutting down engine." << std::endl;
        // <<< FIX: Must use pimpl-> to access all subsystems
        pimpl->event_poller.reset();
        pimpl->event_manager.reset();
        pimpl->current_state.reset();
        pimpl->input_manager.reset();
        pimpl->timer.reset();
        pimpl->asset_manager.reset();
        pimpl->renderer.reset();

        SDL_Quit();
    }

    void Engine::process_input() {
        // <<< FIX: Must use pimpl-> here
        pimpl->event_poller->poll_events([&](IEvent& event) {
            pimpl->input_manager->process_event(event);
            pimpl->event_manager->dispatch(event);
        });

        if (pimpl->input_manager->wants_to_quit()) {
            pimpl->is_running = false;
        }
    }

    void Engine::switch_state(AppStateType /*new_state_type*/) {
        // (This logic will need to be implemented later using a factory)
    }

    void Engine::update(float delta_time) {
        // <<< FIX: Must use pimpl-> here
        if (pimpl->current_state) {
            pimpl->current_state->update(delta_time);
        }
        if (pimpl->input_manager) {
            pimpl->input_manager->update(delta_time);
        }
    }

    void Engine::render() {
        // <<< FIX: Must use pimpl-> here
        if (!pimpl->renderer) return;

        pimpl->renderer->begin_frame();

        if (pimpl->current_state) {
            pimpl->current_state->render(pimpl->renderer.get());
        }

        pimpl->renderer->end_frame();
    }

    // --- GETTERS (Also need to be updated) ---
    IRenderer* Engine::get_renderer() { return pimpl->renderer.get(); }
    AssetManager* Engine::get_asset_manager() { return pimpl->asset_manager.get(); }
    IInputManager* Engine::get_input_manager() { return pimpl->input_manager.get(); }
    EventManager* Engine::get_event_manager() { return pimpl->event_manager.get(); }
    bool Engine::is_running() const { return pimpl->is_running; }
    void Engine::push_state(IAppState* /*state*/) {} // TODO
    void Engine::pop_state() {} // TODO
    void Engine::change_state(IAppState* /*state*/) {} // TODO

} // namespace Salix