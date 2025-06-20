// =================================================================================
// Filename:    Salix/input/SDLInputManager.cpp
// Author:      SalixGameStudio
// Description: Implements the stateful, event-driven, and overloaded IInputManager
//              interface using SDL as the backend.
// =================================================================================
#include <Salix/input/SDLInputManager.h>
#include <Salix/events/SDLEvent.h> // Needed for casting to get event data
#include <SDL.h>                   // For SDLK_* keycodes in the translator

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

        else if (event.get_event_type() == EventType::WindowClose) {
            quit_requested = true;
        }
    }

    void SDLInputManager::update(float delta_time) {
        // --- LIFECYCLE METHOD 2: ADVANCE TIME ---
        // Called once per frame. Handles time-based transitions and updates duration timers.
        // This is the "was ON and still is ON" analogy.

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

    bool SDLInputManager::multiple_are_down(std::vector<KeyCode>& /*keys*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_held_down(std::vector<KeyCode>& /*keys*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_held_down_for(std::vector<KeyCode>& /*keys*/, float /*duration*/) const {
        // not implemented yet
        return false;
    }


    bool SDLInputManager::multiple_were_released(std::vector<KeyCode>& /*keys*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_up(std::vector<KeyCode>& /*keys*/) const {
        // not implemented yet
        return false;
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


    // --- Mouse  Multiple Button events (simultaneous)

    bool SDLInputManager::multiple_are_down(std::vector<MouseButton>& /*buttons*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_held_down(std::vector<MouseButton>& /*buttons*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_held_down_for(std::vector<MouseButton>& /*buttons*/, float/*duration*/) const {
        // not implemented yet
        return false;
    }


    bool SDLInputManager::multiple_were_released(std::vector<MouseButton>& /*buttons*/) const {
        // not implemented yet
        return false;
    }

    bool SDLInputManager::multiple_are_up(std::vector<MouseButton>& /*buttons*/) const {
        // not implemented yet
        return false;
    }

    // --- Addtional Mouse methods ---
    void SDLInputManager::get_mouse_position(int* x, int* y) const {
        if (x) *x = mouse_x;
        if (y) *y = mouse_y;
    }



    // --- SDLInputManager additional methods ---
    bool SDLInputManager::wants_to_quit() const {
        return quit_requested;
    }

    // --- TRANSLATION LAYER ---

    // --- The "salix Game Studio KeyCode's to SDL_SCANCODE's." ---
    SDL_Scancode SDLInputManager::to_sdl_scancode(KeyCode key) const {
        switch (key) {
            case KeyCode::Alpha0: return SDL_SCANCODE_0;
            case KeyCode::Alpha1: return SDL_SCANCODE_1;
            case KeyCode::Alpha2: return SDL_SCANCODE_2;
            case KeyCode::Alpha3: return SDL_SCANCODE_3;
            case KeyCode::Alpha4: return SDL_SCANCODE_4;
            case KeyCode::Alpha5: return SDL_SCANCODE_5;
            case KeyCode::Alpha6: return SDL_SCANCODE_6;
            case KeyCode::Alpha7: return SDL_SCANCODE_7;
            case KeyCode::Alpha8: return SDL_SCANCODE_8;
            case KeyCode::Alpha9: return SDL_SCANCODE_9;
            case KeyCode::A: return SDL_SCANCODE_A;
            case KeyCode::B: return SDL_SCANCODE_B;
            case KeyCode::C: return SDL_SCANCODE_C;
            case KeyCode::D: return SDL_SCANCODE_D;
            case KeyCode::E: return SDL_SCANCODE_E;
            case KeyCode::F: return SDL_SCANCODE_F;
            case KeyCode::G: return SDL_SCANCODE_G;
            case KeyCode::H: return SDL_SCANCODE_H;
            case KeyCode::I: return SDL_SCANCODE_I;
            case KeyCode::J: return SDL_SCANCODE_J;
            case KeyCode::K: return SDL_SCANCODE_K;
            case KeyCode::L: return SDL_SCANCODE_L;
            case KeyCode::M: return SDL_SCANCODE_M;
            case KeyCode::N: return SDL_SCANCODE_N;
            case KeyCode::O: return SDL_SCANCODE_O;
            case KeyCode::P: return SDL_SCANCODE_P;
            case KeyCode::Q: return SDL_SCANCODE_Q;
            case KeyCode::R: return SDL_SCANCODE_R;
            case KeyCode::S: return SDL_SCANCODE_S;
            case KeyCode::T: return SDL_SCANCODE_T;
            case KeyCode::U: return SDL_SCANCODE_U;
            case KeyCode::V: return SDL_SCANCODE_V;
            case KeyCode::W: return SDL_SCANCODE_W;
            case KeyCode::X: return SDL_SCANCODE_X;
            case KeyCode::Y: return SDL_SCANCODE_Y;
            case KeyCode::Z: return SDL_SCANCODE_Z;
            case KeyCode::Space: return SDL_SCANCODE_SPACE;
            case KeyCode::Enter: return SDL_SCANCODE_RETURN;
            case KeyCode::Escape: return SDL_SCANCODE_ESCAPE;
            case KeyCode::LeftShift: return SDL_SCANCODE_LSHIFT;
            case KeyCode::LeftControl: return SDL_SCANCODE_LCTRL;
            case KeyCode::LeftAlt: return SDL_SCANCODE_LALT;
            case KeyCode::RightShift: return SDL_SCANCODE_RSHIFT;
            case KeyCode::RightControl: return SDL_SCANCODE_RCTRL;
            case KeyCode::RightAlt: return SDL_SCANCODE_RALT;
            case KeyCode::Tab: return SDL_SCANCODE_TAB;
            case KeyCode::Delete: return SDL_SCANCODE_DELETE;
            case KeyCode::F1: return SDL_SCANCODE_F1;
            case KeyCode::F2: return SDL_SCANCODE_F2;
            case KeyCode::F3: return SDL_SCANCODE_F3;
            case KeyCode::F4: return SDL_SCANCODE_F4;
            case KeyCode::F5: return SDL_SCANCODE_F5;
            case KeyCode::F6: return SDL_SCANCODE_F6;
            case KeyCode::F7: return SDL_SCANCODE_F7;
            case KeyCode::F8: return SDL_SCANCODE_F8;
            case KeyCode::F9: return SDL_SCANCODE_F9;
            case KeyCode::F10: return SDL_SCANCODE_F10;
            case KeyCode::F11: return SDL_SCANCODE_F11;
            case KeyCode::F12: return SDL_SCANCODE_F12;
            case KeyCode::BackSpace: return SDL_SCANCODE_BACKSPACE;
            case KeyCode::BackSlash: return SDL_SCANCODE_BACKSLASH;
            case KeyCode::CapsLock: return SDL_SCANCODE_CAPSLOCK;
            case KeyCode::Comma: return SDL_SCANCODE_COMMA;
            case KeyCode::Separator: return SDL_SCANCODE_SEPARATOR;
            case KeyCode::Slash: return SDL_SCANCODE_SLASH;
            case KeyCode::Apostrophe: return SDL_SCANCODE_APOSTROPHE;
            case KeyCode::SemiColon: return SDL_SCANCODE_SEMICOLON;
            case KeyCode::Period: return SDL_SCANCODE_PERIOD;
            case KeyCode::Minus: return SDL_SCANCODE_MINUS;
            case KeyCode::Equals: return SDL_SCANCODE_EQUALS;
            case KeyCode::Up: return SDL_SCANCODE_UP;
            case KeyCode::Down: return SDL_SCANCODE_DOWN;
            case KeyCode::Left: return SDL_SCANCODE_LEFT;
            case KeyCode::Right: return SDL_SCANCODE_RIGHT;
            case KeyCode::LeftGui: return SDL_SCANCODE_LGUI;
            case KeyCode::RightGui: return SDL_SCANCODE_RGUI;
            case KeyCode::ContextMenu: return SDL_SCANCODE_APPLICATION;
            case KeyCode::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
            default: return SDL_SCANCODE_UNKNOWN;
        }
    }

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
            default: return KeyCode::None;
        }
    
    }
} // namespace Salix