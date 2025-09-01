// =================================================================================
// Filename:    Salix/events/IEvent.h
// Author:      SalixGameStudio
// Description: Declares the abstract IEvent interface and related types,
//              using modern C++ best practices.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <sstream>

namespace Salix {

    // --- CHANGE 1: EventCategory is now a strongly-typed enum class. ---
    // This prevents its members from "leaking" and causing name conflicts.
    enum class EventCategory {
        None        = 0,
        Application = 1 << 0,
        Input       = 1 << 1,
        Keyboard    = 1 << 2,
        Mouse       = 1 << 3,
        MouseButton = 1 << 4,
        MouseAxis   = 1 << 5,
        Editor      = 1 << 6
    };

    // --- CHANGE 2: Operator Overload for combining categories. ---
    // This function teaches the compiler how to use the '|' operator with our
    // new EventCategory. It's a clean and safe way to handle bitwise flags.
    inline constexpr int operator|(EventCategory lhs, EventCategory rhs) {
        return static_cast<int>(lhs) | static_cast<int>(rhs);
    }
    // Also needed for combining more than two flags.
    inline constexpr int operator|(int lhs, EventCategory rhs) {
        return lhs | static_cast<int>(rhs);
    }


    // The EventType enum is already an enum class, which is perfect.
    enum class EventType {
        None, WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender, KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        ImGuiInput, EditorEntitySelected, EditorElementSelected, EditorThemeReloadEvent,BeforeEntityPurged,
        EditorPropertyValueChanged, EditorOnHierarchyChanged, EditorOnRootEntityAdded, EditorOnEntityAdded,
        EditorOnChildEntityAdded, EditorOnEntityPurged,
        EditorOnEntityFamilyAdded, EditorOnEntityFamilyPurged, EditorOnElementAdded
    };

    class SALIX_API IEvent {
    public:
        virtual ~IEvent() = default;

        virtual EventType get_event_type() const = 0;
        virtual const char* get_name() const = 0;
        virtual int get_category_flags() const = 0;
        virtual std::string to_string() const { return get_name(); }
        // The helper functions now correctly use the scoped enum values.
        inline bool is_in_category(EventCategory category) const {
            return get_category_flags() & static_cast<int>(category);
        }
        inline bool is_application_event() const { return is_in_category(EventCategory::Application); }
        inline bool is_input_event() const { return is_in_category(EventCategory::Input); }
        inline bool is_keyboard_event() const { return is_in_category(EventCategory::Keyboard); }
        inline bool is_mouse_event() const { return is_in_category(EventCategory::Mouse); }
        inline bool is_mouse_button_event() const { return is_in_category(EventCategory::MouseButton); }
        virtual void* get_native_handle() { return nullptr; } // Default implementation returns nullptr
        // This will be overridden by concrete SDL events to return the SDL_Event*
        virtual bool should_block() const { return false; } // Default implementation
        virtual void set_block(bool block) {(void)block;}              // Default does nothing
    
        bool handled = false;
    };

} // namespace Salix