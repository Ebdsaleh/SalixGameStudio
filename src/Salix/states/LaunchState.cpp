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

        // Multiple key input test - typical save command, 'left control' + 'S'.
        if (input_manager_ptr->multiple_are_down({KeyCode::LeftControl, KeyCode::S})) {
            std::cout << "Save Command has been issued. I heard the 'Ctrl + S'." << std::endl;
        }
        // Multiple key input test, hold 'ctrl' + 'left shift' + '0' for 1 second.
        if (input_manager_ptr->multiple_are_held_down_for( {
            KeyCode::LeftControl, KeyCode::LeftShift, KeyCode::P}
            , 1.0f)) 
            {
                std::cout<< "Ehhmm!, Why are you holding 'Ctrl' + 'Shift', and 'P'?" << std::endl;
            }
        
        // Alpha key tests horizontal-row of numbers at the top of the keyboard.
        if (input_manager_ptr->is_down(KeyCode::Alpha0)) {
            std::cout << "Alpha0 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha1)) {
            std::cout << "Alpha1 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha2)) {
            std::cout << "Alpha2 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha3)) {
            std::cout << "Alpha3 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha4)) {
            std::cout << "Alpha4 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha5)) {
            std::cout << "Alpha5 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha6)) {
            std::cout << "Alpha6 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha7)) {
            std::cout << "Alpha7 key pressed." << std::endl;
        }
        
        if (input_manager_ptr->is_down(KeyCode::Alpha8)) {
            std::cout << "Alpha8 key pressed." << std::endl;
        }

        if (input_manager_ptr->is_down(KeyCode::Alpha9)) {
            std::cout << "Alpha9 key pressed." << std::endl;
        }
    }
    

    void LaunchState::render(IRenderer* /*renderer*/) {
        // This state has no game world to render, but it might have UI.
        // For now, this does nothing.
    }
} // namespace Salix