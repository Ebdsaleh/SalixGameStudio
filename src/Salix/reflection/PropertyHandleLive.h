// Salix/reflection/PropertyHandleLive.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/reflection/PropertyHandle.h>


// Forward-declare the Element class to avoid including the full header,
// as we only need a pointer to it here. This improves compile times.
namespace Salix {
    class Element;
}

namespace Salix {

    // Concrete implementation of PropertyHandle for live C++ objects.
    // This class wraps a pointer to a live component (an object derived from Element*)
    // and uses the reflection functions from ByteMirror (via the Property struct)
    // to get and set its properties.
    class SALIX_API PropertyHandleLive : public PropertyHandle {
        public:
            // Constructor: Takes the reflection info for the property and a pointer to the live object instance.
            PropertyHandleLive(const Property* property_info, Element* instance);

            // Explicitly default the virtual destructor. Good practice.
            ~PropertyHandleLive() override = default;

            // Override the pure virtual functions from the PropertyHandle base class.
            // The full implementation for these will be in PropertyHandle_Live.cpp.
            PropertyValue get_value() const override;
            void set_value(const PropertyValue& value) override;

        private:
            // A pointer to the live component instance that this handle operates on.
            Element* instance;
    };

} // namespace Salix