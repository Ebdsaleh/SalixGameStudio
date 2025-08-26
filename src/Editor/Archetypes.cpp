// Editor/Archetypes.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/Archetypes.h>
#include <Salix/reflection/ByteMirror.h>


namespace Salix {

    // --- Implementation for ElementArchetype ---

    bool ElementArchetype::base_properties_are_different(const ElementArchetype& other) const {
        std::cout << "Element Base Propery Comparison..." << std::endl;
       
        if (this->id != other.id) {
            std::cout << "  [MODIFIED] ID is different ('" << this->id.get_value() << "' vs '" << other.id.get_value() << "')" << std::endl;
            return true;
        }
        else {
            std::cout << "  [UNMODIFIED] ID is the same." << std::endl;
        }
        if (this->type_name != other.type_name) {
            std::cout << "  [MODIFIED] Type Name is different ('" << this->type_name << "' vs '" << other.type_name << "')" << std::endl;
            return true;
        }
        else {
             std::cout << "  [UNMODIFIED] Type Name is the same." << std::endl;
        }

        if (this->name != other.name) {
            std::cout << "  [MODIFIED] Name is different ('" << this->name << "' vs '" << other.name << "')" << std::endl;
            return true;
        }
        else {
            std::cout << "  [UNMODIFIED] Name is the same." << std::endl;
        }
        if (this->owner_id != other.owner_id) {
            std::cout << "  [MODIFIED] Owner ID is different ('" << this->owner_id.get_value() << "' vs '" << other.owner_id.get_value() << "')" << std::endl;
        }
        else {
            std::cout << "  [UNMODIFIED] Owner ID is the same." << std::endl;
        }
        if (this->allows_duplication != other.allows_duplication) {
            std::cout << "  [MODIFIED] Allows Duplication is different ('" << this->allows_duplication << "' vs '" << other.allows_duplication << "')" << std::endl;
            return true;
        }
        else {
            std::cout << "  [UNMODIFIED] Allows Duplication is the same." << std::endl;
        }
        return false;               
    }

    bool ElementArchetype::data_is_different(const ElementArchetype& other) const {
        // This is a more robust way to compare YAML nodes than YAML::Dump
        if (this->data.size() != other.data.size()) {
            return true;
        }
        for (const auto& kvp : this->data) {
            std::string key = kvp.first.as<std::string>();
            const YAML::Node& other_node = other.data[key];
            if (!other_node) {
                return true; // Key exists here but not in the other
            }
            // This is still a string comparison, but it's much less likely to fail than a full dump
            if (kvp.second.as<std::string>() != other_node.as<std::string>()) {
                return true;
            }
        }
        return false;
    }
    
    bool ElementArchetype::is_different_from(const ElementArchetype& other) const {
        std::cout << "--- Comparing Element ID " << this->id.get_value() << " (" << this->name << ") ---" << std::endl;

        std::cout << "[DEBUG:] comparing my 'name': " << name << ", with initial 'name': " << other.name << std::endl;
        if (this->name != other.name) {
            std::cout << "  [MODIFIED] Name is different ('" << this->name << "' vs '" << other.name << "')" << std::endl;
            return true;
        }

        std::cout << "[DEBUG:] comparing my 'type_name': " << type_name << ", with initial 'type_name': " << other.type_name << std::endl;
        if (this->type_name != other.type_name) {
            std::cout << "  [MODIFIED] Type name is different" << std::endl;
            return true;
        }

        std::cout << "[DEBUG:] comparing my 'allows_duplication': " << allows_duplication << ", with initial 'allows_duplication': " << other.allows_duplication << std::endl;
        if (this->allows_duplication != other.allows_duplication) {
            std::cout << "  [MODIFIED] Allows duplication is different" << std::endl;
            return true;
        }

        std::cout << "[DEBUG:] comparing YAML data..." << std::endl;
        if (!YAML::debug_compare_yaml(this->data, other.data)) {
            std::cout << "  [MODIFIED] YAML data is different." << std::endl;
            return true;
        }

        std::cout << "  [OK] Element ID " << this->id.get_value() << " is UNMODIFIED." << std::endl;
        return false;
    }


    

    // --- Implementation for EntityArchetype ---

    bool EntityArchetype::is_different_from(const EntityArchetype& other) const {
        std::cout << "--- Comparing Entity ID " << this->id.get_value() << " ('" << this->name << "') ---" << std::endl;
        if (this->name != other.name) {
            std::cout << "  [MODIFIED] Name is different ('" << this->name << "' vs '" << other.name << "')" << std::endl;
            return true;
        }
        if (this->parent_id != other.parent_id) {
            std::cout << "  [MODIFIED] Parent ID is different" << std::endl;
            return true;
        }
        if (this->child_ids != other.child_ids) {
            std::cout << "  [MODIFIED] Child IDs are different" << std::endl;
            return true;
        }
        if (this->elements.size() != other.elements.size()) {
            std::cout << "  [MODIFIED] Element count is different" << std::endl;
            return true;
        }

        // Now, check each element
        for (const auto& this_element : this->elements) {
            std::cout << "  -> Checking child element ID " << this_element.id.get_value() << std::endl;
            auto other_elem_it = std::find_if(other.elements.begin(), other.elements.end(),
                [&](const ElementArchetype& e) { return e.id == this_element.id; });
            if (other_elem_it == other.elements.end()) {
                std::cout << "    [MODIFIED] Element ID " << this_element.id.get_value() << " not found in snapshot." << std::endl;
                return true;
            }

            if (this_element.base_properties_are_different(*other_elem_it)) {
                // The element's function will print the details, so we just return.
                return true;
            }
        }
        
        std::cout << "  [OK] Entity ID " << this->id.get_value() << " is UNMODIFIED." << std::endl;
        return false;
    }

    const std::vector<ElementArchetype*> EntityArchetype::get_elements_by_type_name(const std::string& type_name)  {
        std::vector<ElementArchetype*> found_elements;
        if (type_name.empty()) { return found_elements; }
        for (auto& element_it : this->elements) {
            if (element_it.type_name == type_name) {
                found_elements.push_back(&element_it);
            }
        }
        return found_elements;
    }

    ElementArchetype* EntityArchetype::get_element_by_id(const SimpleGuid& element_id) {
        ElementArchetype* found_element = nullptr;
        for (auto& element_it : this->elements) {
            if (element_it.id == element_id) {
                found_element = &element_it;
                break;
            }
        }
        return found_element;
    }

    SimpleGuid EntityArchetype::get_primary_transform_id() {
        ElementArchetype* primary_transform = nullptr;

        // Find the first transform that is marked as non-duplicable.
        for (auto& element : elements) {
            if (element.type_name == "Transform" && !element.allows_duplication) {
                primary_transform = &element;
                break; // Found the primary, no need to search further.
            }
        }

        // If we found a primary one, return its ID.
        if (primary_transform) {
            return primary_transform->id;
        }

        // If no primary was found, fall back to returning the ID of the very first transform in the list.
        for (auto& element : elements) {
            if (element.type_name == "Transform") {
                return element.id; // Return the first one we find.
            }
        }

        // If no transforms were found at all, return invalid.
        return SimpleGuid::invalid();
    }
    
}  // namespace Salix