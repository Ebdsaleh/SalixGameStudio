// =================================================================================
// Filename:    Salix/events/SDLEvent.h
// Author:      SalixGameStudio
// Description: Declares the concrete event classes that wrap native SDL events,
//              implementing the IEvent interface.
// =================================================================================
#pragma once

#include <Salix/events/IEvent.h>
#include <sstream>

namespace Salix {

    // Helper macro to implement the type-specific virtual functions.
    #define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::##type; }\
                                    virtual EventType get_event_type() const override { return get_static_type(); }\
                                    virtual const char* get_name() const override { return #type; }

    // --- THE FIX IS HERE ---
    // Helper macro for setting the event category flags.
    // We now static_cast the result to an int to handle single enum class values correctly.
    #define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }

    //==============================================================================
    // KEYBOARD EVENTS
    //==============================================================================

    class KeyEvent : public IEvent
    {
    public:
        int get_key_code() const { return key_code; }

        EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

    protected:
        KeyEvent(const int key_code_in)
            : key_code(key_code_in) {}

        int key_code;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(const int key_code_in, bool is_repeat_in = false)
            : KeyEvent(key_code_in), is_repeat(is_repeat_in) {}

        bool was_repeated() const { return is_repeat; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << key_code << " (repeat = " << is_repeat << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool is_repeat;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(const int key_code_in)
            : KeyEvent(key_code_in) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << key_code;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

    //==============================================================================
    // MOUSE EVENTS
    //==============================================================================

    class MouseMovedEvent : public IEvent
    {
    public:
        MouseMovedEvent(const float x_in, const float y_in)
            : mouse_x(x_in), mouse_y(y_in) {}

        float get_x() const { return mouse_x; }
        float get_y() const { return mouse_y; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << mouse_x << ", " << mouse_y;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseAxis)
    private:
        float mouse_x, mouse_y;
    };

    class MouseScrolledEvent : public IEvent
    {
    public:
        MouseScrolledEvent(const float x_offset_in, const float y_offset_in)
            : x_offset(x_offset_in), y_offset(y_offset_in) {}

        float get_x_offset() const { return x_offset; }
        float get_y_offset() const { return y_offset; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << get_x_offset() << ", " << get_y_offset();
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseAxis)
    private:
        float x_offset, y_offset;
    };

    class MouseButtonEvent : public IEvent
    {
    public:
        int get_mouse_button() const { return button; }

        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton)
    protected:
        MouseButtonEvent(const int button_in)
            : button(button_in) {}

        int button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(const int button_in)
            : MouseButtonEvent(button_in) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(const int button_in)
            : MouseButtonEvent(button_in) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };


    //==============================================================================
    // APPLICATION EVENTS
    //==============================================================================

    class WindowResizeEvent : public IEvent
    {
    public:
        WindowResizeEvent(unsigned int width_in, unsigned int height_in)
            : width(width_in), height(height_in) {}

        unsigned int get_width() const { return width; }
        unsigned int get_height() const { return height; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << width << ", " << height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategory::Application)  // Error here
        /*return value type does not match the function typeC/C++(120)
        #define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return category; }
        Helper macro for setting the event category flags.
        Expands to:

        virtual int get_category_flags() const override { return EventCategory::Application; }*/

    private:
        unsigned int width, height;
    };

    class WindowCloseEvent : public IEvent
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategory::Application)  // Error here
        /*return value type does not match the function typeC/C++(120)
    #define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return category; }
    Helper macro for setting the event category flags.
    Expands to:

    virtual int get_category_flags() const override { return EventCategory::Application; }*/
    };

} // namespace Salix