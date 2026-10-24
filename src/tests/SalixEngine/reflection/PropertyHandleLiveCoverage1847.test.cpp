#include <doctest.h>
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Point.h>
#include <Salix/math/Color.h>
#include <Salix/math/Rect.h>
#include <cstdint>
#include <string>
#include <utility>

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

TEST_SUITE("Salix::reflection::PropertyHandleLiveCoverage1847") {
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
