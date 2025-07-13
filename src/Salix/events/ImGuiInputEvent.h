// Salix/events/ImGuiInputEvent.h
#pragma once
#include <Salix/events/IEvent.h> // Inherit from IEvent
#include <imgui.h> // For ImGuiIO flags (used in implementation, not directly here)
#ifndef EVENT_CLASS_TYPE
// Helper macro to implement the type-specific virtual functions.
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                virtual EventType get_event_type() const override { return get_static_type(); }\
                                virtual const char* get_name() const override { return #type; }
#endif

#ifndef EVENT_CLASS_CATEGORY
// Helper macro for setting the event category flags.
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif
namespace Salix {

    

    // This event is dispatched when ImGui captures mouse or keyboard input.
    // It allows other systems (like the game's input manager) to know if ImGui is "eating" input.
    class ImGuiInputEvent : public IEvent {
    public:
        ImGuiInputEvent(bool mouse_captured_in, bool keyboard_captured_in)
            : mouse_captured(mouse_captured_in), keyboard_captured(keyboard_captured_in) {}

        bool is_mouse_captured() const { return mouse_captured; }
        bool is_keyboard_captured() const { return keyboard_captured; }

        std::string to_string() const override {
            std::stringstream ss;
            ss << "ImGuiInputEvent: Mouse Captured = " << mouse_captured
               << ", Keyboard Captured = " << keyboard_captured;
            return ss.str();
        }

        EVENT_CLASS_TYPE(ImGuiInput)
        EVENT_CLASS_CATEGORY(EventCategory::Input) // It's an input-related event

    private:
        bool mouse_captured;    // Whether ImGui wants to capture mouse input
        bool keyboard_captured; // Whether ImGui wants to capture keyboard input
    };

} // namespace Salix