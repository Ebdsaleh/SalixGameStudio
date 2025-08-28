// Editor/events/OnEntityFamilyAddedEvent.h
#pragma once

#include <Salix/events/IEvent.h>
#include <Editor/Archetypes.h>
#include <vector>

#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {
    class OnEntityFamilyAddedEvent : public IEvent {
    public:
        // Pass the vector of new archetypes by value for a safe copy
        OnEntityFamilyAddedEvent(std::vector<EntityArchetype> new_archetypes)
            : archetypes(std::move(new_archetypes)) {}

        std::vector<EntityArchetype> archetypes;

        EVENT_CLASS_TYPE(EditorOnEntityFamilyAdded) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
    };
}