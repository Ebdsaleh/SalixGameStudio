// Salix/events/SDLEvent.h
#pragma once
#include <IEvent.h> // Include our abstract base class
#include <sstream>
#include <functional> // For std::function, our event callback
#include <SDL.h>      // For the SDL_Event structure

namespace Salix {

    // A helper macro to quickly implement the type-specific virtual functions.
    // This reduces boilerplate code in every event class.
    #define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::##type; }\
                                    virtual EventType get_event_type() const override { return get_static_type(); }\
                                    virtual const char* get_name() const override { return #type; }

    // A helper macro for setting the event category flags.
    #define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return category; }

    //==============================================================================
    // KEYBOARD EVENTS
    //==============================================================================

    class KeyEvent : public IEvent
    {
    public:
        int get_key_code() const { return m_key_code; }

        EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

    protected:
        KeyEvent(const int key_code)
            : m_key_code(key_code) {}

        int m_key_code;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(const int key_code, bool is_repeat = false)
            : KeyEvent(key_code), m_is_repeat(is_repeat) {}

        bool is_repeat() const { return m_is_repeat; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << m_key_code << " (repeat = " << m_is_repeat << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool m_is_repeat;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(const int key_code)
            : KeyEvent(key_code) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << m_key_code;
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
        MouseMovedEvent(const float x, const float y)
            : m_mouse_x(x), m_mouse_y(y) {}

        float get_x() const { return m_mouse_x; }
        float get_y() const { return m_mouse_y; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << m_mouse_x << ", " << m_mouse_y;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)
    private:
        float m_mouse_x, m_mouse_y;
    };

    class MouseScrolledEvent : public IEvent
    {
    public:
        MouseScrolledEvent(const float x_offset, const float y_offset)
            : m_x_offset(x_offset), m_y_offset(y_offset) {}

        float get_x_offset() const { return m_x_offset; }
        float get_y_offset() const { return m_y_offset; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << get_x_offset() << ", " << get_y_offset();
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)
    private:
        float m_x_offset, m_y_offset;
    };

    class MouseButtonEvent : public IEvent
    {
    public:
        int get_mouse_button() const { return m_button; }

        EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton)
    protected:
        MouseButtonEvent(const int button)
            : m_button(button) {}

        int m_button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(const int button)
            : MouseButtonEvent(button) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << m_button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(const int button)
            : MouseButtonEvent(button) {}

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << m_button;
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
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_width(width), m_height(height) {}

        unsigned int get_width() const { return m_width; }
        unsigned int get_height() const { return m_height; }

        std::string to_string() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_width << ", " << m_height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategory::Application)
    private:
        unsigned int m_width, m_height;
    };

    class WindowCloseEvent : public IEvent
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategory::Application)
    };

    // The EventHandler is responsible for polling SDL and dispatching our custom events.
class EventHandler
{
public:
    // Define the type for the callback function the application will provide.
    // It's a function that takes a reference to our base event class.
    using event_callback_fn = std::function<void(IEvent&)>;

    // The constructor takes the application's callback function.
    EventHandler(const event_callback_fn& callback);

    // This method will be called once per frame in the main game loop.
    void poll_events();

private:
    // Stores the callback function to be called when an event occurs.
    event_callback_fn m_event_callback;
};

} // namespace Salix