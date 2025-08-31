// Editor/management/EditorRealmManager.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <Editor/panels/WorldTreeNode.h>
#include <Salix/core/SimpleGuid.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Salix {

    class EDITOR_API EditorRealmManager {
    public:
        EditorRealmManager();
        ~EditorRealmManager();
        // --- Modifying Functions ---
        // These are the ONLY ways to change the realm data.
        void add_entity(EntityArchetype archetype);
        void purge_entity(SimpleGuid entity_id);
        void reparent_entity(SimpleGuid child_id, SimpleGuid new_parent_id);
        

        // --- Accessor Functions ---
        const std::vector<EntityArchetype>& get_realm() const;
        const std::vector<std::shared_ptr<WorldTreeNode>>& get_hierarchy() const;
        EntityArchetype* get_archetype(SimpleGuid entity_id);

    private:

        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        
        // The single, private function that guarantees everything stays in sync.
        void synchronize();
    };

}