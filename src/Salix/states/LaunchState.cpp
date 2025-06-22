// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/Engine.h>
#include <Salix/input/IInputManager.h>
#include <iostream>

namespace Salix {

    LaunchState::LaunchState() : engine_ptr(nullptr), input_manager_ptr(nullptr){}
    LaunchState::~LaunchState() {}

    void LaunchState::on_enter(Engine* engine) {
        std::cout << "Entering LaunchState..." << std::endl;
        engine_ptr = engine;
        input_manager_ptr = engine_ptr->get_input_manager();
    }

    void LaunchState::on_exit() {
        std::cout << "Exiting LaunchState..." << std::endl;
    }

    void LaunchState::update(float /*delta_time*/) {
        // In the future we'll check for button clicks here.
        if (input_manager_ptr->is_down(KeyCode::E)) {
            // Switch to 'EditorState'.
            std::cout << "'E' key press detected. Switching to EditorState." << std::endl;
            engine_ptr->switch_state(AppStateType::Editor);
        }
        
        if (input_manager_ptr->is_down(KeyCode::G)) {
            // Switch to 'GameState'.
            std::cout << "'G' key press detected.Switching to GameState." << std::endl;
            engine_ptr->switch_state(AppStateType::Game);
        }
        
        if (input_manager_ptr->is_down(KeyCode::O)) {
            // Switch to 'OptionsMenuState
            std::cout << "'O' key press detected.Switching to OptionsMenuState." << std::endl;
            engine_ptr->switch_state(AppStateType::Options);
        }
    }
    

    void LaunchState::render(IRenderer* /*renderer*/) {
        // This state has no game world to render, but it might have UI.
        // For now, this does nothing.
    }
} // namespace Salix