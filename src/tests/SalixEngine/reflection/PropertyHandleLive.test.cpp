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
#include <limits>
#include <string>
#include <utility>

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

        CHECK((std::get<Salix::Point>(point_handle.get_value()) == Salix::Point{-3, 12}));
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

namespace {
class Batch1847LiveElement : public Salix::Element {
public:
    std::uint64_t u64 = 91;
    float scalar = 2.5f;
    bool flag = true;
    std::string text = "keep";
    Salix::Vector2 v2{1.0f, 2.0f};
    Salix::Vector3 v3{3.0f, 4.0f, 5.0f};
    Salix::Point point{6, 7};
    Salix::Color color{0.1f, 0.2f, 0.3f, 0.4f};
    Salix::Rect rect{8, 9, 10, 11};
    const char* get_class_name() const override { return "Batch1847LiveElement"; }
};

Salix::Property batch1847_live_property(const char* name, Salix::PropertyType type,
    Salix::getter_func getter, Salix::setter_func setter) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    property.get_data = std::move(getter);
    property.set_data = std::move(setter);
    return property;
}
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch1847") {
    TEST_CASE("class and struct property types use the neutral fallback value and ignore writes") {
        Batch1847LiveElement element;
        int setter_calls = 0;
        for (auto type : {Salix::PropertyType::Class, Salix::PropertyType::Struct}) {
            auto property = batch1847_live_property("opaque", type,
                [](void*) -> void* { return nullptr; },
                [&](void*, void*) { ++setter_calls; });
            Salix::PropertyHandleLive handle(property, &element);
            auto value = handle.get_value();
            CHECK(std::holds_alternative<int>(value));
            CHECK(std::get<int>(value) == 0);
            handle.set_value(Salix::PropertyValue{123});
        }
        CHECK(setter_calls == 0);
    }

    TEST_CASE("uint64 property rejects an integer variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("u64", Salix::PropertyType::UInt64,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->u64; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{42});
        CHECK(calls == 0); CHECK(element.u64 == 91);
    }

    TEST_CASE("float property rejects an integer variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("scalar", Salix::PropertyType::Float,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->scalar; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{42});
        CHECK(calls == 0); CHECK(element.scalar == doctest::Approx(2.5f));
    }

    TEST_CASE("bool property rejects an integer variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("flag", Salix::PropertyType::Bool,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->flag; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{1});
        CHECK(calls == 0); CHECK(element.flag);
    }

    TEST_CASE("string property rejects an integer variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("text", Salix::PropertyType::String,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->text; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{17});
        CHECK(calls == 0); CHECK(element.text == "keep");
    }

    TEST_CASE("Vector2 property rejects Vector3 variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("v2", Salix::PropertyType::Vector2,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->v2; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{Salix::Vector3(9.0f, 8.0f, 7.0f)});
        CHECK(calls == 0); CHECK(element.v2 == Salix::Vector2(1.0f, 2.0f));
    }

    TEST_CASE("Vector3 property rejects Vector2 variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("v3", Salix::PropertyType::Vector3,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->v3; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{Salix::Vector2(9.0f, 8.0f)});
        CHECK(calls == 0); CHECK(element.v3 == Salix::Vector3(3.0f, 4.0f, 5.0f));
    }

    TEST_CASE("Point property rejects Rect variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("point", Salix::PropertyType::Point,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->point; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{Salix::Rect(1,2,3,4)});
        CHECK(calls == 0); CHECK((element.point == Salix::Point{6,7}));
    }

    TEST_CASE("Color property rejects Vector3 variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("color", Salix::PropertyType::Color,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->color; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{Salix::Vector3(1.0f, 1.0f, 1.0f)});
        CHECK(calls == 0); CHECK(element.color == Salix::Color(0.1f,0.2f,0.3f,0.4f));
    }

    TEST_CASE("Rect property rejects Point variant without invoking setter") {
        Batch1847LiveElement element; int calls = 0;
        auto property = batch1847_live_property("rect", Salix::PropertyType::Rect,
            [](void* p){ return &static_cast<Batch1847LiveElement*>(p)->rect; },
            [&](void*, void*){ ++calls; });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{Salix::Point{1,2}});
        CHECK(calls == 0); CHECK(element.rect == Salix::Rect(8,9,10,11));
    }
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch2347") {
    TEST_CASE("batch2347 Class property returns default variant and ignores writes") {
        Batch1147LiveElement element; int getter_calls=0,setter_calls=0;
        auto property=batch1147_property("opaque",Salix::PropertyType::Class,
            [&](void*)->void*{++getter_calls; return &element.integer;},
            [&](void*,void*){++setter_calls;});
        Salix::PropertyHandleLive handle(property,&element); const auto value=handle.get_value();
        CHECK(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==0); CHECK(getter_calls==0);
        handle.set_value(Salix::PropertyValue{42}); CHECK(setter_calls==0);
    }
    TEST_CASE("batch2347 Struct property returns default variant and ignores writes") {
        Batch1147LiveElement element; int getter_calls=0,setter_calls=0;
        auto property=batch1147_property("opaque",Salix::PropertyType::Struct,
            [&](void*)->void*{++getter_calls; return &element.integer;},
            [&](void*,void*){++setter_calls;});
        Salix::PropertyHandleLive handle(property,&element); CHECK(std::get<int>(handle.get_value())==0); CHECK(getter_calls==0);
        handle.set_value(Salix::PropertyValue{17}); CHECK(setter_calls==0);
    }
    TEST_CASE("batch2347 Enum property reads and writes integer representation") {
        Batch1147LiveElement element;
        auto property=batch1147_property("enum",Salix::PropertyType::Enum,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->enum_value;},
            [](void* p,void* v){static_cast<Batch1147LiveElement*>(p)->enum_value=*static_cast<int*>(v);});
        Salix::PropertyHandleLive handle(property,&element); CHECK(std::get<int>(handle.get_value())==2); handle.set_value(Salix::PropertyValue{9}); CHECK(element.enum_value==9); CHECK(std::get<int>(handle.get_value())==9);
    }
    TEST_CASE("batch2347 EnumClass property reads and writes integer representation") {
        Batch1147LiveElement element;
        auto property=batch1147_property("enumclass",Salix::PropertyType::EnumClass,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->enum_class_value;},
            [](void* p,void* v){static_cast<Batch1147LiveElement*>(p)->enum_class_value=*static_cast<int*>(v);});
        Salix::PropertyHandleLive handle(property,&element); CHECK(std::get<int>(handle.get_value())==3); handle.set_value(Salix::PropertyValue{-4}); CHECK(element.enum_class_value==-4);
    }
    TEST_CASE("batch2347 GlmMat4 property is readable but intentionally write protected") {
        Batch1147LiveElement element; int setter_calls=0;
        auto property=batch1147_property("matrix",Salix::PropertyType::GlmMat4,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->matrix;},
            [&](void*,void*){++setter_calls;});
        Salix::PropertyHandleLive handle(property,&element); const auto before=std::get<glm::mat4>(handle.get_value()); CHECK(before[0][0]==doctest::Approx(1.f));
        handle.set_value(Salix::PropertyValue{glm::mat4(3.f)}); CHECK(setter_calls==0); CHECK(std::get<glm::mat4>(handle.get_value())[0][0]==doctest::Approx(1.f));
    }
    TEST_CASE("batch2347 Point property roundtrips through live setter") {
        Batch1147LiveElement element;
        auto property=batch1147_property("point",Salix::PropertyType::Point,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->point;},
            [](void* p,void* v){static_cast<Batch1147LiveElement*>(p)->point=*static_cast<Salix::Point*>(v);});
        Salix::PropertyHandleLive handle(property,&element); handle.set_value(Salix::PropertyValue{Salix::Point{-8,12}}); CHECK((std::get<Salix::Point>(handle.get_value())==Salix::Point{-8,12}));
    }
    TEST_CASE("batch2347 Color property roundtrips through live setter") {
        Batch1147LiveElement element;
        auto property=batch1147_property("color",Salix::PropertyType::Color,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->color;},
            [](void* p,void* v){static_cast<Batch1147LiveElement*>(p)->color=*static_cast<Salix::Color*>(v);});
        Salix::PropertyHandleLive handle(property,&element); const Salix::Color expected(0.9f,0.8f,0.7f,0.6f); handle.set_value(Salix::PropertyValue{expected}); CHECK(std::get<Salix::Color>(handle.get_value())==expected);
    }
    TEST_CASE("batch2347 Rect property roundtrips through live setter") {
        Batch1147LiveElement element;
        auto property=batch1147_property("rect",Salix::PropertyType::Rect,
            [](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->rect;},
            [](void* p,void* v){static_cast<Batch1147LiveElement*>(p)->rect=*static_cast<Salix::Rect*>(v);});
        Salix::PropertyHandleLive handle(property,&element); const Salix::Rect expected(-4,5,60,70); handle.set_value(Salix::PropertyValue{expected}); CHECK(std::get<Salix::Rect>(handle.get_value())==expected);
    }
    TEST_CASE("batch2347 missing getter returns default without invoking available setter") {
        Batch1147LiveElement element; int setter_calls=0; Salix::Property property; property.name="integer"; property.type=Salix::PropertyType::Int; property.set_data=[&](void*,void*){++setter_calls;};
        Salix::PropertyHandleLive handle(property,&element); CHECK(std::get<int>(handle.get_value())==0); CHECK(setter_calls==0);
    }
    TEST_CASE("batch2347 missing setter preserves live value while getter remains functional") {
        Batch1147LiveElement element; Salix::Property property; property.name="integer"; property.type=Salix::PropertyType::Int; property.get_data=[](void* p)->void*{return &static_cast<Batch1147LiveElement*>(p)->integer;};
        Salix::PropertyHandleLive handle(property,&element); CHECK(std::get<int>(handle.get_value())==4); handle.set_value(Salix::PropertyValue{99}); CHECK(element.integer==4); CHECK(std::get<int>(handle.get_value())==4);
    }
}

