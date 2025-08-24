// Salix/reflection/PropertyHandleYaml.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <yaml-cpp/yaml.h>
#include <memory>


namespace Salix {

    PropertyHandleYaml::PropertyHandleYaml(const Property& property_info, YAML::Node* element_node)
        : PropertyHandle(property_info), element_node(element_node) {}

    PropertyValue PropertyHandleYaml::get_value() const
    {
        if (!element_node) {
            return {};
        }

        // Get the node for the specific property (e.g., "Position")
        const YAML::Node property_node = (*element_node)[property_info.name];
        if (!property_node) {
            return {}; // Return empty if the property doesn't exist in the YAML
        }

        // This switch uses the clean .as<T>() syntax thanks to our converters
        switch (property_info.type)
        {
            case PropertyType::Int:       return property_node.as<int>();
            case PropertyType::UInt64:    return property_node.as<uint64_t>();
            case PropertyType::Float:     return property_node.as<float>();
            case PropertyType::Bool:      return property_node.as<bool>();
            case PropertyType::String:    return property_node.as<std::string>();
            case PropertyType::Vector2:   return property_node.as<Vector2>();
            case PropertyType::Vector3:   return property_node.as<Vector3>();
            case PropertyType::Color:     return property_node.as<Color>();
            case PropertyType::Point:     return property_node.as<Point>();
            case PropertyType::Rect:      return property_node.as<Rect>();
            case PropertyType::GlmMat4:   return property_node.as<glm::mat4>();
            case PropertyType::Enum:      return property_node.as<int>();
            // NEW: Handle EnumClass for getting values
            case PropertyType::EnumClass: {
                if (property_info.contained_type_info && property_info.contained_type_info->type_index.has_value()) {
                    const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*property_info.contained_type_info->type_index);
                    if (enum_data) {
                        // Read the string from YAML and use the registry to find its int value
                        return enum_data->get_value(property_node.as<std::string>());
                    }
                }
                return {}; // Return empty if registry info not found
            }
            
            default: return {};
        }
    }

    void PropertyHandleYaml::set_value(const PropertyValue& value)
    {
        if (!element_node) {
            return;
        }

        // This switch assigns the value from the variant directly to the YAML node
        switch (property_info.type)
        {
            case PropertyType::Int:
                if (std::holds_alternative<int>(value))
                    (*element_node)[property_info.name] = std::get<int>(value);
                break;
            
            case PropertyType::UInt64:
                if (std::holds_alternative<uint64_t>(value))
                    (*element_node)[property_info.name] = std::get<uint64_t>(value);
                break;

            case PropertyType::Float:
                if (std::holds_alternative<float>(value))
                    (*element_node)[property_info.name] = std::get<float>(value);
                break;

            case PropertyType::Bool:
                if (std::holds_alternative<bool>(value))
                    (*element_node)[property_info.name] = std::get<bool>(value);
                break;

            case PropertyType::String:
                if (std::holds_alternative<std::string>(value))
                    (*element_node)[property_info.name] = std::get<std::string>(value);
                break;

            case PropertyType::Vector2:
                if (std::holds_alternative<Vector2>(value))
                    (*element_node)[property_info.name] = std::get<Vector2>(value);
                break;

            case PropertyType::Vector3:
                if (std::holds_alternative<Vector3>(value))
                    (*element_node)[property_info.name] = std::get<Vector3>(value);
                break;

            case PropertyType::Color:
                if (std::holds_alternative<Color>(value))
                    (*element_node)[property_info.name] = std::get<Color>(value);
                break;

            case PropertyType::Point:
                if (std::holds_alternative<Point>(value))
                    (*element_node)[property_info.name] = std::get<Point>(value);
                break;

            case PropertyType::Rect:
                if (std::holds_alternative<Rect>(value))
                    (*element_node)[property_info.name] = std::get<Rect>(value);
                break;

            case PropertyType::GlmMat4:
                if (std::holds_alternative<glm::mat4>(value))
                    (*element_node)[property_info.name] = std::get<glm::mat4>(value);
                break;
           
            
            case PropertyType::Enum:
                if (std::holds_alternative<int>(value))
                    (*element_node)[property_info.name] = std::get<int>(value);
                break;
            
            case PropertyType::EnumClass: {
                if (property_info.contained_type_info && property_info.contained_type_info->type_index.has_value() && std::holds_alternative<int>(value)) {
                    const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*property_info.contained_type_info->type_index);
                    if (enum_data) {
                        // Get the int from the UI and use the registry to find its string representation
                        (*element_node)[property_info.name] = enum_data->get_name(std::get<int>(value));
                    }
                }
                break;
            }
            
        }
    }


}  // namespace Salix

