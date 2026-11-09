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
        Salix::ByteMirror::register_all_types(); YAML::Node entity,properties; properties["name"]="YamlCollider"; properties["size"]["x"]=1.f; properties["size"]["y"]=2.f; properties["size"]["z"]=3.f; YAML::Node wrapper; wrapper["BoxCollider"]=properties; entity["elements"].push_back(wrapper);
        auto handles=Salix::ByteMirror::create_handles_for_yaml(&entity); std::vector<std::string> names; for(const auto& h:handles){REQUIRE(h!=nullptr); names.push_back(h->get_name());}
        CHECK(std::find(names.begin(),names.end(),"name")!=names.end()); CHECK(std::find(names.begin(),names.end(),"size")!=names.end()); CHECK(std::find(names.begin(),names.end(),"visible")==names.end());
    }
}

namespace {
class Batch2647UnregisteredElement final : public Salix::Element {
public:
    const char* get_class_name() const override { return "Batch2647UnregisteredElement"; }
};
}

TEST_SUITE("Salix::reflection::ByteMirrorBatch2647") {
    TEST_CASE("batch2647 registered Element type is retrievable directly by type index") {
        Salix::ByteMirror::register_all_types();
        const auto* info=Salix::ByteMirror::get_type_info(typeid(Salix::Element));
        REQUIRE(info!=nullptr); CHECK(info->name=="Element"); CHECK(info->ancestor==nullptr);
    }
    TEST_CASE("batch2647 unknown native type has no reflection metadata") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::get_type_info(typeid(int))==nullptr);
        CHECK(Salix::ByteMirror::get_type_info(typeid(Batch2647UnregisteredElement))==nullptr);
    }
    TEST_CASE("batch2647 type name lookup is exact and case sensitive") {
        Salix::ByteMirror::register_all_types();
        REQUIRE(Salix::ByteMirror::get_type_info_by_name("Transform")!=nullptr);
        CHECK(Salix::ByteMirror::get_type_info_by_name("transform")==nullptr);
        CHECK(Salix::ByteMirror::get_type_info_by_name(" Transform ")==nullptr);
    }
    TEST_CASE("batch2647 custom constructor registration creates requested concrete element") {
        Salix::ByteMirror::register_constructor("Batch2647Custom",[]()->Salix::Element*{return new Salix::Transform();});
        std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("Batch2647Custom"));
        REQUIRE(created!=nullptr); CHECK(std::string(created->get_class_name())=="Transform");
    }
    TEST_CASE("batch2647 constructor registration replacement affects future objects only") {
        Salix::ByteMirror::register_constructor("Batch2647Replace",[]()->Salix::Element*{return new Salix::Transform();});
        std::unique_ptr<Salix::Element> first(Salix::ByteMirror::create_element_by_name("Batch2647Replace")); REQUIRE(first!=nullptr);
        Salix::ByteMirror::register_constructor("Batch2647Replace",[]()->Salix::Element*{return new Salix::BoxCollider();});
        std::unique_ptr<Salix::Element> second(Salix::ByteMirror::create_element_by_name("Batch2647Replace")); REQUIRE(second!=nullptr);
        CHECK(std::string(first->get_class_name())=="Transform"); CHECK(std::string(second->get_class_name())=="BoxCollider");
    }
    TEST_CASE("batch2647 live handle creation rejects null and unregistered instances") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::create_handles_for(nullptr).empty());
        Batch2647UnregisteredElement custom;
        CHECK(Salix::ByteMirror::create_handles_for(&custom).empty());
    }
    TEST_CASE("batch2647 flattened Transform properties include own fields before inherited fields") {
        Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("Transform"); REQUIRE(info!=nullptr);
        const auto props=Salix::ByteMirror::get_all_properties_for_type(info); REQUIRE(props.size()>=6);
        CHECK(props[0].name=="position"); CHECK(props[1].name=="rotation"); CHECK(props[2].name=="scale");
        CHECK(batch1147_has_property(props,"id")); CHECK(batch1147_has_property(props,"name")); CHECK(batch1147_has_property(props,"visible"));
    }
    TEST_CASE("batch2647 reflected Sprite2D scalar values track latest live mutations") {
        Salix::ByteMirror::register_all_types(); Salix::Sprite2D sprite;
        sprite.set_local_rotation(73.5f); sprite.set_sorting_layer(-2647); sprite.set_flip_h(true);
        CHECK(std::get<float>(Salix::ByteMirror::get_property_value(&sprite,"local_rotation"))==doctest::Approx(73.5f));
        CHECK(std::get<int>(Salix::ByteMirror::get_property_value(&sprite,"sorting_layer"))==-2647);
        CHECK(std::get<bool>(Salix::ByteMirror::get_property_value(&sprite,"flip_h")));
    }
    TEST_CASE("batch2647 reflected inherited Element values remain visible on Sprite2D") {
        Salix::ByteMirror::register_all_types(); Salix::Sprite2D sprite; sprite.set_name("Sprite2647"); sprite.set_visibility(false);
        CHECK(std::get<std::string>(Salix::ByteMirror::get_property_value(&sprite,"name"))=="Sprite2647");
        CHECK_FALSE(std::get<bool>(Salix::ByteMirror::get_property_value(&sprite,"visible")));
        CHECK(std::get<std::uint64_t>(Salix::ByteMirror::get_property_value(&sprite,"id"))==sprite.get_id().get_value());
    }
    TEST_CASE("batch2647 YAML handle creation skips registered properties absent from node") {
        Salix::ByteMirror::register_all_types(); YAML::Node entity,properties,wrapper;
        properties["name"]="OnlyName2647"; wrapper["Transform"]=properties; entity["elements"].push_back(wrapper);
        auto handles=Salix::ByteMirror::create_handles_for_yaml(&entity); REQUIRE(handles.size()==1); REQUIRE(handles[0]!=nullptr); CHECK(handles[0]->get_name()=="name");
    }
}

