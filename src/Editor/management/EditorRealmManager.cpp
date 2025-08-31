// Editor/management/EditorRealmManager.cpp
#include <Editor/management/EditorRealmManager.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm> 
#include <functional> 

namespace Salix {

    // --- Private Implementation (Pimpl) ---
    struct EditorRealmManager::Pimpl {
        std::vector<EntityArchetype> realm;
        std::unordered_map<SimpleGuid, size_t> realm_map;
        std::vector<std::shared_ptr<WorldTreeNode>> realm_hierarchy;
    };
    
    // --- Constructor & Destructor ---
    EditorRealmManager::EditorRealmManager() : pimpl(std::make_unique<Pimpl>()) {}
    EditorRealmManager::~EditorRealmManager() = default; // The unique_ptr handles cleanup

    // --- Modifying Functions ---

    void EditorRealmManager::add_entity(EntityArchetype archetype) {
        // 1. Add the new archetype to the end of our main data vector.
        pimpl->realm.push_back(std::move(archetype));
        // 2. Immediately re-synchronize all data structures.
        synchronize();
    }

    void EditorRealmManager::purge_entity(SimpleGuid entity_id) {
        // Find the entity to purge using the map to get its index.
        auto map_it = pimpl->realm_map.find(entity_id);
        if (map_it == pimpl->realm_map.end()) {
            return; // Entity doesn't exist.
        }
        size_t index_to_purge = map_it->second;
        EntityArchetype& archetype_to_purge = pimpl->realm[index_to_purge];

        // 1. Orphan the children of the entity being deleted.
        for (const auto& child_id : archetype_to_purge.child_ids) {
            EntityArchetype* child_archetype = get_archetype(child_id);
            if (child_archetype) {
                child_archetype->parent_id = SimpleGuid::invalid();
            }
        }

        // 2. Remove this entity from its parent's child list.
        if (archetype_to_purge.parent_id.is_valid()) {
            EntityArchetype* parent_archetype = get_archetype(archetype_to_purge.parent_id);
            if (parent_archetype) {
                auto& children = parent_archetype->child_ids;
                children.erase(std::remove(children.begin(), children.end(), entity_id), children.end());
            }
        }

        // 3. Erase the entity from the main realm vector.
        pimpl->realm.erase(pimpl->realm.begin() + index_to_purge);

        // 4. Immediately re-synchronize all data structures.
        synchronize();
    }

    void EditorRealmManager::reparent_entity(SimpleGuid child_id, SimpleGuid new_parent_id) {
        EntityArchetype* child_archetype = get_archetype(child_id);
        if (!child_archetype) return;

        // 1. Remove from old parent's child list
        if (child_archetype->parent_id.is_valid()) {
            EntityArchetype* old_parent_archetype = get_archetype(child_archetype->parent_id);
            if (old_parent_archetype) {
                auto& children = old_parent_archetype->child_ids;
                children.erase(std::remove(children.begin(), children.end(), child_id), children.end());
            }
        }

        // 2. Set the new parent ID on the child
        child_archetype->parent_id = new_parent_id;

        // 3. Add to new parent's child list
        if (new_parent_id.is_valid()) {
            EntityArchetype* new_parent_archetype = get_archetype(new_parent_id);
            if (new_parent_archetype) {
                new_parent_archetype->child_ids.push_back(child_id);
            }
        }

        // 4. We only need to rebuild the hierarchy tree, as the vector and map indices are still valid.
        synchronize(); // For simplicity, a full sync is safest.
    }

    // --- Accessor Functions ---

    const std::vector<EntityArchetype>& EditorRealmManager::get_realm() const {
        return pimpl->realm;
    }

    const std::vector<std::shared_ptr<WorldTreeNode>>& EditorRealmManager::get_hierarchy() const {
        return pimpl->realm_hierarchy;
    }

    EntityArchetype* EditorRealmManager::get_archetype(SimpleGuid entity_id) {
        // This is the correct way to use the map.
        // 1. Use .find() to safely check if the ID exists in the map.
        auto it = pimpl->realm_map.find(entity_id);
        if (it != pimpl->realm_map.end()) {
            // 2. If it exists, 'it->second' is the size_t index.
            size_t index = it->second;
            // 3. Return a pointer to the archetype at that index in the vector.
            return &pimpl->realm[index];
        }
        return nullptr; // Return nullptr if the ID wasn't found.
    }

    // --- Private Synchronization Function ---

    void EditorRealmManager::synchronize() {
        // --- 1. Rebuild the Index Map ---
        pimpl->realm_map.clear();
        for (size_t i = 0; i < pimpl->realm.size(); ++i) {
            pimpl->realm_map[pimpl->realm[i].id] = i;
        }

        // --- 2. Rebuild the Hierarchy Tree for the UI ---
        pimpl->realm_hierarchy.clear();
        std::unordered_map<SimpleGuid, std::shared_ptr<WorldTreeNode>> node_map;

        // First pass: create a node for every archetype.
        for (const auto& archetype : pimpl->realm) {
            node_map[archetype.id] = std::make_shared<WorldTreeNode>();
            node_map[archetype.id]->entity_id = archetype.id;
        }

        // Second pass: link the nodes together.
        for (const auto& archetype : pimpl->realm) {
            if (archetype.parent_id.is_valid() && node_map.count(archetype.parent_id)) {
                // This is a child node, add it to its parent's children list.
                node_map[archetype.parent_id]->children.push_back(node_map[archetype.id]);
            } else {
                // This is a root-level node.
                pimpl->realm_hierarchy.push_back(node_map[archetype.id]);
            }
        }
    }

} // namespace Salix