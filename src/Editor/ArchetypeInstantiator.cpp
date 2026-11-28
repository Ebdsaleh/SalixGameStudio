// Editor/ArchetypeInstantiator.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/core/InitContext.h>
#include <vector>
#include <optional>

namespace Salix {

    namespace {
        void apply_authored_properties(Element* element, const ElementArchetype& archetype, const TypeInfo* type_info) {
            for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                if (prop.is_read_only || !prop.set_data) continue;
                const YAML::Node node = archetype.data[prop.name];
                if (!node) continue;

                // The generic YAML converter returns a default int for unsupported
                // types. Never pass that placeholder to an unrelated typed setter.
                std::optional<PropertyValue> value;
                switch (prop.type) {
                    case PropertyType::Enum:
                        value = node.as<int>();
                        break;
                    case PropertyType::EnumClass: {
                        if (!prop.contained_type_info || !prop.contained_type_info->type_index)
                            break;
                        const auto* names = EnumRegistry::get_enum_data_as_ptr(*prop.contained_type_info->type_index);
                        if (!names) break;
                        const auto name = node.as<std::string>();
                        for (const auto& entry : names->value_to_string) {
                            if (entry.second == name) {
                                value = entry.first;
                                break;
                            }
                        }
                        // An unrecognized name leaves the component's default intact.
                        break;
                    }
                    case PropertyType::Class:
                    case PropertyType::Struct:
                        break;
                    default:
                        value = YAML::node_to_property_value(node, prop);
                        break;
                }
                if (value)
                    std::visit([&](auto& typed_value) { prop.set_data(element, &typed_value); }, *value);
            }
        }
    }

    void ArchetypeInstantiator::instantiate(const Salix::EntityArchetype& archetype, Salix::Realm* realm, const Salix::InitContext& context) {
        if (!realm) return;
        
        // 1. Create the live entity. Its constructor adds the default Transform and BoxCollider.
        Salix::Entity* live_entity = realm->create_entity(archetype.id, archetype.name);
        live_entity->set_visible(archetype.is_visible);
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
            live_element->set_visibility(element_archetype.is_visible);
            // DEBUG PRINTS
            if (live_element) {
                std::cout << "[DEBUG]  - Archetype Element '" << element_archetype.name << "' (Archetype ID: " << element_archetype.id.get_value() << ")" << std::endl;
                std::cout << "[DEBUG]    >>> Live Element '" << live_element->get_class_name() << "' created with ID: " << live_element->get_id().get_value() << std::endl;
            }

            // 3. Use reflection to set the properties on the live_element.
            const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
            if (!type_info) continue;

            apply_authored_properties(live_element, element_archetype, type_info);

            live_element->on_load(context);
        }
        
        live_entity->on_load(context);

        
        
    }

    void ArchetypeInstantiator::print_all_entity_ids(Salix::Realm* realm, const std::string& context_message) {
        if (!realm) {
            std::cout << "DEBUG [" << context_message << "]: Scene is nullptr." << std::endl;
            return;
        }

        std::vector<Entity*> entities = realm->get_entities();
        std::cout << "DEBUG [" << context_message << "]: Scene contains " << entities.size() << " entities." << std::endl;

        for (const auto* entity : entities) {
            if (entity) {
                std::cout << "  - Entity: '" << entity->get_name() << "' (ID: " << entity->get_id().get_value() << ")" << std::endl;
            }
        }
    }

   
    void ArchetypeInstantiator::instantiate_realm(const std::vector<Salix::EntityArchetype>& archetype_realm, Salix::Realm* realm, const Salix::InitContext& context) {
    if (!realm) return;
    
    std::map<SimpleGuid, Entity*> live_entity_map;

    // --- PASS 1: Create all entities and their corresponding elements ---
    // We create the raw objects but do NOT apply any properties yet.
    for (const auto& archetype : archetype_realm) {
        Entity* live_entity = realm->create_entity(archetype.id, archetype.name);
        live_entity_map[archetype.id] = live_entity;

        Element* default_transform = live_entity->get_element_by_type_name("Transform");
        Element* default_box_collider = live_entity->get_element_by_type_name("BoxCollider");
        bool has_applied_default_transform = false;
        bool has_applied_default_collider = false;

        for (const auto& element_archetype : archetype.elements) {
            if (element_archetype.type_name == "Transform" && !has_applied_default_transform) {
                default_transform->set_id(element_archetype.id);
                has_applied_default_transform = true;
            } else if (element_archetype.type_name == "BoxCollider" && !has_applied_default_collider) {
                default_box_collider->set_id(element_archetype.id);
                has_applied_default_collider = true;
            } else {
                Element* live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
                if (live_element) {
                    live_element->set_id(element_archetype.id);
                    live_entity->add_element(live_element);
                }
            }
        }
    }

    // --- PASS 2: Apply all properties from archetypes to the live objects ---
    // Now that all objects exist, we can safely set their data (position, rotation, etc.).
    for (const auto& archetype : archetype_realm) {
        Entity* live_entity = live_entity_map.at(archetype.id);
        live_entity->set_visible(archetype.is_visible);

        for (const auto& element_archetype : archetype.elements) {
            Element* live_element = live_entity->get_element_by_id(element_archetype.id);
            if (!live_element) continue;
            
            live_element->set_visibility(element_archetype.is_visible);
            
            const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
            if (!type_info) continue;

            apply_authored_properties(live_element, element_archetype, type_info);
            live_element->on_load(context);
        }
        live_entity->on_load(context);
    }

    // --- PASS 3: Set up the parent-child hierarchy ---
    // This is the final step. Now that all transforms have their correct local values,
    // the set_parent function can perform its math correctly on a fully-formed scene.
    for (const auto& archetype : archetype_realm) {
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