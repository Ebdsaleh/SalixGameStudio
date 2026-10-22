#include <doctest.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Tests/SalixEngine/mocking/reflection/MockPropertyHandle.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::reflection::PropertyHandleBatch1547") {
    TEST_CASE("property handle exposes reflected property name and type") {
        Salix::Property property;
        property.name = "health";
        property.type = Salix::PropertyType::Int;
        MockPropertyHandle handle(property, 100);
        CHECK(handle.get_name() == "health");
        CHECK(handle.get_type() == Salix::PropertyType::Int);
    }

    TEST_CASE("property handle exposes contained type metadata pointer exactly") {
        Salix::TypeInfo contained;
        contained.name = "Nested";
        Salix::Property property;
        property.name = "nested";
        property.type = Salix::PropertyType::Struct;
        property.contained_type_info = &contained;
        MockPropertyHandle handle(property);
        CHECK(handle.get_contained_type_info() == &contained);
        CHECK(handle.get_contained_type_info()->name == "Nested");
    }

    TEST_CASE("property handle defaults contained type pointer to null") {
        Salix::Property property;
        property.name = "value";
        property.type = Salix::PropertyType::Float;
        MockPropertyHandle handle(property, 1.0f);
        CHECK(handle.get_contained_type_info() == nullptr);
    }

    TEST_CASE("property handle exposes UI hint from copied metadata") {
        Salix::Property property;
        property.name = "texture";
        property.type = Salix::PropertyType::String;
        property.hint = Salix::UIHint::ImageFile;
        MockPropertyHandle handle(property, std::string("sprite.png"));
        CHECK(handle.get_hint() == Salix::UIHint::ImageFile);
    }

    TEST_CASE("property handle read only flag is transported from property metadata") {
        Salix::Property property;
        property.name = "matrix";
        property.type = Salix::PropertyType::GlmMat4;
        property.is_read_only = true;
        MockPropertyHandle handle(property, glm::mat4(1.0f));
        CHECK(handle.is_read_only());
    }

    TEST_CASE("property handle display order preserves custom priority") {
        Salix::Property property;
        property.name = "position";
        property.type = Salix::PropertyType::Vector3;
        property.display_order = -25;
        MockPropertyHandle handle(property, Salix::Vector3{});
        CHECK(handle.get_display_order() == -25);
    }

    TEST_CASE("property handle value getter returns stored variant and records access") {
        Salix::Property property;
        property.name = "enabled";
        property.type = Salix::PropertyType::Bool;
        MockPropertyHandle handle(property, true);
        const Salix::PropertyValue value = handle.get_value();
        CHECK(std::holds_alternative<bool>(value));
        CHECK(std::get<bool>(value));
        CHECK(handle.get_calls == 1);
    }

    TEST_CASE("property handle setter replaces variant type and value") {
        Salix::Property property;
        property.name = "dynamic";
        property.type = Salix::PropertyType::String;
        MockPropertyHandle handle(property, std::string("before"));
        handle.set_value(std::string("after"));
        const auto value = handle.get_value();
        CHECK(handle.set_calls == 1);
        CHECK(std::holds_alternative<std::string>(value));
        CHECK(std::get<std::string>(value) == "after");
    }

    TEST_CASE("property handle copies property metadata at construction") {
        Salix::Property property;
        property.name = "original";
        property.type = Salix::PropertyType::Int;
        property.hint = Salix::UIHint::None;
        property.display_order = 10;
        MockPropertyHandle handle(property, 7);

        property.name = "mutated source";
        property.type = Salix::PropertyType::Float;
        property.hint = Salix::UIHint::ColorEdit;
        property.display_order = 99;

        CHECK(handle.get_name() == "original");
        CHECK(handle.get_type() == Salix::PropertyType::Int);
        CHECK(handle.get_hint() == Salix::UIHint::None);
        CHECK(handle.get_display_order() == 10);
    }

    TEST_CASE("property handle supports polymorphic destruction through base ownership") {
        Salix::Property property;
        property.name = "owned";
        property.type = Salix::PropertyType::UInt64;
        std::unique_ptr<Salix::PropertyHandle> handle =
            std::make_unique<MockPropertyHandle>(property, uint64_t{42});
        REQUIRE(handle != nullptr);
        CHECK(handle->get_name() == "owned");
        CHECK(std::get<uint64_t>(handle->get_value()) == 42u);
    }
}
