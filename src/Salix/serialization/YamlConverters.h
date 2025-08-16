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

    

} // namespace YAML