#include <doctest.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/reflection/EnumRegistry.h>
#include <cstdint>
#include <limits>
#include <string>
#include <typeindex>
#include <utility>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <yaml-cpp/yaml.h>

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
        node["Origin"] = Salix::Point{4, -6};
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
        point.set_value(Salix::PropertyValue{Salix::Point{-2, 5}});
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

namespace {
Salix::Property batch1847_yaml_property(const char* name, Salix::PropertyType type) {
    Salix::Property property; property.name = name; property.type = type; return property;
}
struct Batch1847YamlUnregisteredEnum {};
struct Batch1847YamlRegisteredEnum {};
}

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch1847") {
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

namespace { struct Batch2347YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch2347") {
    TEST_CASE("batch2347 integer property creates and then reads exact scalar") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count",Salix::PropertyType::Int),&node);
        handle.set_value(Salix::PropertyValue{-2347}); CHECK(node["Count"].as<int>()==-2347); CHECK(std::get<int>(handle.get_value())==-2347);
    }
    TEST_CASE("batch2347 uint64 property preserves high identifier bits") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Id",Salix::PropertyType::UInt64),&node);
        const std::uint64_t value=(std::uint64_t{1}<<60)+2347; handle.set_value(Salix::PropertyValue{value}); CHECK(node["Id"].as<std::uint64_t>()==value); CHECK(std::get<std::uint64_t>(handle.get_value())==value);
    }
    TEST_CASE("batch2347 float bool and string properties can coexist in one node") {
        YAML::Node node; Salix::PropertyHandleYaml f(batch1047_yaml_property("F",Salix::PropertyType::Float),&node), b(batch1047_yaml_property("B",Salix::PropertyType::Bool),&node), s(batch1047_yaml_property("S",Salix::PropertyType::String),&node);
        f.set_value(Salix::PropertyValue{-3.5f}); b.set_value(Salix::PropertyValue{true}); s.set_value(Salix::PropertyValue{std::string("Batch2347")});
        CHECK(std::get<float>(f.get_value())==doctest::Approx(-3.5f)); CHECK(std::get<bool>(b.get_value())); CHECK(std::get<std::string>(s.get_value())=="Batch2347");
    }
    TEST_CASE("batch2347 Vector2 and Vector3 setters preserve signed components") {
        YAML::Node node; Salix::PropertyHandleYaml v2(batch1047_yaml_property("V2",Salix::PropertyType::Vector2),&node), v3(batch1047_yaml_property("V3",Salix::PropertyType::Vector3),&node);
        const Salix::Vector2 a(-1.25f,8.5f); const Salix::Vector3 b(9.f,-10.f,11.f); v2.set_value(Salix::PropertyValue{a}); v3.set_value(Salix::PropertyValue{b});
        CHECK(std::get<Salix::Vector2>(v2.get_value())==a); CHECK(std::get<Salix::Vector3>(v3.get_value())==b);
    }
    TEST_CASE("batch2347 Color Point and Rect setters create typed YAML values") {
        YAML::Node node; Salix::PropertyHandleYaml c(batch1047_yaml_property("C",Salix::PropertyType::Color),&node), p(batch1047_yaml_property("P",Salix::PropertyType::Point),&node), r(batch1047_yaml_property("R",Salix::PropertyType::Rect),&node);
        const Salix::Color color(0.2f,0.4f,0.6f,0.8f); const Salix::Point point{-7,13}; const Salix::Rect rect(-1,-2,30,40);
        c.set_value(Salix::PropertyValue{color}); p.set_value(Salix::PropertyValue{point}); r.set_value(Salix::PropertyValue{rect});
        CHECK(std::get<Salix::Color>(c.get_value())==color); CHECK(std::get<Salix::Point>(p.get_value())==point); CHECK(std::get<Salix::Rect>(r.get_value())==rect);
    }
    TEST_CASE("batch2347 GlmMat4 property reads back complete written diagonal") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Matrix",Salix::PropertyType::GlmMat4),&node); glm::mat4 matrix(1.f); matrix[0][0]=2.f; matrix[1][1]=3.f; matrix[2][2]=4.f; matrix[3][3]=5.f;
        handle.set_value(Salix::PropertyValue{matrix}); const auto loaded=std::get<glm::mat4>(handle.get_value()); CHECK(loaded[0][0]==doctest::Approx(2.f)); CHECK(loaded[1][1]==doctest::Approx(3.f)); CHECK(loaded[2][2]==doctest::Approx(4.f)); CHECK(loaded[3][3]==doctest::Approx(5.f));
    }
    TEST_CASE("batch2347 plain Enum property stores integer representation") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Mode",Salix::PropertyType::Enum),&node); handle.set_value(Salix::PropertyValue{17}); CHECK(node["Mode"].as<int>()==17); CHECK(std::get<int>(handle.get_value())==17);
    }
    TEST_CASE("batch2347 registered EnumClass maps YAML name to integer and integer back to name") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[4]="Four"; data.value_to_string[9]="Nine"; data.ordered_names={"Four","Nine"}; Salix::EnumRegistry::register_enum(typeid(Batch2347YamlEnumTag),std::move(data));
        Salix::TypeInfo contained; contained.name="Batch2347YamlEnumTag"; contained.type_index=typeid(Batch2347YamlEnumTag); auto property=batch1047_yaml_property("Mode",Salix::PropertyType::EnumClass); property.contained_type_info=&contained;
        YAML::Node node; node["Mode"]="Four"; Salix::PropertyHandleYaml handle(property,&node); CHECK(std::get<int>(handle.get_value())==4); handle.set_value(Salix::PropertyValue{9}); CHECK(node["Mode"].as<std::string>()=="Nine");
    }
    TEST_CASE("batch2347 missing property read leaves unrelated YAML keys untouched") {
        YAML::Node node; node["Keep"]="value"; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Missing2347",Salix::PropertyType::String),&node); CHECK(std::get<int>(handle.get_value())==0); CHECK_FALSE(node["Missing2347"]); CHECK(node["Keep"].as<std::string>()=="value");
    }
    TEST_CASE("batch2347 mismatched variant write preserves existing typed YAML value") {
        YAML::Node node; node["Count"]=88; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count",Salix::PropertyType::Int),&node); handle.set_value(Salix::PropertyValue{std::string("wrong")}); CHECK(node["Count"].as<int>()==88); CHECK(std::get<int>(handle.get_value())==88);
    }
}

