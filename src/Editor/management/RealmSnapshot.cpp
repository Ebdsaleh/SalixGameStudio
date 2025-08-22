// Editor/management/RealmSnapshot.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Editor/management/RealmSnapshot.h>
#include <Editor/management/RealmLoader.h>
#include <Salix/core/Logging.h>
#include <Salix/core/SimpleGuid.h>
#include <Editor/Archetypes.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <sstream> 

namespace Salix {
    struct RealmSnapshot::Pimpl{
        std::unordered_map<SimpleGuid, EntityArchetype> entity_archetype_map;
        std::unordered_map<SimpleGuid, const ElementArchetype*> element_archetype_map;
        // This map will store the truly immutable, deep-copied property data.
        std::unordered_map<SimpleGuid, std::map<std::string, PropertyValue>> fossilized_element_data_map;
        // This map will store the truly immutable, deep-copied YAML data as a string.
        std::unordered_map<SimpleGuid, std::string> initial_element_data_map;
        std::string source_file_path;
        
    };


    RealmSnapshot::RealmSnapshot() : pimpl(std::make_unique<Pimpl>()) {}
    RealmSnapshot::~RealmSnapshot() = default;

    // --- Rule of Five Implementation ---
    RealmSnapshot::RealmSnapshot(RealmSnapshot&& other) noexcept = default;
    RealmSnapshot& RealmSnapshot::operator=(RealmSnapshot&& other) noexcept = default;

    const std::string& RealmSnapshot::get_source_file_path() const {
        return pimpl->source_file_path;
    }

    void RealmSnapshot::set_source_file_path(const std::string& source_file_path) {
        pimpl->source_file_path = source_file_path;
    }
    
    const std::unordered_map<SimpleGuid, EntityArchetype>& RealmSnapshot::get_entity_map() const {
        return pimpl->entity_archetype_map;
    }


    // Deep Copy Methods


    // Deep-copy an ElementArchetype
    
    // Deep copy a single ElementArchetype
    inline ElementArchetype deep_copy_element(const ElementArchetype& source) {
        ElementArchetype copy = source;
        copy.data = YAML::deep_clone_yaml(source.data);
        return copy;
    }

    // Deep copy a full EntityArchetype
    inline EntityArchetype deep_copy_entity(const EntityArchetype& source) {
        EntityArchetype copy = source;
        copy.elements.clear();
        for (const auto& elem : source.elements) {
            copy.elements.push_back(deep_copy_element(elem));
        }
        return copy;
    }

    // Create a snapshot from a vector of entities
    inline std::vector<EntityArchetype> create_realm_snapshot(const std::vector<EntityArchetype>& current_realm) {
        std::vector<EntityArchetype> snapshot;
        std::cout << "\n--- CREATING REALM SNAPSHOT ---\n";
        for (const auto& entity : current_realm) {
            snapshot.push_back(deep_copy_entity(entity));
        }
        std::cout << "--- SNAPSHOT CREATION COMPLETE ---\n";
        return snapshot;
    }


    RealmSnapshot RealmSnapshot::load_from_file(const std::string& file_path) {
        RealmSnapshot snapshot; // Create a new object
        snapshot.pimpl->source_file_path = file_path;
        auto archetypes = load_archetypes_from_file(file_path);
        for (auto& archetype : archetypes) {
            snapshot.pimpl->entity_archetype_map[archetype.id] = std::move(archetype);
        }
        return snapshot; // Return the new object by move
    }




