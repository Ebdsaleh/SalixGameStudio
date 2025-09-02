// Editor/management/EditorRealmManager.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/management/EditorRealmManager.h>
#include <Editor/management/RealmLoader.h>
#include <Editor/management/RealmSnapshot.h>
#include <Editor/EditorContext.h>
#include <Editor/ArchetypeFactory.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Editor/events/OnRootEntityAddedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Editor/events/OnChildEntityAddedEvent.h>
#include <Editor/events/OnEntityFamilyAddedEvent.h>
#include <Editor/events/OnEntityPurgedEvent.h>
#include <Editor/events/OnEntityFamilyPurgedEvent.h>
#include <Editor/events/OnHierarchyChangedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/OnElementAddedEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/events/EventManager.h>
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
        std::unique_ptr<RealmSnapshot> snapshot;
        EditorContext* context = nullptr;
    };
    
    // --- Constructor & Destructor ---
    EditorRealmManager::EditorRealmManager() : pimpl(std::make_unique<Pimpl>()) {}
    // The implementation can now safely use the full definition of EditorContext
    EditorRealmManager::EditorRealmManager(EditorContext* context) : pimpl(std::make_unique<Pimpl>()) {
        pimpl->context = context;
        pimpl->snapshot = std::make_unique<RealmSnapshot>();
    }

    EditorRealmManager::~EditorRealmManager() = default; // The unique_ptr handles cleanup

    // --- Debug and Validation Functions ---

    void EditorRealmManager::print_hierarchy() const {
        // A recursive lambda function to print each node and its children.
        std::function<void(const std::shared_ptr<WorldTreeNode>&, int)> print_node;
        print_node = [&](const std::shared_ptr<WorldTreeNode>& node, int depth) {
            if (!node) return;

            // Indent based on the depth in the hierarchy.
            std::string entity_indent(depth * 4, ' ');

            // Get the archetype data using the manager's fast lookup.
            const EntityArchetype* archetype = get_archetype(node->entity_id);
            std::string entity_name = archetype ? archetype->name : "<Unknown>";

            // Print entity ID and name.
            std::cout << entity_indent << node->entity_id.get_value() << " - " << entity_name << "\n";

            // Print elements with an extra indent.
            if (archetype) {
                for (const auto& elem : archetype->elements) {
                    std::cout << entity_indent << "  * " << elem.name 
                            << " (" << elem.type_name << ")\n";
                }
            }

            // Recursively print children.
            for (const auto& child : node->children) {
                print_node(child, depth + 1);
            }
        };

        // Start the printing process for all root nodes in the hierarchy.
        std::cout << "--- Realm Hierarchy Dump ---\n";
        for (const auto& root : pimpl->realm_hierarchy) {
            print_node(root, 0);
        }
        std::cout << "--------------------------\n";
    }

    

    void EditorRealmManager::validate_realm() const {
        std::cout << "[EditorRealmManager] Validating " << pimpl->realm.size() << " archetypes..." << std::endl;

        // Debug check #1 - Verify vector contents
        for (const auto& archetype : pimpl->realm) {
            assert(!archetype.name.empty() && "Entity archetype has empty name");
            assert(archetype.id.is_valid() && "Entity archetype has invalid ID");
            
            for (const auto& element : archetype.elements) {
                assert(!element.type_name.empty() && "Element has empty type name");
                assert(element.id.is_valid() && "Element has invalid ID");
            }
        }

        // Debug check #2 - Verify YAML node validity
        for (const auto& archetype : pimpl->realm) {
            for (const auto& element : archetype.elements) {
                assert(element.data.IsDefined() && "Element data node is invalid");
                assert(element.data.IsMap() && "Element data is not a map");
            }
        }

        std::cout << "[EditorRealmManager] All archetype data validated successfully." << std::endl;
        
        // Optional: Also validate the snapshot here if you want
        if (pimpl->snapshot) {
            if (pimpl->snapshot->validate_snapshot(pimpl->realm)) {
                std::cout << "[EditorRealmManager] Realm Snapshot is valid." << std::endl;
            } else {
                std::cerr << "[EditorRealmManager] ERROR: Realm Snapshot failed validation!" << std::endl;
            }
        }
    }

    void EditorRealmManager::load_realm_from_file(const std::string& filepath) {
        pimpl->realm = load_archetypes_from_file(filepath);
        synchronize(); // Sync map and hierarchy after loading
        take_snapshot(); // Take a new snapshot of the freshly loaded realm
    }

    void EditorRealmManager::take_snapshot() {
        // This single line does all the work:
        // 1. It calls the static factory function on the RealmSnapshot class.
        // 2. It passes the manager's current realm data as the source.
        // 3. The factory function performs a deep copy, creating a new snapshot.
        // 4. The new snapshot is stored in our pimpl, replacing the old one.
        pimpl->snapshot = std::make_unique<RealmSnapshot>(
            RealmSnapshot::load_from_entity_archetype_vector(pimpl->realm)
        );
    }
    


    bool EditorRealmManager::is_dirty() const {
        if (!pimpl->snapshot) return true; // If no snapshot, it's considered dirty

        const auto& snapshot_map = pimpl->snapshot->get_entity_map();
        if (pimpl->realm.size() != snapshot_map.size()) {
            return true; // Different number of entities means it's dirty
        }

        for (const auto& current_archetype : pimpl->realm) {
            if (pimpl->snapshot->is_entity_modified(current_archetype)) {
                return true; // Found a modified entity, so the realm is dirty
            }
        }

        return false; // No changes found
    }

    void EditorRealmManager::clear_realm() {
        pimpl->realm.clear();
        synchronize();
        take_snapshot();
    }

    // --- Modifying Functions ---

    void EditorRealmManager::add_entity(EntityArchetype archetype) {
        // 1. Add the new archetype to the vector.
        pimpl->realm.push_back(archetype);
        
        // 2. Synchronize the internal data structures.
        synchronize();

        // 3. NOW, notify the rest of the editor what just happened.
        
        pimpl->context->event_manager->dispatch(std::make_unique<OnRootEntityAddedEvent>(archetype));
    }

    void EditorRealmManager::purge_entity(SimpleGuid entity_id) {
        auto map_it = pimpl->realm_map.find(entity_id);
        if (map_it == pimpl->realm_map.end()) return;
        
        size_t index_to_purge = map_it->second;
        EntityArchetype& archetype_to_purge = pimpl->realm[index_to_purge];
        
        // --- THE FIX STARTS HERE ---

        // 1. Before deleting the parent, make a copy of its children's IDs.
        std::vector<SimpleGuid> orphaned_child_ids = archetype_to_purge.child_ids;

        // 2. Orphan the children in the archetype data model.
        for (const auto& child_id : orphaned_child_ids) {
            EntityArchetype* child_archetype = get_archetype(child_id);
            if (child_archetype) {
                child_archetype->parent_id = SimpleGuid::invalid();
            }
        }

        // --- END OF FIX PART 1 ---

        if (archetype_to_purge.parent_id.is_valid()) {
            EntityArchetype* parent_archetype = get_archetype(archetype_to_purge.parent_id);
            if (parent_archetype) {
                auto& children = parent_archetype->child_ids;
                children.erase(std::remove(children.begin(), children.end(), entity_id), children.end());
            }
        }

        pimpl->realm.erase(pimpl->realm.begin() + index_to_purge);
        synchronize();
        
        // Dispatch the event for the entity that was actually purged.
        
        pimpl->context->event_manager->dispatch(std::make_unique<OnEntityPurgedEvent>(entity_id));

        // --- THE FIX PART 2 ---

        // 3. Now, dispatch a hierarchy change event for each child that was orphaned.
        // This tells the RealmDesignerPanel to update their live transforms.
        for (const auto& child_id : orphaned_child_ids) {
            
            pimpl->context->event_manager->dispatch(
                std::make_unique<OnHierarchyChangedEvent>(child_id, SimpleGuid::invalid())
            );
        }
    }

    void EditorRealmManager::purge_entity_descendants(SimpleGuid parent_id) {
        EntityArchetype* parent_archetype = get_archetype(parent_id);
        if (!parent_archetype) return;

        // 1. Find all descendants to purge.
        std::vector<SimpleGuid> descendants_to_purge;
        std::function<void(const SimpleGuid&)> find_descendants = 
            [&](const SimpleGuid& current_id) {
            EntityArchetype* current_archetype = get_archetype(current_id);
            if (current_archetype) {
                for (const auto& child_id : current_archetype->child_ids) {
                    descendants_to_purge.push_back(child_id);
                    find_descendants(child_id);
                }
            }
        };
        find_descendants(parent_id);

        if (descendants_to_purge.empty()) return;

        // 2. Remove the descendant archetypes from the main realm vector.
        pimpl->realm.erase(
            std::remove_if(pimpl->realm.begin(), pimpl->realm.end(),
                [&](const EntityArchetype& e) {
                    return std::find(descendants_to_purge.begin(), descendants_to_purge.end(), e.id) != descendants_to_purge.end();
                }),
            pimpl->realm.end()
        );

        // 3. Clear the parent's child list and update its state.
        parent_archetype->child_ids.clear();
        if (parent_archetype->state != ArchetypeState::New) {
            parent_archetype->state = ArchetypeState::Modified;
        }
        update_ancestor_states(parent_archetype->id);

        // 4. Resynchronize the manager and dispatch the event.
        synchronize();
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnEntityFamilyPurgedEvent>(descendants_to_purge)
        );
    }


    void EditorRealmManager::purge_entity_and_family(SimpleGuid entity_id) {
        std::vector<SimpleGuid> family_to_purge;
        std::function<void(const SimpleGuid&)> find_descendants;
        
        find_descendants = [&](const SimpleGuid& current_id) {
            family_to_purge.push_back(current_id);
            EntityArchetype* current_archetype = get_archetype(current_id);
            if (current_archetype) {
                for (const auto& child_id : current_archetype->child_ids) {
                    find_descendants(child_id);
                }
            }
        };
        find_descendants(entity_id);

        EntityArchetype* top_level_archetype = get_archetype(entity_id);
        if (top_level_archetype && top_level_archetype->parent_id.is_valid()) {
            EntityArchetype* parent = get_archetype(top_level_archetype->parent_id);
            if (parent) {
                auto& children = parent->child_ids;
                children.erase(std::remove(children.begin(), children.end(), entity_id), children.end());
            }
        }

        pimpl->realm.erase(
            std::remove_if(pimpl->realm.begin(), pimpl->realm.end(),
                [&](const EntityArchetype& e) {
                    return std::find(family_to_purge.begin(), family_to_purge.end(), e.id) != family_to_purge.end();
                }),
            pimpl->realm.end()
        );

        synchronize();

        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnEntityFamilyPurgedEvent>(family_to_purge)
        );
    }


    void EditorRealmManager::purge_entity_bloodline(SimpleGuid entity_id) {
        // 1. Find the root of the bloodline for the given entity.
        SimpleGuid root_id = entity_id;
        EntityArchetype* current_archetype = get_archetype(root_id);
        while (current_archetype && current_archetype->parent_id.is_valid()) {
            root_id = current_archetype->parent_id;
            current_archetype = get_archetype(root_id);
        }

        // 2. Collect the root and all of its descendants to be purged.
        std::vector<SimpleGuid> bloodline_to_purge;
        std::function<void(const SimpleGuid&)> find_descendants;
        
        find_descendants = [&](const SimpleGuid& current_id) {
            bloodline_to_purge.push_back(current_id);
            EntityArchetype* current = get_archetype(current_id);
            if (current) {
                for (const auto& child_id : current->child_ids) {
                    find_descendants(child_id);
                }
            }
        };
        find_descendants(root_id);

        if (bloodline_to_purge.empty()) return;

        // 3. Remove all collected archetypes from the main realm vector.
        pimpl->realm.erase(
            std::remove_if(pimpl->realm.begin(), pimpl->realm.end(),
                [&](const EntityArchetype& e) {
                    return std::find(bloodline_to_purge.begin(), bloodline_to_purge.end(), e.id) != bloodline_to_purge.end();
                }),
            pimpl->realm.end()
        );

        // 4. Resynchronize the manager and dispatch the event.
        synchronize();
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnEntityFamilyPurgedEvent>(bloodline_to_purge)
        );
    }

    void EditorRealmManager::reparent_entity(SimpleGuid child_id, SimpleGuid new_parent_id) {
        EntityArchetype* child_archetype = get_archetype(child_id);
        if (!child_archetype) return;

        if (child_archetype->parent_id.is_valid()) {
            EntityArchetype* old_parent_archetype = get_archetype(child_archetype->parent_id);
            if (old_parent_archetype) {
                auto& children = old_parent_archetype->child_ids;
                children.erase(std::remove(children.begin(), children.end(), child_id), children.end());
            }
        }

        child_archetype->parent_id = new_parent_id;

        if (new_parent_id.is_valid()) {
            EntityArchetype* new_parent_archetype = get_archetype(new_parent_id);
            if (new_parent_archetype) {
                new_parent_archetype->child_ids.push_back(child_id);
            }
        }
        synchronize();
    }

    void EditorRealmManager::release_from_parent(SimpleGuid child_id) {
        // 1. Call the existing reparent_entity method to update the archetype data
        //    and synchronize the manager's internal state.
        reparent_entity(child_id, SimpleGuid::invalid());

        // 2. (THE FIX) Dispatch the event to notify listeners like RealmDesignerPanel
        //    that the live hierarchy needs to be updated.
        
        if (pimpl->context && pimpl->context->event_manager) {
            pimpl->context->event_manager->dispatch(
                std::make_unique<OnHierarchyChangedEvent>(child_id, SimpleGuid::invalid())
            );
        }
    }

    void EditorRealmManager::add_child_entity(EntityArchetype child_archetype) {
        if (!child_archetype.parent_id.is_valid()) return;

        // 1. Find the parent archetype and update its child list.
        EntityArchetype* parent_archetype = get_archetype(child_archetype.parent_id);
        if (parent_archetype) {
            parent_archetype->child_ids.push_back(child_archetype.id);
            if (parent_archetype->state != ArchetypeState::New) {
                parent_archetype->state = ArchetypeState::Modified;
            }
        }

        // 2. Add the new child archetype to the main realm vector.
        pimpl->realm.push_back(std::move(child_archetype));
        
        // 3. Synchronize all internal data structures.
        synchronize();

        // 4. Update ancestor states and notify the editor of the change.
        update_ancestor_states(parent_archetype->id);
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnChildEntityAddedEvent>(pimpl->realm.back())
        );
    }

    void EditorRealmManager::duplicate_entity(SimpleGuid source_id) {
        EntityArchetype* source_archetype = get_archetype(source_id);
        if (!source_archetype) return;

        // 1. Call the factory to create the new, correctly transformed archetype.
        //    We pass 'this' as the realm_manager pointer.
        EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(*source_archetype, this, pimpl->context);

        // 2. Call our existing add_entity method. It will handle adding the archetype
        //    to the realm, synchronizing, and dispatching the correct event.
        add_entity(std::move(duplicated_archetype));
    }

    void EditorRealmManager::duplicate_entity_as_sibling(SimpleGuid source_id) {
        EntityArchetype* source_archetype = get_archetype(source_id);
        if (!source_archetype) return;

        EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(*source_archetype, this, pimpl->context);
        duplicated_archetype.parent_id = source_archetype->parent_id; // Set as sibling

        if (duplicated_archetype.parent_id.is_valid()) {
            EntityArchetype* parent = get_archetype(duplicated_archetype.parent_id);
            if (parent) {
                parent->child_ids.push_back(duplicated_archetype.id);
            }
        }
        
        add_entity(std::move(duplicated_archetype));
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnHierarchyChangedEvent>(duplicated_archetype.id,  duplicated_archetype.parent_id)
        );
    }

    void EditorRealmManager::duplicate_entity_with_children(SimpleGuid source_id) {
        EntityArchetype* source_archetype = get_archetype(source_id);
        if (!source_archetype) return;

        std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_and_children(source_id, pimpl->context);
        if (new_family.empty()) return;

        for (const auto& new_member : new_family) {
            pimpl->realm.push_back(new_member);
        }
        
        synchronize();

        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnEntityFamilyAddedEvent>(new_family)
        );
    }


    void EditorRealmManager::duplicate_family_as_sibling(SimpleGuid source_id) {
        EntityArchetype* source_archetype = get_archetype(source_id);
        if (!source_archetype) return;

        std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(*source_archetype, pimpl->context);
        if (new_family.empty()) return;

        for (const auto& new_member : new_family) {
            pimpl->realm.push_back(new_member);
        }

        if (new_family[0].parent_id.is_valid()) {
            EntityArchetype* parent = get_archetype(new_family[0].parent_id);
            if (parent) {
                parent->child_ids.push_back(new_family[0].id);
            }
        }

        synchronize();

        
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnEntityFamilyAddedEvent>(new_family)
        );
    }

    void EditorRealmManager::add_element_to_entity(SimpleGuid entity_id, ElementArchetype element) {
        EntityArchetype* parent_archetype = get_archetype(entity_id);
        if (!parent_archetype) return;

        // The element is captured here before the move
        ElementArchetype element_copy = element;

        parent_archetype->elements.push_back(std::move(element));

        if (parent_archetype->state != ArchetypeState::New) {
            parent_archetype->state = ArchetypeState::Modified;
        }
        update_ancestor_states(entity_id);

        // DISPATCH THE NEW, SPECIFIC EVENT
        pimpl->context->event_manager->dispatch(
            std::make_unique<OnElementAddedEvent>(entity_id, element_copy)
        );
    }


    void EditorRealmManager::duplicate_element(SimpleGuid parent_entity_id, SimpleGuid source_element_id) {
        // 1. Find the parent archetype.
        EntityArchetype* parent_archetype = get_archetype(parent_entity_id);
        if (!parent_archetype) return;

        // 2. Find the source element archetype to be duplicated.
        auto source_it = std::find_if(parent_archetype->elements.begin(), parent_archetype->elements.end(),
            [&](const ElementArchetype& e) { return e.id == source_element_id; });
        if (source_it == parent_archetype->elements.end()) return;

        // 3. Create the duplicate using the factory.
        ElementArchetype duplicated_element = ArchetypeFactory::duplicate_element_archetype(*source_it, *parent_archetype);

        // 4. Add the new element to the parent.
        parent_archetype->elements.push_back(duplicated_element);

        // 5. Update the parent's state and its ancestors.
        if (parent_archetype->state != ArchetypeState::New) {
            parent_archetype->state = ArchetypeState::Modified;
        }
        update_ancestor_states(parent_archetype->id);

        // 6. Mark the realm as dirty for the preview scene to update.
        pimpl->context->realm_is_dirty = true;

        // 7. Dispatch events to notify other systems (like selecting the new element in the UI).
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<ElementSelectedEvent>(duplicated_element.id, parent_archetype->id, nullptr)
        );
    }


    void EditorRealmManager::purge_element(SimpleGuid parent_entity_id, SimpleGuid element_to_purge_id) {
        // 1. Find the parent archetype.
        EntityArchetype* parent_archetype = get_archetype(parent_entity_id);
        if (!parent_archetype) return;

        // 2. Announce that the element is ABOUT to be purged. This is the "heads-up".
        //    Listeners like EditorState and RealmPortalPanel will use this to clear their pointers.
        
        pimpl->context->event_manager->dispatch(
            std::make_unique<BeforeElementPurgedEvent>(element_to_purge_id, parent_entity_id)
        );

        // 3. Use the erase-remove idiom to find and remove the element archetype.
        auto& elements = parent_archetype->elements;
        auto original_size = elements.size();
        elements.erase(
            std::remove_if(elements.begin(), elements.end(),
                [&](const ElementArchetype& e) { return e.id == element_to_purge_id; }),
            elements.end()
        );

        // 4. If an element was actually removed, update the entity's state and mark for rebuild.
        if (elements.size() < original_size) {
            if (parent_archetype->state != ArchetypeState::New) {
                if (get_snapshot()->is_entity_modified(*parent_archetype)) {
                    parent_archetype->state = ArchetypeState::Modified;
                } else {
                    parent_archetype->state = ArchetypeState::UnModified;
                }
            }
            update_ancestor_states(parent_archetype->id);

            // Mark the realm as dirty so the preview scene will be rebuilt without the element.
            pimpl->context->realm_is_dirty = true;
        }
    }

    void EditorRealmManager::update_ancestor_states(SimpleGuid start_entity_id) {
        if (!start_entity_id.is_valid()) {
            return;
        }

        // Use the manager's own map to find the starting entity
        auto current_it = pimpl->realm_map.find(start_entity_id);
        if (current_it == pimpl->realm_map.end()) {
            return;
        }

        // Get the starting archetype to find its parent
        EntityArchetype& start_archetype = pimpl->realm[current_it->second];
        SimpleGuid parent_id = start_archetype.parent_id;

        // Loop up the hierarchy until we reach a root entity
        while (parent_id.is_valid()) {
            EntityArchetype* parent_archetype = get_archetype(parent_id);
            if (!parent_archetype) {
                break; // Parent not found, stop ascending
            }

            // Perform the state check on the current ancestor using the manager's snapshot
            if (parent_archetype->state != ArchetypeState::New) {
                if (pimpl->snapshot->is_entity_modified(*parent_archetype)) {
                    parent_archetype->state = ArchetypeState::Modified;
                } else {
                    parent_archetype->state = ArchetypeState::UnModified;
                }
            }

            // Move up to the next ancestor
            parent_id = parent_archetype->parent_id;
        }
    }

    // --- Accessor Functions ---

    const std::vector<EntityArchetype>& EditorRealmManager::get_realm() const {
        return pimpl->realm;
    }

    const size_t EditorRealmManager::get_realm_size() const {
        return pimpl->realm.size();
    }

    const std::vector<std::shared_ptr<WorldTreeNode>>& EditorRealmManager::get_hierarchy() const {
        return pimpl->realm_hierarchy;
    }

    const RealmSnapshot* EditorRealmManager::get_snapshot() const {
        return pimpl->snapshot.get();
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


    const EntityArchetype* EditorRealmManager::get_archetype(SimpleGuid entity_id) const {
        // 1. Use .find() to safely check if the ID exists in the map.
        auto it = pimpl->realm_map.find(entity_id);
        if (it != pimpl->realm_map.end()) {
            // 2. If it exists, 'it->second' is the size_t index.
            size_t index = it->second;
            // 3. Return a CONST pointer to the archetype at that index.
            return &pimpl->realm[index];
        }
        return nullptr; // Return nullptr if the ID wasn't found.
    }


    std::vector<const EntityArchetype*> EditorRealmManager::get_all_archetypes() const {
        std::vector<const EntityArchetype*> archetypes;
        archetypes.reserve(pimpl->realm.size());
        for (const auto& archetype : pimpl->realm) {
            archetypes.push_back(&archetype);
        }
        return archetypes;
    }


    bool EditorRealmManager::realm_is_empty() {
       return  pimpl->realm.empty();
    }

    bool EditorRealmManager::realm_map_is_empty() {
        return pimpl->realm_map.empty();
    }

    bool EditorRealmManager::does_entity_name_exist(const std::string& name) const {
        for (const auto& archetype : pimpl->realm) {
            if (archetype.name == name) {
                return true;
            }
        }
        return false;
    }
    // --- Private Synchronization Function ---

    void EditorRealmManager::synchronize() {
        pimpl->realm_map.clear();
        for (size_t i = 0; i < pimpl->realm.size(); ++i) {
            pimpl->realm_map[pimpl->realm[i].id] = i;
        }

        pimpl->realm_hierarchy.clear();
        std::unordered_map<SimpleGuid, std::shared_ptr<WorldTreeNode>> node_map;

        for (const auto& archetype : pimpl->realm) {
            node_map[archetype.id] = std::make_shared<WorldTreeNode>();
            node_map[archetype.id]->entity_id = archetype.id;
        }

        for (const auto& archetype : pimpl->realm) {
            if (archetype.parent_id.is_valid() && node_map.count(archetype.parent_id)) {
                node_map[archetype.parent_id]->children.push_back(node_map[archetype.id]);
            } else {
                pimpl->realm_hierarchy.push_back(node_map[archetype.id]);
            }
        }
    }

    void EditorRealmManager::sync_preview_scene() {
        if (!pimpl->context || !pimpl->context->preview_scene) return;

        // This is the logic moved from RealmDesignerPanel
        Scene* preview_scene = pimpl->context->preview_scene.get();
        auto& realm_archetypes = get_realm(); // Use the public getter

        preview_scene->clear_all_entities();
        
        if (!realm_archetypes.empty()) {
            ArchetypeInstantiator::instantiate_realm(realm_archetypes, preview_scene, *pimpl->context->init_context);
        }
    }
} // namespace Salix