namespace { struct Batch2547YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch2547") {
    TEST_CASE("batch2547 null YAML node read returns default property variant") {
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Value",Salix::PropertyType::Int),nullptr);
        const auto value=handle.get_value(); CHECK(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==0);
    }
    TEST_CASE("batch2547 null YAML node write is a harmless no-op") {
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Value",Salix::PropertyType::String),nullptr);
        CHECK_NOTHROW(handle.set_value(Salix::PropertyValue{std::string("ignored")}));
        CHECK(std::get<int>(handle.get_value())==0);
    }
    TEST_CASE("batch2547 missing mapped property read does not create the YAML key") {
        YAML::Node node; node["Keep"]=2547; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Missing",Salix::PropertyType::Float),&node);
        CHECK(std::get<int>(handle.get_value())==0); CHECK_FALSE(node["Missing"]); CHECK(node["Keep"].as<int>()==2547);
    }
    TEST_CASE("batch2547 uint64 setter ignores mismatched signed integer variant") {
        YAML::Node node; node["Id"]=static_cast<std::uint64_t>(99); Salix::PropertyHandleYaml handle(batch1047_yaml_property("Id",Salix::PropertyType::UInt64),&node);
        handle.set_value(Salix::PropertyValue{42}); CHECK(node["Id"].as<std::uint64_t>()==99); CHECK(std::get<std::uint64_t>(handle.get_value())==99);
    }
    TEST_CASE("batch2547 float setter ignores mismatched string variant") {
        YAML::Node node; node["Speed"]=3.25f; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Speed",Salix::PropertyType::Float),&node);
        handle.set_value(Salix::PropertyValue{std::string("fast")}); CHECK(std::get<float>(handle.get_value())==doctest::Approx(3.25f));
    }
    TEST_CASE("batch2547 bool setter ignores mismatched integer variant") {
        YAML::Node node; node["Visible"]=true; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Visible",Salix::PropertyType::Bool),&node);
        handle.set_value(Salix::PropertyValue{0}); CHECK(std::get<bool>(handle.get_value())); CHECK(node["Visible"].as<bool>());
    }
    TEST_CASE("batch2547 string setter preserves embedded null bytes in YAML scalar roundtrip") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Name",Salix::PropertyType::String),&node);
        const std::string value("A\0B2547",7); handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<std::string>(handle.get_value())==value); CHECK(node["Name"].as<std::string>().size()==value.size());
    }
    TEST_CASE("batch2547 vector2 setter and getter preserve negative and fractional components") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Offset",Salix::PropertyType::Vector2),&node);
        const Salix::Vector2 value(-25.47f,0.125f); handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Vector2>(handle.get_value())==value);
    }
    TEST_CASE("batch2547 registered enum class maps exact name and value in both directions") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[-2]="MinusTwo"; data.value_to_string[7]="Seven"; data.ordered_names={"MinusTwo","Seven"};
        Salix::EnumRegistry::register_enum(typeid(Batch2547YamlEnumTag),std::move(data));
        Salix::TypeInfo contained; contained.name="Batch2547YamlEnumTag"; contained.type_index=typeid(Batch2547YamlEnumTag);
        auto property=batch1047_yaml_property("Mode",Salix::PropertyType::EnumClass); property.contained_type_info=&contained;
        YAML::Node node; node["Mode"]="MinusTwo"; Salix::PropertyHandleYaml handle(property,&node);
        CHECK(std::get<int>(handle.get_value())==-2); handle.set_value(Salix::PropertyValue{7}); CHECK(node["Mode"].as<std::string>()=="Seven");
    }
    TEST_CASE("batch2547 enum class without metadata ignores write and preserves existing scalar") {
        YAML::Node node; node["Mode"]="Keep2547"; auto property=batch1047_yaml_property("Mode",Salix::PropertyType::EnumClass);
        Salix::PropertyHandleYaml handle(property,&node); handle.set_value(Salix::PropertyValue{7});
        CHECK(node["Mode"].as<std::string>()=="Keep2547"); CHECK(std::get<int>(handle.get_value())==0);
    }
}