    // Create an immutable RealmSnapshot from a vector of EntityArchetypes
    RealmSnapshot RealmSnapshot::load_from_entity_archetype_vector(const std::vector<EntityArchetype>& archetype_vector) {
    RealmSnapshot snapshot;
    std::cout << "\n--- CREATING IMMUTABLE REALM SNAPSHOT ---\n";

    for (const auto& source_entity : archetype_vector) {
        // 1. Create a copy of the entity archetype's metadata.
        EntityArchetype entity_copy;
        
        // We now correctly copy the name from the source to the new copy.
        entity_copy.name = source_entity.name;

        entity_copy.id = source_entity.id;
        entity_copy.parent_id = source_entity.parent_id;
        entity_copy.child_ids = source_entity.child_ids;
        entity_copy.state = source_entity.state;

        // 2. Deep copy each element and populate the fossilized string map.
        for (const auto& source_element : source_entity.elements) {
            ElementArchetype element_copy;
            
            element_copy.type_name = source_element.type_name;
            element_copy.id = source_element.id;
            element_copy.name = source_element.name;
            element_copy.owner_id = source_element.owner_id;
            element_copy.allows_duplication = source_element.allows_duplication;
            element_copy.state = source_element.state;

            element_copy.data = YAML::Load(YAML::Dump(source_element.data));
            
            snapshot.pimpl->initial_element_data_map[element_copy.id] = YAML::Dump(element_copy.data);
            

        // This populates the fossilized_element_data_map for direct value comparison.
        const TypeInfo* type_info = ByteMirror::get_type_info_by_name(source_element.type_name);
        if (type_info) {
            std::map<std::string, PropertyValue> property_map;
            // Iterate through all reflected properties for this element type
            for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                // If the property exists in the YAML data...
                if (source_element.data[prop.name]) {
                    // ...convert it to a PropertyValue and store it in our map.
                    property_map[prop.name] = YAML::node_to_property_value(source_element.data[prop.name], prop);
                }
            }
            // Store the complete property map for this element in the snapshot
            snapshot.pimpl->fossilized_element_data_map[element_copy.id] = property_map;
        }

            entity_copy.elements.push_back(std::move(element_copy));
        }

        // 3. Move the complete, deep-copied entity into the snapshot's map.
        auto [it, inserted] = snapshot.pimpl->entity_archetype_map.emplace(entity_copy.id, std::move(entity_copy));

        if (inserted) {
            EntityArchetype& stored_entity = it->second;

            // 4. Populate the element pointer map for fast lookups.
            for (const ElementArchetype& stored_element : stored_entity.elements) {
                snapshot.pimpl->element_archetype_map[stored_element.id] = &stored_element;
            }
        }
    }

