// Editor/events/EntitySelectedEvent.h

#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/ecs/Entity.h> // We need to know what an Entity is

// Assuming your macros are in a shared header like "EventMacros.h" or similar.
// If they are defined in SDLEvent.h, you might need to include that.
#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif


namespace Salix {

class EntitySelectedEvent : public IEvent {
public:
    EntitySelectedEvent(Entity* selected_entity)
        : entity(selected_entity) {}

    EVENT_CLASS_TYPE(EditorEntitySelected)
    EVENT_CLASS_CATEGORY(EventCategory::Editor)

    Entity* entity;
};

} // namespace Salix