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

    TEST_CASE("factory class names match their canonical reflection registration keys") {
        Salix::ByteMirror::register_all_types();
        const char* names[] = {"Transform", "Sprite2D", "Camera", "BoxCollider", "CppScript"};

        for (const char* name : names) {
            std::unique_ptr<Salix::Element> element(Salix::ByteMirror::create_element_by_name(name));
            REQUIRE_MESSAGE(element != nullptr, "Factory failed to create ", name);
            CHECK(std::string(element->get_class_name()) == name);
        }
    }

    TEST_CASE("Transform TypeInfo explicitly links Element as its reflection ancestor") {
        Salix::ByteMirror::register_all_types();
        const Salix::TypeInfo* transform = Salix::ByteMirror::get_type_info_by_name("Transform");
        REQUIRE(transform != nullptr);
        REQUIRE(transform->ancestor != nullptr);
        CHECK(transform->ancestor->name == "Element");
        REQUIRE(transform->ancestor->type_index.has_value());
        CHECK(*transform->ancestor->type_index == std::type_index(typeid(Salix::Element)));
    }

    TEST_CASE("Transform reflected position returns the current Vector3 value") {
        Salix::ByteMirror::register_all_types();
        Salix::Transform transform;
        transform.set_position({10.0f, 20.0f, 30.0f});

        const Salix::PropertyValue value =
            Salix::ByteMirror::get_property_value(&transform, "position");

        REQUIRE(std::holds_alternative<Salix::Vector3>(value));
        const Salix::Vector3 position = std::get<Salix::Vector3>(value);
        CHECK(position.x == doctest::Approx(10.0f));
        CHECK(position.y == doctest::Approx(20.0f));
        CHECK(position.z == doctest::Approx(30.0f));
    }

    TEST_CASE("Transform position live handle exposes the current Vector3 through type erasure") {
        Salix::ByteMirror::register_all_types();
        Salix::Transform transform;
        transform.set_position({4.0f, 5.0f, 6.0f});
        auto handles = Salix::ByteMirror::create_handles_for(&transform);

        auto it = std::find_if(handles.begin(), handles.end(), [](const auto& handle) {
            return handle && handle->get_name() == "position";
        });

        REQUIRE(it != handles.end());
        const Salix::PropertyValue value = (*it)->get_value();
        REQUIRE(std::holds_alternative<Salix::Vector3>(value));
        CHECK(std::get<Salix::Vector3>(value) == Salix::Vector3(4.0f, 5.0f, 6.0f));
    }

    TEST_CASE("Camera projection mode reflection carries EnumClass metadata for ProjectionMode") {
        Salix::ByteMirror::register_all_types();
        const Salix::TypeInfo* camera = Salix::ByteMirror::get_type_info_by_name("Camera");
        REQUIRE(camera != nullptr);

        const auto properties = Salix::ByteMirror::get_all_properties_for_type(camera);
        const auto it = std::find_if(properties.begin(), properties.end(), [](const Salix::Property& property) {
            return property.name == "projection_mode";
        });

        REQUIRE(it != properties.end());
        CHECK(it->type == Salix::PropertyType::EnumClass);
        REQUIRE(it->contained_type_info != nullptr);
        CHECK(it->contained_type_info->name == "Salix::ProjectionMode");
        REQUIRE(it->contained_type_info->type_index.has_value());
        CHECK(*it->contained_type_info->type_index == std::type_index(typeid(Salix::ProjectionMode)));
    }

    TEST_CASE("Sprite2D texture path reflection carries the ImageFile UI hint") {
        Salix::ByteMirror::register_all_types();
        const Salix::TypeInfo* sprite = Salix::ByteMirror::get_type_info_by_name("Sprite2D");
        REQUIRE(sprite != nullptr);

        const auto properties = Salix::ByteMirror::get_all_properties_for_type(sprite);
        const auto it = std::find_if(properties.begin(), properties.end(), [](const Salix::Property& property) {
            return property.name == "texture_path";
        });

        REQUIRE(it != properties.end());
        CHECK(it->type == Salix::PropertyType::String);
        CHECK(it->hint == Salix::UIHint::ImageFile);
        CHECK_FALSE(it->is_read_only);
    }

    TEST_CASE("Sprite2D derived width is read only and ignores attempted handle writes") {
        Salix::ByteMirror::register_all_types();
        Salix::Sprite2D sprite;
        REQUIRE(sprite.get_texture_width() == 0);

        auto handles = Salix::ByteMirror::create_handles_for(&sprite);
        auto it = std::find_if(handles.begin(), handles.end(), [](const auto& handle) {
            return handle && handle->get_name() == "width";
        });

        REQUIRE(it != handles.end());
        CHECK((*it)->is_read_only());
        CHECK(std::get<int>((*it)->get_value()) == 0);
        (*it)->set_value(Salix::PropertyValue{999});
        CHECK(sprite.get_texture_width() == 0);
        CHECK(std::get<int>(Salix::ByteMirror::get_property_value(&sprite, "width")) == 0);
    }

    TEST_CASE("Camera view matrix live handle ignores writes to the read only reflected matrix") {
        Salix::ByteMirror::register_all_types();
        Salix::Camera camera;

        auto handles = Salix::ByteMirror::create_handles_for(&camera);
        auto it = std::find_if(handles.begin(), handles.end(), [](const auto& handle) {
            return handle && handle->get_name() == "view_matrix";
        });

        REQUIRE(it != handles.end());
        const auto before = std::get<glm::mat4>((*it)->get_value());
        CHECK(before[0][0] == doctest::Approx(1.0f));

        (*it)->set_value(Salix::PropertyValue{glm::mat4(2.0f)});

        const auto after =
            std::get<glm::mat4>(Salix::ByteMirror::get_property_value(&camera, "view_matrix"));
        CHECK(after[0][0] == doctest::Approx(1.0f));
        CHECK(after[1][1] == doctest::Approx(1.0f));
    }

    TEST_CASE("Camera projection mode live handle writes the reflected Orthographic enum value") {
        Salix::ByteMirror::register_all_types();
        Salix::Camera camera;
        auto handles = Salix::ByteMirror::create_handles_for(&camera);

        auto it = std::find_if(handles.begin(), handles.end(), [](const auto& handle) {
            return handle && handle->get_name() == "projection_mode";
        });

        REQUIRE(it != handles.end());
        const int orthographic = static_cast<int>(Salix::ProjectionMode::Orthographic);
        (*it)->set_value(Salix::PropertyValue{orthographic});
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Orthographic);
        CHECK(std::get<int>((*it)->get_value()) == orthographic);
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