namespace { struct Batch2947YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch2947") {
    TEST_CASE("batch2947 signed integer property supports negative roundtrip") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count2947",Salix::PropertyType::Int),&node);
        handle.set_value(Salix::PropertyValue{-2947}); CHECK(node["Count2947"].as<int>()==-2947); CHECK(std::get<int>(handle.get_value())==-2947);
    }
    TEST_CASE("batch2947 uint64 property preserves value above signed range") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Id2947",Salix::PropertyType::UInt64),&node);
        const std::uint64_t value=(std::uint64_t{1}<<63)+2947ULL; handle.set_value(Salix::PropertyValue{value});
        CHECK(node["Id2947"].as<std::uint64_t>()==value); CHECK(std::get<std::uint64_t>(handle.get_value())==value);
    }
    TEST_CASE("batch2947 Vector3 property preserves fractional signed coordinates") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Position2947",Salix::PropertyType::Vector3),&node);
        const Salix::Vector3 value(-2.947f,0.125f,99.5f); handle.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Vector3>(handle.get_value())==value);
    }
    TEST_CASE("batch2947 plain Enum property preserves negative integer representation") {
        YAML::Node node; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Enum2947",Salix::PropertyType::Enum),&node);
        handle.set_value(Salix::PropertyValue{-17}); CHECK(node["Enum2947"].as<int>()==-17); CHECK(std::get<int>(handle.get_value())==-17);
    }
    TEST_CASE("batch2947 registered EnumClass unknown YAML name resolves to registry missing sentinel") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[1]="One2947"; data.value_to_string[2]="Two2947"; data.ordered_names={"One2947","Two2947"};
        Salix::EnumRegistry::register_enum(typeid(Batch2947YamlEnumTag),std::move(data)); Salix::TypeInfo contained; contained.name="Batch2947YamlEnumTag"; contained.type_index=typeid(Batch2947YamlEnumTag);
        auto property=batch1047_yaml_property("Mode2947",Salix::PropertyType::EnumClass); property.contained_type_info=&contained; YAML::Node node; node["Mode2947"]="Missing2947";
        Salix::PropertyHandleYaml handle(property,&node); CHECK(std::get<int>(handle.get_value())==-1);
    }
    TEST_CASE("batch2947 registered EnumClass mismatched variant write preserves existing YAML name") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[3]="Three2947"; data.ordered_names={"Three2947"};
        Salix::EnumRegistry::register_enum(typeid(Batch2947YamlEnumTag),std::move(data)); Salix::TypeInfo contained; contained.name="Batch2947YamlEnumTag"; contained.type_index=typeid(Batch2947YamlEnumTag);
        auto property=batch1047_yaml_property("Mode2947B",Salix::PropertyType::EnumClass); property.contained_type_info=&contained; YAML::Node node; node["Mode2947B"]="Three2947";
        Salix::PropertyHandleYaml handle(property,&node); handle.set_value(Salix::PropertyValue{std::string("wrong")}); CHECK(node["Mode2947B"].as<std::string>()=="Three2947");
    }
    TEST_CASE("batch2947 integer setter ignores mismatched float without replacing YAML scalar") {
        YAML::Node node; node["Count2947C"]=81; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count2947C",Salix::PropertyType::Int),&node);
        handle.set_value(Salix::PropertyValue{8.1f}); CHECK(node["Count2947C"].as<int>()==81); CHECK(std::get<int>(handle.get_value())==81);
    }
    TEST_CASE("batch2947 Vector3 setter ignores mismatched Vector2 and preserves existing value") {
        YAML::Node node; node["V2947"]=Salix::Vector3(1.f,2.f,3.f); Salix::PropertyHandleYaml handle(batch1047_yaml_property("V2947",Salix::PropertyType::Vector3),&node);
        handle.set_value(Salix::PropertyValue{Salix::Vector2(9.f,9.f)}); CHECK(std::get<Salix::Vector3>(handle.get_value())==Salix::Vector3(1.f,2.f,3.f));
    }
    TEST_CASE("batch2947 missing property remains absent after read and unrelated key survives") {
        YAML::Node node; node["Keep2947"]="untouched"; Salix::PropertyHandleYaml handle(batch1047_yaml_property("Absent2947",Salix::PropertyType::Bool),&node);
        const auto value=handle.get_value(); CHECK(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==0); CHECK_FALSE(node["Absent2947"]); CHECK(node["Keep2947"].as<std::string>()=="untouched");
    }
    TEST_CASE("batch2947 null node rejects repeated reads and writes without side effects") {
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Null2947",Salix::PropertyType::String),nullptr);
        CHECK(std::get<int>(handle.get_value())==0); CHECK_NOTHROW(handle.set_value(Salix::PropertyValue{std::string("ignored")})); CHECK(std::get<int>(handle.get_value())==0);
    }
}

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch3347") {
    TEST_CASE("batch3347 integer YAML handle exposes latest repeated write") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Count3347",Salix::PropertyType::Int),&node); h.set_value(Salix::PropertyValue{1}); h.set_value(Salix::PropertyValue{-3347});
        CHECK(node["Count3347"].as<int>()==-3347); CHECK(std::get<int>(h.get_value())==-3347);
    }
    TEST_CASE("batch3347 uint64 YAML handle preserves maximum value") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Id3347",Salix::PropertyType::UInt64),&node); const std::uint64_t value=(std::numeric_limits<std::uint64_t>::max)(); h.set_value(Salix::PropertyValue{value});
        CHECK(node["Id3347"].as<std::uint64_t>()==value); CHECK(std::get<std::uint64_t>(h.get_value())==value);
    }
    TEST_CASE("batch3347 float YAML handle preserves negative fractional value") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Float3347",Salix::PropertyType::Float),&node); h.set_value(Salix::PropertyValue{-33.47f});
        CHECK(node["Float3347"].as<float>()==doctest::Approx(-33.47f)); CHECK(std::get<float>(h.get_value())==doctest::Approx(-33.47f));
    }
    TEST_CASE("batch3347 boolean YAML handle toggles true false true") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Bool3347",Salix::PropertyType::Bool),&node); h.set_value(Salix::PropertyValue{true}); h.set_value(Salix::PropertyValue{false}); CHECK_FALSE(std::get<bool>(h.get_value())); h.set_value(Salix::PropertyValue{true}); CHECK(std::get<bool>(h.get_value()));
    }
    TEST_CASE("batch3347 string YAML handle preserves multiline whitespace") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Text3347",Salix::PropertyType::String),&node); const std::string value="  line one\nline two\t3347  "; h.set_value(Salix::PropertyValue{value});
        CHECK(node["Text3347"].as<std::string>()==value); CHECK(std::get<std::string>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Vector2 YAML handle preserves signed coordinates") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("V2_3347",Salix::PropertyType::Vector2),&node); const Salix::Vector2 value(-3.347f,47.25f); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Vector2>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Color YAML handle preserves four channels") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Color3347",Salix::PropertyType::Color),&node); const Salix::Color value(.1f,.2f,.3f,.4f); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Color>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Point YAML handle preserves negative coordinates") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Point3347",Salix::PropertyType::Point),&node); const Salix::Point value{-33,47}; h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Point>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Rect YAML handle preserves four integer channels") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Rect3347",Salix::PropertyType::Rect),&node); const Salix::Rect value(-3,4,33,47); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Rect>(h.get_value())==value);
    }
    TEST_CASE("batch3347 matrix YAML handle preserves independently modified diagonal") {
        YAML::Node node; Salix::PropertyHandleYaml h(batch1047_yaml_property("Matrix3347",Salix::PropertyType::GlmMat4),&node); glm::mat4 value(1.f); value[0][0]=3.347f; value[1][1]=4.347f; value[2][2]=5.347f; value[3][3]=6.347f; h.set_value(Salix::PropertyValue{value});
        const auto roundtrip=std::get<glm::mat4>(h.get_value()); CHECK(roundtrip[0][0]==doctest::Approx(3.347f)); CHECK(roundtrip[1][1]==doctest::Approx(4.347f)); CHECK(roundtrip[2][2]==doctest::Approx(5.347f)); CHECK(roundtrip[3][3]==doctest::Approx(6.347f));
    }
}
namespace { struct Batch3747YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch3747") {
    TEST_CASE("batch3747 contained type metadata pointer is preserved exactly") {
        YAML::Node node;
        Salix::TypeInfo contained;
        contained.name = "Contained3747";
        contained.type_index = typeid(Batch3747YamlEnumTag);
        auto property = batch1047_yaml_property("Mode3747", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(handle.get_contained_type_info() == &contained);
        CHECK(handle.get_type() == Salix::PropertyType::EnumClass);
    }

    TEST_CASE("batch3747 boolean false write creates an explicit false scalar") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Flag3747", Salix::PropertyType::Bool), &node);
        REQUIRE_FALSE(node["Flag3747"]);
        handle.set_value(Salix::PropertyValue{false});
        REQUIRE(node["Flag3747"]);
        CHECK_FALSE(node["Flag3747"].as<bool>());
        CHECK_FALSE(std::get<bool>(handle.get_value()));
    }

