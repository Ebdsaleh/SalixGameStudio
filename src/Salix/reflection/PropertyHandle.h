// Salix/reflection/PropertyHandle.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <ostream>
#include <variant>
#include <Salix/rendering/ICamera.h> 

namespace Salix {

    // A variant to hold any possible type a property could be.
    // This now becomes the primary way we pass data.
    using PropertyValue = std::variant<
        int, uint64_t, float, bool, std::string, Vector2, Vector3, Color, Point, Rect, glm::mat4
        // Note: You must add every type from your PropertyType enum here
    >;
    inline std::ostream& operator<<(std::ostream& os, const Salix::PropertyValue& value) {
        // std::visit calls the correct lambda based on the type currently held by the variant.
        std::visit([&os](const auto& val) {
            os << val;
        }, value);
        // Return the stream to allow for chaining (e.g., std::cout << val1 << val2;)
        return os;
    }

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
    protected:
        PropertyHandle(const Property& property_info) : property_info(property_info) {}
        const Property property_info;
    };

} // namespace Salix