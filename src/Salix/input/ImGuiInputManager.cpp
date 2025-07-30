// Salix/input/ImGuiInputManager.cpp
#include <Salix/input/ImGuiInputManager.h>
#include <imgui.h> // Essential for ImGui::GetIO() and ImGuiKey/ImGuiMouseButton
#include <iostream> // For temporary std::cerr/cout

namespace Salix {
    // --- MUST BE DEFINED HERE TO APPEASE THE COMPILER ---
    // --- Helper functions for KeyCode/MouseButton conversion (static, internal linkage) ---
    // These need to be comprehensive and map all your Salix KeyCodes to ImGuiKeys.
    // Ensure you have ImGuiKey_COUNT from imgui.h to use in an array if preferred.
    static ImGuiKey SalixKeyCodeToImGuiKey(KeyCode key) {
        switch (key) {
            case KeyCode::Alpha1: return ImGuiKey_1;
            case KeyCode::Alpha2: return ImGuiKey_2;
            case KeyCode::Alpha3: return ImGuiKey_3;
            case KeyCode::Alpha4: return ImGuiKey_4;
            case KeyCode::Alpha5: return ImGuiKey_5;
            case KeyCode::Alpha6: return ImGuiKey_6;
            case KeyCode::Alpha7: return ImGuiKey_7;
            case KeyCode::Alpha8: return ImGuiKey_8;
            case KeyCode::Alpha9: return ImGuiKey_9;
            case KeyCode::Alpha0: return ImGuiKey_0;
            case KeyCode::A: return ImGuiKey_A;
            case KeyCode::B: return ImGuiKey_B;
            case KeyCode::C: return ImGuiKey_C;
            case KeyCode::D: return ImGuiKey_D;
            case KeyCode::E: return ImGuiKey_E;
            case KeyCode::F: return ImGuiKey_F;
            case KeyCode::G: return ImGuiKey_G;
            case KeyCode::H: return ImGuiKey_H;
            case KeyCode::I: return ImGuiKey_I;
            case KeyCode::J: return ImGuiKey_J;
            case KeyCode::K: return ImGuiKey_K;
            case KeyCode::L: return ImGuiKey_L;
            case KeyCode::M: return ImGuiKey_M;
            case KeyCode::N: return ImGuiKey_N;
            case KeyCode::O: return ImGuiKey_O;
            case KeyCode::P: return ImGuiKey_P;
            case KeyCode::Q: return ImGuiKey_Q;
            case KeyCode::R: return ImGuiKey_R;
            case KeyCode::S: return ImGuiKey_S;
            case KeyCode::T: return ImGuiKey_T;
            case KeyCode::U: return ImGuiKey_U;
            case KeyCode::V: return ImGuiKey_V;
            case KeyCode::W: return ImGuiKey_W;
            case KeyCode::X: return ImGuiKey_X;
            case KeyCode::Y: return ImGuiKey_Y;
            case KeyCode::Z: return ImGuiKey_Z;
            case KeyCode::Space: return ImGuiKey_Space;
            case KeyCode::Enter: return ImGuiKey_Enter;
            case KeyCode::Escape: return ImGuiKey_Escape;
            case KeyCode::LeftShift: return ImGuiKey_LeftShift;
            case KeyCode::LeftControl: return ImGuiKey_LeftCtrl;
            case KeyCode::LeftAlt: return ImGuiKey_LeftAlt;
            case KeyCode::RightShift: return ImGuiKey_RightShift;
            case KeyCode::RightControl: return ImGuiKey_RightCtrl;
            case KeyCode::RightAlt: return ImGuiKey_RightAlt;
            case KeyCode::Tab: return ImGuiKey_Tab;
            case KeyCode::Delete: return ImGuiKey_Delete;
            case KeyCode::F1: return ImGuiKey_F1;
            case KeyCode::F2: return ImGuiKey_F2;
            case KeyCode::F3: return ImGuiKey_F3;
            case KeyCode::F4: return ImGuiKey_F4;
            case KeyCode::F5: return ImGuiKey_F5;
            case KeyCode::F6: return ImGuiKey_F6;
            case KeyCode::F7: return ImGuiKey_F7;
            case KeyCode::F8: return ImGuiKey_F8;
            case KeyCode::F9: return ImGuiKey_F9;
            case KeyCode::F10: return ImGuiKey_F10;
            case KeyCode::F11: return ImGuiKey_F11;
            case KeyCode::F12: return ImGuiKey_F12;
            case KeyCode::BackSpace: return ImGuiKey_Backspace;
            case KeyCode::BackSlash: return ImGuiKey_Backslash;
            case KeyCode::CapsLock: return ImGuiKey_CapsLock;
            case KeyCode::Comma: return ImGuiKey_Comma;
            case KeyCode::Slash: return ImGuiKey_Slash;
            case KeyCode::Apostrophe: return ImGuiKey_Apostrophe; // Or quote
            case KeyCode::SemiColon: return ImGuiKey_Semicolon;
            case KeyCode::Period: return ImGuiKey_Period;
            case KeyCode::Minus: return ImGuiKey_Minus;
            case KeyCode::Equals: return ImGuiKey_Equal;
            case KeyCode::Up: return ImGuiKey_UpArrow;
            case KeyCode::Down: return ImGuiKey_DownArrow;
            case KeyCode::Left: return ImGuiKey_LeftArrow;
            case KeyCode::Right: return ImGuiKey_RightArrow;
            case KeyCode::LeftGui: return ImGuiKey_LeftSuper; // Left Windows/Command key
            case KeyCode::RightGui: return ImGuiKey_RightSuper; // Right Windows/Command key
            case KeyCode::ContextMenu: return ImGuiKey_Menu;
            case KeyCode::PrintScreen: return ImGuiKey_PrintScreen;
            // Add other keys you might have in your KeyCode enum
            default: return ImGuiKey_None; // Return ImGuiKey_None for unmapped keys
        }
    }

