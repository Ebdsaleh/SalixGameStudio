#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/ecs/Element.h> 
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

class ElementSelectedEvent : public IEvent {
    public:
        ElementSelectedEvent(SimpleGuid id, SimpleGuid owning_entity_id, Element* live_element = nullptr)
            : selected_id(id), owner_id(owning_entity_id), element(live_element) {}

        EVENT_CLASS_TYPE(EditorElementSelected) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)
        Element* element;
        // The stable ID, which works in both modes
        SimpleGuid owner_id;
        SimpleGuid selected_id;
    };

} // namespace Salix