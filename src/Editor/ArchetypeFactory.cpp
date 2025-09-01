// Editor/ArchetypeFactory.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/ArchetypeFactory.h>
#include <Editor/EditorContext.h>
#include <Editor/management/EditorRealmManager.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/EnumRegistry.h>
// Required ECS includes.
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Scene.h>
#include <vector>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        archetype.is_visible = true;
        archetype.data["name"] = type_name;         // Set the data node for the reflection system
        archetype.data["visible"] = true;
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
        archetype.is_visible = true;
        archetype.state = ArchetypeState::New;
        ElementArchetype transform_archetype = create_element_archetype("Transform");
        ElementArchetype box_collider_archetype = create_element_archetype("BoxCollider"); 
        transform_archetype.allows_duplication = false;
        box_collider_archetype.allows_duplication = false;
        transform_archetype.owner_id = archetype.id;
        box_collider_archetype.owner_id = archetype.id;
        // All new entities should have a Transform component by default.
        archetype.elements.push_back(transform_archetype);
        archetype.elements.push_back(box_collider_archetype);
        return archetype;
    }


     // --- NEW PRIVATE HELPER FUNCTION ---
    // This contains the core logic from your original duplicate_entity_archetype.
    // Its job is to create a single copy with the world transform preserved.
    static EntityArchetype duplicate_single_archetype_internal(const EntityArchetype& source, EditorContext* context)
    {
        EntityArchetype new_archetype;
        
        // 1. Copy base properties but generate a new ID. The name will be set by the public-facing functions.
        new_archetype.name = source.name;
        new_archetype.id = SimpleGuid::generate();
        new_archetype.state = ArchetypeState::New;

        // 2. Deep copy all elements, giving each a new ID.
        for (const auto& source_element : source.elements) {
            ElementArchetype new_element;
            new_element.type_name = source_element.type_name;
            new_element.name = source_element.name;
            new_element.id = SimpleGuid::generate();
            new_element.owner_id = new_archetype.id;
            new_element.is_visible = source_element.is_visible;
            new_element.data = YAML::Clone(source_element.data);
            new_element.allows_duplication = source_element.allows_duplication;
            new_element.state = ArchetypeState::New;
            new_archetype.elements.push_back(new_element);
        }

        // 3. --- World Transform Preservation using Live Entity ---
        Entity* source_live_entity = context->preview_scene->get_entity_by_id(source.id);
        if (source_live_entity && source_live_entity->get_transform()) {
            Transform* source_transform = source_live_entity->get_transform();
            Vector3 world_pos = source_transform->get_world_position();
            Vector3 world_rot = source_transform->get_world_rotation();
            Vector3 world_scale = source_transform->get_world_scale();

            // Find the transform archetype in our new duplicate
            ElementArchetype* new_transform_arch = new_archetype.get_element_by_id(new_archetype.get_primary_transform_id());
            if (new_transform_arch) {
                // Since the new duplicate is a root, its local transform IS its world transform.
                new_transform_arch->data["position"] = world_pos;
                new_transform_arch->data["rotation"] = world_rot;
                new_transform_arch->data["scale"] = world_scale;
            }
        }

        return new_archetype;
    }



    // This one is refactored correcty!
    EntityArchetype ArchetypeFactory::duplicate_entity_archetype(const EntityArchetype& source, 
        EditorRealmManager* realm_manager, EditorContext* context) {
        // 1. Create the base duplicate using our internal helper.
        // This correctly preserves the entity's world transform.
        EntityArchetype new_archetype = duplicate_single_archetype_internal(source, context);

        // 2. Generate and apply the unique name using the new helper method.
        new_archetype.name = generate_unique_entity_name(source.name, realm_manager, {});

        return new_archetype;
    }

    // This one doesn't need modifying
    // This is the private recursive helper function's implementation.
    void ArchetypeFactory::duplicate_recursive_helper(
        const SimpleGuid& source_id,
        EditorRealmManager* realm_manager,
        EditorContext* context,
        std::vector<EntityArchetype>& out_new_family,
        std::map<SimpleGuid, SimpleGuid>& out_id_map) {
        // --- Method Start ---
        // 1. Use the manager for a fast, direct lookup of the original archetype.
        const EntityArchetype* source_archetype = realm_manager->get_archetype(source_id);
        if (!source_archetype) {
            return; // Safety check in case an ID is invalid.
        }

        // 2. THIS IS THE KEY: Call our trusted internal helper.
        // This single function call creates a perfect, world-preserved copy.
        // All of the old, manual property copying is now gone.
        EntityArchetype new_archetype = duplicate_single_archetype_internal(*source_archetype, context);
        
        // 3. Record the mapping from the original ID to the new duplicate's ID.
        out_id_map[source_archetype->id] = new_archetype.id;

        // 4. Add the newly created archetype to our list of results.
        out_new_family.push_back(new_archetype);

        // 5. Recursively call this function for all children of the original archetype.
        for (const auto& child_id : source_archetype->child_ids) {
            duplicate_recursive_helper(child_id, realm_manager, context, out_new_family, out_id_map);
        }
    }


    
    std::vector<EntityArchetype> ArchetypeFactory::duplicate_entity_archetype_and_children(
        SimpleGuid source_id, 
        EditorContext* context) {
        // --- Initial Setup ---
        if (!context || !context->editor_realm_manager) {
            return {}; // Safety check
        }
        EditorRealmManager* realm_manager = context->editor_realm_manager.get();
        
        std::vector<EntityArchetype> new_family;
        std::map<SimpleGuid, SimpleGuid> id_map; // Maps original ID -> new ID

        // --- Step 1: CREATE all the new parts using our powerful recursive helper ---
        // This single call traverses the entire hierarchy and creates a flat list of 
        // world-preserved duplicates in the 'new_family' vector.
        duplicate_recursive_helper(source_id, realm_manager, context, new_family, id_map);

        if (new_family.empty()) {
            return {};
        }

        // --- Step 2: ASSEMBLE the new family by wiring up relationships and names ---
        // This final loop connects all the parts and gives them unique names.
        for (auto& new_archetype : new_family) {
            // Find the original archetype that this new one was copied from
            SimpleGuid original_id;
            for (const auto& pair : id_map) {
                if (pair.second == new_archetype.id) {
                    original_id = pair.first;
                    break;
                }
            }
            const EntityArchetype* original_archetype = realm_manager->get_archetype(original_id);
            if (!original_archetype) continue;
            
            // A. Generate a unique name for EVERY entity in the family
            new_archetype.name = generate_unique_entity_name(original_archetype->name, realm_manager, new_family);

            // B. Re-wire the parent ID using our map
            new_archetype.parent_id = id_map.count(original_archetype->parent_id) 
                ? id_map.at(original_archetype->parent_id) 
                : SimpleGuid::invalid();
            
            // C. Re-wire the child IDs using our map
            new_archetype.child_ids.clear();
            for (const auto& old_child_id : original_archetype->child_ids) {
                if (id_map.count(old_child_id)) {
                    new_archetype.child_ids.push_back(id_map.at(old_child_id));
                }
            }
        }
        
        // The factory's job is done. Return the fully assembled, correct new family.
        return new_family;
    }              
            

   
    std::vector<EntityArchetype> ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(
        const EntityArchetype& source,
        EditorContext* context) {
        // --- Method Start ---
        // --- 1. Setup ---
        if (!context || !context->editor_realm_manager) return {};
        EditorRealmManager* realm_manager = context->editor_realm_manager.get();

        std::vector<EntityArchetype> new_family;
        std::map<SimpleGuid, SimpleGuid> id_map;

        // --- 2. CREATE all parts using our robust recursive helper ---
        // This creates a flat list of disconnected, world-preserved duplicates.
        duplicate_recursive_helper(source.id, realm_manager, context, new_family, id_map);

        if (new_family.empty()) {
            return {};
        }

        // --- 3. ASSEMBLE the new family ---
        for (auto& new_archetype : new_family) {
            // Find the original archetype corresponding to this new one
            SimpleGuid original_id;
            for (const auto& pair : id_map) {
                if (pair.second == new_archetype.id) {
                    original_id = pair.first;
                    break;
                }
            }
            const EntityArchetype* original_archetype = realm_manager->get_archetype(original_id);
            if (!original_archetype) continue;
            
            // A. Generate a unique name for EVERY entity
            new_archetype.name = generate_unique_entity_name(original_archetype->name, realm_manager, new_family);

            // B. Re-wire Parent ID. THIS IS THE KEY DIFFERENCE.
            if (original_archetype->id == source.id) {
                // If this is the root of the new family, make it a sibling of the source.
                new_archetype.parent_id = source.parent_id;
            } else {
                // Otherwise, it's a child within the new family, so find its new parent.
                new_archetype.parent_id = id_map.count(original_archetype->parent_id) 
                    ? id_map.at(original_archetype->parent_id) 
                    : SimpleGuid::invalid();
            }
            
            // C. Re-wire Child IDs
            new_archetype.child_ids.clear();
            for (const auto& old_child_id : original_archetype->child_ids) {
                if (id_map.count(old_child_id)) {
                    new_archetype.child_ids.push_back(id_map.at(old_child_id));
                }
            }
        }
        
        return new_family;
    }

    ElementArchetype ArchetypeFactory::duplicate_element_archetype(const ElementArchetype& source, const EntityArchetype& parent) {
        ElementArchetype new_element;
        new_element.type_name = source.type_name;
        new_element.id = SimpleGuid::generate();
        new_element.state = ArchetypeState::New;
        new_element.allows_duplication = source.allows_duplication;

        // --- START: Robust Name Generation Logic ---
        std::string base_name = source.name;
        size_t copy_pos = base_name.find(" (Copy");
        if (copy_pos != std::string::npos) {
            base_name = base_name.substr(0, copy_pos);
        }

        std::string potential_name = base_name + " (Copy)";
        int copy_number = 2; // Start numbering at 2 for "(Copy 2)"

        bool name_is_unique = false;
        while (!name_is_unique) {
            bool name_found = false;
            // Check against all SIBLING elements on the parent.
            for (const auto& existing_element : parent.elements) {
                if (existing_element.name == potential_name) {
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
        new_element.name = potential_name;
        // --- END: Robust Name Generation Logic ---

        new_element.data = YAML::Clone(source.data);
        new_element.data["name"] = new_element.name;

        return new_element;
    }



    // Returns true if the entity is found, false otherwise
    bool ArchetypeFactory::entity_exists_in_realm(const std::vector<EntityArchetype>& realm,
                                       const EntityArchetype& entity) {
        auto it = std::find_if(realm.begin(), realm.end(),
                               [&](const EntityArchetype& e) { return e.id == entity.id; });
        if (it == realm.end()) {
            std::cerr << "[ArchetypeFactory:] ERROR: Just added child "
                      << entity.id.get_value() << " but cannot find it in current_realm!\n";
            return false;
        }
        return true;
    }


    std::string ArchetypeFactory::generate_unique_entity_name(const std::string& source_name, EditorRealmManager* realm_manager,
        const std::vector<EntityArchetype>& current_batch) {
        
        std::string base_name = source_name;
        size_t copy_pos = base_name.find(" (Copy");
        if (copy_pos != std::string::npos) {
            base_name = base_name.substr(0, copy_pos);
        }
        
        std::string potential_name = base_name + " (Copy)";
        int copy_number = 2;
        
        while (true) {
            // Check 1: Does the name exist in the main realm?
            bool name_exists_in_realm = realm_manager->does_entity_name_exist(potential_name);

            // Check 2: Does the name exist in the batch we are currently creating?
            bool name_exists_in_batch = false;
            for (const auto& archetype : current_batch) {
                if (archetype.name == potential_name) {
                    name_exists_in_batch = true;
                    break;
                }
            }

            if (!name_exists_in_realm && !name_exists_in_batch) {
                // The name is truly unique, we can exit the loop.
                break;
            }

            // If the name was found, generate the next one and try again.
            potential_name = base_name + " (Copy " + std::to_string(copy_number++) + ")";
        }
        
        return potential_name;
    }
    
} // namespace Salix