TEST_SUITE("Salix::reflection::ByteMirrorBatch2947") {
    TEST_CASE("batch2947 repeated register all types remains idempotently queryable") {
        Salix::ByteMirror::register_all_types(); Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::get_type_info_by_name("Transform")!=nullptr); CHECK(Salix::ByteMirror::get_type_info_by_name("Sprite2D")!=nullptr);
    }
    TEST_CASE("batch2947 registered Transform constructor creates concrete transform") {
        Salix::ByteMirror::register_all_types(); std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("Transform"));
        REQUIRE(created!=nullptr); CHECK(dynamic_cast<Salix::Transform*>(created.get())!=nullptr);
    }
    TEST_CASE("batch2947 registered BoxCollider constructor creates concrete collider") {
        Salix::ByteMirror::register_all_types(); std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("BoxCollider"));
        REQUIRE(created!=nullptr); CHECK(dynamic_cast<Salix::BoxCollider*>(created.get())!=nullptr);
    }
    TEST_CASE("batch2947 registered Sprite2D constructor creates concrete sprite") {
        Salix::ByteMirror::register_all_types(); std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("Sprite2D"));
        REQUIRE(created!=nullptr); CHECK(dynamic_cast<Salix::Sprite2D*>(created.get())!=nullptr);
    }
    TEST_CASE("batch2947 registered CppScript constructor creates concrete script") {
        Salix::ByteMirror::register_all_types(); std::unique_ptr<Salix::Element> created(Salix::ByteMirror::create_element_by_name("CppScript"));
        REQUIRE(created!=nullptr); CHECK(dynamic_cast<Salix::CppScript*>(created.get())!=nullptr);
    }
    TEST_CASE("batch2947 Element flattened properties expose id name and visible") {
        Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("Element"); REQUIRE(info!=nullptr);
        const auto props=Salix::ByteMirror::get_all_properties_for_type(info); CHECK(batch1147_has_property(props,"id")); CHECK(batch1147_has_property(props,"name")); CHECK(batch1147_has_property(props,"visible"));
    }
    TEST_CASE("batch2947 Transform flattened properties include local transform and inherited metadata") {
        Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("Transform"); REQUIRE(info!=nullptr);
        const auto props=Salix::ByteMirror::get_all_properties_for_type(info); CHECK(batch1147_has_property(props,"position")); CHECK(batch1147_has_property(props,"rotation")); CHECK(batch1147_has_property(props,"scale")); CHECK(batch1147_has_property(props,"name"));
    }
    TEST_CASE("batch2947 live Transform handles contain each editable local transform property") {
        Salix::ByteMirror::register_all_types(); Salix::Transform transform; const auto handles=Salix::ByteMirror::create_handles_for(&transform); std::vector<std::string> names;
        for(const auto& handle:handles){REQUIRE(handle!=nullptr); names.push_back(handle->get_name());}
        CHECK(std::find(names.begin(),names.end(),"position")!=names.end()); CHECK(std::find(names.begin(),names.end(),"rotation")!=names.end()); CHECK(std::find(names.begin(),names.end(),"scale")!=names.end());
    }
    TEST_CASE("batch2947 reflected Transform position follows latest live mutation") {
        Salix::ByteMirror::register_all_types(); Salix::Transform transform; transform.set_position(Salix::Vector3(29.f,47.f,-3.f));
        const auto value=Salix::ByteMirror::get_property_value(&transform,"position"); REQUIRE(std::holds_alternative<Salix::Vector3>(value)); CHECK(std::get<Salix::Vector3>(value)==Salix::Vector3(29.f,47.f,-3.f));
    }
    TEST_CASE("batch2947 exact type lookup and name lookup point to same Transform metadata") {
        Salix::ByteMirror::register_all_types(); const auto* by_type=Salix::ByteMirror::get_type_info(typeid(Salix::Transform)); const auto* by_name=Salix::ByteMirror::get_type_info_by_name("Transform");
        REQUIRE(by_type!=nullptr); REQUIRE(by_name!=nullptr); CHECK(by_type==by_name);
    }
}

