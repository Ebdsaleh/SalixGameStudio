// Salix/input/InputCodes.h
#pragma once

namespace Salix {

    // We can add to these enums as we support more keys.
    // The values don't matter, only the names.
    enum class KeyCode {
        None, Alpha1, Alpha2, Alpha3, Alpha4, Alpha5,
        Alpha6, Alpha7, Alpha8, Alpha9, Alpha0, 
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Space, Enter, Escape, LeftShift, LeftControl, LeftAlt,
        RightShift, RightControl, RightAlt, Tab, Delete,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        BackSpace, BackSlash, CapsLock, Comma, Separator, 
        Slash, Apostrophe, SemiColon, Period, Minus, Equals,
        Up, Down, Left, Right, LeftGui, RightGui, ContextMenu, PrintScreen,
    };

    enum class MouseButton {
        None,
        Left,
        Middle,
        Right
    };

    enum class MouseScroll {
        None,
        Forward,
        Backward
    };
    // Add more later.
} // namespace Salix