#include <doctest.h>
#include <Salix/serialization/YamlConverters.h>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <Salix/reflection/EnumRegistry.h>
#include <typeindex>
#include <utility>
#include <vector>

TEST_SUITE("Salix::serialization::YamlConvertersBatch1547") {
    TEST_CASE("YAML converter roundtrips SimpleGuid including high bits") {
        const Salix::SimpleGuid source = Salix::SimpleGuid::from_value(0xF000000012345678ULL);
        YAML::Node node = YAML::convert<Salix::SimpleGuid>::encode(source);
        Salix::SimpleGuid restored;
        REQUIRE(YAML::convert<Salix::SimpleGuid>::decode(node, restored));
        CHECK(restored == source);
    }

    TEST_CASE("YAML converter roundtrips Vector2 exactly within float precision") {
        Salix::Vector2 source{1.25f, -9.5f};
        YAML::Node node = YAML::convert<Salix::Vector2>::encode(source);
        Salix::Vector2 restored{};
        REQUIRE(YAML::convert<Salix::Vector2>::decode(node, restored));
        CHECK(restored.x == doctest::Approx(1.25f));
        CHECK(restored.y == doctest::Approx(-9.5f));
    }

    TEST_CASE("YAML Vector3 converter rejects malformed maps with wrong member count") {
        YAML::Node node;
        node["x"] = 1.0f;
        node["y"] = 2.0f;
        Salix::Vector3 restored{};
        CHECK_FALSE(YAML::convert<Salix::Vector3>::decode(node, restored));
    }

    TEST_CASE("YAML converter roundtrips color channels") {
        Salix::Color source{0.1f, 0.25f, 0.75f, 0.5f};
        YAML::Node node = YAML::convert<Salix::Color>::encode(source);
        Salix::Color restored;
        REQUIRE(YAML::convert<Salix::Color>::decode(node, restored));
        CHECK(restored.r == doctest::Approx(source.r));
        CHECK(restored.g == doctest::Approx(source.g));
        CHECK(restored.b == doctest::Approx(source.b));
        CHECK(restored.a == doctest::Approx(source.a));
    }

    TEST_CASE("YAML converters preserve point and rectangle integer geometry") {
        Salix::Point point{7, -4};
        Salix::Rect rect{1, 2, 320, 200};
        Salix::Point restored_point{};
        Salix::Rect restored_rect{};
        REQUIRE(YAML::convert<Salix::Point>::decode(YAML::convert<Salix::Point>::encode(point), restored_point));
        REQUIRE(YAML::convert<Salix::Rect>::decode(YAML::convert<Salix::Rect>::encode(rect), restored_rect));
        CHECK(restored_point.x == 7);
        CHECK(restored_point.y == -4);
        CHECK(restored_rect == rect);
    }

    TEST_CASE("YAML converter roundtrips full four by four matrix") {
        glm::mat4 source(1.0f);
        source[0][3] = 2.5f;
        source[2][1] = -7.0f;
        YAML::Node node = YAML::convert<glm::mat4>::encode(source);
        glm::mat4 restored(0.0f);
        REQUIRE(YAML::convert<glm::mat4>::decode(node, restored));
        CHECK(restored[0][0] == doctest::Approx(1.0f));
        CHECK(restored[0][3] == doctest::Approx(2.5f));
        CHECK(restored[2][1] == doctest::Approx(-7.0f));
    }

    TEST_CASE("property value to YAML node preserves primitive variant alternatives") {
        YAML::Node integer = YAML::property_value_to_node(Salix::PropertyValue{42});
        YAML::Node boolean = YAML::property_value_to_node(Salix::PropertyValue{true});
        YAML::Node text = YAML::property_value_to_node(Salix::PropertyValue{std::string("hello")});
        CHECK(integer.as<int>() == 42);
        CHECK(boolean.as<bool>());
        CHECK(text.as<std::string>() == "hello");
    }

    TEST_CASE("YAML node to property value respects reflected property type") {
        Salix::Property int_property;
        int_property.type = Salix::PropertyType::Int;
        Salix::Property vector_property;
        vector_property.type = Salix::PropertyType::Vector3;

        YAML::Node int_node = YAML::Load("123");
        YAML::Node vector_node;
        vector_node["x"] = 1.0f;
        vector_node["y"] = 2.0f;
        vector_node["z"] = 3.0f;

        auto int_value = YAML::node_to_property_value(int_node, int_property);
        auto vector_value = YAML::node_to_property_value(vector_node, vector_property);

        CHECK(std::get<int>(int_value) == 123);
        REQUIRE(std::holds_alternative<Salix::Vector3>(vector_value));
        CHECK(std::get<Salix::Vector3>(vector_value).z == doctest::Approx(3.0f));
    }

    TEST_CASE("YAML structural comparators detect nested equality and value differences") {
        YAML::Node first = YAML::Load("{name: test, values: [1, 2, 3], nested: {ok: true}}");
        YAML::Node same = YAML::Load("{nested: {ok: true}, values: [1, 2, 3], name: test}");
        YAML::Node different = YAML::Load("{name: test, values: [1, 9, 3], nested: {ok: true}}");
        CHECK(YAML::nodes_are_equal(first, same));
        CHECK(YAML::deep_compare_yaml(first, same));
        CHECK_FALSE(YAML::nodes_are_equal(first, different));
        CHECK_FALSE(YAML::deep_compare_yaml(first, different));
    }

    TEST_CASE("YAML deep clone duplicates nested structure without aliasing source") {
        YAML::Node source = YAML::Load("{root: {items: [alpha, beta], count: 2}}");
        YAML::Node clone = YAML::deep_clone_yaml(source);
        REQUIRE(YAML::deep_compare_yaml(source, clone));
        clone["root"]["items"][0] = "changed";
        clone["root"]["count"] = 9;
        CHECK(source["root"]["items"][0].as<std::string>() == "alpha");
        CHECK(source["root"]["count"].as<int>() == 2);
        CHECK_FALSE(YAML::deep_compare_yaml(source, clone));
    }
}

