// Editor/events/EntitySelectedEvent.h

#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/ecs/Entity.h>
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

class EntitySelectedEvent : public IEvent { 
    public:
        
        static bool block_selection;
        EntitySelectedEvent(SimpleGuid id, Entity* live_entity = nullptr)
            : selected_id(id), entity(live_entity) {}

        EVENT_CLASS_TYPE(EditorEntitySelected)
        EVENT_CLASS_CATEGORY(EventCategory::Editor)

        bool should_block() const override { return block_selection; }
        void set_block(bool block) override { block_selection = block;}
        Entity* entity;
        // The stable ID, which works in both modes
        SimpleGuid selected_id;
    };

} // namespace Salix