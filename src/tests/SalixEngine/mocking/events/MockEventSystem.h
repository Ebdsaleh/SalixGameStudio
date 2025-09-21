// 
// =================================================================================
// Filename:    src/Tests/SalixEngine/mocking/events/MockEventSystem.h
// Description: Contains Mocking components for testing the event system.
// =================================================================================
#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/events/IEventListener.h>
#include <functional>

// Since these macros are needed by the mock events, this is a good
// shared location for them until they are moved into IEvent.h
#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; } \
                                 virtual EventType get_event_type() const override { return get_static_type(); } \
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {

// --- Mock Classes for Testing ---

class MockEvent : public IEvent {
public:
    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategory::Application)
    CLONE_EVENT_METHOD(MockEvent)
};

class MockBlockingEvent : public IEvent {
public:
    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategory::Application)
    CLONE_EVENT_METHOD(MockBlockingEvent)

    bool should_block() const override { return true; }
};

class MockListener : public IEventListener {
public:
    bool event_received = false;
    std::function<void(IEvent&)> on_event_handler;

    void on_event(IEvent& event) override {
        if (on_event_handler) {
            on_event_handler(event);
        } else {
            event_received = true;
            (void)event;
        }
    }
};

} // namespace Salix