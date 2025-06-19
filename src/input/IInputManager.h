// IInputManager.h
#pragma once
#include "InputCodes.h"
#include <vector>


class IInputManager {
    public:
        virtual ~IInputManager() = default;

        // The main update call that processes events for the frame.
        virtual void update() = 0;

        // Keyboard Queries

        // Returns true the moment the frame detects the input
        virtual bool is_down(KeyCode key) const = 0;

        // Returns true if the input 'is still down' from the previous frame
        virtual bool is_held_down(KeyCode key) const = 0;

        // Returns true if the input 'is held down' for the target_duration value (measured in seconds).
        virtual bool is_held_down_for(KeyCode key, int target_duration) const = 0;

        // Returns true when the input goes from 'down to up' (is_down to is_up).
        virtual bool was_released(KeyCode key) const = 0;

        //Returns true when the input goes from 'down to up, after being in the 'is_held_down' state.
        virtual bool was_just_released(KeyCode key) const = 0;
        
        // Returns true if the input is not detect this frame. 
        virtual bool is_up(KeyCode key) const = 0;

        virtual bool multiple_are_down(std::vector<KeyCode>& keys) const = 0;
        virtual bool multiple_are_held_down(std::vector<KeyCode>& keys) const = 0;
        virtual bool multiple_are_held_down_for(std::vector<KeyCode>& keys, int target_duration) const = 0;
        virtual bool multiple_were_just_released(std::vector<KeyCode>& keys) const = 0;
        virtual bool multiple_were_released(std::vector<KeyCode>& keys) const = 0;
        virtual bool multiple_are_up(std::vector<KeyCode>& keys) const = 0;

        // Mouse Queries
        // Mouse Buttons
        virtual bool is_down(MouseButton button) const = 0;
        virtual bool is_held_down(MouseButton button) const = 0;
        virtual bool is_held_down_for(MouseButton button, int target_duration) const = 0;
        virtual bool was_just_released(MouseButton button) const = 0;
        virtual bool was_released(MouseButton button) const = 0;
        virtual bool is_up(MouseButton button) const = 0;
        virtual bool multiple_are_down(std::vector<MouseButton>& buttons) const = 0;
        virtual bool multiple_are_held_down(std::vector<MouseButton>& buttons) const= 0;
        virtual bool multiple_are_held_down_for(std::vector<MouseButton>& buttons, int target_duration) const = 0;
        virtual bool multiple_were_just_released(std::vector<MouseButton>& buttons) const = 0;
        virtual bool multiple_were_released(std::vector<MouseButton>& buttons) const = 0;
        virtual bool multiple_are_up(std::vector<MouseButton>& buttons) const = 0;

        // Additional queries
        virtual void get_mouse_position(int* x, int* y) const = 0;
        virtual bool wants_to_quit() const = 0;
    
};