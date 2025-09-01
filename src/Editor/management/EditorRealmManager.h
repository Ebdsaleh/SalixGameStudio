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

    class RealmSnapshot;
    struct EditorContext; 
    class EDITOR_API EditorRealmManager {
    public:
        EditorRealmManager();
        // The constructor can now safely use the forward-declared type.
        EditorRealmManager(EditorContext* context);
        ~EditorRealmManager();

        // --- Debug and Validation Functions ---
        void print_hierarchy() const;
        void validate_realm() const;
        // --- Realm Loading & State Management ---
        void load_realm_from_file(const std::string& filepath);
        void take_snapshot();
        bool is_dirty() const;
        void clear_realm();

        // --- Modifying Functions ---
        void add_entity(EntityArchetype archetype);
        void purge_entity(SimpleGuid entity_id);
        void purge_entity_descendants(SimpleGuid parent_id);
        void purge_entity_and_family(SimpleGuid entity_id);
        void purge_entity_bloodline(SimpleGuid entity_id);
        void reparent_entity(SimpleGuid child_id, SimpleGuid new_parent_id);
        void release_from_parent(SimpleGuid child_id);
        void add_child_entity(EntityArchetype child_archetype);
        void duplicate_entity(SimpleGuid source_id);
        void duplicate_entity_as_sibling(SimpleGuid source_id);
        void duplicate_entity_with_children(SimpleGuid source_id);
        void duplicate_family_as_sibling(SimpleGuid source_id);
        
        void add_element_to_entity(SimpleGuid entity_id, ElementArchetype element);
        void duplicate_element(SimpleGuid parent_entity_id, SimpleGuid source_element_id);
        void purge_element(SimpleGuid parent_entity_id, SimpleGuid element_to_purge_id);
        void update_ancestor_states(SimpleGuid start_entity_id);


        // --- Accessor Functions ---
        const std::vector<EntityArchetype>& get_realm() const;
        const size_t get_realm_size() const;
        const std::vector<std::shared_ptr<WorldTreeNode>>& get_hierarchy() const;
        EntityArchetype* get_archetype(SimpleGuid entity_id);
        const EntityArchetype* get_archetype(SimpleGuid entity_id) const;
        std::vector<const EntityArchetype*> get_all_archetypes() const;
        const RealmSnapshot* get_snapshot() const; // Getter for the snapshot
        bool realm_is_empty();
        bool realm_map_is_empty();
        bool does_entity_name_exist(const std::string& name) const;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
        void synchronize();
        void sync_preview_scene();
    };
}