// Editor/events/OnElementAddedEvent.h
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
    // Dispatched AFTER a new element has been added to an entity's archetype
    class OnElementAddedEvent : public IEvent {
    public:
        OnElementAddedEvent(SimpleGuid parent_id, ElementArchetype new_element)
            : parent_entity_id(parent_id), element_archetype(std::move(new_element)) {}

        SimpleGuid parent_entity_id;
        ElementArchetype element_archetype;

        EVENT_CLASS_TYPE(EditorOnElementAdded) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        CLONE_EVENT_METHOD(OnElementAddedEvent)
    };
}