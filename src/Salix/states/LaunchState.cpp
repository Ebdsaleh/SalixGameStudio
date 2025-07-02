// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/IInputManager.h>
#include <iostream>

namespace Salix {

    LaunchState::LaunchState() : input_manager_ptr(nullptr){}
    LaunchState::~LaunchState() {}

    void LaunchState::on_enter(const InitContext& new_context) {
        std::cout << "Entering LaunchState..." << std::endl;
        context = new_context;
        context.engine->set_mode(EngineMode::Launch);
        input_manager_ptr = context.input_manager;
    }

    void LaunchState::on_exit() {
        std::cout << "Exiting LaunchState..." << std::endl;
    }

    void LaunchState::update(float /*delta_time*/) {
        // In the future we'll check for button clicks here.
        if (input_manager_ptr->is_down(KeyCode::E)) {
            // Switch to 'EditorState'.
            std::cout << "'E' key press detected. Switching to EditorState." << std::endl;
            context.engine->switch_state(AppStateType::Editor);
        }
        
        if (input_manager_ptr->is_down(KeyCode::G)) {
            // Switch to 'GameState'.
            std::cout << "'G' key press detected.Switching to GameState." << std::endl;
            context.engine->switch_state(AppStateType::Game);
        }
        
        if (input_manager_ptr->is_down(KeyCode::O)) {
            // Switch to 'OptionsMenuState
            std::cout << "'O' key press detected.Switching to OptionsMenuState." << std::endl;
            context.engine->switch_state(AppStateType::Options);
        }
    }
    

    void LaunchState::render(IRenderer* /*renderer*/) {
        // This state has no game world to render, but it might have UI.
        // For now, this does nothing.
    }
} // namespace Salix