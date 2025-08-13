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

        // 1. Create the base live entity
        Salix::Entity* live_entity = scene->create_entity(archetype.id, archetype.name);

        // 2. Loop through the element archetypes
        for (const auto& element_archetype : archetype.elements) {
            
            // 3. Use the ByteMirror factory to create a live element instance
            Salix::Element* live_element = Salix::ByteMirror::create_element_by_name(element_archetype.type_name);
            if (!live_element) continue; // Skip unknown elements

            // 4. Add the element to the entity
            live_entity->add_element(live_element);

            // 5. Use reflection to set the properties from the YAML data
            const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
            if (!type_info) continue;

            for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                const YAML::Node& property_node = element_archetype.data[prop.name];
                if (!property_node) continue; // Skip properties not in the YAML file

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
                        // Use the EnumRegistry to convert the string to an int.
                        if (prop.contained_type_info && prop.contained_type_info->type_index.has_value()) {
                            const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*prop.contained_type_info->type_index);
                            if (enum_data) {
                                // 1. Read the value from YAML as a string.
                                std::string string_value = property_node.as<std::string>();
                                // 2. Use the registry to find the corresponding integer value.
                                int int_value = enum_data->get_value(string_value);
                                // 3. Set the integer value on the live component.
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
        
    }
} // namespace Salix