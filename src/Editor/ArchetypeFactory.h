// Editor/ArchetypeFactory.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <vector>
#include <string>
#include <map>

namespace Salix {

    class EDITOR_API ArchetypeFactory {
    public:
        // Creates an ElementArchetype (e.g., a Transform) with default values.
        static ElementArchetype create_element_archetype(const std::string& type_name);

        // Creates an EntityArchetype with a default name and a Transform component.
        static EntityArchetype create_entity_archetype(const std::string& name = "Entity");

        // Creates a shallow copy of an entity archetype with new, unique IDs.
        static EntityArchetype duplicate_entity_archetype(const EntityArchetype& source, const std::vector<EntityArchetype>& all_archetypes);

        //  Duplicates an entity AND its entire hierarchy of children, performing deep copy.
        static std::vector<EntityArchetype> duplicate_entity_archetype_and_children(const EntityArchetype& source, const std::vector<EntityArchetype>& all_archetypes);

        // Duplicates and entity and its entire hierarchy of child, and makes it a sibling of the source Entity, sharing the same parent_id.
        static std::vector<EntityArchetype> duplicate_entity_archetype_family_as_sibling(const EntityArchetype& source, const std::vector<EntityArchetype>& all_archetypes);

        private:
        // Private helper function to handle the recursion for the second function.
        static void duplicate_recursive_helper(
            const SimpleGuid& source_id, 
            const std::vector<EntityArchetype>& all_archetypes,
            std::vector<EntityArchetype>& new_family, 
            std::map<SimpleGuid, SimpleGuid>& id_map
        );
    };

} // namespace Salix