TEST_SUITE("Salix::reflection::ByteMirrorBatch3247") {
 TEST_CASE("batch3247 ProjectionMode reflection record is registered without ancestor") { Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info(typeid(Salix::ProjectionMode)); REQUIRE(info); CHECK(info->name=="Salix::ProjectionMode"); CHECK(info->ancestor==nullptr); CHECK(info->properties.empty()); }
 TEST_CASE("batch3247 Camera reflection record links Element ancestor") { Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("Camera"); REQUIRE(info); REQUIRE(info->ancestor); CHECK(info->ancestor->name=="Element"); }
 TEST_CASE("batch3247 Camera flattened properties expose projection and inherited metadata") { Salix::ByteMirror::register_all_types(); const auto* info=Salix::ByteMirror::get_type_info_by_name("Camera"); REQUIRE(info); const auto props=Salix::ByteMirror::get_all_properties_for_type(info); CHECK(batch1147_has_property(props,"projection_mode")); CHECK(batch1147_has_property(props,"field_of_view")); CHECK(batch1147_has_property(props,"name")); CHECK(batch1147_has_property(props,"visible")); }
 TEST_CASE("batch3247 CppScript type lookup by type and name shares metadata address") { Salix::ByteMirror::register_all_types(); const auto* a=Salix::ByteMirror::get_type_info(typeid(Salix::CppScript)); const auto* b=Salix::ByteMirror::get_type_info_by_name("CppScript"); REQUIRE(a); REQUIRE(b); CHECK(a==b); }
 TEST_CASE("batch3247 BoxCollider type lookup by type and name shares metadata address") { Salix::ByteMirror::register_all_types(); const auto* a=Salix::ByteMirror::get_type_info(typeid(Salix::BoxCollider)); const auto* b=Salix::ByteMirror::get_type_info_by_name("BoxCollider"); REQUIRE(a); REQUIRE(b); CHECK(a==b); }
 TEST_CASE("batch3247 live CppScript handles expose runtime script name") { Salix::ByteMirror::register_all_types(); Salix::CppScript s; auto handles=Salix::ByteMirror::create_handles_for(&s); auto it=std::find_if(handles.begin(),handles.end(),[](const auto& h){return h&&h->get_name()=="script_name";}); REQUIRE(it!=handles.end()); (*it)->set_value(Salix::PropertyValue{std::string("Runtime3247")}); CHECK(s.get_script_name()=="Runtime3247"); }
 TEST_CASE("batch3247 reflected Camera field of view tracks latest live mutation") { Salix::ByteMirror::register_all_types(); Salix::Camera c; c.set_field_of_view(73.25f); const auto v=Salix::ByteMirror::get_property_value(&c,"field_of_view"); REQUIRE(std::holds_alternative<float>(v)); CHECK(std::get<float>(v)==doctest::Approx(73.25f)); }
 TEST_CASE("batch3247 reflected Element visibility tracks latest live mutation") { Salix::ByteMirror::register_all_types(); Salix::BoxCollider c; c.set_visibility(false); auto v=Salix::ByteMirror::get_property_value(&c,"visible"); REQUIRE(std::holds_alternative<bool>(v)); CHECK_FALSE(std::get<bool>(v)); c.set_visibility(true); v=Salix::ByteMirror::get_property_value(&c,"visible"); CHECK(std::get<bool>(v)); }
 TEST_CASE("batch3247 custom constructor may intentionally return null") { Salix::ByteMirror::register_constructor("Batch3247Null",[]()->Salix::Element*{return nullptr;}); CHECK(Salix::ByteMirror::create_element_by_name("Batch3247Null")==nullptr); }
 TEST_CASE("batch3247 YAML unknown element is skipped while known element property survives") { Salix::ByteMirror::register_all_types(); YAML::Node entity,known,unknown; known["Transform"]["position"]["x"]=1.f; known["Transform"]["position"]["y"]=2.f; known["Transform"]["position"]["z"]=3.f; unknown["Batch3247Unknown"]["name"]="Ignored"; entity["elements"].push_back(unknown); entity["elements"].push_back(known); const auto handles=Salix::ByteMirror::create_handles_for_yaml(&entity); REQUIRE(handles.size()==1); REQUIRE(handles[0]); CHECK(handles[0]->get_name()=="position"); }
}
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/ScriptElement.h>

