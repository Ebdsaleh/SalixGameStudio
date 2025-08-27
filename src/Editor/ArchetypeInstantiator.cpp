// Editor/ArchetypeInstantiator.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/core/InitContext.h>
#include <vector>

namespace Salix {

    void ArchetypeInstantiator::instantiate(const Salix::EntityArchetype& archetype, Salix::Scene* scene, const Salix::InitContext& context) {
        if (!scene) return;
        
        // 1. Create the live entity. Its constructor adds the default Transform and BoxCollider.
        Salix::Entity* live_entity = scene->create_entity(archetype.id, archetype.name);
        // Debug prints
        std::cout << "[DEBUG] --- Instantiating Archetype: " << archetype.name << " (Archetype ID: " << archetype.id.get_value() << ") ---" << std::endl;
        std::cout << "[DEBUG] >>> Live Entity created with ID: " << live_entity->get_id().get_value() << std::endl;

        // Get the default elements that were just created.
        Element* default_transform = live_entity->get_element_by_type_name("Transform");
        Element* default_box_collider = live_entity->get_element_by_type_name("BoxCollider");

        bool has_applied_default_transform = false;
        bool has_applied_default_collider = false;

        // 2. Loop through all element archetypes to instantiate them.
        for (const auto& element_archetype : archetype.elements) {
            Element* live_element = nullptr;

            // DEBUG PRINTS
            if (live_element) {
                std::cout << "[DEBUG]  - Archetype Element '" << element_archetype.name << "' (Archetype ID: " << element_archetype.id.get_value() << ")" << std::endl;
                std::cout << "[DEBUG]    >>> Live Element '" << live_element->get_class_name() << "' created with ID: " << live_element->get_id().get_value() << std::endl;
            }
            // --- CORRECTED LOGIC ---
            // Check if this archetype is for a default element that we haven't processed yet.
            if (element_archetype.type_name == "Transform" && !has_applied_default_transform) {
                live_element = default_transform;
                live_element->set_id(element_archetype.id);
                has_applied_default_transform = true; // Mark as processed
            }
            else if (element_archetype.type_name == "BoxCollider" && !has_applied_default_collider) {
                live_element = default_box_collider;
                live_element->set_id(element_archetype.id);
                has_applied_default_collider = true; // Mark as processed
            }
            else {
                // For ALL other elements (including duplicate BoxColliders or Sprite2Ds), create them new.
                live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
                if (live_element) {
                    live_entity->add_element(live_element);

                    // This synchronizes the live element's ID with its archetype's ID.
                    live_element->set_id(element_archetype.id);
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
            // --- NEW: Write derived data back to the archetype ---
                if (auto* live_sprite = dynamic_cast<Sprite2D*>(live_element)) {
                // If the element is a Sprite2D, update its archetype's data node
                // with the actual dimensions from the loaded texture.
                element_archetype.data["width"].as<int>(live_sprite->get_texture_width());
                element_archetype.data["height"].as<int>(live_sprite->get_texture_height());
            }
        }
        
        live_entity->on_load(context);

        
        
    }

    void ArchetypeInstantiator::print_all_entity_ids(Salix::Scene* scene, const std::string& context_message) {
        if (!scene) {
            std::cout << "DEBUG [" << context_message << "]: Scene is nullptr." << std::endl;
            return;
        }

        std::vector<Entity*> entities = scene->get_entities();
        std::cout << "DEBUG [" << context_message << "]: Scene contains " << entities.size() << " entities." << std::endl;

        for (const auto* entity : entities) {
            if (entity) {
                std::cout << "  - Entity: '" << entity->get_name() << "' (ID: " << entity->get_id().get_value() << ")" << std::endl;
            }
        }
    }

    // Replace your old 'instantiate' method with this new 'instantiate_realm' method.

    void ArchetypeInstantiator::instantiate_realm(const std::vector<Salix::EntityArchetype>& realm, Salix::Scene* scene, const Salix::InitContext& context) {
        if (!scene) return;
        
        // A map for fast lookup of live entities by their archetype ID
        std::map<SimpleGuid, Entity*> live_entity_map;

        // --- PASS 1: Instantiate all entities and their elements ---
        for (const auto& archetype : realm) {
            Entity* live_entity = scene->create_entity(archetype.id, archetype.name);
            live_entity->set_visible(archetype.is_visible);
            live_entity_map[archetype.id] = live_entity;

            Element* default_transform = live_entity->get_element_by_type_name("Transform");
            Element* default_box_collider = live_entity->get_element_by_type_name("BoxCollider");
            bool has_applied_default_transform = false;
            bool has_applied_default_collider = false;

            for (const auto& element_archetype : archetype.elements) {
                Element* live_element = nullptr;
                if (element_archetype.type_name == "Transform" && !has_applied_default_transform) {
                    live_element = default_transform;
                    live_element->set_visibility(false);
                    live_element->set_id(element_archetype.id);
                    has_applied_default_transform = true;
                }
                else if (element_archetype.type_name == "BoxCollider" && !has_applied_default_collider) {
                    live_element = default_box_collider;
                    live_element->set_visibility(element_archetype.is_visible);
                    live_element->set_id(element_archetype.id);
                    has_applied_default_collider = true;
                }
                else {
                    live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
                    if (live_element) {
                        live_element->set_visibility(element_archetype.is_visible);
                        live_entity->add_element(live_element);
                        live_element->set_id(element_archetype.id);
                    }
                }

                if (!live_element) continue;

                const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
                if (!type_info) continue;

                for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                    const YAML::Node& property_node = element_archetype.data[prop.name];
                    if (!property_node) continue;
                    // This is a simplified version of the property setting logic from your on_event handler
                    std::visit([&](auto&& arg) {
                        auto value_copy = arg;
                        prop.set_data(live_element, &value_copy);
                    }, YAML::node_to_property_value(property_node, prop));
                }
                live_element->on_load(context);
            }
            live_entity->on_load(context);
        }

        // --- PASS 2: Set up the parent-child hierarchy ---
        for (const auto& archetype : realm) {
            if (archetype.parent_id.is_valid()) {
                Entity* child_entity = live_entity_map.count(archetype.id) ? live_entity_map.at(archetype.id) : nullptr;
                Entity* parent_entity = live_entity_map.count(archetype.parent_id) ? live_entity_map.at(archetype.parent_id) : nullptr;

                if (child_entity && parent_entity) {
                    child_entity->set_parent(parent_entity);
                }
            }
        }
    }
}  // namespace Salix