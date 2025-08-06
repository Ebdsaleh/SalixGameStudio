// Salix/reflection/PropertyHandleYaml.cpp
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML {
    template<>
    struct convert<Salix::Vector2> {
        static Node encode(const Salix::Vector2& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }
        static bool decode(const Node& node, Salix::Vector2& rhs) {
            if (!node.IsMap() || node.size() != 2) return false;
            rhs.x = node["x"].as<float>();
            rhs.y = node["y"].as<float>();
            return true;
        }
    };

    template<>
    struct convert<Salix::Vector3> {
        static Node encode(const Salix::Vector3& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["z"] = rhs.z;
            return node;
        }
        static bool decode(const Node& node, Salix::Vector3& rhs) {
            if (!node.IsMap() || node.size() != 3) return false;
            rhs.x = node["x"].as<float>();
            rhs.y = node["y"].as<float>();
            rhs.z = node["z"].as<float>();
            return true;
        }
    };


    // --- Converter for Salix::Color ---
    template<>
    struct convert<Salix::Color> {
        static Node encode(const Salix::Color& rhs) {
            Node node;
            node["r"] = static_cast<int>(rhs.r);
            node["g"] = static_cast<int>(rhs.g);
            node["b"] = static_cast<int>(rhs.b);
            node["a"] = static_cast<int>(rhs.a);
            return node;
        }
        static bool decode(const Node& node, Salix::Color& rhs) {
            if (!node.IsMap() || node.size() != 4) return false;
            rhs.r = node["r"].as<uint8_t>();
            rhs.g = node["g"].as<uint8_t>();
            rhs.b = node["b"].as<uint8_t>();
            rhs.a = node["a"].as<uint8_t>();
            return true;
        }
    };

    // --- Converter for Salix::Point ---
    template<>
    struct convert<Salix::Point> {
        static Node encode(const Salix::Point& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }
        static bool decode(const Node& node, Salix::Point& rhs) {
            if (!node.IsMap() || node.size() != 2) return false;
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();
            return true;
        }
    };

    // --- Converter for Salix::Rect ---
    template<>
    struct convert<Salix::Rect> {
        static Node encode(const Salix::Rect& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["w"] = rhs.w;
            node["h"] = rhs.h;
            return node;
        }
        static bool decode(const Node& node, Salix::Rect& rhs) {
            if (!node.IsMap() || node.size() != 4) return false;
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();
            rhs.w = node["w"].as<int>();
            rhs.h = node["h"].as<int>();
            return true;
        }
    };

    // --- Converter for glm::mat4 ---
    template<>
    struct convert<glm::mat4> {
        // A mat4 will be encoded as a sequence of 4 sequences (columns)
        static Node encode(const glm::mat4& rhs) {
            Node node(NodeType::Sequence);
            for (int i = 0; i < 4; ++i) {
                Node column(NodeType::Sequence);
                column.push_back(rhs[i][0]);
                column.push_back(rhs[i][1]);
                column.push_back(rhs[i][2]);
                column.push_back(rhs[i][3]);
                node.push_back(column);
            }
            return node;
        }
        static bool decode(const Node& node, glm::mat4& rhs) {
            if (!node.IsSequence() || node.size() != 4) return false;
            for (int i = 0; i < 4; ++i) {
                Node column = node[i];
                if (!column.IsSequence() || column.size() != 4) return false;
                for (int j = 0; j < 4; ++j) {
                    rhs[i][j] = column[j].as<float>();
                }
            }
            return true;
        }
    };

    // --- Converter for SimpleGuid ---
    template<>
    struct convert<Salix::SimpleGuid> {
        // This function tells yaml-cpp how to turn a SimpleGuid INTO a YAML node
        static Node encode(const Salix::SimpleGuid& guid) {
            return Node(guid.get_value());
        }

        // This function tells yaml-cpp how to turn a YAML node INTO a SimpleGuid
        static bool decode(const Node& node, Salix::SimpleGuid& guid) {
            // We use the private constructor by calling it through our friend declaration
            guid = Salix::SimpleGuid(node.as<uint64_t>());
            return true;
        }
    };
    
} // namespace YAML

namespace Salix {

    PropertyHandleYaml::PropertyHandleYaml(const Property* property_info, YAML::Node* component_node)
        : PropertyHandle(property_info), component_node(component_node) {}

    PropertyValue PropertyHandleYaml::get_value() const
    {
        if (!component_node) {
            return {};
        }

        // Get the node for the specific property (e.g., "Position")
        const YAML::Node property_node = (*component_node)[property_info->name];
        if (!property_node) {
            return {}; // Return empty if the property doesn't exist in the YAML
        }

        // This switch uses the clean .as<T>() syntax thanks to our converters
        switch (property_info->type)
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
            default: return {};
        }
    }

    void PropertyHandleYaml::set_value(const PropertyValue& value)
    {
        if (!component_node) {
            return;
        }

        // This switch assigns the value from the variant directly to the YAML node
        switch (property_info->type)
        {
            case PropertyType::Int:
                if (std::holds_alternative<int>(value))
                    (*component_node)[property_info->name] = std::get<int>(value);
                break;
            
            case PropertyType::UInt64:
                if (std::holds_alternative<uint64_t>(value))
                    (*component_node)[property_info->name] = std::get<uint64_t>(value);
                break;

            case PropertyType::Float:
                if (std::holds_alternative<float>(value))
                    (*component_node)[property_info->name] = std::get<float>(value);
                break;

            case PropertyType::Bool:
                if (std::holds_alternative<bool>(value))
                    (*component_node)[property_info->name] = std::get<bool>(value);
                break;

            case PropertyType::String:
                if (std::holds_alternative<std::string>(value))
                    (*component_node)[property_info->name] = std::get<std::string>(value);
                break;

            case PropertyType::Vector2:
                if (std::holds_alternative<Vector2>(value))
                    (*component_node)[property_info->name] = std::get<Vector2>(value);
                break;

            case PropertyType::Vector3:
                if (std::holds_alternative<Vector3>(value))
                    (*component_node)[property_info->name] = std::get<Vector3>(value);
                break;

            case PropertyType::Color:
                if (std::holds_alternative<Color>(value))
                    (*component_node)[property_info->name] = std::get<Color>(value);
                break;

            case PropertyType::Point:
                if (std::holds_alternative<Point>(value))
                    (*component_node)[property_info->name] = std::get<Point>(value);
                break;

            case PropertyType::Rect:
                if (std::holds_alternative<Rect>(value))
                    (*component_node)[property_info->name] = std::get<Rect>(value);
                break;

            case PropertyType::GlmMat4:
                if (std::holds_alternative<glm::mat4>(value))
                    (*component_node)[property_info->name] = std::get<glm::mat4>(value);
                break;
        }
    }


}  // namespace Salix