// =================================================================================
// Filename:    Salix/input/sdl/SDLInputManager.cpp
// Author:      SalixGameStudio
// Description: Implements the stateful, event-driven, and overloaded IInputManager
//              interface using SDL as the backend.
// =================================================================================
#include <Salix/input/sdl/SDLInputManager.h>
#include <Salix/events/sdl/SDLEvent.h> // Needed for casting to get event data
#include <SDL.h>   
#include <iostream>                // For SDLK_* keycodes in the translator

namespace Salix {

    SDLInputManager::SDLInputManager() :
        mouse_x(0),
        mouse_y(0),
        quit_requested(false)
    {
        // --- CONSTRUCTOR ---
        // Pre-populate our state maps. This is crucial for our logic.
        // Every possible key and button starts in the 'Up' state with a 0-second timer.

        // Initialize all keyboard keys.
        for (int i = static_cast<int>(KeyCode::None) + 1; i <= static_cast<int>(KeyCode::PrintScreen); ++i) {
            KeyCode key = static_cast<KeyCode>(i);
            key_states[key] = InputState::Up;
            key_held_durations[key] = 0.0f;
        }

        // Initialize all mouse buttons.
        for (int i = static_cast<int>(MouseButton::None) + 1; i <= static_cast<int>(MouseButton::Right); ++i) {
            MouseButton button = static_cast<MouseButton>(i);
            mouse_button_states[button] = InputState::Up;
            mouse_button_held_durations[button] = 0.0f;
        }
    }

    SDLInputManager::~SDLInputManager() {}

    void SDLInputManager::process_event(IEvent& event) {
        // --- LIFECYCLE METHOD 1: RECEIVE EVENTS ---
        // Called by the Engine for each event. Updates our internal state maps based on what just happened.
        // This is the "ON" / "OFF" part of your analogy.

        if (event.is_in_category(EventCategory::Keyboard)) {
            // It's a keyboard event, find out which key.
            auto& key_event = static_cast<KeyEvent&>(event);

            KeyCode key = to_salix_keycode(key_event.get_key_code());

            if (key != KeyCode::None) {
                if (event.get_event_type() == EventType::KeyPressed) {
                    // Only trigger "Down" if it was previously up. This handles key repeats correctly.
                    if (key_states[key] == InputState::Up || key_states[key] == InputState::Released) {
                        key_states[key] = InputState::Down;
                    }
                } else if (event.get_event_type() == EventType::KeyReleased) {
                    key_states[key] = InputState::Released;
                }
            }
        }

        else if (event.is_in_category(EventCategory::MouseButton)) {
            // It's a mouse button event.
            auto& mouse_event = static_cast<MouseButtonEvent&>(event);
            MouseButton button = static_cast<MouseButton>(mouse_event.get_mouse_button());

            if (mouse_button_states.count(button)) {
                if (event.get_event_type() == EventType::MouseButtonPressed) {
                    if (mouse_button_states[button] == InputState::Up || mouse_button_states[button] == InputState::Released) {
                        mouse_button_states[button] = InputState::Down;
                    }
                } else if (event.get_event_type() == EventType::MouseButtonReleased) {
                    mouse_button_states[button] = InputState::Released;
                }
            }
        }

        else if (event.get_event_type() == EventType::MouseMoved) {
            auto& mouse_event = static_cast<MouseMovedEvent&>(event);
            mouse_x = static_cast<int>(mouse_event.get_x());
            mouse_y = static_cast<int>(mouse_event.get_y());
        }

        else if (event.get_event_type() == EventType::MouseScrolled){
            auto& scroll_event = static_cast<MouseScrolledEvent&>(event);
            // Store the vertical scroll amount for this frame
            mouse_scroll_this_frame = scroll_event.get_y_offset();

        }


        else if (event.get_event_type() == EventType::WindowClose) {
            quit_requested = true;
        }

        
    }

