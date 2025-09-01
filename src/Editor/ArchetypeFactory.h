// Editor/ArchetypeFactory.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <Editor/EditorContext.h>
#include <vector>
#include <string>
#include <map>

namespace Salix {

    class EditorRealmManager;
    class EDITOR_API ArchetypeFactory {
    public:
        // Creates an ElementArchetype (e.g., a Transform) with default values.
        static ElementArchetype create_element_archetype(const std::string& type_name);

        // Creates an EntityArchetype with a default name and a Transform component.
        static EntityArchetype create_entity_archetype(const std::string& name = "Entity");

        // Creates a shallow copy of an entity archetype with new, unique IDs.
        static EntityArchetype duplicate_entity_archetype(const EntityArchetype& source, 
        EditorRealmManager* realm_manager, EditorContext* context);

        // Duplicates an entity AND its entire hierarchy of children, performing deep copy.
        static std::vector<EntityArchetype> duplicate_entity_archetype_and_children( SimpleGuid source_id, 
            EditorContext* context);

        // Duplicates and entity and its entire hierarchy of child, and makes it a sibling of the source Entity, sharing the same parent_id.
        static std::vector<EntityArchetype> duplicate_entity_archetype_family_as_sibling(const EntityArchetype& source,
            EditorContext* context);

        // Creates a deep copy of a single element archetype with a new ID.
        static ElementArchetype duplicate_element_archetype(const ElementArchetype& source, const EntityArchetype& parent);

        static bool entity_exists_in_realm(const std::vector<EntityArchetype>& realm, const EntityArchetype& entity);

        static std::string generate_unique_entity_name(const std::string& source_name, EditorRealmManager* realm_manager);

        private:
        // Private helper function to handle the recursion for the second function.
        static void duplicate_recursive_helper(const SimpleGuid& source_id, EditorRealmManager* realm_manager,
            EditorContext* context, std::vector<EntityArchetype>& out_new_family,
            std::map<SimpleGuid, SimpleGuid>& out_id_map
        );
    };

} // namespace Salix