    TEST_CASE("batch3747 external YAML mutation is observed by an existing integer handle") {
        YAML::Node node;
        node["Count3747"] = 1;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count3747", Salix::PropertyType::Int), &node);
        CHECK(std::get<int>(handle.get_value()) == 1);
        node["Count3747"] = -3747;
        CHECK(std::get<int>(handle.get_value()) == -3747);
    }

    TEST_CASE("batch3747 two handles sharing one node update independent keys") {
        YAML::Node node;
        Salix::PropertyHandleYaml a(batch1047_yaml_property("A3747", Salix::PropertyType::Int), &node);
        Salix::PropertyHandleYaml b(batch1047_yaml_property("B3747", Salix::PropertyType::String), &node);
        a.set_value(Salix::PropertyValue{3747});
        b.set_value(Salix::PropertyValue{std::string("value3747")});
        CHECK(node["A3747"].as<int>() == 3747);
        CHECK(node["B3747"].as<std::string>() == "value3747");
        CHECK(std::get<int>(a.get_value()) == 3747);
        CHECK(std::get<std::string>(b.get_value()) == "value3747");
    }

    TEST_CASE("batch3747 Point setter and getter preserve signed integer limits") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Point3747", Salix::PropertyType::Point), &node);
        const Salix::Point value{(std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()};
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Point>(handle.get_value()) == value);
    }

    TEST_CASE("batch3747 Rect setter and getter preserve signed boundary channels") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Rect3747", Salix::PropertyType::Rect), &node);
        const Salix::Rect value((std::numeric_limits<int>::min)(), -1, 0, (std::numeric_limits<int>::max)());
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Rect>(handle.get_value()) == value);
    }

    TEST_CASE("batch3747 matrix handle preserves signed off diagonal values") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Matrix3747", Salix::PropertyType::GlmMat4), &node);
        glm::mat4 value(1.0f);
        value[0][3] = -37.47f;
        value[1][2] = 3.747f;
        value[2][1] = -0.3747f;
        value[3][0] = 3747.0f;
        handle.set_value(Salix::PropertyValue{value});
        const auto restored = std::get<glm::mat4>(handle.get_value());
        CHECK(restored[0][3] == doctest::Approx(-37.47f));
        CHECK(restored[1][2] == doctest::Approx(3.747f));
        CHECK(restored[2][1] == doctest::Approx(-0.3747f));
        CHECK(restored[3][0] == doctest::Approx(3747.0f));
    }

    TEST_CASE("batch3747 registered EnumClass maps negative and positive values exactly") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[-7] = "Negative3747";
        data.value_to_string[9] = "Positive3747";
        data.ordered_names = {"Negative3747", "Positive3747"};
        Salix::EnumRegistry::register_enum(typeid(Batch3747YamlEnumTag), std::move(data));
        Salix::TypeInfo contained;
        contained.name = "Batch3747YamlEnumTag";
        contained.type_index = typeid(Batch3747YamlEnumTag);
        auto property = batch1047_yaml_property("Mode3747", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        YAML::Node node;
        node["Mode3747"] = "Negative3747";
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == -7);
        handle.set_value(Salix::PropertyValue{9});
        CHECK(node["Mode3747"].as<std::string>() == "Positive3747");
        CHECK(std::get<int>(handle.get_value()) == 9);
    }

    TEST_CASE("batch3747 unsupported Class type read and write leave YAML untouched") {
        YAML::Node node;
        node["Keep3747"] = "stable";
        auto property = batch1047_yaml_property("Class3747", Salix::PropertyType::Class);
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == 0);
        handle.set_value(Salix::PropertyValue{3747});
        CHECK_FALSE(node["Class3747"]);
        CHECK(node["Keep3747"].as<std::string>() == "stable");
    }

    TEST_CASE("batch3747 mismatched Color write preserves existing YAML color") {
        YAML::Node node;
        const Salix::Color original(0.1f, 0.2f, 0.3f, 0.4f);
        node["Tint3747"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Tint3747", Salix::PropertyType::Color), &node);
        handle.set_value(Salix::PropertyValue{std::string("wrong-type")});
        CHECK(std::get<Salix::Color>(handle.get_value()) == original);
    }
}