    void SDLInputManager::update(float delta_time) {
        // --- LIFECYCLE METHOD 2: ADVANCE TIME ---
        // Called once per frame. Handles time-based transitions and updates duration timers.
        // This is the "was ON and still is ON" analogy.

        mouse_scroll_this_frame = 0.0f;

        // 1. Transition single-frame states ('Down', 'Released') to multi-frame states ('Held', 'Up').
        for (auto& pair : key_states) {
            if (pair.second == InputState::Down)       pair.second = InputState::Held;
            else if (pair.second == InputState::Released) pair.second = InputState::Up;
        }

        for (auto& pair : mouse_button_states) {
            if (pair.second == InputState::Down)       pair.second = InputState::Held;
            else if (pair.second == InputState::Released) pair.second = InputState::Up;
        }

        // 2. Update held duration timers.
        for (auto const& [key, state] : key_states) {
            if (state == InputState::Down || state == InputState::Held) {
                key_held_durations[key] += delta_time;
            } else {
                key_held_durations[key] = 0.0f;
            }
        }

        for (auto const& [button, state] : mouse_button_states) {
            if (state == InputState::Down || state == InputState::Held) {
                mouse_button_held_durations[button] += delta_time;
            } else {
                mouse_button_held_durations[button] = 0.0f;
            }
        }
    }


    // --- Keyboard Queries ---

    // ---Single Keyboard key events

    bool SDLInputManager::is_down(KeyCode key) const { return key_states.at(key) == InputState::Down; }
    
    bool SDLInputManager::is_held_down(KeyCode key) const { return key_states.at(key) == InputState::Held; }
    
    bool SDLInputManager::was_released(KeyCode key) const { return key_states.at(key) == InputState::Released; }
    
    bool SDLInputManager::is_up(KeyCode key) const { return key_states.at(key) == InputState::Up; }
    
    bool SDLInputManager::is_held_down_for(KeyCode key, float duration) const {
         return key_held_durations.at(key) >= duration; 
        }

    // --- Keyboard Multiple keystrokes events (simultaneous)

    bool SDLInputManager::multiple_are_down(const std::vector<KeyCode>& keys) const {
        if (keys.empty()) {
            return false;
        }

        bool one_key_was_just_pressed = false;

        for (const KeyCode key : keys) {
            const InputState state = key_states.at(key);

            // First, ensure all keys are physically down. If any key is up, the combo fails.
            if (state == InputState::Up || state == InputState::Released) {
                return false;
            }

            // Keep track if at least one of the keys is in the "just pressed" state.
            // This is the trigger for the shortcut.
            if (state == InputState::Down) {
                one_key_was_just_pressed = true;
            }
        }

        // The function only returns true if all keys are physically down AND
        // at least one of them was the final key pressed this frame.
        return one_key_was_just_pressed;
    }

    // For checking if a combo is being maintained.
    bool SDLInputManager::multiple_are_held_down(const std::vector<KeyCode>& keys) const {
        if (keys.empty()) {
            return false;
        }
        // Every single key must be in the 'Held' state.
        for (const KeyCode key : keys) {
            if (key_states.at(key) != InputState::Held) {
                return false;
            }
        }
        return true;
    }

    
    bool SDLInputManager::multiple_are_held_down_for(const std::vector<KeyCode>& keys, float duration) const {
        if (keys.empty()) {
            return false;
        }

        // Iterate through every key required for the combination.
        for (const KeyCode key : keys) {
            // This function now adheres strictly to your logic.
            // For a key to contribute to this check, it must meet two criteria:

            // Criterion 1: The key's state MUST be 'Held'.
            // We no longer accept the 'Down' state. This eliminates the race condition.
            if (key_states.at(key) != InputState::Held) {
                return false;
            }

            // Criterion 2: The key's held duration must meet the target.
            if (key_held_durations.at(key) < duration) {
                return false;
            }
        }

        // If the loop completes, it means every single key was in the 'Held' state
        // AND every single key had met the duration requirement.
        return true;
    }
    
    // For checking when a combo is broken.
    bool SDLInputManager::multiple_were_released(const std::vector<KeyCode>& keys) const {
        if (keys.empty()) {
            return false;
        }

        bool one_key_was_just_released = false;

        for (const KeyCode key : keys) {
            const InputState state = key_states.at(key);

            // Ensure no keys in the combo are still being held down.
            if (state == InputState::Down || state == InputState::Held) {
                return false;
            }

            // Check if at least one of the keys was the one just let go.
            if (state == InputState::Released) {
                one_key_was_just_released = true;
            }
        }

        // The function only returns true if no keys are held AND at least one was
        // the final key released this frame.
        return one_key_was_just_released;
    }

    bool SDLInputManager::multiple_are_up(const std::vector<KeyCode>& keys) const {
        if (keys.empty()) { return false; }
        for (const KeyCode key: keys) {
            if (key_states.at(key) != InputState::Up ) { return false;}
        }

        // If we reach here, the conditions have been met.
        return true;
    }