    std::cout << "--- IMMUTABLE SNAPSHOT CREATION COMPLETE ---\n";
    return snapshot;
}


    bool RealmSnapshot::validate_snapshot(const std::vector<EntityArchetype>& source) const {
        // 1. Size validation (basic sanity check)
        if (source.size() != pimpl->entity_archetype_map.size()) {
            LOG_ERROR("Snapshot validation failed: Size mismatch (source: " << source.size() << ", map: " << pimpl->entity_archetype_map.size() << ")");
            return false;
        }

        // 2. Create tracking structures
        std::unordered_set<SimpleGuid> source_ids;
        int mismatch_count = 0;

        // 3. First pass - verify all source entities exist in the map and match
        for (const auto& source_archetype : source) {
            source_ids.insert(source_archetype.id);
            
            auto map_it = pimpl->entity_archetype_map.find(source_archetype.id);
            if (map_it == pimpl->entity_archetype_map.end()) {
                LOG_ERROR("Snapshot validation failed: Source ID " << source_archetype.id.get_value() << " not found in map");
                mismatch_count++;
                continue;
            }

            // 4. Deep comparison of archetype contents
            const EntityArchetype& map_archetype = map_it->second;
            
            if (source_archetype.name != map_archetype.name) {
                LOG_ERROR("Name mismatch for ID " << source_archetype.id.get_value() << ": source='" << source_archetype.name << "', map='" << map_archetype.name << "'");
                mismatch_count++;
            }
            if (source_archetype.parent_id != map_archetype.parent_id) {
                LOG_ERROR("Parent ID mismatch for ID " << source_archetype.id.get_value() << ": source=" << source_archetype.parent_id.get_value() << ", map=" << map_archetype.parent_id.get_value());
                mismatch_count++;
            }
            if (source_archetype.child_ids != map_archetype.child_ids) {
                LOG_ERROR("Child IDs mismatch for ID " << source_archetype.id.get_value());
                mismatch_count++;
            }
            if (source_archetype.elements.size() != map_archetype.elements.size()) {
                LOG_ERROR("Elements count mismatch for ID " << source_archetype.id.get_value() << ": source=" << source_archetype.elements.size() << ", map=" << map_archetype.elements.size());
                mismatch_count++;
                continue; // Skip element comparison if counts differ
            }

            // 5. Element-by-element comparison
            for (size_t i = 0; i < source_archetype.elements.size(); ++i) {
                const auto& source_element = source_archetype.elements[i];
                const auto& map_element = map_archetype.elements[i];

                if (source_element.id != map_element.id) {
                    LOG_ERROR("Element ID mismatch at index " << i << " for entity " << source_archetype.id.get_value() << ": source=" << source_element.id.get_value() << ", map=" << map_element.id.get_value());
                    mismatch_count++;
                }
                if (source_element.type_name != map_element.type_name) {
                    LOG_ERROR("Element type mismatch for element " << source_element.id.get_value() << " in entity " << source_archetype.id.get_value() << ": source='" << source_element.type_name << "', map='" << map_element.type_name << "'");
                    mismatch_count++;
                }
                if (YAML::Dump(source_element.data) != YAML::Dump(map_element.data)) {
                    LOG_ERROR("Element data mismatch for element " << source_element.id.get_value() << " in entity " << source_archetype.id.get_value());
                    mismatch_count++;
                }
            }
        }

        // 6. Verify no extra IDs in map
        for (const auto& pair : pimpl->entity_archetype_map) {
            if (source_ids.find(pair.first) == source_ids.end()) {
                LOG_ERROR("Map contains extra ID " << pair.first.get_value() << " not present in source");
                mismatch_count++;
            }
        }

        // 7. Final result
        if (mismatch_count > 0) {
            LOG_ERROR("Snapshot validation failed with " << mismatch_count << " mismatches");
            return false;
        }

        LOG_INFO("Snapshot validation passed successfully");
        return true;
    }


    const EntityArchetype* RealmSnapshot::get_entity_by_id(const SimpleGuid& entity_id) const {
        // 1. Use the .find() method with the entity_id to search the map.
        // 'auto' makes 'it' an iterator pointing to the result.
        auto it = pimpl->entity_archetype_map.find(entity_id);

        // 2. Check if the iterator is valid. 
        // If 'it' is not equal to the end of the map, it means we found the entity.
        if (it != pimpl->entity_archetype_map.end()) {
            // 3. Return the found entity. The value is stored in the 'second' part of the iterator.
            return &it->second;
        }

        // 4. If the entity was not found, return a default-constructed (empty) EntityArchetype.
        return nullptr;
    }
    
    const ElementArchetype* RealmSnapshot::get_element_by_id(const SimpleGuid& element_id) const {
        auto element_iterator = pimpl->element_archetype_map.find(element_id);

        if (element_iterator != pimpl->element_archetype_map.end()) {
            return element_iterator->second;
        } else {
            return nullptr;
        }
    }    

    const std::map<std::string, PropertyValue>* RealmSnapshot::get_fossilized_data_for_element(const SimpleGuid& element_id) const {
        // Search the new map for the given element ID.
        auto it = pimpl->fossilized_element_data_map.find(element_id);

        if (it != pimpl->fossilized_element_data_map.end()) {
           
            std::cout << "Found fossilized map for element ID " << element_id.get_value() << ". Contents:" << std::endl;
            
            // it->second is the map of properties. We loop through it to print each one.
            for (const auto& pair : it->second) { 
                std::cout << "  - Key: " << pair.first << ", Value: " << pair.second << std::endl;
            }
            
            
            return &it->second;
        }

        return nullptr;
    }

    bool RealmSnapshot::has_element_property_changed(const SimpleGuid& element_id, const std::string& property_name, const PropertyValue& live_property_value) const {
        // 1. Get the map of this element's original properties from the snapshot.
        const auto* fossilized_map = get_fossilized_data_for_element(element_id);

        // If the element doesn't exist in the snapshot, it's new and therefore "changed".
        if (!fossilized_map) {
            std::cout << "Found a NEW element!" << std::endl;
            return true;
        }

        // 2. Find the original value of the specific property that changed.
        auto it = fossilized_map->find(property_name);

        // If the property doesn't exist in the snapshot, it was added and is therefore "changed".
        if (it == fossilized_map->end()) {
            return true;
        }
        
        const PropertyValue& original_value = it->second;

        // 3. The Direct Comparison: return true if the live value is different from the original.
        return live_property_value != original_value;
    }

    const std::string* RealmSnapshot::get_initial_element_data_as_string(const SimpleGuid& element_id) const {
        // Search the new map for the given element ID.
        // Using .find() is efficient and safe.
        auto it = pimpl->initial_element_data_map.find(element_id);

        // If the iterator is not at the end, it means we found the element.
        if (it != pimpl->initial_element_data_map.end()) {
            // Return a const pointer to the found string value.
            std::cout << "[RealmSnapshot: get_initial_element_data_as_string] Found property: " <<
            it->second << std::endl;
            return &it->second;
        }

        // If the element ID doesn't exist in the snapshot, return nullptr.
        return nullptr;
    }

    
    bool RealmSnapshot::is_element_modified(const ElementArchetype& live_element) const {
        if (live_element.state == ArchetypeState::New) {
            return true;
        }

        const ElementArchetype* snapshot_element = get_element_by_id(live_element.id);
        if (!snapshot_element) {
            return true;
        }

        // 1. Compare the simple base properties (name, type_name, etc.).
        if (live_element.name != snapshot_element->name ||
            live_element.owner_id != snapshot_element->owner_id ||
            live_element.type_name != snapshot_element->type_name ||
            live_element.allows_duplication != snapshot_element->allows_duplication) {
            return true;
        }

        // --- FINAL, PROPERTY-BY-PROPERTY COMPARISON ---

        // 2. Get the map of original property values. This is our source of truth.
        const auto* fossilized_map = get_fossilized_data_for_element(live_element.id);
        const YAML::Node& live_data = live_element.data;

        // 3. If the snapshot has no data for this element, or if property counts differ, it's modified.
        if (!fossilized_map || live_data.size() != fossilized_map->size()) {
            return true;
        }

        // 4. Get reflection data to correctly convert live YAML nodes to PropertyValue.
        const TypeInfo* type_info = ByteMirror::get_type_info_by_name(live_element.type_name);
        if (!type_info) return true; // Cannot compare without reflection data.

        // 5. Iterate through the ORIGINAL properties from the snapshot.
        for (const auto& pair : *fossilized_map) {
            const std::string& property_name = pair.first;
            const PropertyValue& original_value = pair.second;

            // a. Find the corresponding property in the live data.
            const YAML::Node live_value_node = live_data[property_name];
            if (!live_value_node) {
                return true; // Property was deleted from live data.
            }
            
            // b. Convert the live YAML node to a PropertyValue for a direct, type-safe comparison.
            PropertyValue current_value;
            for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
                if (prop.name == property_name) {
                    current_value = YAML::node_to_property_value(live_value_node, prop);
                    break;
                }
            }

            // c. The Direct Comparison.
            if (current_value != original_value) {
                return true; // The values are different.
            }
        }
        
        // If all properties in the snapshot exist in the live data and have the same value, it's unmodified.
        return false;
    }

    bool RealmSnapshot::is_entity_modified(const EntityArchetype& live_archetype) const {
        // An entity that is new is always considered "modified".
        if (live_archetype.state == ArchetypeState::New) {
            return true;
        }

        // 1. Get the original archetype from the snapshot's internal map.
        const EntityArchetype* snapshot_archetype = get_entity_by_id(live_archetype.id);
        if (!snapshot_archetype) {
            return true; // Not found in snapshot, so it's a new entity.
        }

        // 2. Compare the entity's own base properties.
        if (live_archetype.name != snapshot_archetype->name ||
            live_archetype.parent_id != snapshot_archetype->parent_id ||
            live_archetype.child_ids != snapshot_archetype->child_ids ||
            live_archetype.elements.size() != snapshot_archetype->elements.size()) {
            return true;
        }

        // 3. Compare each element using our robust is_element_modified function.
        
        for (const auto& live_element : live_archetype.elements) {
            if (is_element_modified(live_element)) {
                // If even ONE element is modified, the entire entity is considered modified.
                return true;
            }
        }

        // 4. If all base properties and all elements are identical to the snapshot, it's unmodified.
        return false;
    }

    


    const std::string RealmSnapshot::get_initial_data_property_value(const SimpleGuid& element_id, const std::string& property_name) const {
        // 1. Get the map of this element's original properties from the snapshot.
        const auto* fossilized_map = get_fossilized_data_for_element(element_id);

        if (fossilized_map) {
            // 2. Find the original value of the specific property that changed.
            auto it = fossilized_map->find(property_name);

            if (it != fossilized_map->end()) {
                // 3. Convert the found PropertyValue to a string and return it.
                const PropertyValue& original_value = it->second;
                std::stringstream ss;
                ss << original_value; // This uses the operator<< you already created
                return ss.str();
            }
        }

        // 4. Return an empty string if the element or property was not found.
        return "";
    }
} // namespace Salix