namespace {
class Batch2647LiveElement final : public Salix::Element {
public:
    int integer=1;
    std::uint64_t u64=2;
    int enum_value=3;
    int enum_class_value=4;
    float scalar=5.5f;
    bool flag=false;
    std::string text="old";
    Salix::Vector2 v2{1.f,2.f};
    Salix::Vector3 v3{3.f,4.f,5.f};
    glm::mat4 matrix{1.f};
    const char* get_class_name() const override { return "Batch2647LiveElement"; }
};
Salix::Property batch2647_live_property(const char* name, Salix::PropertyType type,
    Salix::getter_func getter, Salix::setter_func setter) {
    Salix::Property property;
    property.name=name; property.type=type; property.get_data=std::move(getter); property.set_data=std::move(setter);
    return property;
}
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch2647") {
    TEST_CASE("batch2647 Int handle reads and writes live storage") {
        Batch2647LiveElement e; auto p=batch2647_live_property("i",Salix::PropertyType::Int,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->integer;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->integer=*static_cast<int*>(v);});
        Salix::PropertyHandleLive h(p,&e); CHECK(std::get<int>(h.get_value())==1); h.set_value(Salix::PropertyValue{42}); CHECK(e.integer==42);
    }
    TEST_CASE("batch2647 UInt64 handle preserves high bits through setter") {
        Batch2647LiveElement e; auto p=batch2647_live_property("u",Salix::PropertyType::UInt64,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->u64;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->u64=*static_cast<std::uint64_t*>(v);});
        Salix::PropertyHandleLive h(p,&e); const std::uint64_t value=(std::uint64_t{1}<<60)+2647ULL; h.set_value(Salix::PropertyValue{value});
        CHECK(std::get<std::uint64_t>(h.get_value())==value); CHECK(e.u64==value);
    }
    TEST_CASE("batch2647 Enum handle uses integer transport in both directions") {
        Batch2647LiveElement e; auto p=batch2647_live_property("enum",Salix::PropertyType::Enum,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->enum_value;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->enum_value=*static_cast<int*>(v);});
        Salix::PropertyHandleLive h(p,&e); CHECK(std::get<int>(h.get_value())==3); h.set_value(Salix::PropertyValue{17}); CHECK(e.enum_value==17);
    }
    TEST_CASE("batch2647 EnumClass handle uses integer transport in both directions") {
        Batch2647LiveElement e; auto p=batch2647_live_property("enumclass",Salix::PropertyType::EnumClass,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->enum_class_value;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->enum_class_value=*static_cast<int*>(v);});
        Salix::PropertyHandleLive h(p,&e); CHECK(std::get<int>(h.get_value())==4); h.set_value(Salix::PropertyValue{-9}); CHECK(e.enum_class_value==-9);
    }
    TEST_CASE("batch2647 Float handle accepts exact float alternative") {
        Batch2647LiveElement e; auto p=batch2647_live_property("float",Salix::PropertyType::Float,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->scalar;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->scalar=*static_cast<float*>(v);});
        Salix::PropertyHandleLive h(p,&e); h.set_value(Salix::PropertyValue{12.25f}); CHECK(e.scalar==doctest::Approx(12.25f)); CHECK(std::get<float>(h.get_value())==doctest::Approx(12.25f));
    }
    TEST_CASE("batch2647 Bool handle toggles exact live flag") {
        Batch2647LiveElement e; auto p=batch2647_live_property("bool",Salix::PropertyType::Bool,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->flag;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->flag=*static_cast<bool*>(v);});
        Salix::PropertyHandleLive h(p,&e); CHECK_FALSE(std::get<bool>(h.get_value())); h.set_value(Salix::PropertyValue{true}); CHECK(e.flag); CHECK(std::get<bool>(h.get_value()));
    }
    TEST_CASE("batch2647 String handle replaces complete live string including whitespace") {
        Batch2647LiveElement e; auto p=batch2647_live_property("string",Salix::PropertyType::String,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->text;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->text=*static_cast<std::string*>(v);});
        Salix::PropertyHandleLive h(p,&e); h.set_value(Salix::PropertyValue{std::string("  Batch 2647  ")}); CHECK(e.text=="  Batch 2647  "); CHECK(std::get<std::string>(h.get_value())==e.text);
    }
    TEST_CASE("batch2647 Vector2 handle replaces both coordinates") {
        Batch2647LiveElement e; auto p=batch2647_live_property("v2",Salix::PropertyType::Vector2,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->v2;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->v2=*static_cast<Salix::Vector2*>(v);});
        Salix::PropertyHandleLive h(p,&e); h.set_value(Salix::PropertyValue{Salix::Vector2(-2.f,6.f)}); CHECK(e.v2==Salix::Vector2(-2.f,6.f)); CHECK(std::get<Salix::Vector2>(h.get_value())==e.v2);
    }
    TEST_CASE("batch2647 Vector3 handle replaces all three coordinates") {
        Batch2647LiveElement e; auto p=batch2647_live_property("v3",Salix::PropertyType::Vector3,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->v3;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->v3=*static_cast<Salix::Vector3*>(v);});
        Salix::PropertyHandleLive h(p,&e); h.set_value(Salix::PropertyValue{Salix::Vector3(7.f,-8.f,9.f)}); CHECK(e.v3==Salix::Vector3(7.f,-8.f,9.f)); CHECK(std::get<Salix::Vector3>(h.get_value())==e.v3);
    }
    TEST_CASE("batch2647 GlmMat4 handle remains intentionally read only despite setter callback") {
        Batch2647LiveElement e; int setter_calls=0; e.matrix[2][1]=6.25f;
        auto p=batch2647_live_property("matrix",Salix::PropertyType::GlmMat4,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->matrix;},
            [&](void*,void*){++setter_calls;});
        Salix::PropertyHandleLive h(p,&e); auto before=std::get<glm::mat4>(h.get_value()); CHECK(before[2][1]==doctest::Approx(6.25f));
        h.set_value(Salix::PropertyValue{glm::mat4(3.f)}); CHECK(setter_calls==0); CHECK(e.matrix[2][1]==doctest::Approx(6.25f));
    }
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch2947") {
    TEST_CASE("batch2947 null live instance read returns default property variant") {
        auto p=batch2647_live_property("i",Salix::PropertyType::Int,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->integer;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->integer=*static_cast<int*>(v);});
        Salix::PropertyHandleLive handle(p,nullptr); const auto value=handle.get_value();
        CHECK(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==0);
    }
    TEST_CASE("batch2947 null live instance write never invokes setter callback") {
        int setter_calls=0; auto p=batch2647_live_property("i",Salix::PropertyType::Int,
            [](void*)->void*{return nullptr;},[&](void*,void*){++setter_calls;});
        Salix::PropertyHandleLive handle(p,nullptr); handle.set_value(Salix::PropertyValue{42}); CHECK(setter_calls==0);
    }
    TEST_CASE("batch2947 missing getter still allows valid setter to update live storage") {
        Batch2647LiveElement e; Salix::Property p; p.name="i"; p.type=Salix::PropertyType::Int;
        p.set_data=[](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->integer=*static_cast<int*>(v);};
        Salix::PropertyHandleLive handle(p,&e); CHECK(std::get<int>(handle.get_value())==0); handle.set_value(Salix::PropertyValue{2947}); CHECK(e.integer==2947);
    }
    TEST_CASE("batch2947 missing setter still exposes current live value") {
        Batch2647LiveElement e; Salix::Property p; p.name="i"; p.type=Salix::PropertyType::Int;
        p.get_data=[](void* x)->void*{return &static_cast<Batch2647LiveElement*>(x)->integer;};
        Salix::PropertyHandleLive handle(p,&e); CHECK(std::get<int>(handle.get_value())==1); handle.set_value(Salix::PropertyValue{99}); CHECK(e.integer==1);
    }
    TEST_CASE("batch2947 int setter ignores mismatched string variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("i",Salix::PropertyType::Int,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->integer;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->integer=*static_cast<int*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{std::string("wrong")}); CHECK(e.integer==1);
    }
    TEST_CASE("batch2947 uint64 setter ignores mismatched signed integer variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("u",Salix::PropertyType::UInt64,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->u64;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->u64=*static_cast<std::uint64_t*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{7}); CHECK(e.u64==2);
    }
    TEST_CASE("batch2947 enum setter ignores mismatched float variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("enum",Salix::PropertyType::Enum,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->enum_value;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->enum_value=*static_cast<int*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{3.5f}); CHECK(e.enum_value==3);
    }
    TEST_CASE("batch2947 float setter ignores mismatched integer variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("float",Salix::PropertyType::Float,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->scalar;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->scalar=*static_cast<float*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{12}); CHECK(e.scalar==doctest::Approx(5.5f));
    }
    TEST_CASE("batch2947 bool setter ignores mismatched integer variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("bool",Salix::PropertyType::Bool,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->flag;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->flag=*static_cast<bool*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{1}); CHECK_FALSE(e.flag);
    }
    TEST_CASE("batch2947 string setter ignores mismatched boolean variant") {
        Batch2647LiveElement e; auto p=batch2647_live_property("string",Salix::PropertyType::String,
            [](void* x){return &static_cast<Batch2647LiveElement*>(x)->text;},
            [](void* x,void* v){static_cast<Batch2647LiveElement*>(x)->text=*static_cast<std::string*>(v);});
        Salix::PropertyHandleLive handle(p,&e); handle.set_value(Salix::PropertyValue{true}); CHECK(e.text=="old");
    }
}

TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch3347") {
    TEST_CASE("batch3347 repeated integer writes always expose latest live value") {
        Batch1147LiveElement e; auto p=batch1147_property("integer",Salix::PropertyType::Int,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->integer;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->integer=*static_cast<int*>(v);}); Salix::PropertyHandleLive h(p,&e);
        h.set_value(Salix::PropertyValue{1}); h.set_value(Salix::PropertyValue{-3347}); CHECK(e.integer==-3347); CHECK(std::get<int>(h.get_value())==-3347);
    }
    TEST_CASE("batch3347 uint64 handle preserves maximum value") {
        Batch1147LiveElement e; auto p=batch1147_property("identifier",Salix::PropertyType::UInt64,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->identifier;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->identifier=*static_cast<std::uint64_t*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const std::uint64_t value=(std::numeric_limits<std::uint64_t>::max)(); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<std::uint64_t>(h.get_value())==value);
    }
    TEST_CASE("batch3347 float handle preserves negative fractional value") {
        Batch1147LiveElement e; auto p=batch1147_property("scalar",Salix::PropertyType::Float,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->scalar;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->scalar=*static_cast<float*>(v);}); Salix::PropertyHandleLive h(p,&e);
        h.set_value(Salix::PropertyValue{-33.47f}); CHECK(e.scalar==doctest::Approx(-33.47f)); CHECK(std::get<float>(h.get_value())==doctest::Approx(-33.47f));
    }
    TEST_CASE("batch3347 string handle preserves embedded null bytes") {
        Batch1147LiveElement e; auto p=batch1147_property("text",Salix::PropertyType::String,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->text;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->text=*static_cast<std::string*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const std::string value("A\0B3347",7); h.set_value(Salix::PropertyValue{value}); CHECK(e.text==value); CHECK(std::get<std::string>(h.get_value()).size()==value.size());
    }
    TEST_CASE("batch3347 Vector2 handle preserves signed fractional coordinates") {
        Batch1147LiveElement e; auto p=batch1147_property("vector2",Salix::PropertyType::Vector2,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->vector2;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->vector2=*static_cast<Salix::Vector2*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const Salix::Vector2 value(-3.347f,99.25f); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Vector2>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Vector3 handle preserves all independently signed coordinates") {
        Batch1147LiveElement e; auto p=batch1147_property("vector3",Salix::PropertyType::Vector3,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->vector3;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->vector3=*static_cast<Salix::Vector3*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const Salix::Vector3 value(-1.f,2.5f,-3.75f); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Vector3>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Point handle replaces both integer coordinates") {
        Batch1147LiveElement e; auto p=batch1147_property("point",Salix::PropertyType::Point,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->point;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->point=*static_cast<Salix::Point*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const Salix::Point value{-33,47}; h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Point>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Color handle preserves alpha independently of RGB") {
        Batch1147LiveElement e; auto p=batch1147_property("color",Salix::PropertyType::Color,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->color;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->color=*static_cast<Salix::Color*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const Salix::Color value(.9f,.1f,.4f,.3347f); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Color>(h.get_value())==value);
    }
    TEST_CASE("batch3347 Rect handle preserves all four integer channels") {
        Batch1147LiveElement e; auto p=batch1147_property("rect",Salix::PropertyType::Rect,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->rect;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->rect=*static_cast<Salix::Rect*>(v);}); Salix::PropertyHandleLive h(p,&e);
        const Salix::Rect value(-3,4,33,47); h.set_value(Salix::PropertyValue{value}); CHECK(std::get<Salix::Rect>(h.get_value())==value);
    }
    TEST_CASE("batch3347 matrix handle reads latest live storage mutation while remaining write protected") {
        Batch1147LiveElement e; auto p=batch1147_property("matrix",Salix::PropertyType::GlmMat4,[](void* i){return &static_cast<Batch1147LiveElement*>(i)->matrix;},[](void* i,void* v){static_cast<Batch1147LiveElement*>(i)->matrix=*static_cast<glm::mat4*>(v);}); Salix::PropertyHandleLive h(p,&e);
        e.matrix[0][0]=3.347f; CHECK(std::get<glm::mat4>(h.get_value())[0][0]==doctest::Approx(3.347f)); h.set_value(Salix::PropertyValue{glm::mat4(9.f)}); CHECK(e.matrix[0][0]==doctest::Approx(3.347f));
    }
}
TEST_SUITE("Salix::reflection::PropertyHandleLiveBatch3747") {
    TEST_CASE("batch3747 null integer getter result returns default variant without dereference") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "NullInt3747", Salix::PropertyType::Int,
            [](void*) -> void* { return nullptr; },
            {});
        Salix::PropertyHandleLive handle(property, &element);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
    }

    TEST_CASE("batch3747 null Vector3 getter result returns default variant without dereference") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "NullVector3747", Salix::PropertyType::Vector3,
            [](void*) -> void* { return nullptr; },
            {});
        Salix::PropertyHandleLive handle(property, &element);
        const auto value = handle.get_value();
        CHECK(std::holds_alternative<int>(value));
        CHECK(std::get<int>(value) == 0);
    }

    TEST_CASE("batch3747 successful EnumClass getter is invoked exactly once per read") {
        Batch1147LiveElement element;
        int calls = 0;
        auto property = batch1147_property(
            "EnumClass3747", Salix::PropertyType::EnumClass,
            [&](void* instance) -> void* {
                ++calls;
                return &static_cast<Batch1147LiveElement*>(instance)->enum_class_value;
            },
            {});
        Salix::PropertyHandleLive handle(property, &element);
        CHECK(std::get<int>(handle.get_value()) == element.enum_class_value);
        CHECK(calls == 1);
        CHECK(std::get<int>(handle.get_value()) == element.enum_class_value);
        CHECK(calls == 2);
    }

    TEST_CASE("batch3747 unsupported Class type returns default without invoking getter") {
        Batch1147LiveElement element;
        int calls = 0;
        auto property = batch1147_property(
            "Class3747", Salix::PropertyType::Class,
            [&](void*) -> void* { ++calls; return &element.integer; },
            {});
        Salix::PropertyHandleLive handle(property, &element);
        const auto value = handle.get_value();
        CHECK(std::get<int>(value) == 0);
        CHECK(calls == 0);
    }

    TEST_CASE("batch3747 unsupported Struct type returns default without invoking getter") {
        Batch1147LiveElement element;
        int calls = 0;
        auto property = batch1147_property(
            "Struct3747", Salix::PropertyType::Struct,
            [&](void*) -> void* { ++calls; return &element.integer; },
            {});
        Salix::PropertyHandleLive handle(property, &element);
        CHECK(std::get<int>(handle.get_value()) == 0);
        CHECK(calls == 0);
    }

    TEST_CASE("batch3747 float handle preserves negative zero sign bit") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "Float3747", Salix::PropertyType::Float,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->scalar; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->scalar = *static_cast<float*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        handle.set_value(Salix::PropertyValue{-0.0f});
        const float value = std::get<float>(handle.get_value());
        CHECK(value == 0.0f);
        CHECK(std::signbit(value));
    }

    TEST_CASE("batch3747 string handle preserves multiline text with embedded null bytes") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "String3747", Salix::PropertyType::String,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->text; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->text = *static_cast<std::string*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        const std::string value("line1\0line2\n3747", 16);
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<std::string>(handle.get_value()) == value);
        CHECK(element.text.size() == value.size());
    }

    TEST_CASE("batch3747 Point handle preserves signed integer limits") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "Point3747", Salix::PropertyType::Point,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->point; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->point = *static_cast<Salix::Point*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        const Salix::Point value{(std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()};
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Point>(handle.get_value()) == value);
    }

    TEST_CASE("batch3747 Rect handle preserves signed integer boundary channels") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "Rect3747", Salix::PropertyType::Rect,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->rect; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->rect = *static_cast<Salix::Rect*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        const Salix::Rect value((std::numeric_limits<int>::min)(), -1, 0, (std::numeric_limits<int>::max)());
        handle.set_value(Salix::PropertyValue{value});
        CHECK(std::get<Salix::Rect>(handle.get_value()) == value);
    }

    TEST_CASE("batch3747 Color handle transports nonfinite diagnostic channels without normalization") {
        Batch1147LiveElement element;
        auto property = batch1147_property(
            "Color3747", Salix::PropertyType::Color,
            [](void* instance) { return &static_cast<Batch1147LiveElement*>(instance)->color; },
            [](void* instance, void* value) { static_cast<Batch1147LiveElement*>(instance)->color = *static_cast<Salix::Color*>(value); });
        Salix::PropertyHandleLive handle(property, &element);
        const Salix::Color value(
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            0.5f,
            -0.0f);
        handle.set_value(Salix::PropertyValue{value});
        const auto roundtrip = std::get<Salix::Color>(handle.get_value());
        CHECK(std::isinf(roundtrip.r));
        CHECK(roundtrip.r > 0.0f);
        CHECK(std::isinf(roundtrip.g));
        CHECK(roundtrip.g < 0.0f);
        CHECK(roundtrip.b == doctest::Approx(0.5f));
        CHECK(std::signbit(roundtrip.a));
    }
}

