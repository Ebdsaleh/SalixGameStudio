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
        virtual bool is_down(KeyCode key) const = 0;
        virtual bool is_held_down(KeyCode key) const = 0;
        virtual bool is_held_down_for(KeyCode key) const = 0;
        virtual bool was_just_released(KeyCode key) const = 0;
        virtual bool was_released(KeyCode key) const = 0;
        virtual bool is_up(KeyCode key) const = 0;
        virtual bool multiple_are_down(std::vector<KeyCode> keys) const = 0;
        virtual bool multiple_are_held_down(std::vector<KeyCode> keys) const = 0;
        virtual bool multiple_are_held_down_for(std::vector<KeyCode> keys) const = 0;
        virtual bool multiple_were_just_released(std::vector<KeyCode> keys) const = 0;
        virtual bool multiple_were_released(std::vector<KeyCode> keys) const = 0;
        virtual bool multiple_are_up(std::vector<KeyCode> keys) const = 0;

        // Mouse Queries
        // Mouse Buttons
        virtual bool is_down(MouseButton button) const = 0;
        virtual bool is_held_down(MouseButton button) const = 0;
        virtual bool is_held_down_for(MouseButton button) const = 0;
        virtual bool was_just_released(MouseButton button) const = 0;
        virtual bool was_released(MouseButton button) const = 0;
        virtual bool is_up(MouseButton button) const = 0;
        virtual bool multiple_are_down(std::vector<MouseButton> buttons) = 0;
        virtual bool multiple_are_held_down(std::vector<MouseButton> buttons) = 0;
        virtual bool multiple_are_held_down_for(std::vector<MouseButton> buttons) = 0;
        virtual bool multiple_were_just_released(std::vector<MouseButton> buttons) = 0;
        virtual bool multiple_were_released(std::vector<MouseButton> buttons) = 0;
        virtual bool multiple_are_up(std::vector<MouseButton> buttons) = 0;

        // Additional queries
        virtual void get_mouse_position(int* x, int* y) const = 0;
        virtual bool wants_to_quit() const = 0;
    
};