namespace { struct Batch4147YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch4147") {
    TEST_CASE("batch4147 contained type metadata pointer is preserved exactly") {
        YAML::Node node;
        Salix::TypeInfo contained;
        contained.name = "Contained4147";
        contained.type_index = typeid(Batch4147YamlEnumTag);
        auto property = batch1047_yaml_property("Mode4147", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(handle.get_contained_type_info() == &contained);
    }

    TEST_CASE("batch4147 existing integer handle observes external YAML replacement") {
        YAML::Node node;
        node["Count4147"] = 1;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Count4147", Salix::PropertyType::Int), &node);
        node["Count4147"] = -4147;
        CHECK(std::get<int>(handle.get_value()) == -4147);
    }

    TEST_CASE("batch4147 false boolean write creates explicit YAML scalar") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Flag4147", Salix::PropertyType::Bool), &node);
        REQUIRE_FALSE(node["Flag4147"]);
        handle.set_value(Salix::PropertyValue{false});
        REQUIRE(node["Flag4147"]);
        CHECK_FALSE(node["Flag4147"].as<bool>());
    }

    TEST_CASE("batch4147 uint64 handle preserves maximum unsigned value") {
        YAML::Node node;
        const std::uint64_t value = (std::numeric_limits<std::uint64_t>::max)();
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Id4147", Salix::PropertyType::UInt64), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<std::uint64_t>(handle.get_value()) == value);
    }