    // --- Mouse Queries ---
    // --- Mouse Single Button events ---

    bool SDLInputManager::is_down(MouseButton button) const { return mouse_button_states.at(button) == InputState::Down; }
    
    bool SDLInputManager::is_held_down(MouseButton button) const {
         return mouse_button_states.at(button) == InputState::Held; 
        }
    bool SDLInputManager::was_released(MouseButton button) const {
         return mouse_button_states.at(button) == InputState::Released;
        }
    bool SDLInputManager::is_up(MouseButton button) const { return mouse_button_states.at(button) == InputState::Up; }
    
    bool SDLInputManager::is_held_down_for(MouseButton button, float duration) const {
         return mouse_button_held_durations.at(button) >= duration;
        }


    // --- Mouse  Multiple Button events (Additive)

    bool SDLInputManager::multiple_are_down(const std::vector<MouseButton>& buttons) const {
        if (buttons.empty()) {
        return false;
        }

        bool one_button_was_just_pressed = false;

        for (const MouseButton button : buttons) {
            const InputState state = mouse_button_states.at(button);

            // First, ensure all buttons are physically down. If any is up, the combo fails.
            if (state == InputState::Up || state == InputState::Released) {
                return false;
            }

            // Keep track if at least one button was the one just pressed this frame.
            if (state == InputState::Down) {
                one_button_was_just_pressed = true;
            }
        }

        // The function only returns true if all buttons are physically down AND
        // at least one of them was the final button pressed this frame.
        return one_button_was_just_pressed;
    }


    bool SDLInputManager::multiple_are_held_down(const std::vector<MouseButton>& buttons) const {
        if (buttons.empty()) { return false; }
        // Every single button must be in the 'Held' state.
        for (const MouseButton button : buttons) {
            if (mouse_button_states.at(button) != InputState::Held) {
                return false;
            }
        }
            return true;
        }

    bool SDLInputManager::multiple_are_held_down_for(const std::vector<MouseButton>& buttons, float duration) const {
        if (buttons.empty()) {  return false; }
    
        for (const MouseButton button : buttons) {
            const InputState state = mouse_button_states.at(button);
            if (state != InputState::Held) { return false; }

            if (mouse_button_held_durations.at(button) < duration) { return false; }
       
        }
        return true;
    }


    bool SDLInputManager::multiple_were_released(const std::vector<MouseButton>& buttons) const {
        if (buttons.empty()) { return false; }
        bool at_least_one_button_was_released = false;

        for (const MouseButton button : buttons) {
            const InputState state = mouse_button_states.at(button);
            if (state == InputState::Down || state == InputState::Held) { return false; }
            if (state == InputState::Released) { at_least_one_button_was_released = true;}
        }
        return at_least_one_button_was_released;
    }

    bool SDLInputManager::multiple_are_up(const std::vector<MouseButton>& buttons) const {
        if (buttons.empty()) {
        return false;
        }
        // If any button is NOT in the "Up" state, the check fails.
        for (const MouseButton button : buttons) {
            if (mouse_button_states.at(button) != InputState::Up) {
                return false;
            }
        }
        return true;
    }

    // --- Addtional Mouse methods ---
    void SDLInputManager::get_mouse_position(int* x, int* y) const {
        if (x) *x = mouse_x;
        if (y) *y = mouse_y;
    }

    bool SDLInputManager::did_scroll(MouseScroll direction) {
        float scroll_delta = get_mouse_scroll_delta();
        if (direction == MouseScroll::Forward) {
            return scroll_delta > 0.0f;
        }

        if (direction == MouseScroll::Backward) {
            return scroll_delta < 0.0f;
        }
        return false;
    }


    float SDLInputManager::get_mouse_scroll_delta() const {
        float scroll_delta = mouse_scroll_this_frame;

        return scroll_delta;
    }


    // --- SDLInputManager additional methods ---
    bool SDLInputManager::wants_to_quit() const {
        return quit_requested;
    }

    // --- TRANSLATION LAYER ---