namespace {
struct Batch1847ConverterMissingEnum {};
}

TEST_SUITE("Salix::serialization::YamlConvertersBatch1847") {
    TEST_CASE("SimpleGuid decoder rejects non-scalar YAML without mutating destination") {
        YAML::Node node; node["value"] = 7;
        Salix::SimpleGuid destination = Salix::SimpleGuid::from_value(55);
        CHECK_FALSE(YAML::convert<Salix::SimpleGuid>::decode(node, destination));
        CHECK(destination.get_value() == 55);
    }

    TEST_CASE("Vector2 decoder rejects both non-map and extra-member map shapes") {
        Salix::Vector2 value{9.0f, 8.0f};
        YAML::Node sequence(YAML::NodeType::Sequence); sequence.push_back(1.0f); sequence.push_back(2.0f);
        CHECK_FALSE(YAML::convert<Salix::Vector2>::decode(sequence, value));
        YAML::Node map; map["x"]=1.0f; map["y"]=2.0f; map["extra"]=3.0f;
        CHECK_FALSE(YAML::convert<Salix::Vector2>::decode(map, value));
    }

    TEST_CASE("matrix decoder rejects malformed inner column length after valid outer shape") {
        YAML::Node matrix(YAML::NodeType::Sequence);
        matrix.push_back(std::vector<float>{1,0,0,0});
        matrix.push_back(std::vector<float>{0,1,0});
        matrix.push_back(std::vector<float>{0,0,1,0});
        matrix.push_back(std::vector<float>{0,0,0,1});
        glm::mat4 value(1.0f);
        CHECK_FALSE(YAML::convert<glm::mat4>::decode(matrix, value));
    }

    TEST_CASE("property value conversion covers uint64 and float alternatives") {
        const std::uint64_t large = (std::uint64_t{1} << 60) + 5;
        CHECK(YAML::property_value_to_node(Salix::PropertyValue{large}).as<std::uint64_t>() == large);
        CHECK(YAML::property_value_to_node(Salix::PropertyValue{-3.25f}).as<float>() == doctest::Approx(-3.25f));
    }

    TEST_CASE("property value conversion covers Vector2 and Vector3 alternatives") {
        auto v2 = YAML::property_value_to_node(Salix::PropertyValue{Salix::Vector2(1.5f,-2.5f)}).as<Salix::Vector2>();
        auto v3 = YAML::property_value_to_node(Salix::PropertyValue{Salix::Vector3(3.0f,4.0f,5.0f)}).as<Salix::Vector3>();
        CHECK(v2 == Salix::Vector2(1.5f,-2.5f));
        CHECK(v3 == Salix::Vector3(3.0f,4.0f,5.0f));
    }

    TEST_CASE("property value conversion covers Color Point and Rect alternatives") {
        auto color = YAML::property_value_to_node(Salix::PropertyValue{Salix::Color(0.1f,0.2f,0.3f,0.4f)}).as<Salix::Color>();
        auto point = YAML::property_value_to_node(Salix::PropertyValue{Salix::Point{-2,7}}).as<Salix::Point>();
        auto rect = YAML::property_value_to_node(Salix::PropertyValue{Salix::Rect(1,2,3,4)}).as<Salix::Rect>();
        CHECK(color == Salix::Color(0.1f,0.2f,0.3f,0.4f));
        CHECK((point == Salix::Point{-2,7}));
        CHECK(rect == Salix::Rect(1,2,3,4));
    }

    TEST_CASE("property value conversion covers full matrix alternative") {
        glm::mat4 matrix(1.0f); matrix[1][3] = 6.5f; matrix[3][0] = -2.0f;
        auto restored = YAML::property_value_to_node(Salix::PropertyValue{matrix}).as<glm::mat4>();
        CHECK(restored[1][3] == doctest::Approx(6.5f));
        CHECK(restored[3][0] == doctest::Approx(-2.0f));
    }

    TEST_CASE("node to property value covers remaining primitive and geometry switch alternatives") {
        Salix::Property p;
        p.type=Salix::PropertyType::UInt64; CHECK(std::get<std::uint64_t>(YAML::node_to_property_value(YAML::Node(std::uint64_t{77}),p))==77);
        p.type=Salix::PropertyType::Float; CHECK(std::get<float>(YAML::node_to_property_value(YAML::Node(2.5f),p))==doctest::Approx(2.5f));
        p.type=Salix::PropertyType::Bool; CHECK(std::get<bool>(YAML::node_to_property_value(YAML::Node(true),p)));
        p.type=Salix::PropertyType::String; CHECK(std::get<std::string>(YAML::node_to_property_value(YAML::Node("text"),p))=="text");
        YAML::Node point; point["x"]=4; point["y"]=-3; p.type=Salix::PropertyType::Point; CHECK((std::get<Salix::Point>(YAML::node_to_property_value(point,p))==Salix::Point{4,-3}));
        YAML::Node rect; rect["x"]=1; rect["y"]=2; rect["w"]=3; rect["h"]=4; p.type=Salix::PropertyType::Rect; CHECK(std::get<Salix::Rect>(YAML::node_to_property_value(rect,p))==Salix::Rect(1,2,3,4));
    }

    TEST_CASE("node to property value returns neutral variant for missing node and unsupported type") {
        Salix::Property p; p.type=Salix::PropertyType::Int;
        const YAML::Node parent = YAML::Load("{present: 1}");
        const YAML::Node missing_node = parent["missing"];
        REQUIRE_FALSE(missing_node.IsDefined());
        auto missing = YAML::node_to_property_value(missing_node, p);
        CHECK(std::holds_alternative<int>(missing)); CHECK(std::get<int>(missing)==0);
        YAML::Node scalar(9); p.type=Salix::PropertyType::Class;
        auto unsupported = YAML::node_to_property_value(scalar,p);
        CHECK(std::holds_alternative<int>(unsupported)); CHECK(std::get<int>(unsupported)==0);
    }

    TEST_CASE("enum class node conversion falls back when type metadata points to an unregistered enum") {
        Salix::TypeInfo contained; contained.name="Missing"; contained.type_index=typeid(Batch1847ConverterMissingEnum);
        Salix::Property property; property.type=Salix::PropertyType::EnumClass; property.contained_type_info=&contained;
        auto value = YAML::node_to_property_value(YAML::Node("AnyName"), property);
        CHECK(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==0);
    }
}
