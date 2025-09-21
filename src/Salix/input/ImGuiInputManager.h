// Salix/input/ImGuiInputManager.h
#pragma once
#include <Salix/input/IInputManager.h>
#include <Salix/core/Core.h>
#include <imgui.h> 
#include <string>
#include <vector>

namespace Salix {

    class SALIX_API ImGuiInputManager : public IInputManager {
    public:
        ImGuiInputManager();
        virtual ~ImGuiInputManager() override;

        void update(float delta_time) override;
        void process_event(IEvent& event) override;

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

        virtual bool did_scroll(MouseScroll direction) override;
        // Additional queries
        float get_mouse_scroll_delta() const override;
        void get_mouse_position(int* x, int* y) const override;
        bool wants_to_quit() const override;

    private:
        // REMOVED: imgui_wants_mouse_capture and imgui_wants_keyboard_capture members.
        // The responsibility for "capture" is handled by the Engine selecting the correct IInputManager.
    };

} // namespace Salix