    // This function keeps the SDL dependency completely contained within this one file.
    KeyCode SDLInputManager::to_salix_keycode(int sdl_keycode) const {
        switch (sdl_keycode) {
            case SDLK_a: return KeyCode::A;
            case SDLK_b: return KeyCode::B;
            case SDLK_c: return KeyCode::C;
            case SDLK_d: return KeyCode::D;
            case SDLK_e: return KeyCode::E;
            case SDLK_f: return KeyCode::F;
            case SDLK_g: return KeyCode::G;
            case SDLK_h: return KeyCode::H;
            case SDLK_i: return KeyCode::I;
            case SDLK_j: return KeyCode::J;
            case SDLK_k: return KeyCode::K;
            case SDLK_l: return KeyCode::L;
            case SDLK_m: return KeyCode::M;
            case SDLK_n: return KeyCode::N;
            case SDLK_o: return KeyCode::O;
            case SDLK_p: return KeyCode::P;
            case SDLK_q: return KeyCode::Q;
            case SDLK_r: return KeyCode::R;
            case SDLK_s: return KeyCode::S;
            case SDLK_t: return KeyCode::T;
            case SDLK_u: return KeyCode::U;
            case SDLK_v: return KeyCode::V;
            case SDLK_w: return KeyCode::W;
            case SDLK_x: return KeyCode::X;
            case SDLK_y: return KeyCode::Y;
            case SDLK_z: return KeyCode::Z;
            case SDLK_SPACE: return KeyCode::Space;
            case SDLK_RETURN: return KeyCode::Enter;
            case SDLK_ESCAPE: return KeyCode::Escape;
            case SDLK_LSHIFT: return KeyCode::LeftShift;
            case SDLK_LCTRL: return KeyCode::LeftControl;
            case SDLK_LALT: return KeyCode::LeftAlt;
            case SDLK_RSHIFT: return KeyCode::RightShift;
            case SDLK_RCTRL: return KeyCode::RightControl;
            case SDLK_RALT: return KeyCode::RightAlt;
            case SDLK_TAB: return KeyCode::Tab;
            case SDLK_DELETE: return KeyCode::Delete;
            case SDLK_F1: return KeyCode::F1;
            case SDLK_F2: return KeyCode::F2;
            case SDLK_F3: return KeyCode::F3;
            case SDLK_F4: return KeyCode::F4;
            case SDLK_F5: return KeyCode::F5;
            case SDLK_F6: return KeyCode::F6;
            case SDLK_F7: return KeyCode::F7;
            case SDLK_F8: return KeyCode::F8;
            case SDLK_F9: return KeyCode::F9;
            case SDLK_F10: return KeyCode::F10;
            case SDLK_F11: return KeyCode::F11;
            case SDLK_F12: return KeyCode::F12;
            case SDLK_BACKSPACE: return KeyCode::BackSpace;
            case SDLK_BACKSLASH: return KeyCode::BackSlash;
            case SDLK_CAPSLOCK: return KeyCode::CapsLock;
            case SDLK_COMMA: return KeyCode::Comma;
            case SDLK_SEPARATOR: return KeyCode::Separator;
            case SDLK_SLASH: return KeyCode::Slash;
            case SDLK_QUOTE: return KeyCode::Apostrophe;
            case SDLK_SEMICOLON: return KeyCode::SemiColon;
            case SDLK_PERIOD: return KeyCode::Period;
            case SDLK_MINUS: return KeyCode::Minus;
            case SDLK_EQUALS: return KeyCode::Equals;
            case SDLK_UP: return KeyCode::Up;
            case SDLK_DOWN: return KeyCode::Down;
            case SDLK_LEFT: return KeyCode::Left;
            case SDLK_RIGHT: return KeyCode::Right;
            case SDLK_LGUI: return KeyCode::LeftGui;
            case SDLK_RGUI: return KeyCode::RightGui;
            case SDLK_APPLICATION: return KeyCode::ContextMenu;
            case SDLK_PRINTSCREEN: return KeyCode::PrintScreen;
            case SDLK_0: return KeyCode::Alpha0;
            case SDLK_1: return KeyCode::Alpha1;
            case SDLK_2: return KeyCode::Alpha2;
            case SDLK_3: return KeyCode::Alpha3;
            case SDLK_4: return KeyCode::Alpha4;
            case SDLK_5: return KeyCode::Alpha5;
            case SDLK_6: return KeyCode::Alpha6;
            case SDLK_7: return KeyCode::Alpha7;
            case SDLK_8: return KeyCode::Alpha8;
            case SDLK_9: return KeyCode::Alpha9;
            default: return KeyCode::None;
        }
    
    }
} // namespace Salix