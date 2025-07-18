// Salix/input/IInputManager.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/input/InputCodes.h>
#include <Salix/events/IEvent.h>
#include <vector>

namespace Salix {

    enum class InputState {
        Up,         // The key is not pressed.
        Down,       // The key was just pressed THIS FRAME.
        Held,       // The key was already down last frame and still is.
        Released    // The key was just released THIS FRAME.
    };

    class SALIX_API IInputManager {
        public:
            virtual ~IInputManager() = default;

            // The main update call that processes events for the frame.
            virtual void update(float delta_time) = 0;

            // NEW: This is the primary input method. The engine sends events here.
            virtual void process_event(IEvent& event) = 0;

            // Keyboard Queries

            // Returns true the moment the frame detects the input
            virtual bool is_down(KeyCode key) const = 0;

            // Returns true if the input 'is still down' from the previous frame-cycle.
            virtual bool is_held_down(KeyCode key) const = 0;

            // Returns true if the input 'is held down' for the target_duration value (measured in seconds).
            virtual bool is_held_down_for(KeyCode key, float duration) const = 0;

            // Returns true when the input goes from 'down to up' (is_down to is_up).
            virtual bool was_released(KeyCode key) const = 0;

            // Returns true if the input is not detect this frame. 
            virtual bool is_up(KeyCode key) const = 0;

            virtual bool multiple_are_down(const std::vector<KeyCode>& keys) const = 0;
            virtual bool multiple_are_held_down(const std::vector<KeyCode>& keys) const = 0;
            virtual bool multiple_are_held_down_for(const std::vector<KeyCode>& keys, float duration) const = 0;
            virtual bool multiple_were_released(const std::vector<KeyCode>& keys) const = 0;
            virtual bool multiple_are_up(const std::vector<KeyCode>& keys) const = 0;

            // Mouse Queries
            // Mouse Buttons
            virtual bool is_down(MouseButton button) const = 0;
            virtual bool is_held_down(MouseButton button) const = 0;
            virtual bool is_held_down_for(MouseButton button, float duration) const = 0;
            virtual bool was_released(MouseButton button) const = 0;
            virtual bool is_up(MouseButton button) const = 0;
            virtual bool multiple_are_down(const std::vector<MouseButton>& buttons) const = 0;
            virtual bool multiple_are_held_down(const std::vector<MouseButton>& buttons) const= 0;
            virtual bool multiple_are_held_down_for(const std::vector<MouseButton>& buttons, float duration) const = 0;
            virtual bool multiple_were_released(const std::vector<MouseButton>& buttons) const = 0;
            virtual bool multiple_are_up(const std::vector<MouseButton>& buttons) const = 0;

            virtual bool did_scroll(MouseScroll direction) = 0;
            // Additional queries
            virtual void get_mouse_position(int* x, int* y) const = 0;
            virtual float get_mouse_scroll_delta() const = 0;
            virtual bool wants_to_quit() const = 0;
    };
} // namespace Salix