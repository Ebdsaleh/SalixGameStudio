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

    void LaunchState::update(float delta_time) {
        // In the future we'll check for button clicks here.
        if (input_manager_ptr->is_down(KeyCode::E)) std::cout << "'E' key press detected. Switching to EditorState." << std::endl;
        
        if (input_manager_ptr->is_down(KeyCode::G)) std::cout << "'G' key press detected.Switching to GameState." << std::endl;
        
        if (input_manager_ptr->is_down(KeyCode::O)) std::cout << "'O' key press detected.Switching to OptionsMenuState." << std::endl;
        bool reached_held_down_state = false;
        if (input_manager_ptr->is_held_down_for(KeyCode::Space, 2)) { 
            reached_held_down_state = true;
            std::cout << "Hey, Why so much whitespace?! Take your thumb off the SPACEBAR!!!" << std::endl;
        }

        if (input_manager_ptr->was_released(KeyCode::Space)) {
            std::cout << "There you go! See, that wasn't so hard, now was it? Hmph! Developers..." << std::endl;
            reached_held_down_state = false;
        }
    }
    

    void LaunchState::render(IRenderer* renderer) {
        // This state has no game world to render, but it might have UI.
        // For now, this does nothing.
    }
} // namespace Salix