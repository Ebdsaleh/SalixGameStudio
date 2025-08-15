// Editor/ArchetypeFactory.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeFactory.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/ecs/Element.h>
#include <vector>
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
        archetype.state = ArchetypeState::New;

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
        archetype.state = ArchetypeState::New;
        ElementArchetype transform_archetype = create_element_archetype("Transform");
        ElementArchetype box_collider_archetype = create_element_archetype("BoxCollider"); 
        transform_archetype.allows_duplication = false;
        box_collider_archetype.allows_duplication = false;
        // All new entities should have a Transform component by default.
        archetype.elements.push_back(transform_archetype);
        archetype.elements.push_back(box_collider_archetype);
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
        new_archetype.state = ArchetypeState::New;
        // 3. Perform a deep copy of all elements, giving each a new ID.
        for (const auto& source_element : source.elements) {
            ElementArchetype new_element;
            new_element.type_name = source_element.type_name;
            new_element.name = source_element.name;
            new_element.id = SimpleGuid::generate();
            new_element.data = YAML::Clone(source_element.data);
            new_element.allows_duplication = source_element.allows_duplication;
            new_element.state = ArchetypeState::New;
            new_archetype.elements.push_back(new_element);
        }

        return new_archetype;
        
    }


    // This is the private recursive helper function's implementation.
    void ArchetypeFactory::duplicate_recursive_helper(
        const SimpleGuid& source_id,
        const std::vector<EntityArchetype>& all_archetypes,
        std::vector<EntityArchetype>& new_family,
        std::map<SimpleGuid, SimpleGuid>& id_map) 
    {
        auto it = std::find_if(all_archetypes.begin(), all_archetypes.end(),
            [&](const EntityArchetype& e) { return e.id == source_id; });
        if (it == all_archetypes.end()) return;

        const EntityArchetype& source_archetype = *it;

        EntityArchetype new_archetype;
        new_archetype.name = source_archetype.name;
        new_archetype.id = SimpleGuid::generate();
        new_archetype.state = ArchetypeState::New;
        id_map[source_archetype.id] = new_archetype.id;

        for (const auto& source_element : source_archetype.elements) {
            ElementArchetype new_element;
            new_element.type_name = source_element.type_name;
            new_element.name = source_element.name;
            new_element.id = SimpleGuid::generate();
            new_element.data = YAML::Clone(source_element.data);
            new_element.state = ArchetypeState::New;
            new_archetype.elements.push_back(new_element);
        }

        new_family.push_back(new_archetype);

        for (const auto& child_id : source_archetype.child_ids) {
            duplicate_recursive_helper(child_id, all_archetypes, new_family, id_map);
        }
    }

    std::vector<EntityArchetype> ArchetypeFactory::duplicate_entity_archetype_and_children(
        const EntityArchetype& source,
        const std::vector<EntityArchetype>& all_archetypes) {
        std::vector<EntityArchetype> new_family;
        std::map<SimpleGuid, SimpleGuid> id_map;
        duplicate_recursive_helper(source.id, all_archetypes, new_family, id_map);

        for (auto& new_archetype : new_family) {
            SimpleGuid original_id;
            for (const auto& pair : id_map) { if (pair.second == new_archetype.id) { original_id = pair.first; break; } }
            
            auto it = std::find_if(all_archetypes.begin(), all_archetypes.end(),
                [&](const EntityArchetype& e) { return e.id == original_id; });
            if (it == all_archetypes.end()) continue;
            const EntityArchetype& original_archetype = *it;

            if (id_map.count(original_archetype.parent_id)) {
                new_archetype.parent_id = id_map.at(original_archetype.parent_id);
            } else {
                new_archetype.parent_id = SimpleGuid::invalid();
            }

            new_archetype.child_ids.clear();
            for (const auto& old_child_id : original_archetype.child_ids) {
                if (id_map.count(old_child_id)) {
                    new_archetype.child_ids.push_back(id_map.at(old_child_id));
                }
            }
        }

        if (!new_family.empty()) {
            std::string base_name = new_family[0].name;
            size_t copy_pos = base_name.find(" (Copy");
            if (copy_pos != std::string::npos) {
                base_name = base_name.substr(0, copy_pos);
            }
            
            std::string potential_name = base_name + " (Copy)";
            int copy_number = 2;
            bool name_is_unique = false;
            while (!name_is_unique) {
                bool name_found = false;
                for (const auto& existing_archetype : all_archetypes) {
                    if (existing_archetype.name == potential_name) {
                        name_found = true;
                        break;
                    }
                }
                if (name_found) {
                    potential_name = base_name + " (Copy " + std::to_string(copy_number++) + ")";
                } else {
                    name_is_unique = true;
                }
            }
            new_family[0].name = potential_name;
        }
        
        return new_family;
    }

    std::vector<EntityArchetype> ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(
        const EntityArchetype& source,
        const std::vector<EntityArchetype>& all_archetypes) {
        
        std::vector<EntityArchetype> new_family;
        std::map<SimpleGuid, SimpleGuid> id_map;
        duplicate_recursive_helper(source.id, all_archetypes, new_family, id_map);

        for (auto& new_archetype : new_family) {
            SimpleGuid original_id;
            for (const auto& pair : id_map) { if (pair.second == new_archetype.id) { original_id = pair.first; break; } }
            
            auto it = std::find_if(all_archetypes.begin(), all_archetypes.end(),
                [&](const EntityArchetype& e) { return e.id == original_id; });
            if (it == all_archetypes.end()) continue;
            const EntityArchetype& original_archetype = *it;

            if (id_map.count(original_archetype.parent_id)) {
                new_archetype.parent_id = id_map.at(original_archetype.parent_id);
            } else {
                new_archetype.parent_id = source.parent_id;
            }

            new_archetype.child_ids.clear();
            for (const auto& old_child_id : original_archetype.child_ids) {
                if (id_map.count(old_child_id)) {
                    new_archetype.child_ids.push_back(id_map.at(old_child_id));
                }
            }
        }

        if (!new_family.empty()) {
            std::string base_name = new_family[0].name;
            size_t copy_pos = base_name.find(" (Copy");
            if (copy_pos != std::string::npos) {
                base_name = base_name.substr(0, copy_pos);
            }
            
            std::string potential_name = base_name + " (Copy)";
            int copy_number = 2;
            bool name_is_unique = false;
            while (!name_is_unique) {
                bool name_found = false;
                for (const auto& existing_archetype : all_archetypes) {
                    if (existing_archetype.name == potential_name) {
                        name_found = true;
                        break;
                    }
                }
                if (name_found) {
                    potential_name = base_name + " (Copy " + std::to_string(copy_number++) + ")";
                } else {
                    name_is_unique = true;
                }
            }
            new_family[0].name = potential_name;
        }
        
        return new_family;
    }



    ElementArchetype ArchetypeFactory::duplicate_element_archetype(const ElementArchetype& source) {
        ElementArchetype new_element;

        // 1. Copy the type name and generate a new, unique ID.
        new_element.type_name = source.type_name;
        new_element.id = SimpleGuid::generate();
        new_element.state = ArchetypeState::New;
        // 2. Give the new element a unique instance name.
        new_element.name = source.name + " (Copy)"; // You can implement a more robust unique naming system later if needed.

        // 3. Create a deep copy of the YAML data.
        new_element.data = YAML::Clone(source.data);
        
        // 4. Update the name in the YAML data as well.
        new_element.data["name"] = new_element.name;

        return new_element;
    }
    
} // namespace Salix