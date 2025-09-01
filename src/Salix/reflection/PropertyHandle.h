// Salix/reflection/PropertyHandle.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/ReflectionTypes.h>


namespace Salix {
   

    // Abstract base class for a generic property handle.
    class SALIX_API PropertyHandle {
    public:
        virtual ~PropertyHandle() = default;

        const std::string& get_name() const { return property_info.name; }
        PropertyType get_type() const { return property_info.type; }

        // These are now NON-template virtual functions. This will compile.
        virtual PropertyValue get_value() const = 0;
        virtual void set_value(const PropertyValue& value) = 0;
        const TypeInfo* get_contained_type_info() const {
            // Safely return the contained_type_info from the internal property struct
            return property_info.contained_type_info;
        }
        UIHint get_hint() const { return property_info.hint; } // UIHint getter.
        bool is_read_only() const { return property_info.is_read_only; } 
        int get_display_order() const {return property_info.display_order; }

    protected:
        PropertyHandle(const Property& property_info) : property_info(property_info) {}
        const Property property_info;
    };

} // namespace Salix