TEST_SUITE("Salix::reflection::ByteMirrorBatch3647") {
    TEST_CASE("batch3647 RenderableElement metadata links directly to Element with no local properties") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::RenderableElement));
        REQUIRE(info != nullptr);
        CHECK(info->name == "RenderableElement");
        REQUIRE(info->ancestor != nullptr);
        CHECK(info->ancestor->name == "Element");
        CHECK(info->properties.empty());
    }

    TEST_CASE("batch3647 RenderableElement2D metadata links to RenderableElement") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::RenderableElement2D));
        REQUIRE(info != nullptr);
        CHECK(info->name == "RenderableElement2D");
        REQUIRE(info->ancestor != nullptr);
        CHECK(info->ancestor->name == "RenderableElement");
        CHECK(info->properties.empty());
    }

    TEST_CASE("batch3647 ScriptElement metadata links directly to Element") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::ScriptElement));
        REQUIRE(info != nullptr);
        CHECK(info->name == "ScriptElement");
        REQUIRE(info->ancestor != nullptr);
        CHECK(info->ancestor->name == "Element");
    }

    TEST_CASE("batch3647 CppScript metadata links ScriptElement and exposes string script name") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::CppScript));
        REQUIRE(info != nullptr);
        REQUIRE(info->ancestor != nullptr);
        CHECK(info->ancestor->name == "ScriptElement");
        const auto it = std::find_if(info->properties.begin(), info->properties.end(),
            [](const Salix::Property& property) { return property.name == "script_name"; });
        REQUIRE(it != info->properties.end());
        CHECK(it->type == Salix::PropertyType::String);
    }

    TEST_CASE("batch3647 Sprite2D metadata preserves complete renderable ancestor chain") {
        Salix::ByteMirror::register_all_types();
        const auto* sprite = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        REQUIRE(sprite != nullptr);
        REQUIRE(sprite->ancestor != nullptr);
        CHECK(sprite->ancestor->name == "RenderableElement2D");
        REQUIRE(sprite->ancestor->ancestor != nullptr);
        CHECK(sprite->ancestor->ancestor->name == "RenderableElement");
        REQUIRE(sprite->ancestor->ancestor->ancestor != nullptr);
        CHECK(sprite->ancestor->ancestor->ancestor->name == "Element");
    }

    TEST_CASE("batch3647 Sprite2D derived property list is exactly width then height") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        REQUIRE(info != nullptr);
        REQUIRE(info->derived_properties.size() == 2);
        CHECK(info->derived_properties[0] == "width");
        CHECK(info->derived_properties[1] == "height");
    }

    TEST_CASE("batch3647 Sprite2D texture path property carries image file UI hint") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        REQUIRE(info != nullptr);
        const auto it = std::find_if(info->properties.begin(), info->properties.end(),
            [](const Salix::Property& property) { return property.name == "texture_path"; });
        REQUIRE(it != info->properties.end());
        CHECK(it->type == Salix::PropertyType::String);
        CHECK(it->hint == Salix::UIHint::ImageFile);
    }

    TEST_CASE("batch3647 Sprite2D width and height metadata are integer read only properties") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::Sprite2D));
        REQUIRE(info != nullptr);
        for (const std::string name : {"width", "height"}) {
            const auto it = std::find_if(info->properties.begin(), info->properties.end(),
                [&](const Salix::Property& property) { return property.name == name; });
            REQUIRE(it != info->properties.end());
            CHECK(it->type == Salix::PropertyType::Int);
            CHECK(it->is_read_only);
        }
    }

    TEST_CASE("batch3647 Camera projection mode metadata points to registered ProjectionMode type") {
        Salix::ByteMirror::register_all_types();
        const auto* camera = Salix::ByteMirror::get_type_info(typeid(Salix::Camera));
        const auto* projection = Salix::ByteMirror::get_type_info(typeid(Salix::ProjectionMode));
        REQUIRE(camera != nullptr); REQUIRE(projection != nullptr);
        const auto it = std::find_if(camera->properties.begin(), camera->properties.end(),
            [](const Salix::Property& property) { return property.name == "projection_mode"; });
        REQUIRE(it != camera->properties.end());
        CHECK(it->type == Salix::PropertyType::EnumClass);
        CHECK(it->contained_type_info == projection);
    }

    TEST_CASE("batch3647 Camera view and projection matrices are advertised as read only GLM matrices") {
        Salix::ByteMirror::register_all_types();
        const auto* camera = Salix::ByteMirror::get_type_info(typeid(Salix::Camera));
        REQUIRE(camera != nullptr);
        for (const std::string name : {"view_matrix", "projection_matrix"}) {
            const auto it = std::find_if(camera->properties.begin(), camera->properties.end(),
                [&](const Salix::Property& property) { return property.name == name; });
            REQUIRE(it != camera->properties.end());
            CHECK(it->type == Salix::PropertyType::GlmMat4);
            CHECK(it->is_read_only);
        }
    }
}

