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
    // A queued family addition owns its authored data independently of the
    // realm and of other event copies, including nested YAML values.
    class EDITOR_API OnEntityFamilyAddedEvent : public IEvent {
    public:
        OnEntityFamilyAddedEvent(std::vector<EntityArchetype> new_archetypes);
        OnEntityFamilyAddedEvent(const OnEntityFamilyAddedEvent& other);
        OnEntityFamilyAddedEvent& operator=(const OnEntityFamilyAddedEvent& other);

        std::vector<EntityArchetype> archetypes;

        static EventType get_static_type();
        EventType get_event_type() const override;
        const char* get_name() const override;
        int get_category_flags() const override;
        std::unique_ptr<IEvent> clone() const override;
    };
}
