#include <doctest.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/ecs/CppScript.h>
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace {
class Batch1147UnregisteredElement : public Salix::Element {
public:
    const char* get_class_name() const override { return "Batch1147UnregisteredElement"; }
};

bool batch1147_has_property(const std::vector<Salix::Property>& properties, const std::string& name) {
    return std::any_of(properties.begin(), properties.end(), [&](const Salix::Property& property) {
        return property.name == name;
    });
}
}

TEST_SUITE("Salix::reflection::ByteMirrorBatch1147") {
    TEST_CASE("register all types exposes canonical base and concrete reflection records") {
        Salix::ByteMirror::register_all_types();
        const auto* element = Salix::ByteMirror::get_type_info(typeid(Salix::Element));
        const auto* transform = Salix::ByteMirror::get_type_info(typeid(Salix::Transform));
        const auto* sprite = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        const auto* camera = Salix::ByteMirror::get_type_info(typeid(Salix::Camera));
        REQUIRE(element != nullptr);
        REQUIRE(transform != nullptr);
        REQUIRE(sprite != nullptr);
        REQUIRE(camera != nullptr);
        CHECK(element->name == "Element");
        CHECK(transform->name == "Transform");
        CHECK(sprite->name == "Sprite2D");
        CHECK(camera->name == "Camera");
    }

    TEST_CASE("name lookup rejects empty and unknown names while finding registered types") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::get_type_info_by_name("") == nullptr);
        CHECK(Salix::ByteMirror::get_type_info_by_name("Batch1147MissingType") == nullptr);
        const auto* transform = Salix::ByteMirror::get_type_info_by_name("Transform");
        REQUIRE(transform != nullptr);
        CHECK(transform->type_index.has_value());
        CHECK(*transform->type_index == std::type_index(typeid(Salix::Transform)));
    }

    TEST_CASE("transform reflection inherits Element properties after its own transform fields") {
        Salix::ByteMirror::register_all_types();
        const auto* transform = Salix::ByteMirror::get_type_info(typeid(Salix::Transform));
        REQUIRE(transform != nullptr);
        const auto properties = Salix::ByteMirror::get_all_properties_for_type(transform);
        CHECK(batch1147_has_property(properties, "position"));
        CHECK(batch1147_has_property(properties, "rotation"));
        CHECK(batch1147_has_property(properties, "scale"));
        CHECK(batch1147_has_property(properties, "id"));
        CHECK(batch1147_has_property(properties, "name"));
        CHECK(batch1147_has_property(properties, "visible"));
        REQUIRE(properties.size() >= 6);
        CHECK(properties.front().name == "position");
    }

    TEST_CASE("sprite reflection advertises derived dimensions and inherited element identity") {
        Salix::ByteMirror::register_all_types();
        const auto* sprite = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        REQUIRE(sprite != nullptr);
        CHECK(std::find(sprite->derived_properties.begin(), sprite->derived_properties.end(), "width") != sprite->derived_properties.end());
        CHECK(std::find(sprite->derived_properties.begin(), sprite->derived_properties.end(), "height") != sprite->derived_properties.end());
        const auto properties = Salix::ByteMirror::get_all_properties_for_type(sprite);
        CHECK(batch1147_has_property(properties, "texture_path"));
        CHECK(batch1147_has_property(properties, "width"));
        CHECK(batch1147_has_property(properties, "height"));
        CHECK(batch1147_has_property(properties, "name"));
    }

    TEST_CASE("registered element constructors create the requested concrete runtime types") {
        Salix::ByteMirror::register_all_types();
        std::unique_ptr<Salix::Element> transform(Salix::ByteMirror::create_element_by_name("Transform"));
        std::unique_ptr<Salix::Element> sprite(Salix::ByteMirror::create_element_by_name("Sprite2D"));
        std::unique_ptr<Salix::Element> camera(Salix::ByteMirror::create_element_by_name("Camera"));
        std::unique_ptr<Salix::Element> collider(Salix::ByteMirror::create_element_by_name("BoxCollider"));
        std::unique_ptr<Salix::Element> script(Salix::ByteMirror::create_element_by_name("CppScript"));
        CHECK(dynamic_cast<Salix::Transform*>(transform.get()) != nullptr);
        CHECK(dynamic_cast<Salix::Sprite2D*>(sprite.get()) != nullptr);
        CHECK(dynamic_cast<Salix::Camera*>(camera.get()) != nullptr);
        CHECK(dynamic_cast<Salix::BoxCollider*>(collider.get()) != nullptr);
        CHECK(dynamic_cast<Salix::CppScript*>(script.get()) != nullptr);
    }

    TEST_CASE("constructor registry returns null for unknown names and supports custom constructors") {
        CHECK(Salix::ByteMirror::create_element_by_name("Batch1147UnknownConstructor") == nullptr);
        Salix::ByteMirror::register_constructor(
            "Batch1147Custom",
            []() -> Salix::Element* { return new Batch1147UnregisteredElement(); });
        std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("Batch1147Custom"));
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1147UnregisteredElement*>(created.get()) != nullptr);
        CHECK(std::string(created->get_class_name()) == "Batch1147UnregisteredElement");
    }

    TEST_CASE("live handle creation rejects null and unregistered dynamic element types") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::create_handles_for(nullptr).empty());
        Batch1147UnregisteredElement unregistered;
        CHECK(Salix::ByteMirror::create_handles_for(&unregistered).empty());
    }

    TEST_CASE("live handle creation for Transform includes own and inherited property handles") {
        Salix::ByteMirror::register_all_types();
        Salix::Transform transform;
        auto handles = Salix::ByteMirror::create_handles_for(&transform);
        REQUIRE(handles.size() >= 6);
        std::vector<std::string> names;
        for (const auto& handle : handles) {
            REQUIRE(handle != nullptr);
            names.push_back(handle->get_name());
        }
        CHECK(std::find(names.begin(), names.end(), "position") != names.end());
        CHECK(std::find(names.begin(), names.end(), "rotation") != names.end());
        CHECK(std::find(names.begin(), names.end(), "scale") != names.end());
        CHECK(std::find(names.begin(), names.end(), "id") != names.end());
        CHECK(std::find(names.begin(), names.end(), "name") != names.end());
        CHECK(std::find(names.begin(), names.end(), "visible") != names.end());
    }

    TEST_CASE("property value lookup reads inherited Element identity name and visibility") {
        Salix::ByteMirror::register_all_types();
        Salix::Transform transform;
        transform.set_name("PlayerTransform");
        transform.set_visibility(false);
        const auto name = Salix::ByteMirror::get_property_value(&transform, "name");
        const auto visible = Salix::ByteMirror::get_property_value(&transform, "visible");
        const auto identifier = Salix::ByteMirror::get_property_value(&transform, "id");
        CHECK(std::get<std::string>(name) == "PlayerTransform");
        CHECK_FALSE(std::get<bool>(visible));
        CHECK(std::get<std::uint64_t>(identifier) == transform.get_id().get_value());
        CHECK(std::get<int>(Salix::ByteMirror::get_property_value(nullptr, "name")) == 0);
        CHECK(std::get<int>(Salix::ByteMirror::get_property_value(&transform, "missing")) == 0);
    }

    TEST_CASE("YAML handle creation selects known present properties and skips unknown elements") {
        Salix::ByteMirror::register_all_types();
        YAML::Node entity;
        YAML::Node transform_properties;
        transform_properties["name"] = "YamlTransform";
        transform_properties["visible"] = true;
        YAML::Node position;
        position["x"] = 1.0f;
        position["y"] = 2.0f;
        position["z"] = 3.0f;
        transform_properties["position"] = position;
        YAML::Node transform_wrapper;
        transform_wrapper["Transform"] = transform_properties;
        entity["elements"].push_back(transform_wrapper);

        YAML::Node unknown_properties;
        unknown_properties["anything"] = 1;
        YAML::Node unknown_wrapper;
        unknown_wrapper["Batch1147Unknown"] = unknown_properties;
        entity["elements"].push_back(unknown_wrapper);

        auto handles = Salix::ByteMirror::create_handles_for_yaml(&entity);
        REQUIRE(handles.size() == 3);
        std::vector<std::string> names;
        for (const auto& handle : handles) {
            REQUIRE(handle != nullptr);
            names.push_back(handle->get_name());
        }
        CHECK(std::find(names.begin(), names.end(), "name") != names.end());
        CHECK(std::find(names.begin(), names.end(), "visible") != names.end());
        CHECK(std::find(names.begin(), names.end(), "position") != names.end());

        YAML::Node empty_entity;
        CHECK(Salix::ByteMirror::create_handles_for_yaml(nullptr).empty());
        CHECK(Salix::ByteMirror::create_handles_for_yaml(&empty_entity).empty());
    }
}
