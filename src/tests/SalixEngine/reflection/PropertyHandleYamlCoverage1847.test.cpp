#include <doctest.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <yaml-cpp/yaml.h>
#include <cstdint>
#include <string>
#include <typeindex>
#include <utility>

namespace {
Salix::Property batch1847_yaml_property(const char* name, Salix::PropertyType type) {
    Salix::Property property; property.name = name; property.type = type; return property;
}
struct Batch1847YamlUnregisteredEnum {};
struct Batch1847YamlRegisteredEnum {};
}

TEST_SUITE("Salix::reflection::PropertyHandleYamlCoverage1847") {
    TEST_CASE("class and struct properties fall back without rewriting existing YAML") {
        for (auto type : {Salix::PropertyType::Class, Salix::PropertyType::Struct}) {
            YAML::Node node; node["Opaque"] = "keep";
            Salix::PropertyHandleYaml handle(batch1847_yaml_property("Opaque", type), &node);
            auto value = handle.get_value();
            CHECK(std::holds_alternative<int>(value));
            CHECK(std::get<int>(value) == 0);
            handle.set_value(Salix::PropertyValue{42});
            CHECK(node["Opaque"].as<std::string>() == "keep");
        }
    }

    TEST_CASE("uint64 property rejects integer variant and preserves YAML scalar") {
        YAML::Node node; node["Value"] = std::uint64_t{99};
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::UInt64), &node);
        handle.set_value(Salix::PropertyValue{7});
        CHECK(node["Value"].as<std::uint64_t>() == 99);
    }

    TEST_CASE("float property rejects bool variant and preserves YAML scalar") {
        YAML::Node node; node["Value"] = 1.5f;
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::Float), &node);
        handle.set_value(Salix::PropertyValue{true});
        CHECK(node["Value"].as<float>() == doctest::Approx(1.5f));
    }

    TEST_CASE("bool property rejects integer variant and preserves YAML scalar") {
        YAML::Node node; node["Value"] = true;
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::Bool), &node);
        handle.set_value(Salix::PropertyValue{1});
        CHECK(node["Value"].as<bool>());
    }

    TEST_CASE("string property rejects integer variant and preserves YAML scalar") {
        YAML::Node node; node["Value"] = "keep";
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::String), &node);
        handle.set_value(Salix::PropertyValue{7});
        CHECK(node["Value"].as<std::string>() == "keep");
    }

    TEST_CASE("Vector2 property rejects Vector3 variant and preserves mapped YAML") {
        YAML::Node node; node["Value"] = Salix::Vector2(1.0f, 2.0f);
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::Vector2), &node);
        handle.set_value(Salix::PropertyValue{Salix::Vector3(9.0f, 8.0f, 7.0f)});
        CHECK(node["Value"].as<Salix::Vector2>() == Salix::Vector2(1.0f, 2.0f));
    }

    TEST_CASE("Vector3 property rejects Vector2 variant and preserves mapped YAML") {
        YAML::Node node; node["Value"] = Salix::Vector3(3.0f, 4.0f, 5.0f);
        Salix::PropertyHandleYaml handle(batch1847_yaml_property("Value", Salix::PropertyType::Vector3), &node);
        handle.set_value(Salix::PropertyValue{Salix::Vector2(9.0f, 8.0f)});
        CHECK(node["Value"].as<Salix::Vector3>() == Salix::Vector3(3.0f, 4.0f, 5.0f));
    }

    TEST_CASE("enum class without contained type metadata falls back and preserves YAML name") {
        YAML::Node node; node["Mode"] = "Existing";
        auto property = batch1847_yaml_property("Mode", Salix::PropertyType::EnumClass);
        Salix::PropertyHandleYaml handle(property, &node);
        auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
        handle.set_value(Salix::PropertyValue{3});
        CHECK(node["Mode"].as<std::string>() == "Existing");
    }

    TEST_CASE("enum class with unregistered contained type falls back without rewriting YAML") {
        YAML::Node node; node["Mode"] = "Existing";
        Salix::TypeInfo contained; contained.name = "Missing"; contained.type_index = typeid(Batch1847YamlUnregisteredEnum);
        auto property = batch1847_yaml_property("Mode", Salix::PropertyType::EnumClass); property.contained_type_info = &contained;
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == 0);
        handle.set_value(Salix::PropertyValue{5});
        CHECK(node["Mode"].as<std::string>() == "Existing");
    }

    TEST_CASE("registered enum class maps unknown YAML name to sentinel and unknown integer to Unknown string") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[1] = "One"; data.ordered_names = {"One"};
        Salix::EnumRegistry::register_enum(typeid(Batch1847YamlRegisteredEnum), std::move(data));
        Salix::TypeInfo contained; contained.name = "Registered"; contained.type_index = typeid(Batch1847YamlRegisteredEnum);
        auto property = batch1847_yaml_property("Mode", Salix::PropertyType::EnumClass); property.contained_type_info = &contained;
        YAML::Node node; node["Mode"] = "Missing";
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == -1);
        handle.set_value(Salix::PropertyValue{999});
        CHECK(node["Mode"].as<std::string>() == "Unknown");
    }
}