    static ImGuiMouseButton SalixMouseButtonToImGuiButton(MouseButton button) {
        switch (button) {
            case MouseButton::Left: return ImGuiMouseButton_Left;
            case MouseButton::Middle: return ImGuiMouseButton_Middle;
            case MouseButton::Right: return ImGuiMouseButton_Right;
            // Add other mouse buttons if you have them in your MouseButton enum
            default: return ImGuiMouseButton_COUNT; 
        }
    }


    ImGuiInputManager::ImGuiInputManager() {
        std::cout << "ImGuiInputManager: Constructor." << std::endl;
    }

    ImGuiInputManager::~ImGuiInputManager() {
        std::cout << "ImGuiInputManager: Destructor." << std::endl;
    }

    void ImGuiInputManager::update(float delta_time) {
        // ImGui's internal input state is already updated by SDLImGui::new_frame()
        // (which calls ImGui_ImplSDL2_NewFrame() and ImGui::NewFrame()).
        // This manager simply queries that state. No internal capture flags here.
        (void)delta_time; // Suppress unused warning
    }

    void ImGuiInputManager::process_event(IEvent& event) {
        // In the two-input-manager setup, this manager does NOT process
        // raw IEvents to determine its state. Its state is derived from
        // ImGui::GetIO() which is fed by SDLEventPoller's raw callback.
        // So, this method remains minimal/empty.
        (void)event; // Silence unused parameter warning
    }

    // --- Keyboard Queries ---
    bool ImGuiInputManager::is_down(KeyCode key) const {
        return ImGui::IsKeyPressed(SalixKeyCodeToImGuiKey(key), false); // false for no repeat
    }

    bool ImGuiInputManager::is_held_down(KeyCode key) const {
        return ImGui::IsKeyDown(SalixKeyCodeToImGuiKey(key));
    }

    bool ImGuiInputManager::is_held_down_for(KeyCode key, float duration) const {
        ImGuiKey imgui_key = SalixKeyCodeToImGuiKey(key);
        // Ensure the ImGuiKey is valid (not ImGuiKey_None) before accessing KeysData
        if (imgui_key == ImGuiKey_None) return false;
        return ImGui::IsKeyDown(imgui_key) && ImGui::GetIO().KeysData[static_cast<int>(imgui_key)].DownDuration >= duration;
    }

    bool ImGuiInputManager::was_released(KeyCode key) const {
        return ImGui::IsKeyReleased(SalixKeyCodeToImGuiKey(key));
    }

    bool ImGuiInputManager::is_up(KeyCode key) const {
        return !ImGui::IsKeyDown(SalixKeyCodeToImGuiKey(key));
    }

