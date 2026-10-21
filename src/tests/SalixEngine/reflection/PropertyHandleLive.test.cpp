#include <doctest.h>
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>

namespace {
class Batch1147LiveElement : public Salix::Element {
public:
    int integer = 4;
    std::uint64_t identifier = 9;
    float scalar = 1.5f;
    bool flag = false;
    std::string text = "initial";
    Salix::Vector2 vector2{1.0f, 2.0f};
    Salix::Vector3 vector3{3.0f, 4.0f, 5.0f};
    Salix::Point point{6, 7};
    Salix::Color color{0.1f, 0.2f, 0.3f, 0.4f};
    Salix::Rect rect{8, 9, 10, 11};
    int enum_value = 2;
    int enum_class_value = 3;
    glm::mat4 matrix{1.0f};

    const char* get_class_name() const override { return "Batch1147LiveElement"; }
};

Salix::Property batch1147_property(
    const std::string& name,
    Salix::PropertyType type,
    Salix::getter_func getter,
    Salix::setter_func setter) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    property.get_data = std::move(getter);
    property.set_data = std::move(setter);
    return property;
}
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch1147") {
    TEST_CASE("handle exposes reflection metadata supplied by Property") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "integer", Salix::PropertyType::Int,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->integer; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->integer = *static_cast<int*>(value); });
        property.hint = Salix::UIHint::MultilineText;
        property.is_read_only = true;
        property.display_order = 27;
        Salix::PropertyHandleLive handle(property, &element);
        CHECK(handle.get_name() == "integer");
        CHECK(handle.get_type() == Salix::PropertyType::Int);
        CHECK(handle.get_hint() == Salix::UIHint::MultilineText);
        CHECK(handle.is_read_only());
        CHECK(handle.get_display_order() == 27);
    }

    TEST_CASE("null instance returns default variant and ignores writes") {
        auto property = batch1147_property(
            "integer", Salix::PropertyType::Int,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->integer; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->integer = *static_cast<int*>(value); });
        Salix::PropertyHandleLive handle(property, nullptr);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
        CHECK_NOTHROW(handle.set_value(Salix::PropertyValue{123}));
    }

    TEST_CASE("missing getter produces default value while missing setter preserves live state") {
        Batch1147LiveElement element;
        Salix::Property no_getter;
        no_getter.name = "integer";
        no_getter.type = Salix::PropertyType::Int;
        no_getter.set_data = [](void* instance, void* value) {
            static_cast<Batch1147LiveElement*>(instance)->integer = *static_cast<int*>(value);
        };
        Salix::PropertyHandleLive read_handle(no_getter, &element);
        CHECK(std::get<int>(read_handle.get_value()) == 0);

        Salix::Property no_setter;
        no_setter.name = "integer";
        no_setter.type = Salix::PropertyType::Int;
        no_setter.get_data = [](void* instance) {
            return &static_cast<Batch1147LiveElement*>(instance)->integer;
        };
        Salix::PropertyHandleLive write_handle(no_setter, &element);
        write_handle.set_value(Salix::PropertyValue{99});
        CHECK(element.integer == 4);
    }

    TEST_CASE("integer property reads writes and rejects mismatched variant type") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "integer", Salix::PropertyType::Int,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->integer; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->integer = *static_cast<int*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        CHECK(std::get<int>(handle.get_value()) == 4);
        handle.set_value(Salix::PropertyValue{-17});
        CHECK(element.integer == -17);
        handle.set_value(Salix::PropertyValue{std::string("wrong")});
        CHECK(element.integer == -17);
    }

    TEST_CASE("uint64 and float properties preserve their exact numeric domains") {
        Batch1147LiveElement element;
        auto id_property = batch1147_property(
            "identifier", Salix::PropertyType::UInt64,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->identifier; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->identifier = *static_cast<std::uint64_t*>(value); });
        auto scalar_property = batch1147_property(
            "scalar", Salix::PropertyType::Float,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->scalar; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->scalar = *static_cast<float*>(value); });
        Salix::PropertyHandleLive id_handle(id_property, &element);
        Salix::PropertyHandleLive scalar_handle(scalar_property, &element);

        const std::uint64_t large = (std::uint64_t{1} << 54) + 3;
        id_handle.set_value(Salix::PropertyValue{large});
        scalar_handle.set_value(Salix::PropertyValue{-2.75f});
        CHECK(std::get<std::uint64_t>(id_handle.get_value()) == large);
        CHECK(std::get<float>(scalar_handle.get_value()) == doctest::Approx(-2.75f));
    }

    TEST_CASE("bool and string properties roundtrip independently through the type-erased bridge") {
        Batch1147LiveElement element;
        auto bool_property = batch1147_property(
            "flag", Salix::PropertyType::Bool,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->flag; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->flag = *static_cast<bool*>(value); });
        auto string_property = batch1147_property(
            "text", Salix::PropertyType::String,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->text; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->text = *static_cast<std::string*>(value); });
        Salix::PropertyHandleLive bool_handle(bool_property, &element);
        Salix::PropertyHandleLive string_handle(string_property, &element);

        bool_handle.set_value(Salix::PropertyValue{true});
        string_handle.set_value(Salix::PropertyValue{std::string("updated")});
        CHECK(std::get<bool>(bool_handle.get_value()));
        CHECK(std::get<std::string>(string_handle.get_value()) == "updated");
    }

    TEST_CASE("Vector2 and Vector3 properties preserve each component on read and write") {
        Batch1147LiveElement element;
        auto p2 = batch1147_property(
            "vector2", Salix::PropertyType::Vector2,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->vector2; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->vector2 = *static_cast<Salix::Vector2*>(value); });
        auto p3 = batch1147_property(
            "vector3", Salix::PropertyType::Vector3,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->vector3; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->vector3 = *static_cast<Salix::Vector3*>(value); });
        Salix::PropertyHandleLive h2(p2, &element);
        Salix::PropertyHandleLive h3(p3, &element);

        h2.set_value(Salix::PropertyValue{Salix::Vector2(-1.0f, 8.5f)});
        h3.set_value(Salix::PropertyValue{Salix::Vector3(9.0f, -4.0f, 2.5f)});
        CHECK(std::get<Salix::Vector2>(h2.get_value()) == Salix::Vector2(-1.0f, 8.5f));
        CHECK(std::get<Salix::Vector3>(h3.get_value()) == Salix::Vector3(9.0f, -4.0f, 2.5f));
    }

    TEST_CASE("Point Color and Rect properties preserve structured values") {
        Batch1147LiveElement element;
        auto point_property = batch1147_property(
            "point", Salix::PropertyType::Point,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->point; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->point = *static_cast<Salix::Point*>(value); });
        auto color_property = batch1147_property(
            "color", Salix::PropertyType::Color,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->color; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->color = *static_cast<Salix::Color*>(value); });
        auto rect_property = batch1147_property(
            "rect", Salix::PropertyType::Rect,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->rect; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->rect = *static_cast<Salix::Rect*>(value); });

        Salix::PropertyHandleLive point_handle(point_property, &element);
        Salix::PropertyHandleLive color_handle(color_property, &element);
        Salix::PropertyHandleLive rect_handle(rect_property, &element);
        point_handle.set_value(Salix::PropertyValue{Salix::Point{-3, 12}});
        color_handle.set_value(Salix::PropertyValue{Salix::Color(1.0f, 0.5f, 0.25f, 0.75f)});
        rect_handle.set_value(Salix::PropertyValue{Salix::Rect(-4, 5, 60, 70)});

        CHECK(std::get<Salix::Point>(point_handle.get_value()) == Salix::Point{-3, 12});
        CHECK(std::get<Salix::Color>(color_handle.get_value()) == Salix::Color(1.0f, 0.5f, 0.25f, 0.75f));
        CHECK(std::get<Salix::Rect>(rect_handle.get_value()) == Salix::Rect(-4, 5, 60, 70));
    }

    TEST_CASE("enum and enum class properties use integer variant representation") {
        Batch1147LiveElement element;
        auto plain = batch1147_property(
            "enum_value", Salix::PropertyType::Enum,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->enum_value; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->enum_value = *static_cast<int*>(value); });
        auto scoped = batch1147_property(
            "enum_class_value", Salix::PropertyType::EnumClass,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->enum_class_value; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->enum_class_value = *static_cast<int*>(value); });
        Salix::PropertyHandleLive plain_handle(plain, &element);
        Salix::PropertyHandleLive scoped_handle(scoped, &element);
        plain_handle.set_value(Salix::PropertyValue{7});
        scoped_handle.set_value(Salix::PropertyValue{11});
        CHECK(std::get<int>(plain_handle.get_value()) == 7);
        CHECK(std::get<int>(scoped_handle.get_value()) == 11);
    }

    TEST_CASE("glm matrix can be read but write path intentionally remains read only") {
        Batch1147LiveElement element;
        element.matrix[2][1] = 6.5f;
        auto property = batch1147_property(
            "matrix", Salix::PropertyType::GlmMat4,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->matrix; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->matrix = *static_cast<glm::mat4*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        const auto initial = std::get<glm::mat4>(handle.get_value());
        CHECK(initial[2][1] == doctest::Approx(6.5f));

        glm::mat4 replacement(2.0f);
        replacement[2][1] = -9.0f;
        handle.set_value(Salix::PropertyValue{replacement});
        CHECK(element.matrix[0][0] == doctest::Approx(1.0f));
        CHECK(element.matrix[2][1] == doctest::Approx(6.5f));
    }
}
