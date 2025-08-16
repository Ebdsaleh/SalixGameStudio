// Editor/events/PropertyValueChangedEvent.h
#pragma once

#include <Salix/events/IEvent.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/reflection/PropertyHandle.h> 
#include <string>

#ifndef EVENT_CLASS_TYPE
#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; }\
                                virtual EventType get_event_type() const override { return get_static_type(); }\
                                virtual const char* get_name() const override { return #type; }
#endif
#ifndef EVENT_CLASS_CATEGORY
#define EVENT_CLASS_CATEGORY(category) virtual int get_category_flags() const override { return static_cast<int>(category); }
#endif

namespace Salix {

class PropertyValueChangedEvent : public IEvent {
    public:
        // Constructor to initialize all the event data
        PropertyValueChangedEvent(SimpleGuid in_entity_id, SimpleGuid in_element_id, const std::string& in_element_type, const std::string& in_property_name, const PropertyValue& in_new_value)
            : entity_id(in_entity_id), element_id(in_element_id), element_type_name(in_element_type), property_name(in_property_name), new_value(in_new_value) {}

        EVENT_CLASS_TYPE(EditorPropertyValueChanged) 
        EVENT_CLASS_CATEGORY(EventCategory::Editor)

        // The ID of the entity that was changed.
        SimpleGuid entity_id;
        // The string name of the element that was changed (e.g., "Transform").
        std::string element_type_name;
        // The ID of the element that was changed, this is required to compare against snapshots.
        SimpleGuid element_id;
        // The string name of the property that was changed (e.g., "position").
        std::string property_name;
        // The new value for the property, stored in our std::variant.
        PropertyValue new_value;
    };

} // namespace Salix