    TEST_CASE("batch4147 string handle preserves multiline and surrounding whitespace") {
        YAML::Node node;
        const std::string value = "  first\nsecond\t4147  ";
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Text4147", Salix::PropertyType::String), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(node["Text4147"].as<std::string>() == value);
        CHECK(std::get<std::string>(handle.get_value()) == value);
    }

    TEST_CASE("batch4147 Vector3 handle ignores mismatched Vector2 write") {
        YAML::Node node;
        const Salix::Vector3 original(1.0f, 2.0f, 3.0f);
        node["Vector4147"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Vector4147", Salix::PropertyType::Vector3), &node);
        handle.set_value(Salix::PropertyValue{Salix::Vector2(9.0f, 9.0f)});
        CHECK(std::get<Salix::Vector3>(handle.get_value()) == original);
    }

    TEST_CASE("batch4147 Point handle preserves signed integer limits") {
        YAML::Node node;
        const Salix::Point value{(std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()};
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Point4147", Salix::PropertyType::Point), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Point>(handle.get_value()) == value);
    }

    TEST_CASE("batch4147 matrix handle observes external YAML matrix replacement") {
        YAML::Node node;
        glm::mat4 first(1.0f);
        node["Matrix4147"] = first;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Matrix4147", Salix::PropertyType::GlmMat4), &node);
        glm::mat4 second(1.0f);
        second[0][3] = -41.47f;
        second[3][0] = 4147.0f;
        node["Matrix4147"] = second;
        const auto restored = std::get<glm::mat4>(handle.get_value());
        CHECK(restored[0][3] == doctest::Approx(-41.47f));
        CHECK(restored[3][0] == doctest::Approx(4147.0f));
    }

    TEST_CASE("batch4147 unregistered EnumClass metadata returns default without changing YAML") {
        YAML::Node node;
        node["Mode4147Missing"] = "Unregistered";
        Salix::TypeInfo contained;
        contained.name = "Batch4147YamlEnumTag";
        contained.type_index = typeid(Batch4147YamlEnumTag);
        auto property = batch1047_yaml_property("Mode4147Missing", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        Salix::PropertyHandleYaml handle(property, &node);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
        CHECK(node["Mode4147Missing"].as<std::string>() == "Unregistered");
    }

    TEST_CASE("batch4147 unsupported Struct read and write preserve unrelated YAML") {
        YAML::Node node;
        node["Keep4147"] = "stable";
        auto property = batch1047_yaml_property("Struct4147", Salix::PropertyType::Struct);
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == 0);
        handle.set_value(Salix::PropertyValue{4147});
        CHECK_FALSE(node["Struct4147"]);
        CHECK(node["Keep4147"].as<std::string>() == "stable");
    }
}

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch4547") {
    TEST_CASE("batch4547 integer handle preserves signed minimum value") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Int4547", Salix::PropertyType::Int), &node);
        handle.set_value(Salix::PropertyValue{(std::numeric_limits<int>::min)()});
        CHECK(node["Int4547"].as<int>() == (std::numeric_limits<int>::min)());
        CHECK(std::get<int>(handle.get_value()) == (std::numeric_limits<int>::min)());
    }

