#include <doctest.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/reflection/EnumRegistry.h>
#include <cstdint>
#include <string>
#include <typeindex>

namespace {
Salix::Property batch1047_yaml_property(const std::string& name, Salix::PropertyType type) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    return property;
}
struct Batch1047YamlEnumTag {};
}

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch1047") {
    TEST_CASE("handle exposes reflection metadata supplied at construction") {
        YAML::Node node;
        auto property = batch1047_yaml_property("Health", Salix::PropertyType::Int);
        property.hint = Salix::UIHint::MultilineText;
        property.is_read_only = true;
        property.display_order = 17;
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(handle.get_name() == "Health");
        CHECK(handle.get_type() == Salix::PropertyType::Int);
        CHECK(handle.get_hint() == Salix::UIHint::MultilineText);
        CHECK(handle.is_read_only());
        CHECK(handle.get_display_order() == 17);
    }

    TEST_CASE("null YAML node returns default value and safely ignores writes") {
        auto property = batch1047_yaml_property("Value", Salix::PropertyType::Int);
        Salix::PropertyHandleYaml handle(property, nullptr);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
        CHECK_NOTHROW(handle.set_value(Salix::PropertyValue{42}));
    }

    TEST_CASE("missing YAML property returns the default variant without creating a key") {
        YAML::Node node;
        node["Other"] = 9;
        auto property = batch1047_yaml_property("Missing", Salix::PropertyType::Int);
        Salix::PropertyHandleYaml handle(property, &node);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
        CHECK_FALSE(node["Missing"]);
        CHECK(node["Other"].as<int>() == 9);
    }

    TEST_CASE("integer and uint64 properties support typed read and write without cross-type writes") {
        YAML::Node node;
        node["Count"] = 7;
        auto int_property = batch1047_yaml_property("Count", Salix::PropertyType::Int);
        Salix::PropertyHandleYaml int_handle(int_property, &node);
        CHECK(std::get<int>(int_handle.get_value()) == 7);
        int_handle.set_value(Salix::PropertyValue{31});
        CHECK(node["Count"].as<int>() == 31);
        int_handle.set_value(Salix::PropertyValue{std::string("wrong")});
        CHECK(node["Count"].as<int>() == 31);

        const std::uint64_t large = (std::uint64_t{1} << 50) + 9;
        node["Identifier"] = large;
        auto id_property = batch1047_yaml_property("Identifier", Salix::PropertyType::UInt64);
        Salix::PropertyHandleYaml id_handle(id_property, &node);
        CHECK(std::get<std::uint64_t>(id_handle.get_value()) == large);
        id_handle.set_value(Salix::PropertyValue{large + 1});
        CHECK(node["Identifier"].as<std::uint64_t>() == large + 1);
    }

    TEST_CASE("float bool and string properties preserve their exact variant types") {
        YAML::Node node;
        node["Speed"] = 1.25f;
        node["Enabled"] = true;
        node["Label"] = "Player";

        Salix::PropertyHandleYaml speed(batch1047_yaml_property("Speed", Salix::PropertyType::Float), &node);
        Salix::PropertyHandleYaml enabled(batch1047_yaml_property("Enabled", Salix::PropertyType::Bool), &node);
        Salix::PropertyHandleYaml label(batch1047_yaml_property("Label", Salix::PropertyType::String), &node);

        CHECK(std::get<float>(speed.get_value()) == doctest::Approx(1.25f));
        CHECK(std::get<bool>(enabled.get_value()));
        CHECK(std::get<std::string>(label.get_value()) == "Player");

        speed.set_value(Salix::PropertyValue{-2.5f});
        enabled.set_value(Salix::PropertyValue{false});
        label.set_value(Salix::PropertyValue{std::string("Enemy")});
        CHECK(node["Speed"].as<float>() == doctest::Approx(-2.5f));
        CHECK_FALSE(node["Enabled"].as<bool>());
        CHECK(node["Label"].as<std::string>() == "Enemy");
    }

    TEST_CASE("Vector2 and Vector3 properties roundtrip through YAML converters") {
        YAML::Node node;
        node["Position2"] = Salix::Vector2(3.5f, -7.25f);
        node["Position3"] = Salix::Vector3(1.0f, 2.0f, 3.0f);
        Salix::PropertyHandleYaml two(batch1047_yaml_property("Position2", Salix::PropertyType::Vector2), &node);
        Salix::PropertyHandleYaml three(batch1047_yaml_property("Position3", Salix::PropertyType::Vector3), &node);

        CHECK(std::get<Salix::Vector2>(two.get_value()) == Salix::Vector2(3.5f, -7.25f));
        CHECK(std::get<Salix::Vector3>(three.get_value()) == Salix::Vector3(1.0f, 2.0f, 3.0f));

        two.set_value(Salix::PropertyValue{Salix::Vector2(-4.0f, 8.0f)});
        three.set_value(Salix::PropertyValue{Salix::Vector3(9.0f, -8.0f, 7.0f)});
        CHECK(node["Position2"].as<Salix::Vector2>() == Salix::Vector2(-4.0f, 8.0f));
        CHECK(node["Position3"].as<Salix::Vector3>() == Salix::Vector3(9.0f, -8.0f, 7.0f));
    }

    TEST_CASE("Color Point and Rect properties roundtrip through their YAML representations") {
        YAML::Node node;
        node["Tint"] = Salix::Color(0.1f, 0.2f, 0.3f, 0.4f);
        node["Origin"] = Salix::Point(4, -6);
        node["Bounds"] = Salix::Rect(1, 2, 30, 40);

        Salix::PropertyHandleYaml color(batch1047_yaml_property("Tint", Salix::PropertyType::Color), &node);
        Salix::PropertyHandleYaml point(batch1047_yaml_property("Origin", Salix::PropertyType::Point), &node);
        Salix::PropertyHandleYaml rect(batch1047_yaml_property("Bounds", Salix::PropertyType::Rect), &node);

        const auto tint = std::get<Salix::Color>(color.get_value());
        CHECK(tint == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));
        const auto origin = std::get<Salix::Point>(point.get_value());
        CHECK(origin.x == 4);
        CHECK(origin.y == -6);
        const auto bounds = std::get<Salix::Rect>(rect.get_value());
        CHECK(bounds.x == 1);
        CHECK(bounds.y == 2);
        CHECK(bounds.w == 30);
        CHECK(bounds.h == 40);

        color.set_value(Salix::PropertyValue{Salix::Color(1.0f, 0.0f, 0.5f, 1.0f)});
        point.set_value(Salix::PropertyValue{Salix::Point(-2, 5)});
        rect.set_value(Salix::PropertyValue{Salix::Rect(-1, -2, 8, 9)});
        CHECK(node["Tint"].as<Salix::Color>() == Salix::Color(1.0f, 0.0f, 0.5f, 1.0f));
        CHECK(node["Origin"].as<Salix::Point>().x == -2);
        CHECK(node["Bounds"].as<Salix::Rect>().w == 8);
    }

    TEST_CASE("glm matrix property preserves all sixteen matrix components") {
        YAML::Node node;
        glm::mat4 source(1.0f);
        source[0][1] = 2.0f;
        source[1][2] = 3.0f;
        source[2][3] = 4.0f;
        source[3][0] = 5.0f;
        node["Matrix"] = source;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Matrix", Salix::PropertyType::GlmMat4), &node);
        const auto restored = std::get<glm::mat4>(handle.get_value());
        for (int column = 0; column < 4; ++column)
            for (int row = 0; row < 4; ++row)
                CHECK(restored[column][row] == doctest::Approx(source[column][row]));

        glm::mat4 replacement(2.0f);
        replacement[3][2] = -9.0f;
        handle.set_value(Salix::PropertyValue{replacement});
        const auto saved = node["Matrix"].as<glm::mat4>();
        CHECK(saved[0][0] == doctest::Approx(2.0f));
        CHECK(saved[3][2] == doctest::Approx(-9.0f));
    }

    TEST_CASE("plain enum properties are persisted as their integer representation") {
        YAML::Node node;
        node["Mode"] = 4;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Mode", Salix::PropertyType::Enum), &node);
        CHECK(std::get<int>(handle.get_value()) == 4);
        handle.set_value(Salix::PropertyValue{11});
        CHECK(node["Mode"].as<int>() == 11);
    }

    TEST_CASE("enum class properties translate between YAML names and registered integer values") {
        Salix::EnumRegistry::EnumData enum_data;
        enum_data.value_to_string[3] = "First";
        enum_data.value_to_string[7] = "Second";
        enum_data.ordered_names = {"First", "Second"};
        Salix::EnumRegistry::register_enum(typeid(Batch1047YamlEnumTag), std::move(enum_data));

        Salix::TypeInfo contained;
        contained.name = "Batch1047YamlEnum";
        contained.type_index = typeid(Batch1047YamlEnumTag);
        auto property = batch1047_yaml_property("Mode", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;

        YAML::Node node;
        node["Mode"] = "Second";
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == 7);

        handle.set_value(Salix::PropertyValue{3});
        CHECK(node["Mode"].as<std::string>() == "First");

        handle.set_value(Salix::PropertyValue{std::string("wrong variant")});
        CHECK(node["Mode"].as<std::string>() == "First");
    }
}
