// Editor/events/OnHierarchyChangedEvent.h
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

class OnHierarchyChangedEvent : public IEvent {
    public:
        OnHierarchyChangedEvent(SimpleGuid child_id, SimpleGuid new_parent_id)
            : entity_id(child_id), parent_id(new_parent_id) {}

        SimpleGuid entity_id;
        SimpleGuid parent_id; // This will be SimpleGuid::invalid() if the child was orphaned.

        EVENT_CLASS_TYPE(EditorOnHierarchyChanged)
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
    };

} // namespace Salix