#pragma once

#include <Salix/input/IInputManager.h>
#include <Salix/events/IEvent.h>
#include <map>
#include <utility>
#include <vector>

class MockIInputManager : public Salix::IInputManager {
public:
    void update(float delta_time) override {
        last_delta_time = delta_time;
        ++update_calls;
    }

    void process_event(Salix::IEvent& event) override {
        last_event = &event;
        ++process_event_calls;
    }

    bool is_down(Salix::KeyCode key) const override { return key_state(key) == Salix::InputState::Down; }
    bool is_held_down(Salix::KeyCode key) const override { return key_state(key) == Salix::InputState::Held; }
    bool is_held_down_for(Salix::KeyCode key, float duration) const override {
        return key_state(key) == Salix::InputState::Held && duration <= key_held_seconds(key);
    }
    bool was_released(Salix::KeyCode key) const override { return key_state(key) == Salix::InputState::Released; }
    bool is_up(Salix::KeyCode key) const override { return key_state(key) == Salix::InputState::Up; }

    bool multiple_are_down(const std::vector<Salix::KeyCode>& keys) const override { return all_keys(keys, Salix::InputState::Down); }
    bool multiple_are_held_down(const std::vector<Salix::KeyCode>& keys) const override { return all_keys(keys, Salix::InputState::Held); }
    bool multiple_are_held_down_for(const std::vector<Salix::KeyCode>& keys, float duration) const override {
        for (auto key : keys) if (!is_held_down_for(key, duration)) return false;
        return true;
    }
    bool multiple_were_released(const std::vector<Salix::KeyCode>& keys) const override { return all_keys(keys, Salix::InputState::Released); }
    bool any_of_combo_was_released(const std::vector<Salix::KeyCode>& keys) const override {
        for (auto key : keys) if (was_released(key)) return true;
        return false;
    }
    bool multiple_are_up(const std::vector<Salix::KeyCode>& keys) const override { return all_keys(keys, Salix::InputState::Up); }

    bool is_down(Salix::MouseButton button) const override { return mouse_state(button) == Salix::InputState::Down; }
    bool is_held_down(Salix::MouseButton button) const override { return mouse_state(button) == Salix::InputState::Held; }
    bool is_held_down_for(Salix::MouseButton button, float duration) const override {
        return mouse_state(button) == Salix::InputState::Held && duration <= mouse_held_seconds(button);
    }
    bool was_released(Salix::MouseButton button) const override { return mouse_state(button) == Salix::InputState::Released; }
    bool is_up(Salix::MouseButton button) const override { return mouse_state(button) == Salix::InputState::Up; }

    bool multiple_are_down(const std::vector<Salix::MouseButton>& buttons) const override { return all_buttons(buttons, Salix::InputState::Down); }
    bool multiple_are_held_down(const std::vector<Salix::MouseButton>& buttons) const override { return all_buttons(buttons, Salix::InputState::Held); }
    bool multiple_are_held_down_for(const std::vector<Salix::MouseButton>& buttons, float duration) const override {
        for (auto button : buttons) if (!is_held_down_for(button, duration)) return false;
        return true;
    }
    bool multiple_were_released(const std::vector<Salix::MouseButton>& buttons) const override { return all_buttons(buttons, Salix::InputState::Released); }
    bool any_of_combo_was_released(const std::vector<Salix::MouseButton>& buttons) const override {
        for (auto button : buttons) if (was_released(button)) return true;
        return false;
    }
    bool multiple_are_up(const std::vector<Salix::MouseButton>& buttons) const override { return all_buttons(buttons, Salix::InputState::Up); }

    bool did_scroll(Salix::MouseScroll direction) override { return direction == scroll_direction && scroll_delta != 0.0f; }

    void get_mouse_position(int* x, int* y) const override {
        if (x) *x = mouse_x;
        if (y) *y = mouse_y;
    }

    float get_mouse_scroll_delta() const override { return scroll_delta; }
    bool wants_to_quit() const override { return quit_requested; }

    void set_key_state(Salix::KeyCode key, Salix::InputState state, float held_seconds = 0.0f) {
        key_states[key] = state;
        key_durations[key] = held_seconds;
    }

    void set_mouse_state(Salix::MouseButton button, Salix::InputState state, float held_seconds = 0.0f) {
        mouse_states[button] = state;
        mouse_durations[button] = held_seconds;
    }

    float last_delta_time = 0.0f;
    int update_calls = 0;
    Salix::IEvent* last_event = nullptr;
    int process_event_calls = 0;
    int mouse_x = 0;
    int mouse_y = 0;
    float scroll_delta = 0.0f;
    Salix::MouseScroll scroll_direction = Salix::MouseScroll::None;
    bool quit_requested = false;

private:
    Salix::InputState key_state(Salix::KeyCode key) const {
        const auto it = key_states.find(key);
        return it == key_states.end() ? Salix::InputState::Up : it->second;
    }

    Salix::InputState mouse_state(Salix::MouseButton button) const {
        const auto it = mouse_states.find(button);
        return it == mouse_states.end() ? Salix::InputState::Up : it->second;
    }

    float key_held_seconds(Salix::KeyCode key) const {
        const auto it = key_durations.find(key);
        return it == key_durations.end() ? 0.0f : it->second;
    }

    float mouse_held_seconds(Salix::MouseButton button) const {
        const auto it = mouse_durations.find(button);
        return it == mouse_durations.end() ? 0.0f : it->second;
    }

    bool all_keys(const std::vector<Salix::KeyCode>& keys, Salix::InputState state) const {
        for (auto key : keys) if (key_state(key) != state) return false;
        return true;
    }

    bool all_buttons(const std::vector<Salix::MouseButton>& buttons, Salix::InputState state) const {
        for (auto button : buttons) if (mouse_state(button) != state) return false;
        return true;
    }

    std::map<Salix::KeyCode, Salix::InputState> key_states;
    std::map<Salix::KeyCode, float> key_durations;
    std::map<Salix::MouseButton, Salix::InputState> mouse_states;
    std::map<Salix::MouseButton, float> mouse_durations;
};
