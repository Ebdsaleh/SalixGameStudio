// =================================================================================
// Filename:    Salix/events/sdl/SDLEvent.h
// Author:      SalixGameStudio
// Description: Declares the concrete event classes that wrap native SDL events,
//              implementing the SDLEventBase, which inherits from IEvent interface.
// =================================================================================
#pragma once

#include <Salix/events/sdl/SDLEventBase.h>
#include <Salix/events/IEvent.h>
#include <sstream>



// For EVENT_CLASS_TYPE (multiple lines, needs backslashes)
#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::##type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif // EVENT_CLASS_TYPE <-- END THE IFNDEF BLOCK HERE
    
    // Helper macro for setting the event category flags.
    // We now static_cast the result to an int to handle single enum class values correctly.
   // For EVENT_CLASS_CATEGORY (single line)
#ifndef EVENT_CLASS_CATEGORY // <-- CHECK IF NOT DEFINED
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif // EVENT_CLASS_CATEGORY <-- END THE IFNDEF BLOCK HERE
namespace Salix {
    //==============================================================================
    // KEYBOARD EVENTS
    //==============================================================================

    // Change: Inherit from SDLEventBase instead of IEvent
    class KeyEvent : public SDLEventBase
    {
    public:
        int get_key_code() const { return key_code; }

        EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)
        CLONE_EVENT_METHOD(KeyEvent)
    protected:
        // Change: Add const SDL_Event& sdl_event_in to constructor
        // Change: Pass sdl_event_in to the base SDLEventBase constructor
        KeyEvent(const int key_code_in, const SDL_Event& sdl_event_in)
            : SDLEventBase(sdl_event_in), key_code(key_code_in) {}

        int key_code;
    };

    class KeyPressedEvent : public KeyEvent {
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
        // And pass it up to the base 'KeyEvent' constructor.
        KeyPressedEvent(const int key_code_in, bool is_repeat_in, const SDL_Event& sdl_event_in) 
            : KeyEvent(key_code_in, sdl_event_in), is_repeat(is_repeat_in) {} 
        bool was_repeated() const { return is_repeat; }

        std::string to_string() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << key_code << " (repeat = " << is_repeat << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)
        CLONE_EVENT_METHOD(KeyPressedEvent)
        private:
            bool is_repeat;
};

    class KeyReleasedEvent : public KeyEvent {
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'KeyEvent' constructor.
            KeyReleasedEvent(const int key_code_in, const SDL_Event& sdl_event_in) 
                : KeyEvent(key_code_in, sdl_event_in) {} 

            std::string to_string() const override {
                std::stringstream ss;
                ss << "KeyReleasedEvent: " << key_code;
                return ss.str();
            }

            EVENT_CLASS_TYPE(KeyReleased)
            CLONE_EVENT_METHOD(KeyReleasedEvent)
    };
    //==============================================================================
    // MOUSE EVENTS
    //==============================================================================

    // Change: Inherit from SDLEventBase instead of IEvent
    class MouseMovedEvent : public SDLEventBase { 
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'SDLEventBase' constructor.
            MouseMovedEvent(const float x_in, const float y_in, const SDL_Event& sdl_event_in) 
                : SDLEventBase(sdl_event_in), mouse_x(x_in), mouse_y(y_in) {} 

            float get_x() const { return mouse_x; }
            float get_y() const { return mouse_y; }

            std::string to_string() const override {
                std::stringstream ss;
                ss << "MouseMovedEvent: " << mouse_x << ", " << mouse_y;
                return ss.str();
            }

            EVENT_CLASS_TYPE(MouseMoved)
            EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseAxis)
            CLONE_EVENT_METHOD(MouseMovedEvent)
        private:
            float mouse_x, mouse_y;
    };

    class MouseScrolledEvent : public SDLEventBase {
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'SDLEventBase' constructor.
            MouseScrolledEvent(const float x_offset_in, const float y_offset_in, const SDL_Event& sdl_event_in) 
                : SDLEventBase(sdl_event_in), x_offset(x_offset_in), y_offset(y_offset_in) {} 

            float get_x_offset() const { return x_offset; }
            float get_y_offset() const { return y_offset; }

            std::string to_string() const override {
                std::stringstream ss;
                ss << "MouseScrolledEvent: " << get_x_offset() << ", " << get_y_offset();
                return ss.str();
            }

            EVENT_CLASS_TYPE(MouseScrolled)
            EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseAxis)
            CLONE_EVENT_METHOD(MouseScrolledEvent)
        private:
            float x_offset, y_offset;
    };

    class MouseButtonEvent : public SDLEventBase { 
        public:
            int get_mouse_button() const { return button; }

            EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton)
            CLONE_EVENT_METHOD(MouseButtonEvent)
        protected:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'SDLEventBase' constructor.
            MouseButtonEvent(const int button_in, const SDL_Event& sdl_event_in) 
                : SDLEventBase(sdl_event_in), button(button_in) {} 

            int button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'MouseButtonEvent' constructor.
            MouseButtonPressedEvent(const int button_in, const SDL_Event& sdl_event_in)
            : MouseButtonEvent(button_in, sdl_event_in) {}

        std::string to_string() const override {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << button;
            return ss.str();
        }

            EVENT_CLASS_TYPE(MouseButtonPressed)
            CLONE_EVENT_METHOD(MouseButtonPressedEvent)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
        public:
            // **CRUCIAL CHANGE HERE:** Add 'const SDL_Event& sdl_event_in' to the constructor
            // And pass it up to the base 'MouseButtonEvent' constructor.
            MouseButtonReleasedEvent(const int button_in, const SDL_Event& sdl_event_in)
                : MouseButtonEvent(button_in, sdl_event_in) {}

            std::string to_string() const override {
                std::stringstream ss;
                ss << "MouseButtonReleasedEvent: " << button;
                return ss.str();
            }
            EVENT_CLASS_TYPE(MouseButtonReleased)
            CLONE_EVENT_METHOD(MouseButtonReleasedEvent)
    };


    //==============================================================================
    // APPLICATION EVENTS
    //==============================================================================

    class WindowResizeEvent : public IEvent {
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
        EVENT_CLASS_CATEGORY(EventCategory::Application)
        CLONE_EVENT_METHOD(WindowResizeEvent)
       
        private:
            unsigned int width, height;
    };

    class WindowCloseEvent : public IEvent {
        public:
            WindowCloseEvent() = default;

            EVENT_CLASS_TYPE(WindowClose)
            EVENT_CLASS_CATEGORY(EventCategory::Application)
            CLONE_EVENT_METHOD(WindowCloseEvent)
    };

} // namespace Salix