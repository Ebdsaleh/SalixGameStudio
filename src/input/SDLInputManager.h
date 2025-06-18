// SDLInputManager.h
#pragma once
#include "IInputManager.h"
#include <SDL.h>
#include <vector>
class SDLInputManager : public IInputManager {
    public:
        SDLInputManager();
        ~SDLInputManager();

        // The main update call that polls all devices.
        void update() override;

        // Keyboard Queries
        bool is_down(KeyCode key) const override;
        bool is_held_down(KeyCode key) const override;
        bool is_held_down_for(KeyCode key, int target_duration) const override;
        bool was_just_released(KeyCode key) const override;
        bool was_released(KeyCode key) const override;
        bool is_up(KeyCode key) const override;
        bool multiple_are_down(std::vector<KeyCode>& keys) const override;
        bool multiple_are_held_down(std::vector<KeyCode>& keys) const override;
        bool multiple_are_held_down_for(std::vector<KeyCode>& keys, int target_duration) const override;
        bool multiple_were_just_released(std::vector<KeyCode>& keys) const override;
        bool multiple_were_released(std::vector<KeyCode>& keys) const override;
        bool multiple_are_up(std::vector<KeyCode>& keys) const override;

        // Mouse Queries
        // Mouse Buttons
        bool is_down(MouseButton button) const override;
        bool is_held_down(MouseButton button) const override;
        bool is_held_down_for(MouseButton button, int target_duration) const override;
        bool was_just_released(MouseButton button) const override;
        bool was_released(MouseButton button) const override;
        bool is_up(MouseButton button) const override;
        bool multiple_are_down(std::vector<MouseButton>& buttons) const override;
        bool multiple_are_held_down(std::vector<MouseButton>& buttons) const override;
        bool multiple_are_held_down_for(std::vector<MouseButton>& buttons, int target_duration) const override;
        bool multiple_were_just_released(std::vector<MouseButton>& buttons) const override;
        bool multiple_were_released(std::vector<MouseButton>& buttons) const override;
        bool multiple_are_up(std::vector<MouseButton>& buttons) const override;

        // Additional queries
        void get_mouse_position(int* x, int* y) const override;
        bool wants_to_quit() const override;

    private:
        // The private "translator" function.
        SDL_Scancode to_sdl_scancode(KeyCode key) const;

        // State tracking varaiables 
        const Uint8* current_key_states;
        std::vector<Uint8> previous_key_states;

        Uint32 current_mouse_state;
        Uint32 previous_mouse_state;
        int mouse_x;
        int mouse_y;

        bool quit_requested;
};