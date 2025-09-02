// Salix/events/BeforeEntityPurgedEvent.h
#pragma once
#include <Salix/events/IEvent.h>
#include <Salix/ecs/Entity.h>

// Note: These macros should probably be moved into a shared header like IEvent.h
// to avoid re-defining them, but for now, this works.
#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                 virtual EventType get_event_type() const override { return get_static_type(); }\
                                 virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {

// This event is dispatched right before an entity is purged from the scene.
class BeforeEntityPurgedEvent : public IEvent { 
    public:
        BeforeEntityPurgedEvent(Entity* entity_to_be_purged)
            : entity(entity_to_be_purged) {}

        EVENT_CLASS_TYPE(BeforeEntityPurged)
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        CLONE_EVENT_METHOD(BeforeEntityPurgedEvent)

        Entity* entity;
};

} // namespace Salix