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

        // 1. Create the base live entity. Its constructor gives it a default Transform and BoxCollider.
        Salix::Entity* live_entity = scene->create_entity(archetype.id, archetype.name);

        // 2. Loop through the element archetypes from the YAML data.
        for (const auto& element_archetype : archetype.elements) {
            
            Salix::Element* live_element = nullptr;

            // For mandatory components, get the one created by the constructor.
            if (element_archetype.type_name == "Transform") {
                live_element = live_entity->get_element_by_type_name("Transform");
            } 
            else if (element_archetype.type_name == "BoxCollider") {
                live_element = live_entity->get_element_by_type_name("BoxCollider");
            } 
            else {
                // For all other optional components, create them new using the factory.
                live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
                if (live_element) {
                    // IMPORTANT: Only add the element to the entity if it's a new one.
                    live_entity->add_element(live_element);
                }
            }

            if (!live_element) continue; // Skip if element couldn't be found or created.
            
            // 3. Use reflection to set the properties on the correct live_element.
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
} // namespace Salix