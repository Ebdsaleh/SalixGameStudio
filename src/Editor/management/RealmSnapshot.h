// Editor/management/RealmSnapshot.h

#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <Salix/reflection/PropertyHandle.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace Salix {

   
    class EDITOR_API RealmSnapshot {
    public:
        RealmSnapshot();
        ~RealmSnapshot();

        // --- Rule of Five: Make the class Movable but not Copyable ---
        RealmSnapshot(RealmSnapshot&& other) noexcept; // Move Constructor.
        RealmSnapshot& operator=(RealmSnapshot&& other) noexcept; // Move Assignment.
        RealmSnapshot(const RealmSnapshot&) = delete; // Explicitly delete copy constructor.
        RealmSnapshot& operator=(const RealmSnapshot&) = delete; // Explicitly delete copy assign.
 
        // These static "factory" functions create and return a new snapshot object.
        static RealmSnapshot load_from_file(const std::string& file_path);
        static RealmSnapshot load_from_entity_archetype_vector(const std::vector<EntityArchetype>& archetype_vector);
        
        // This is now a regular member function so it can access this snapshot's data.
        bool validate_snapshot(const std::vector<EntityArchetype>& source) const;

        // These are also regular member functions.
        const std::string& get_source_file_path() const;
        void set_source_file_path(const std::string& source_file_path);

        // This gives other classes read-only access to the map for comparison.
        const std::unordered_map<SimpleGuid, EntityArchetype>& get_entity_map() const;
        const EntityArchetype* get_entity_by_id(const SimpleGuid& entity_id)const;
        const ElementArchetype* get_element_by_id(const SimpleGuid& element_id) const;
        // This method is the new heart of the comparison logic.
        // It takes a live element and compares it against its original state stored inside this snapshot.
        bool is_element_modified(const ElementArchetype& live_element) const;
        bool is_entity_modified(const EntityArchetype& live_entity_archetype) const;
        bool has_element_property_changed(const SimpleGuid& element_id, const std::string& property_name, const PropertyValue& live_property_value) const;
        const std::map<std::string, PropertyValue>* get_fossilized_data_for_element(const SimpleGuid& element_id) const;
        const std::string* get_initial_element_data_as_string(const SimpleGuid& element_id) const;
        const std::string get_initial_data_property_value(const SimpleGuid& element_id, const std::string& property_name) const;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}