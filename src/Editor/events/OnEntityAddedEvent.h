// Editor/events/OnEntityAddedEvent.h
#pragma once

#include <Salix/events/IEvent.h>
#include <Editor/Archetypes.h>

#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {
    // This event is dispatched AFTER a new entity archetype has been added to the current_realm
    class OnEntityAddedEvent : public IEvent {
    public:
        // We pass the new archetype by value to ensure a safe copy is sent
        OnEntityAddedEvent(EntityArchetype new_archetype)
            : archetype(std::move(new_archetype)) {}

        EntityArchetype archetype;

        EVENT_CLASS_TYPE(EditorOnEntityAdded) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        CLONE_EVENT_METHOD(OnEntityAddedEvent)
    };
}