    bool ImGuiInputManager::multiple_are_down(const std::vector<KeyCode>& keys) const {
        for (KeyCode key : keys) {
            if (!is_down(key)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_held_down(const std::vector<KeyCode>& keys) const {
        for (KeyCode key : keys) {
            if (!is_held_down(key)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_held_down_for(const std::vector<KeyCode>& keys, float duration) const {
        for (KeyCode key : keys) {
            if (!is_held_down_for(key, duration)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_were_released(const std::vector<KeyCode>& keys) const {
        for (KeyCode key : keys) {
            if (!was_released(key)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_up(const std::vector<KeyCode>& keys) const {
        for (KeyCode key : keys) {
            if (!is_up(key)) return false;
        }
        return true;
    }

    // --- Mouse Queries ---
    bool ImGuiInputManager::is_down(MouseButton button) const {
        ImGuiMouseButton imgui_button = SalixMouseButtonToImGuiButton(button);
        if (imgui_button == ImGuiMouseButton_COUNT) return false; // Invalid button mapping
        return ImGui::IsMouseClicked(imgui_button, false); // false for no repeat
    }

    bool ImGuiInputManager::is_held_down(MouseButton button) const {
        ImGuiMouseButton imgui_button = SalixMouseButtonToImGuiButton(button);
        if (imgui_button == ImGuiMouseButton_COUNT) return false;
        return ImGui::IsMouseDown(imgui_button);
    }

    bool ImGuiInputManager::is_held_down_for(MouseButton button, float duration) const {
        ImGuiMouseButton imgui_button = SalixMouseButtonToImGuiButton(button);
        if (imgui_button == ImGuiMouseButton_COUNT) return false;
        return ImGui::IsMouseDown(imgui_button) && ImGui::GetIO().MouseDownDuration[imgui_button] >= duration;
    }

    bool ImGuiInputManager::was_released(MouseButton button) const {
        ImGuiMouseButton imgui_button = SalixMouseButtonToImGuiButton(button);
        if (imgui_button == ImGuiMouseButton_COUNT) return false;
        return ImGui::IsMouseReleased(imgui_button);
    }

    bool ImGuiInputManager::is_up(MouseButton button) const {
        ImGuiMouseButton imgui_button = SalixMouseButtonToImGuiButton(button);
        if (imgui_button == ImGuiMouseButton_COUNT) return true; // If invalid button, it's considered "up"
        return !ImGui::IsMouseDown(imgui_button);
    }

    bool ImGuiInputManager::multiple_are_down(const std::vector<MouseButton>& buttons) const {
        for (MouseButton button : buttons) {
            if (!is_down(button)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_held_down(const std::vector<MouseButton>& buttons) const {
        for (MouseButton button : buttons) {
            if (!is_held_down(button)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_held_down_for(const std::vector<MouseButton>& buttons, float duration) const {
        for (MouseButton button : buttons) {
            if (!is_held_down_for(button, duration)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_were_released(const std::vector<MouseButton>& buttons) const {
        for (MouseButton button : buttons) {
            if (!was_released(button)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::multiple_are_up(const std::vector<MouseButton>& buttons) const {
        for (MouseButton button : buttons) {
            if (!is_up(button)) return false;
        }
        return true;
    }

    bool ImGuiInputManager::did_scroll(MouseScroll direction) {
        float scroll_delta = ImGui::GetIO().MouseWheel;
        if (direction == MouseScroll::Forward) {
            return scroll_delta > 0.0f;
        }
        if (direction == MouseScroll::Backward) {
            return scroll_delta < 0.0f;
        }
        return false;
    }
    // Additional queries
    float ImGuiInputManager::get_mouse_scroll_delta() const {
        const float mouse_scroll_delta = ImGui::GetIO().MouseWheel;
        return mouse_scroll_delta ;
    }

    void ImGuiInputManager::get_mouse_position(int* x, int* y) const {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        if (x) *x = static_cast<int>(mouse_pos.x);
        if (y) *y = static_cast<int>(mouse_pos.y);
    }

    bool ImGuiInputManager::wants_to_quit() const {
        return false; // Handled by engine's main event loop / SDLInputManager
    }


    

} // namespace Salix