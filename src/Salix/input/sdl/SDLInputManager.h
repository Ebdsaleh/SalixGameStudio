// Salix/input/sdl/SDLInputManager.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/input/IInputManager.h>
#include <SDL.h>
#include <vector>
#include <map>
#include <memory>

namespace Salix {

    class SALIX_API SDLInputManager : public IInputManager {
        public:
            SDLInputManager();
            ~SDLInputManager();

            // The main update call that polls all devices.
            void update(float delta_time) override;
            void process_event(IEvent& event)override;
            // Keyboard Queries
            bool is_down(KeyCode key) const override;
            bool is_held_down(KeyCode key) const override;
            bool is_held_down_for(KeyCode key, float duration) const override;
            bool was_released(KeyCode key) const override;
            bool is_up(KeyCode key) const override;
            bool multiple_are_down(const std::vector<KeyCode>& keys) const override;
            bool multiple_are_held_down(const std::vector<KeyCode>& keys) const override;
            bool multiple_are_held_down_for(const std::vector<KeyCode>& keys, float duration) const override;
            bool multiple_were_released(const std::vector<KeyCode>& keys) const override;
            bool any_of_combo_was_released(const std::vector<KeyCode>& keys) const override;
            bool multiple_are_up(const std::vector<KeyCode>& keys) const override;

            // Mouse Queries
            // Mouse Buttons
            bool is_down(MouseButton button) const override;
            bool is_held_down(MouseButton button) const override;
            bool is_held_down_for(MouseButton button, float duration) const override;
            bool was_released(MouseButton button) const override;
            bool is_up(MouseButton button) const override;
            bool multiple_are_down(const std::vector<MouseButton>& buttons) const override;
            bool multiple_are_held_down(const std::vector<MouseButton>& buttons) const override;
            bool multiple_are_held_down_for(const std::vector<MouseButton>& buttons, float duration) const override;
            bool multiple_were_released(const std::vector<MouseButton>& buttons) const override;
            bool any_of_combo_was_released(const std::vector<MouseButton>& buttons) const override;
            bool multiple_are_up(const std::vector<MouseButton>& buttons) const override;

            bool did_scroll(MouseScroll direction) override;
            // Additional queries
            float get_mouse_scroll_delta() const override;
            void get_mouse_position(int* x, int* y) const override;
            bool wants_to_quit() const override;

        private:

        #ifdef SALIX_TESTS_ENABLED
        public: // This block is only public when SALIX_TESTS_ENABLED is defined
        #endif
            // A helper to convert SDL's keycode from an event to our abstract KeyCode.
            KeyCode to_salix_keycode(int sdl_keycode) const;

        private:
            // --- The NEW State-Tracking Maps ---
            // These are the brains of the new system.
            std::map<KeyCode, InputState> key_states;
            std::map<MouseButton, InputState> mouse_button_states;
            std::map<KeyCode, float> key_held_durations;
            std::map<MouseButton, float> mouse_button_held_durations; // Also need this for mouse!
            // --- Other member variables ---
            int mouse_x;
            int mouse_y;
            bool quit_requested;
            float mouse_scroll_this_frame = 0.0f;
    };
} // namespace Salix