    TEST_CASE("batch4547 float handle preserves ordinary negative fractional value") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Float4547", Salix::PropertyType::Float), &node);
        handle.set_value(Salix::PropertyValue{-45.47f});
        CHECK(node["Float4547"].as<float>() == doctest::Approx(-45.47f));
        CHECK(std::get<float>(handle.get_value()) == doctest::Approx(-45.47f));
    }

    TEST_CASE("batch4547 Vector2 handle preserves signed component values") {
        YAML::Node node;
        const Salix::Vector2 value(-4547.0f, 0.4547f);
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Vector24547", Salix::PropertyType::Vector2), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Vector2>(handle.get_value()) == value);
    }

    TEST_CASE("batch4547 Color handle roundtrips all four finite channels") {
        YAML::Node node;
        const Salix::Color value(0.45f, 0.47f, -0.25f, 1.25f);
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Color4547", Salix::PropertyType::Color), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Color>(handle.get_value()) == value);
    }

    TEST_CASE("batch4547 Rect handle ignores mismatched string write") {
        YAML::Node node;
        const Salix::Rect original(-4, 5, 47, 54);
        node["Rect4547"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Rect4547", Salix::PropertyType::Rect), &node);
        handle.set_value(Salix::PropertyValue{std::string("wrong4547")});
        CHECK(std::get<Salix::Rect>(handle.get_value()) == original);
    }

    TEST_CASE("batch4547 Enum handle stores signed integer directly") {
        YAML::Node node;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Enum4547", Salix::PropertyType::Enum), &node);
        handle.set_value(Salix::PropertyValue{-4547});
        CHECK(node["Enum4547"].as<int>() == -4547);
        CHECK(std::get<int>(handle.get_value()) == -4547);
    }

    TEST_CASE("batch4547 existing boolean handle observes external YAML mutation") {
        YAML::Node node;
        node["Flag4547"] = false;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Flag4547", Salix::PropertyType::Bool), &node);
        REQUIRE_FALSE(std::get<bool>(handle.get_value()));
        node["Flag4547"] = true;
        CHECK(std::get<bool>(handle.get_value()));
    }

    TEST_CASE("batch4547 matrix handle ignores mismatched integer write and preserves matrix") {
        YAML::Node node;
        glm::mat4 original(1.0f);
        original[1][3] = 45.47f;
        node["Matrix4547"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Matrix4547", Salix::PropertyType::GlmMat4), &node);
        handle.set_value(Salix::PropertyValue{4547});
        const auto restored = std::get<glm::mat4>(handle.get_value());
        CHECK(restored[1][3] == doctest::Approx(45.47f));
    }

    TEST_CASE("batch4547 ordinary property exposes null contained type metadata") {
        YAML::Node node;
        auto property = batch1047_yaml_property("Plain4547", Salix::PropertyType::String);
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(handle.get_contained_type_info() == nullptr);
    }

    TEST_CASE("batch4547 null YAML node safely ignores a string write and returns default variant") {
        auto property = batch1047_yaml_property("Null4547", Salix::PropertyType::String);
        Salix::PropertyHandleYaml handle(property, nullptr);
        CHECK_NOTHROW(handle.set_value(Salix::PropertyValue{std::string("value4547")}));
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
    }
}

namespace { struct Batch4747YamlEnumTag {}; }

TEST_SUITE("Salix::reflection::PropertyHandleYamlBatch4747") {
    TEST_CASE("batch4747 integer write creates only target key and preserves sibling node") {
        YAML::Node node;
        node["Sibling4747"] = "stable";
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Int4747", Salix::PropertyType::Int), &node);
        handle.set_value(Salix::PropertyValue{-4747});
        CHECK(node["Int4747"].as<int>() == -4747);
        CHECK(node["Sibling4747"].as<std::string>() == "stable");
    }

    TEST_CASE("batch4747 UInt64 handle ignores mismatched signed integer write") {
        YAML::Node node;
        const std::uint64_t original = 4747474747ULL;
        node["Id4747"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Id4747", Salix::PropertyType::UInt64), &node);
        handle.set_value(Salix::PropertyValue{4747});
        CHECK(node["Id4747"].as<std::uint64_t>() == original);
        CHECK(std::get<std::uint64_t>(handle.get_value()) == original);
    }

    TEST_CASE("batch4747 string handle observes external whitespace rich replacement") {
        YAML::Node node;
        node["Text4747"] = "initial";
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Text4747", Salix::PropertyType::String), &node);
        const std::string replacement = "  line one\nline two\t4747  ";
        node["Text4747"] = replacement;
        CHECK(std::get<std::string>(handle.get_value()) == replacement);
    }

    TEST_CASE("batch4747 Vector2 handle ignores mismatched Vector3 write") {
        YAML::Node node;
        const Salix::Vector2 original(-47.0f, 74.0f);
        node["Vector24747"] = original;
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Vector24747", Salix::PropertyType::Vector2), &node);
        handle.set_value(Salix::PropertyValue{Salix::Vector3(1.0f, 2.0f, 3.0f)});
        CHECK(std::get<Salix::Vector2>(handle.get_value()) == original);
    }

    TEST_CASE("batch4747 Point handle roundtrips signed coordinates") {
        YAML::Node node;
        const Salix::Point value{-4747, 7474};
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Point4747", Salix::PropertyType::Point), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Point>(handle.get_value()) == value);
    }

    TEST_CASE("batch4747 Rect handle roundtrips negative position and extents") {
        YAML::Node node;
        const Salix::Rect value{-47, -74, -4747, -7474};
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Rect4747", Salix::PropertyType::Rect), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Rect>(handle.get_value()) == value);
    }

    TEST_CASE("batch4747 plain Enum handle preserves signed maximum integer value") {
        YAML::Node node;
        const int value = (std::numeric_limits<int>::max)();
        Salix::PropertyHandleYaml handle(batch1047_yaml_property("Enum4747", Salix::PropertyType::Enum), &node);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(node["Enum4747"].as<int>() == value);
        CHECK(std::get<int>(handle.get_value()) == value);
    }

    TEST_CASE("batch4747 registered EnumClass unknown YAML name maps to minus one") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One4747";
        data.value_to_string[2] = "Two4747";
        data.ordered_names = {"One4747", "Two4747"};
        Salix::EnumRegistry::register_enum(typeid(Batch4747YamlEnumTag), std::move(data));
        Salix::TypeInfo contained;
        contained.name = "Batch4747YamlEnumTag";
        contained.type_index = typeid(Batch4747YamlEnumTag);
        auto property = batch1047_yaml_property("Mode4747", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        YAML::Node node;
        node["Mode4747"] = "Missing4747";
        Salix::PropertyHandleYaml handle(property, &node);
        CHECK(std::get<int>(handle.get_value()) == -1);
    }

    TEST_CASE("batch4747 registered EnumClass unknown numeric write stores Unknown name") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One4747";
        data.value_to_string[2] = "Two4747";
        data.ordered_names = {"One4747", "Two4747"};
        Salix::EnumRegistry::register_enum(typeid(Batch4747YamlEnumTag), std::move(data));
        Salix::TypeInfo contained;
        contained.name = "Batch4747YamlEnumTag";
        contained.type_index = typeid(Batch4747YamlEnumTag);
        auto property = batch1047_yaml_property("ModeWrite4747", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        YAML::Node node;
        Salix::PropertyHandleYaml handle(property, &node);
        handle.set_value(Salix::PropertyValue{999});
        CHECK(node["ModeWrite4747"].as<std::string>() == "Unknown");
        CHECK(std::get<int>(handle.get_value()) == -1);
    }

    TEST_CASE("batch4747 EnumClass metadata without type index leaves existing YAML unchanged") {
        Salix::TypeInfo contained;
        contained.name = "NoTypeIndex4747";
        auto property = batch1047_yaml_property("ModeNoIndex4747", Salix::PropertyType::EnumClass);
        property.contained_type_info = &contained;
        YAML::Node node;
        node["ModeNoIndex4747"] = "Stable4747";
        Salix::PropertyHandleYaml handle(property, &node);
        const auto read = handle.get_value();
        CHECK(std::holds_alternative<int>(read));
        CHECK(std::get<int>(read) == 0);
        handle.set_value(Salix::PropertyValue{47});
        CHECK(node["ModeNoIndex4747"].as<std::string>() == "Stable4747");
    }
}
