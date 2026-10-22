#include <doctest.h>
#include <Salix/serialization/YamlConverters.h>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>

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
