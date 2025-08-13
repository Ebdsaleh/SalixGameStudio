// Editor/ArchetypeFactory.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeFactory.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
// Add other property type includes as needed...

namespace Salix {

    ElementArchetype ArchetypeFactory::create_element_archetype(const std::string& type_name) {
        // 1. Create a temporary live instance using the ByteMirror component factory.
        Element* temp_element = ByteMirror::create_element_by_name(type_name);
        if (!temp_element) {
            return {}; // Return empty archetype on failure
        }

        // 2. Get the reflection info for this type.
        const TypeInfo* type_info = ByteMirror::get_type_info(typeid(*temp_element));
        if (!type_info) {
            delete temp_element;
            return {};
        }

        ElementArchetype archetype;
        archetype.type_name = type_name;
        archetype.id = SimpleGuid::generate();

        archetype.name = type_name;                 // Set the struct member for the UI
        archetype.data["name"] = type_name;         // Set the data node for the reflection system


        // 3. Iterate through all reflected properties to get their default values.
        for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
            void* data_ptr = prop.get_data(temp_element);
            
            // 4. Read the default value from the temporary object and write it to the archetype's data.
            switch (prop.type) {
                case PropertyType::Int:       archetype.data[prop.name] = *static_cast<int*>(data_ptr); break;
                case PropertyType::Float:     archetype.data[prop.name] = *static_cast<float*>(data_ptr); break;
                case PropertyType::Bool:      archetype.data[prop.name] = *static_cast<bool*>(data_ptr); break;
                case PropertyType::String:    archetype.data[prop.name] = *static_cast<std::string*>(data_ptr); break;
                case PropertyType::Vector2:   archetype.data[prop.name] = *static_cast<Vector2*>(data_ptr); break;
                case PropertyType::Vector3:   archetype.data[prop.name] = *static_cast<Vector3*>(data_ptr); break;
                case PropertyType::Color:     archetype.data[prop.name] = *static_cast<Color*>(data_ptr); break;
                case PropertyType::EnumClass: {
                    const EnumRegistry::EnumData* enum_data = EnumRegistry::get_enum_data_as_ptr(*prop.contained_type_info->type_index);
                    if (enum_data) {
                        int value = *static_cast<int*>(data_ptr);
                        archetype.data[prop.name] = enum_data->get_name(value); // Store as string
                    }
                    break;
                }
                // Will add more property types here later...
            }
        }
        
        // 5. Clean up the temporary instance.
        delete temp_element;

        return archetype;
    }

    EntityArchetype ArchetypeFactory::create_entity_archetype(const std::string& name) {
        EntityArchetype archetype;
        archetype.id = SimpleGuid::generate();
        archetype.name = name;
        
        // All new entities should have a Transform component by default.
        archetype.elements.push_back(create_element_archetype("Transform"));
        archetype.elements.push_back(create_element_archetype("BoxCollider"));
        return archetype;
    }
}