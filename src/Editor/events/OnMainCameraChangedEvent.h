// Editor/events/OnMainCameraChangedEvent.h
#pragma once
#include <Salix/events/IEvent.h>
#include <Salix/core/SimpleGuid.h>

#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif


namespace Salix {
    class OnMainCameraChangedEvent : public IEvent {
    public:
        OnMainCameraChangedEvent(SimpleGuid camera_entity_id) 
            : entity_id(camera_entity_id) {}

        SimpleGuid entity_id;

        EVENT_CLASS_TYPE(EditorOnMainCameraChanged) // Add this to your EventType enum
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        CLONE_EVENT_METHOD(OnMainCameraChangedEvent)
    };
}