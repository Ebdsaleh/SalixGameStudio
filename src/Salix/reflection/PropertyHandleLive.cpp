// Salix/reflection/PropertyHandleLive.cpp
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/ecs/Element.h>

namespace Salix {

// The constructor's only job is to pass the required info to the base class
// and store the pointer to the live element instance.
PropertyHandleLive::PropertyHandleLive(const Property& property_info, Element* instance)
    : PropertyHandle(property_info), instance(instance) {}

// --- getValue() ---
// This function's whole job is to look at the property's type, call the
// generic get_data() function, and then use the correct static_cast to turn
// the void* into a specific type that we can safely store in our PropertyValue variant.
PropertyValue PropertyHandleLive::get_value() const
{
    if (!instance || !property_info.get_data) {
        return {}; // Return empty variant if something is wrong
    }

    // Reject unsupported reflection types before invoking a getter.
    switch (property_info.type) {
        case PropertyType::Int:
        case PropertyType::UInt64:
        case PropertyType::Enum:
        case PropertyType::EnumClass:
        case PropertyType::Float:
        case PropertyType::Bool:
        case PropertyType::String:
        case PropertyType::Vector2:
        case PropertyType::Vector3:
        case PropertyType::Point:
        case PropertyType::Color:
        case PropertyType::Rect:
        case PropertyType::GlmMat4:
            break;
        default:
            return {};
    }

    void* data_ptr = property_info.get_data(instance);
    if (!data_ptr) {
        return {};
    }

    // This switch is the bridge between the void* world and the variant world
    switch (property_info.type)
    {
        case PropertyType::Int:
        return *static_cast<int*>(data_ptr);
        case PropertyType::UInt64:
            return *static_cast<uint64_t*>(data_ptr);

        case PropertyType::Enum:{ 
            // Enums are handled as ints at this level
            // 1. Get the pointer from the reflection getter, which we know is an int*.
            void* data_ptr = data_ptr;
            // 2. Cast it to the correct type (int*) and dereference it.
            return *static_cast<int*>(data_ptr);
        }
        case PropertyType::EnumClass: {
        // 1. Get the pointer from the reflection getter, which we know is an int*.
        void* data_ptr = data_ptr;
        // 2. Cast it to the correct type (int*) and dereference it.
        return *static_cast<int*>(data_ptr);
        }

        case PropertyType::Float:
            return *static_cast<float*>(data_ptr);

        case PropertyType::Bool:
            return *static_cast<bool*>(data_ptr);

        case PropertyType::String:
            return *static_cast<std::string*>(data_ptr);

        case PropertyType::Vector2:
            return *static_cast<Vector2*>(data_ptr);

        case PropertyType::Vector3:
            return *static_cast<Vector3*>(data_ptr);

        case PropertyType::Point:
            return *static_cast<Point*>(data_ptr);

        case PropertyType::Color:
            return *static_cast<Color*>(data_ptr);

        case PropertyType::Rect: // Your new Rect type
            return *static_cast<Rect*>(data_ptr);

        case PropertyType::GlmMat4:
            return *static_cast<glm::mat4*>(data_ptr);

        default:
            // For unhandled types like Class/Struct, we return an empty variant
            return {};
    }
}

// --- setValue() ---
// This function does the reverse. It checks the property's type, makes sure
// the PropertyValue variant we received actually contains the correct type of data
// (that's what std::holds_alternative does), and then passes a pointer to that
// data to your generic set_data function.
void PropertyHandleLive::set_value(const PropertyValue& value)
{
    if (!instance || !property_info.set_data) {
        return; // Can't set if there's no instance or no setter
    }

    switch (property_info.type)
    {
        case PropertyType::Int:
            if (std::holds_alternative<int>(value)) {
                    auto copy = std::get<int>(value);
                    property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::UInt64:
            if (std::holds_alternative<uint64_t>(value)) {
                auto copy = std::get<uint64_t>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Enum:
        if (std::holds_alternative<int>(value)) {
                auto copy = std::get<int>(value);
                property_info.set_data(instance, &copy);
            }
            
            break;
        case PropertyType::EnumClass:
            if (std::holds_alternative<int>(value)) {
                auto copy = std::get<int>(value);
                property_info.set_data(instance, &copy);
            }
            
            break;

        case PropertyType::Float:
            if (std::holds_alternative<float>(value)) {
                auto copy = std::get<float>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Bool:
            if (std::holds_alternative<bool>(value)) {
                auto copy = std::get<bool>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::String:
            if (std::holds_alternative<std::string>(value)) {
                auto copy = std::get<std::string>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Vector2:
            if (std::holds_alternative<Vector2>(value)) {
                auto copy = std::get<Vector2>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Vector3:
            if (std::holds_alternative<Vector3>(value)) {
                auto copy = std::get<Vector3>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Point:
            if (std::holds_alternative<Point>(value)) {
                auto copy = std::get<Point>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Color:
            if (std::holds_alternative<Color>(value)) {
                auto copy = std::get<Color>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::Rect:
            if (std::holds_alternative<Rect>(value)) {
                auto copy = std::get<Rect>(value);
                property_info.set_data(instance, &copy);
            }
            break;

        case PropertyType::GlmMat4:
             // Note: setValue is intentionally not implemented for a read-only matrix
            break;
    }
}

} // namespace Salix