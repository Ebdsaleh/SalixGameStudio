// Editor/ArchetypeInstantiator.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/core/InitContext.h>

namespace Salix {

    void ArchetypeInstantiator::instantiate(const Salix::EntityArchetype& archetype, Salix::Scene* scene, const Salix::InitContext& context) {
        if (!scene) return;

        // 1. Create the live entity. Its constructor adds the default Transform and BoxCollider.
        Salix::Entity* live_entity = scene->create_entity(archetype.id, archetype.name);

        // Get the default elements that were just created.
        Element* default_transform = live_entity->get_element_by_type_name("Transform");
        Element* default_box_collider = live_entity->get_element_by_type_name("BoxCollider");

        bool has_applied_default_transform = false;
        bool has_applied_default_collider = false;

        // 2. Loop through all element archetypes to instantiate them.
        for (const auto& element_archetype : archetype.elements) {
            Element* live_element = nullptr;

            // --- CORRECTED LOGIC ---
            // Check if this archetype is for a default element that we haven't processed yet.
            if (element_archetype.type_name == "Transform" && !has_applied_default_transform) {
                live_element = default_transform;
                has_applied_default_transform = true; // Mark as processed
            }
            else if (element_archetype.type_name == "BoxCollider" && !has_applied_default_collider) {
                live_element = default_box_collider;
                has_applied_default_collider = true; // Mark as processed
            }
            else {
                // For ALL other elements (including duplicate BoxColliders or Sprite2Ds), create them new.
                live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
                if (live_element) {
                    live_entity->add_element(live_element);
                }
            }

            if (!live_element) continue; // Skip if element couldn't be found or created.

            // 3. Use reflection to set the properties on the live_element.
            const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
            if (!type_info) continue;

            for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                const YAML::Node& property_node = element_archetype.data[prop.name];
                if (!property_node) continue;

                // This switch uses reflection-driven setters to apply the data
                switch (prop.type) {
                    case Salix::PropertyType::Vector3: {
                        auto value = property_node.as<Salix::Vector3>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Vector2: {
                        auto value = property_node.as<Salix::Vector2>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Color: {
                        auto value = property_node.as<Salix::Color>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Float: {
                        auto value = property_node.as<float>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Int: {
                        auto value = property_node.as<int>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Bool: {
                        auto value = property_node.as<bool>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::String: {
                        auto value = property_node.as<std::string>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Enum: {
                        auto value = property_node.as<int>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::EnumClass: {
                        if (prop.contained_type_info && prop.contained_type_info->type_index.has_value()) {
                            const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*prop.contained_type_info->type_index);
                            if (enum_data) {
                                std::string string_value = property_node.as<std::string>();
                                int int_value = enum_data->get_value(string_value);
                                prop.set_data(live_element, &int_value);
                            }
                        }
                        break;
                    }
                    case Salix::PropertyType::UInt64: {
                        auto value = property_node.as<uint64_t>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Point: {
                        auto value = property_node.as<Salix::Point>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    case Salix::PropertyType::Rect: {
                        auto value = property_node.as<Salix::Rect>();
                        prop.set_data(live_element, &value);
                        break;
                    }
                    // Add cases for all other reflected property types...
                }
            }
            
            live_element->on_load(context);
        }
        
        live_entity->on_load(context);
    }
}  // namespace Salix