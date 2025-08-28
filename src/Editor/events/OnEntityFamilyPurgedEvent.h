// Editor/events/OnEntityFamilyRemovedEvent.h
#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/core/SimpleGuid.h>
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
    class OnEntityFamilyPurgedEvent : public IEvent {
    public:
        OnEntityFamilyPurgedEvent(std::vector<SimpleGuid> removed_ids)
            : entity_ids(std::move(removed_ids)) {}

        std::vector<SimpleGuid> entity_ids;

        EVENT_CLASS_TYPE(EditorOnEntityFamilyPurged) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
    };
}