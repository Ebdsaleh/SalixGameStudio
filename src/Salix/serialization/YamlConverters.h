// Salix/serialization/YamlConverters.h
#pragma once
#include <Salix/reflection/PropertyHandle.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/reflection/EnumRegistry.h>
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

// This namespace contains all the template specializations that teach
// yaml-cpp how to encode and decode the custom engine types.
namespace YAML {

    // --- Converter for Salix::SimpleGuid ---
    template<>
    struct convert<Salix::SimpleGuid> {
        static inline Node encode(const Salix::SimpleGuid& guid) {
            return Node(guid.get_value());
        }
        static inline bool decode(const Node& node, Salix::SimpleGuid& guid) {
            if (!node.IsScalar()) {
                return false;
            }
            // The key is here: Use the public static factory method
            // to create the SimpleGuid, instead of trying to call
            // a private constructor.
            guid = Salix::SimpleGuid::from_value(node.as<uint64_t>());
            return true;
        }
    };

    // --- Converter for Salix::Vector2 ---
    template<>
    struct convert<Salix::Vector2> {
        static inline Node encode(const Salix::Vector2& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }
        static inline bool decode(const Node& node, Salix::Vector2& rhs) {
            if (!node.IsMap() || node.size() != 2) return false;
            rhs.x = node["x"].as<float>();
            rhs.y = node["y"].as<float>();
            return true;
        }
    };

    // --- Converter for Salix::Vector3 ---
    template<>
    struct convert<Salix::Vector3> {
        static inline Node encode(const Salix::Vector3& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["z"] = rhs.z;
            return node;
        }
        static inline bool decode(const Node& node, Salix::Vector3& rhs) {
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
        static inline Node encode(const Salix::Color& rhs) {
            Node node;
            node["r"] = rhs.r;
            node["g"] = rhs.g;
            node["b"] = rhs.b;
            node["a"] = rhs.a;
            return node;
        }
        static inline bool decode(const Node& node, Salix::Color& rhs) {
            if (!node.IsMap() || node.size() != 4) return false;
            rhs.r = node["r"].as<float>();
            rhs.g = node["g"].as<float>();
            rhs.b = node["b"].as<float>();
            rhs.a = node["a"].as<float>();
            return true;
        }
    };

    // --- Converter for Salix::Point ---
    template<>
    struct convert<Salix::Point> {
        static inline Node encode(const Salix::Point& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }
        static inline bool decode(const Node& node, Salix::Point& rhs) {
            if (!node.IsMap() || node.size() != 2) return false;
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();
            return true;
        }
    };

    // --- Converter for Salix::Rect ---
    template<>
    struct convert<Salix::Rect> {
        static inline Node encode(const Salix::Rect& rhs) {
            Node node;
            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["w"] = rhs.w;
            node["h"] = rhs.h;
            return node;
        }
        static inline bool decode(const Node& node, Salix::Rect& rhs) {
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
        static inline Node encode(const glm::mat4& rhs) {
            Node node(NodeType::Sequence);
            for (int i = 0; i < 4; ++i) {
                node.push_back(std::vector<float>{rhs[i][0], rhs[i][1], rhs[i][2], rhs[i][3]});
            }
            return node;
        }
        static inline bool decode(const Node& node, glm::mat4& rhs) {
            if (!node.IsSequence() || node.size() != 4) return false;
            for (int i = 0; i < 4; ++i) {
                std::vector<float> column = node[i].as<std::vector<float>>();
                if (column.size() != 4) return false;
                for (int j = 0; j < 4; ++j) {
                    rhs[i][j] = column[j];
                }
            }
            return true;
        }
    };

