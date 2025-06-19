// SDLInputManager.cpp
#include "SDLInputManager.h"

SDLInputManager::SDLInputManager() :
    quit_requested(false),
    current_mouse_state(0),
    previous_mouse_state(0),
    mouse_x(0),
    mouse_y(0)
    
    {
        // Initialize Keyboard state arrays.
        current_key_states = SDL_GetKeyboardState(NULL);
        int num_keys;
        SDL_GetKeyboardState(&num_keys);
        previous_key_states.resize(num_keys);
        memcpy(previous_key_states.data(), current_key_states, num_keys);
       
    }

SDLInputManager::~SDLInputManager() {}


void SDLInputManager::update(float delta_time) {
    // --- State snapshotting is the same ---
    // We still need this for our was_released checks.
    memcpy(previous_key_states.data(), current_key_states, previous_key_states.size());
    previous_mouse_state = current_mouse_state;
    current_mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

    // --- NEW: Event-Driven Logic ---
    // Instead of iterating all keys, we react to specific events from SDL.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_requested = true;
        }
    }

    // NEW: Update the key hold timers.
    // This is more efficient since we only update once per frame.
    int num_keys;
    const Uint8* keys = SDL_GetKeyboardState(&num_keys);
    for (int scancode = 0; scancode < num_keys; ++ scancode){
        // if a key is currently being held down.
        if (keys[scancode]) {
            // ... add the delta_time to its stopwatch.
            // If it's not in the map yet, this will create it with a value of 0, then add
            key_held_durations[static_cast<SDL_Scancode>(scancode)] += delta_time;
        } else {
            // if the key is up, reset the stopwatch to 0.
            key_held_durations[static_cast<SDL_Scancode>(scancode)] = 0.0f;
        }
    }
}

// --- Keyboard Queries ---

// ---Single Keyboard key events

// Returns true the moment the frame detects the input
bool SDLInputManager::is_down(KeyCode key) const {
    SDL_Scancode sc = to_sdl_scancode(key);
    return (current_key_states[sc] == 1 && previous_key_states[sc] == 0);
}

// Returns true if the input 'is still down' from the previous frame-cycle
bool SDLInputManager::is_held_down(KeyCode key) const {
    SDL_Scancode sc = to_sdl_scancode(key);
    return (current_key_states[sc] == 1 && previous_key_states[sc] == 1);
}

// Returns true if the input 'is held down' for the target_duration value (measured in seconds).
bool SDLInputManager::is_held_down_for(KeyCode key, int target_duration) const {
    // Trial implementation:
    // Find the key in the key_held_durations map.
    SDL_Scancode sc = to_sdl_scancode(key);
    auto durations_iterator = key_held_durations.find(sc);

    // If the key is not found in the map, it's not being held.
    if (durations_iterator == key_held_durations.end()) {
        return false;
    }

    // Return true if the accumulated time is greater than or equal to the target.
    return durations_iterator->second >= target_duration;
}

// Returns true when the input goes from 'down to up' (is_down to is_up).
bool SDLInputManager::was_released(KeyCode key) const {
    SDL_Scancode scancode = to_sdl_scancode(key);
    return (current_key_states[scancode] == 0 && previous_key_states[scancode] == 1);
}

//Returns true when the input goes from 'down to up, after being in the 'is_held_down' state.
bool SDLInputManager::was_just_released(KeyCode key) const {
    // not implemented yet
    return false;
}

// Returns true if the input is not detect this frame. 
bool SDLInputManager::is_up(KeyCode key) const {
    // not implemented yet
    return false;
}

// --- Keyboard Multiple keystrokes events (simultaneous)

bool SDLInputManager::multiple_are_down(std::vector<KeyCode>& keys) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_held_down(std::vector<KeyCode>& keys) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_held_down_for(std::vector<KeyCode>& keys, int target_duration) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_were_just_released(std::vector<KeyCode>& keys) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_were_released(std::vector<KeyCode>& keys) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_up(std::vector<KeyCode>& keys) const {
    // not implemented yet
    return false;
}

// --- Mouse Queries ---
// --- Mouse Single Button events ---

bool SDLInputManager::is_down(MouseButton button) const {
    Uint32 mask = 0;
    if (button == MouseButton::Left) mask = SDL_BUTTON_LMASK;
    else if (button == MouseButton::Right) mask = SDL_BUTTON_RMASK;
    else if (button == MouseButton::Middle) mask = SDL_BUTTON_MMASK;
    return (current_mouse_state & mask) != 0;
}

bool SDLInputManager::is_held_down(MouseButton button) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::is_held_down_for(MouseButton button, int target_duration) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::was_just_released(MouseButton button) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::was_released(MouseButton button) const {
   Uint32 mask = 0;
    if (button == MouseButton::Left) mask = SDL_BUTTON_LMASK;
    else if (button == MouseButton::Right) mask = SDL_BUTTON_RMASK;
    else if (button == MouseButton::Middle) mask = SDL_BUTTON_MMASK;
    return ((current_mouse_state & mask) == 0 && (previous_mouse_state && mask) != 0);
}

bool SDLInputManager::is_up(MouseButton button) const {
    // not implemented yet
    return false;
}

// --- Mouse  Multiple Button events (simultaneous)

bool SDLInputManager::multiple_are_down(std::vector<MouseButton>& buttons) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_held_down(std::vector<MouseButton>& buttons) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_held_down_for(std::vector<MouseButton>& buttons, int target_duration) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_were_just_released(std::vector<MouseButton>& buttons) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_were_released(std::vector<MouseButton>& buttons) const {
    // not implemented yet
    return false;
}

bool SDLInputManager::multiple_are_up(std::vector<MouseButton>& buttons) const {
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


// --- The "Ubiquitus Translator" ---
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