TEST_SUITE("Salix::reflection::ByteMirrorBatch4047") {
    TEST_CASE("batch4047 null type metadata flattens to an empty property list") {
        CHECK(Salix::ByteMirror::get_all_properties_for_type(nullptr).empty());
    }

    TEST_CASE("batch4047 null live element creates no property handles") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::create_handles_for(nullptr).empty());
    }

    TEST_CASE("batch4047 unregistered live element creates no property handles") {
        Salix::ByteMirror::register_all_types();
        Batch1147UnregisteredElement element;
        CHECK(Salix::ByteMirror::create_handles_for(&element).empty());
    }

    TEST_CASE("batch4047 empty and unknown type names remain safe misses") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::get_type_info_by_name("") == nullptr);
        CHECK(Salix::ByteMirror::get_type_info_by_name("Batch4047MissingType") == nullptr);
    }

    TEST_CASE("batch4047 unknown element constructor lookup remains null") {
        Salix::ByteMirror::register_all_types();
        CHECK(Salix::ByteMirror::create_element_by_name("Batch4047MissingConstructor") == nullptr);
    }

    TEST_CASE("batch4047 custom constructor can create a concrete collider") {
        Salix::ByteMirror::register_constructor("Batch4047Collider",
            []() -> Salix::Element* { return new Salix::BoxCollider(); });
        std::unique_ptr<Salix::Element> created(
            Salix::ByteMirror::create_element_by_name("Batch4047Collider"));
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Salix::BoxCollider*>(created.get()) != nullptr);
    }

    TEST_CASE("batch4047 Element metadata flattens to exactly its three local properties") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::Element));
        REQUIRE(info != nullptr);
        const auto properties = Salix::ByteMirror::get_all_properties_for_type(info);
        CHECK(properties.size() == 3);
        CHECK(batch1147_has_property(properties, "id"));
        CHECK(batch1147_has_property(properties, "name"));
        CHECK(batch1147_has_property(properties, "visible"));
    }

    TEST_CASE("batch4047 BoxCollider flattened metadata contains size plus inherited identity") {
        Salix::ByteMirror::register_all_types();
        const auto* info = Salix::ByteMirror::get_type_info(typeid(Salix::BoxCollider));
        REQUIRE(info != nullptr);
        const auto properties = Salix::ByteMirror::get_all_properties_for_type(info);
        CHECK(batch1147_has_property(properties, "size"));
        CHECK(batch1147_has_property(properties, "id"));
        CHECK(batch1147_has_property(properties, "name"));
        CHECK(batch1147_has_property(properties, "visible"));
    }

    TEST_CASE("batch4047 live collider handles expose local and inherited properties together") {
        Salix::ByteMirror::register_all_types();
        Salix::BoxCollider collider;
        const auto handles = Salix::ByteMirror::create_handles_for(&collider);
        std::vector<std::string> names;
        for (const auto& handle : handles) {
            REQUIRE(handle != nullptr);
            names.push_back(handle->get_name());
        }
        CHECK(std::find(names.begin(), names.end(), "size") != names.end());
        CHECK(std::find(names.begin(), names.end(), "id") != names.end());
        CHECK(std::find(names.begin(), names.end(), "name") != names.end());
        CHECK(std::find(names.begin(), names.end(), "visible") != names.end());
    }

    TEST_CASE("batch4047 reflected collider size follows latest live mutation") {
        Salix::ByteMirror::register_all_types();
        Salix::BoxCollider collider;
        collider.set_size({4.0f, -0.5f, 40.47f});
        const auto value = Salix::ByteMirror::get_property_value(&collider, "size");
        REQUIRE(std::holds_alternative<Salix::Vector3>(value));
        CHECK(std::get<Salix::Vector3>(value) == Salix::Vector3(4.0f, -0.5f, 40.47f));
    }
}
