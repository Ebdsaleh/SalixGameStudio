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



    EntityArchetype ArchetypeFactory::duplicate_entity_archetype(const EntityArchetype& source, 
        const std::vector<EntityArchetype>& all_archetypes) {
    
        EntityArchetype new_archetype;

        // 1. Determine the base name for the copy.
        // This finds the root name if we are copying an existing copy (e.g., "Player" from "Player (Copy)").
        std::string base_name = source.name;
        size_t copy_pos = base_name.find(" (Copy");
        if (copy_pos != std::string::npos) {
            base_name = base_name.substr(0, copy_pos);
        }
        
        // 2. Generate and check for a unique name.
        std::string potential_name = base_name + " (Copy)";
        int copy_number = 1;

        // This loop continues until a unique name is found.
        bool name_is_unique = false;
        while (!name_is_unique) {
            bool name_found = false;
            // Check against all existing archetypes.
            for (const auto& existing_archetype : all_archetypes) {
                if (existing_archetype.name == potential_name) {
                    name_found = true;
                    break;
                }
            }

            if (name_found) {
                // If the name exists, append the next number and try again.
                potential_name = base_name + " (Copy " + std::to_string(copy_number++) + ")";
            } else {
                // If we looped through all entities and didn't find the name, it's unique.
                name_is_unique = true;
            }
        }
        
        new_archetype.name = potential_name;
        new_archetype.id = SimpleGuid::generate();

        // 3. Perform a deep copy of all elements, giving each a new ID.
        for (const auto& source_element : source.elements) {
            ElementArchetype new_element;
            new_element.type_name = source_element.type_name;
            new_element.name = source_element.name;
            new_element.id = SimpleGuid::generate();
            new_element.data = YAML::Clone(source_element.data);
            new_archetype.elements.push_back(new_element);
        }

        return new_archetype;
        
    }
}