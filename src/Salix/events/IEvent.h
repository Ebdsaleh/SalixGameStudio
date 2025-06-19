// src/events/IEvent.h
// src/Salix/events/IEvent.h  <--- Future implemented structure.

#pragma once

#include <string>
#include <sstream>

namespace Salix { // Let's use Salix, and the user can alias to slx if they want.
    // PascalCase for enums

    enum EventCategory {
        None = 0,
        Application = 1 << 0,
        Input       = 1 << 1,
        Keyboard    = 1 << 2,
        Mouse       = 1 << 3,
        MouseButton = 1 << 4,
        MouseAxis   = 1 << 5,
    };

    // PascalCase for enums
    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    // PascalCase for classes
    class IEvent {
    public:
        virtual ~IEvent() = default;

        // snake_case for methods
        virtual EventType get_event_type() const = 0;
        virtual const char* get_name() const = 0;
        virtual int get_category_flags() const = 0;
        virtual std::string to_string() const { return get_name(); }

        // snake_case for methods
        inline bool is_in_category(EventCategory category) const {
            return get_category_flags() & category;
        }

        // snake_case for methods
        inline bool is_application_event() const { return is_in_category(EventCategory::Application); }
        inline bool is_input_event() const { return is_in_category(EventCategory::Input); }
        inline bool is_keyboard_event() const { return is_in_category(EventCategory::Keyboard); }
        inline bool is_mouse_event() const { return is_in_category(EventCategory::Mouse); }
        inline bool is_mouse_button_event() const { return is_in_category(EventCategory::MouseButton); }
        
        // snake_case for member variables
        bool handled = false;
    };

} // namespace Salix