    inline Node property_value_to_node(const Salix::PropertyValue& value) {
        using namespace Salix;

        if (std::holds_alternative<int>(value)) {
            return Node(std::get<int>(value));
        }
        if (std::holds_alternative<uint64_t>(value)) {
            return Node(std::get<uint64_t>(value));
        }
        if (std::holds_alternative<float>(value)) {
            return Node(std::get<float>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            return Node(std::get<bool>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            return Node(std::get<std::string>(value));
        }
        if (std::holds_alternative<Vector2>(value)) {
            return convert<Vector2>::encode(std::get<Vector2>(value));
        }
        if (std::holds_alternative<Vector3>(value)) {
            return convert<Vector3>::encode(std::get<Vector3>(value));
        }
        if (std::holds_alternative<Color>(value)) {
            return convert<Color>::encode(std::get<Color>(value));
        }
        if (std::holds_alternative<Point>(value)) {
            return convert<Point>::encode(std::get<Point>(value));
        }
        if (std::holds_alternative<Rect>(value)) {
            return convert<Rect>::encode(std::get<Rect>(value));
        }
        if (std::holds_alternative<glm::mat4>(value)) {
            return convert<glm::mat4>::encode(std::get<glm::mat4>(value));
        }

        // Fallback: empty node if type not recognized
        return Node();
    }

    inline Salix::PropertyValue node_to_property_value(const YAML::Node& node, const Salix::Property& prop) {
        using namespace Salix;
        if (!node) return {}; // Return empty if the node doesn't exist

        switch (prop.type) {
            case PropertyType::Int:       return node.as<int>();
            case PropertyType::UInt64:    return node.as<uint64_t>();
            case PropertyType::Float:     return node.as<float>();
            case PropertyType::Bool:      return node.as<bool>();
            case PropertyType::String:    return node.as<std::string>();
            case PropertyType::Vector2:   return node.as<Vector2>();
            case PropertyType::Vector3:   return node.as<Vector3>();
            case PropertyType::Color:     return node.as<Color>();
            case PropertyType::Point:     return node.as<Point>();
            case PropertyType::Rect:      return node.as<Rect>();
            case PropertyType::GlmMat4:   return node.as<glm::mat4>();
            case PropertyType::EnumClass: {
                if (prop.contained_type_info && prop.contained_type_info->type_index.has_value()) {
                    const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*prop.contained_type_info->type_index);
                    if (enum_data) {
                        return enum_data->get_value(node.as<std::string>());
                    }
                }
                return {};
            }
            default: 
                return {};
        }
    }


    // YAML::Node comparitors
    inline bool nodes_are_equal(const Node& a, const Node& b) {
        if (a.Type() != b.Type()) return false;

        switch (a.Type()) {
            case NodeType::Scalar:
                return a.as<std::string>() == b.as<std::string>();
            case NodeType::Map:
                if (a.size() != b.size()) return false;
                for (auto it : a) {
                    auto key = it.first.as<std::string>();
                    if (!b[key] || !nodes_are_equal(it.second, b[key])) return false;
                }
                return true;
            case NodeType::Sequence:
                if (a.size() != b.size()) return false;
                for (size_t i = 0; i < a.size(); ++i)
                    if (!nodes_are_equal(a[i], b[i])) return false;
                return true;
            case NodeType::Null:
                return true;
        }
        return false;
    }

    // Deep comparison.
    inline bool deep_compare_yaml(const YAML::Node& a, const YAML::Node& b) {
        if (!a && !b) return true;
        if (!a || !b) return false;
        if (a.Type() != b.Type()) return false;

        switch (a.Type()) {
            case YAML::NodeType::Scalar:
                return a.Scalar() == b.Scalar();
            case YAML::NodeType::Sequence:
                if (a.size() != b.size()) return false;
                for (std::size_t i = 0; i < a.size(); ++i) {
                    if (!deep_compare_yaml(a[i], b[i])) return false;
                }
                return true;
            case YAML::NodeType::Map:
                if (a.size() != b.size()) return false;
                for (auto it = a.begin(); it != a.end(); ++it) {
                    auto key = it->first.as<std::string>();
                    if (!b[key]) return false;
                    if (!deep_compare_yaml(it->second, b[key])) return false;
                }
                return true;
            default:
                return true;
        }
    }

 
    // Debugging deep comparison of YAML nodes
    inline bool debug_compare_yaml(const YAML::Node& a, const YAML::Node& b, const std::string& path = "") {
        if (!a && !b) return true;
        if (!a || !b) {
            std::cout << "  [DIFF] One node missing at path: " << path << std::endl;
            return false;
        }

        if (a.Type() != b.Type()) {
            std::cout << "  [DIFF] Type mismatch at path: " << path
                    << " (" << a.Type() << " vs " << b.Type() << ")" << std::endl;
            return false;
        }

        switch (a.Type()) {
            case YAML::NodeType::Scalar: {
                std::string aVal = a.Scalar();
                std::string bVal = b.Scalar();
                if (aVal != bVal) {
                    std::cout << "  [DIFF] Scalar mismatch at " << path
                            << " ('" << aVal << "' vs '" << bVal << "')" << std::endl;
                    return false;
                }
                std::cout << "  [OK] " << path << " = '" << aVal << "'" << std::endl;
                return true;
            }
            case YAML::NodeType::Sequence: {
                if (a.size() != b.size()) {
                    std::cout << "  [DIFF] Sequence size mismatch at " << path
                            << " (" << a.size() << " vs " << b.size() << ")" << std::endl;
                    return false;
                }
                for (std::size_t i = 0; i < a.size(); ++i) {
                    if (!debug_compare_yaml(a[i], b[i], path + "[" + std::to_string(i) + "]")) {
                        return false;
                    }
                }
                return true;
            }
            case YAML::NodeType::Map: {
                if (a.size() != b.size()) {
                    std::cout << "  [DIFF] Map size mismatch at " << path
                            << " (" << a.size() << " vs " << b.size() << ")" << std::endl;
                    return false;
                }
                for (auto it = a.begin(); it != a.end(); ++it) {
                    std::string key = it->first.as<std::string>();
                    if (!b[key]) {
                        std::cout << "  [DIFF] Missing key '" << key
                                << "' in other node at path: " << path << std::endl;
                        return false;
                    }
                    if (!debug_compare_yaml(it->second, b[key], path.empty() ? key : path + "." + key)) {
                        return false;
                    }
                }
                return true;
            }
            default:
                std::cout << "  [OK] " << path << " is Null/Undefined" << std::endl;
                return true;
        }
    }

    // instanced cloning.
    inline YAML::Node deep_clone_yaml(const YAML::Node& node) {
        if (!node) return YAML::Node();

        YAML::Node clone;

        switch (node.Type()) {
            case YAML::NodeType::Scalar:
                clone = YAML::Node(node.Scalar());
                break;

            case YAML::NodeType::Sequence:
                for (std::size_t i = 0; i < node.size(); ++i) {
                    clone.push_back(deep_clone_yaml(node[i]));
                }
                break;

            case YAML::NodeType::Map:
                for (auto it = node.begin(); it != node.end(); ++it) {
                    // Force key as string to avoid dangling references
                    std::string key = it->first.as<std::string>();
                    clone[key] = deep_clone_yaml(it->second);
                }
                break;

            case YAML::NodeType::Null:
            default:
                clone = YAML::Node(); // empty node
                break;
        }

        return clone;
    }

} // namespace YAML