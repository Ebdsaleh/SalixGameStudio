// Create a new file: Editor/events/OnChildEntityAddedEvent.h
#pragma once

#include <Salix/events/IEvent.h>
#include <Editor/Archetypes.h>

// Macros for event boilerplate
#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {
    // Dispatched AFTER a new CHILD entity has been added to the current_realm
    class OnChildEntityAddedEvent : public IEvent {
    public:
        OnChildEntityAddedEvent(EntityArchetype new_archetype)
            : archetype(std::move(new_archetype)) {}

        EntityArchetype archetype;

        EVENT_CLASS_TYPE(EditorOnChildEntityAdded) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        CLONE_EVENT_METHOD(OnChildEntityAddedEvent)
    };
}