TEST_SUITE("Salix::reflection::ByteMirrorBatch2347") {
    TEST_CASE("batch2347 repeated register all types remains idempotently queryable") {
        Salix::ByteMirror::register_all_types(); Salix::ByteMirror::register_all_types();
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("Transform")!=nullptr);
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("Sprite2D")!=nullptr);
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("Camera")!=nullptr);
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("BoxCollider")!=nullptr);
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("CppScript")!=nullptr);
    }
    TEST_CASE("batch2347 BoxCollider reflection links Element ancestor and size property") {
        Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("BoxCollider"); REQUIRE(info!=nullptr); REQUIRE(info->ancestor!=nullptr); CHECK(info->ancestor->name=="Element");
        const auto props=Salix::ByteMirror::get_all_properties_for_type(info); CHECK(batch1147_has_property(props,"size")); CHECK(batch1147_has_property(props,"id")); CHECK(batch1147_has_property(props,"name")); CHECK(batch1147_has_property(props,"visible"));
    }
    TEST_CASE("batch2347 CppScript reflection includes script name and inherited element properties") {
        Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("CppScript"); REQUIRE(info!=nullptr);
        const auto props=Salix::ByteMirror::get_all_properties_for_type(info); CHECK(batch1147_has_property(props,"script_name")); CHECK(batch1147_has_property(props,"name")); CHECK(batch1147_has_property(props,"visible")); CHECK(batch1147_has_property(props,"id"));
    }
    TEST_CASE("batch2347 null TypeInfo produces an empty flattened property list") {
        CHECK(Salix::ByteMirror::get_all_properties_for_type(nullptr).empty());
    }
    TEST_CASE("batch2347 BoxCollider live handles expose size and inherited metadata") {
        Salix::ByteMirror::register_all_types(); Salix::BoxCollider collider; auto handles=Salix::ByteMirror::create_handles_for(&collider); std::vector<std::string> names;
        for(const auto& h:handles){REQUIRE(h!=nullptr); names.push_back(h->get_name());}
        CHECK(std::find(names.begin(),names.end(),"size")!=names.end()); CHECK(std::find(names.begin(),names.end(),"name")!=names.end()); CHECK(std::find(names.begin(),names.end(),"visible")!=names.end());
    }
    TEST_CASE("batch2347 BoxCollider reflected size follows live object mutation") {
        Salix::ByteMirror::register_all_types(); Salix::BoxCollider collider; collider.set_size(Salix::Vector3(2.f,4.f,8.f));
        const auto value=Salix::ByteMirror::get_property_value(&collider,"size"); REQUIRE(std::holds_alternative<Salix::Vector3>(value)); CHECK(std::get<Salix::Vector3>(value)==Salix::Vector3(2.f,4.f,8.f));
    }
    TEST_CASE("batch2347 CppScript script name can be changed through reflected live handle") {
        Salix::ByteMirror::register_all_types(); Salix::CppScript script; auto handles=Salix::ByteMirror::create_handles_for(&script);
        auto it=std::find_if(handles.begin(),handles.end(),[](const auto& h){return h&&h->get_name()=="script_name";}); REQUIRE(it!=handles.end());
        (*it)->set_value(Salix::PropertyValue{std::string("Batch2347Script")}); CHECK(script.get_script_name()=="Batch2347Script"); CHECK(std::get<std::string>((*it)->get_value())=="Batch2347Script");
    }
    TEST_CASE("batch2347 Camera projection mode property value reflects latest enum selection") {
        Salix::ByteMirror::register_all_types(); Salix::Camera camera; camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        const auto value=Salix::ByteMirror::get_property_value(&camera,"projection_mode"); REQUIRE(std::holds_alternative<int>(value)); CHECK(std::get<int>(value)==static_cast<int>(Salix::ProjectionMode::Orthographic));
    }
    TEST_CASE("batch2347 empty and unknown constructor names return null") {
        Salix::ByteMirror::register_all_types(); CHECK(Salix::ByteMirror::create_element_by_name("")==nullptr); CHECK(Salix::ByteMirror::create_element_by_name("Batch2347Missing")==nullptr);
    }
    TEST_CASE("batch2347 YAML BoxCollider handles include only properties present in node") {
        Salix::ByteMirror::register_all_types(); YAML::Node entity,properties; properties["name"]="YamlCollider"; properties["size"]=Salix::Vector3(1.f,2.f,3.f); YAML::Node wrapper; wrapper["BoxCollider"]=properties; entity["elements"].push_back(wrapper);
        auto handles=Salix::ByteMirror::create_handles_for_yaml(&entity); std::vector<std::string> names; for(const auto& h:handles){REQUIRE(h!=nullptr); names.push_back(h->get_name());}
        CHECK(std::find(names.begin(),names.end(),"name")!=names.end()); CHECK(std::find(names.begin(),names.end(),"size")!=names.end()); CHECK(std::find(names.begin(),names.end(),"visible")==names.end());
    }
}

