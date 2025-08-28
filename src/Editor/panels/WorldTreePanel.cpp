// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================

#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/panels/WorldTreeNode.h>
#include <Editor/ArchetypeFactory.h>
#include <imgui/imgui.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Editor/events/OnHierarchyChangedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/core/SimpleGuid.h>
#include <Editor/EditorContext.h>
#include <Editor/camera/EditorCamera.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Transform.h> // Needed for focus_on
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

namespace Salix {
    struct WorldTreePanel::Pimpl {
        EditorContext* context = nullptr;
        SimpleGuid entity_to_rename_id = SimpleGuid::invalid();
        SimpleGuid element_to_rename_id = SimpleGuid::invalid();
        char rename_buffer[256];
        float child_indent = 22.0f;
        void render_entity_tree(EntityArchetype& archetype);
        void show_empty_space_context_menu();
        void show_entity_context_menu(EntityArchetype& archetype);
        void show_element_context_menu(EntityArchetype& parent_archetype, ElementArchetype& element_archetype);
        void setup_entity_drag_source(EntityArchetype& archetype);
        void process_entity_drop(SimpleGuid dragged_id, SimpleGuid target_id);
        void handle_entity_drop_target(EntityArchetype& archetype);
        void handle_inter_entity_drop_target(EntityArchetype& archetype);
        void build_world_tree_hierarchy();
        void print_world_tree_hierarchy() const;
        ImVec4 get_entity_archetype_text_color(const EntityArchetype& current);
        ImVec4 get_element_archetype_text_color(const ElementArchetype& current);
        // Event helper methods
        void handle_entity_selection(const EntitySelectedEvent& e);
        void handle_element_selection(const ElementSelectedEvent& e);
        void handle_property_value_change(const PropertyValueChangedEvent& e);
        void update_entity_archetype_state(EntityArchetype& archetype);
        void update_element_archetype_state(ElementArchetype& archetype);
        void rebuild_current_realm_map_internal();
        void update_all_ancestor_states(SimpleGuid start_entity_id);
        std::vector<std::function<void()>> deferred_commands;

        
    };

    void WorldTreePanel::rebuild_current_realm_map() {
        pimpl->rebuild_current_realm_map_internal();
    }


   

    WorldTreePanel::WorldTreePanel() : pimpl(std::make_unique<Pimpl>()) {
        set_name("World Tree Panel");
        set_title("World Tree");
        set_lock_icon_size(ImVec2(16, 16)); 
    }

    WorldTreePanel::~WorldTreePanel() = default;
    
    void WorldTreePanel::initialize(EditorContext* context) {
        if (!context) return;
        LockablePanel::initialize(context);
        pimpl->context = context;
        set_visibility(true);
        set_locked_state_tint_color(ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        set_unlocked_state_tint_color(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        set_lock_icon("Panel Unlocked");
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
        pimpl->build_world_tree_hierarchy();
        pimpl->print_world_tree_hierarchy();

    }


    // --- PIMPL Methods ---

    void WorldTreePanel::Pimpl::rebuild_current_realm_map_internal() {
        if(!context) {
            std::cerr << "[WorldTreePanel] Rebuild Current Realm Map Failed, Editor Context is nullptr." << std::endl;
        }
        context->current_realm_map.clear();
        for (EntityArchetype& archetype : context->current_realm) {
            context->current_realm_map[archetype.id] = &archetype;
        }
    }


    
    void WorldTreePanel::Pimpl::update_all_ancestor_states(SimpleGuid start_entity_id) {
        if (!start_entity_id.is_valid()) {
            return;
        }

        auto current_it = context->current_realm_map.find(start_entity_id);
        if (current_it == context->current_realm_map.end()) {
            return;
        }

        // Start with the parent of the initial entity
        SimpleGuid parent_id = current_it->second->parent_id;

        // Loop up the hierarchy until we reach a root entity (invalid parent_id)
        while (parent_id.is_valid()) {
            auto parent_it = context->current_realm_map.find(parent_id);
            if (parent_it == context->current_realm_map.end()) {
                break; // Parent not found, stop ascending
            }

            EntityArchetype* parent_archetype = parent_it->second;

            // Perform the state check on the current ancestor
            if (parent_archetype->state != ArchetypeState::New) {
                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                    parent_archetype->state = ArchetypeState::Modified;
                } else {
                    parent_archetype->state = ArchetypeState::UnModified;
                }
            }

            // Move up to the next ancestor
            parent_id = parent_archetype->parent_id;
        }
    }


    void WorldTreePanel::Pimpl::render_entity_tree(EntityArchetype& archetype) {
        ImGui::PushID(static_cast<int>(archetype.id.get_value()));
        handle_inter_entity_drop_target(archetype);
        
        const bool has_elements = !archetype.elements.empty();
        const bool has_children = !archetype.child_ids.empty();
        const bool is_populated = has_children || has_elements;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (context->selected_entity_id == archetype.id) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!is_populated) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
        }

        bool is_renaming = (entity_to_rename_id == archetype.id);
        if (is_renaming) {
            ImGui::SetKeyboardFocusHere(0);
            context->is_editing_property = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                entity_to_rename_id = SimpleGuid::invalid();
            }
            // Check if the user confirmed the edit (pressed Enter)
            bool rename_confirmed = ImGui::InputText("##RenameBox", rename_buffer, sizeof(rename_buffer),
                                                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

            // Check if the user clicked away, which also confirms the edit
            bool rename_deactivated = ImGui::IsItemDeactivatedAfterEdit();

            if (rename_confirmed || rename_deactivated) {
                // 1. Create a temporary copy of the archetype to test the change.
                EntityArchetype temp_archetype = archetype;

                // 2. Apply the new name to the temporary copy.
                temp_archetype.name = rename_buffer; 
                // 3. Use this temporary copy to check against the snapshot. This gives us the
                //    correct future state of the object.
                if (context->loaded_realm_snapshot.is_entity_modified(temp_archetype)) {
                    // The change results in a modified state.
                    archetype.state = ArchetypeState::Modified;
                    std::cout << "[DEBUG:] " << temp_archetype.name << ": SETTING STATE TO MODIFIED" << std::endl;
                } else {
                    // The change results in a state identical to the snapshot.
                    archetype.state = ArchetypeState::UnModified;
                    std::cout << "[DEBUG:] " << temp_archetype.name << ": SETTING STATE TO UNMODIFIED" << std::endl;
                }
                
                // 4. Finally, apply the new name to the REAL archetype.
                archetype.name = rename_buffer;
                
                entity_to_rename_id = SimpleGuid::invalid(); // End renaming
            
            }
        } else {
            // --- NEW: Color Logic ---
            ImVec4 entity_text_color = get_entity_archetype_text_color(archetype);
            ImGui::PushStyleColor(ImGuiCol_Text, entity_text_color);
          
            bool node_open = ImGui::TreeNodeEx((void*)archetype.id.get_value(), flags, "%s", archetype.name.c_str());
            ImGui::PopStyleColor();

            setup_entity_drag_source(archetype);
            handle_entity_drop_target(archetype);

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                context->selected_entity_id = archetype.id;
                context->selected_element_id = SimpleGuid::invalid();
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);

                // Focus on Entity on double-click.
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    Transform focus_transform;
                    for (const auto& element : archetype.elements) {
                        if (element.type_name == "Transform") {
                            focus_transform.set_position(element.data["position"].as<Salix::Vector3>());
                            focus_transform.set_rotation(element.data["rotation"].as<Salix::Vector3>());
                            focus_transform.set_scale(element.data["scale"].as<Salix::Vector3>());
                            context->editor_camera->focus_on(&focus_transform, 3.0f);
                            break;
                        }
                    }
                }
            }
            
            show_entity_context_menu(archetype);

            

            if (node_open) {
                for (auto& element : archetype.elements) {
                    ImGui::PushID(static_cast<int>(element.id.get_value()));

                    bool is_renaming_element = (element_to_rename_id == element.id);
                    if (is_renaming_element) {
                        ImGui::SetKeyboardFocusHere(0);
                        
                        context->is_editing_property = true;
                        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                            element_to_rename_id = SimpleGuid::invalid();
                        }
                        // Check if the user confirmed the edit (pressed Enter)
                        bool rename_confirmed = ImGui::InputText("##RenameBox", rename_buffer, sizeof(rename_buffer), 
                                                                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

                        // Check if the user clicked away, which also confirms the edit
                        bool rename_deactivated = ImGui::IsItemDeactivatedAfterEdit();

                        if (rename_confirmed || rename_deactivated) {
                            const std::string new_name = rename_buffer;
                            element.name = new_name;
                            element.data["name"] = new_name;
                            //  Update the ElementArchetype state.
                            if(context->loaded_realm_snapshot.is_element_modified(element)){
                                element.state = ArchetypeState::Modified;
                            }
                            else {
                                element.state = ArchetypeState::UnModified;
                            }
                            
                            // Update the EntityArchetype state.
                            if (context->loaded_realm_snapshot.is_entity_modified(archetype)) {
                                archetype.state = ArchetypeState::Modified;
                            }
                            else {
                                if (archetype.state == ArchetypeState::Modified) {
                                    archetype.state = ArchetypeState::UnModified;
                                }
                            }
                            element_to_rename_id = SimpleGuid::invalid(); // End renaming
                            context->realm_is_dirty = true;
                        }

                    }
                    ImGuiTreeNodeFlags element_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (context->selected_element_id == element.id) {
                        element_flags |= ImGuiTreeNodeFlags_Selected;
                    }

                    // --- NEW: Color Logic ---
                    ImVec4 element_text_color = get_element_archetype_text_color(element);
                    ImGui::PushStyleColor(ImGuiCol_Text, element_text_color);
                    // --- END NEW ---
                    ImGui::TreeNodeEx((void*)element.id.get_value(), element_flags, "%s", element.name.c_str());
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemClicked()) {
                        context->selected_element_id = element.id;
                        context->selected_entity_id = archetype.id;
                        ElementSelectedEvent event(context->selected_element_id, context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                    show_element_context_menu(archetype, element);
                    ImGui::PopID();
                }
                for (const auto& child_id : archetype.child_ids) {
                    auto child_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == child_id; });
                    if (child_it != context->current_realm.end()) {
                        ImGui::Indent(child_indent);
                        render_entity_tree(*child_it);
                        ImGui::Unindent(child_indent);
                    }
                }
                ImGui::TreePop();
            }
        }
        //ImGui::PopStyleColor();
        ImGui::PopID();
    }

    void WorldTreePanel::Pimpl::show_empty_space_context_menu() {
        if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                // Defer the creation and modification of the realm to the end of the frame.
                deferred_commands.push_back([this]() {
                    EntityArchetype new_entity = ArchetypeFactory::create_entity_archetype("Entity");
                    if (new_entity.id.is_valid()) {
                        // All of these actions modify the editor's state and should happen together.
                        context->current_realm.push_back(new_entity);
                        rebuild_current_realm_map_internal(); // <-- Crucial: update the map
                        
                        context->selected_entity_id = new_entity.id;
                        context->selected_element_id = SimpleGuid::invalid();
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                        
                        context->realm_is_dirty = true;

                        build_world_tree_hierarchy();
                        print_world_tree_hierarchy();
                    }
                });
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Refresh View##RefreshWorldTree")) {
                // This is a safe, non-modifying action, so it doesn't need to be deferred.
            }
            ImGui::EndPopup();
        }
    }

    void WorldTreePanel::Pimpl::show_entity_context_menu(EntityArchetype& archetype) {
        if (ImGui::BeginPopupContextItem("EntityContextMenu")) {
            // Header with entity name
            ImGui::TextDisabled("%s", archetype.name.c_str());
            ImGui::Separator();

            // Element Creation Submenu
            if (ImGui::BeginMenu("Add Element##AddElementMenu")) {
                if (ImGui::MenuItem("Transform##AddTransform")) {
                    // Defer the action to prevent modifying the archetype during UI iteration.
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        
                        // Find the parent archetype again using the fast map to ensure the pointer is valid.
                        auto parent_it = context->current_realm_map.find(archetype_id);
                        if (parent_it == context->current_realm_map.end()) {
                            return; // Parent not found, cannot add element.
                        }
                        EntityArchetype* parent_archetype = parent_it->second;

                        // Create the new element.
                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Transform");
                        if (new_element.id.is_valid()) {
                            new_element.owner_id = parent_archetype->id;
                            parent_archetype->elements.push_back(new_element);
                            
                            
                            // Check and update the parent's state, then update all its ancestors.
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);
                            

                            context->realm_is_dirty = true;
                            
                            // Reselect the entity to refresh the Inspector.
                            context->selected_entity_id = parent_archetype->id;
                            EntitySelectedEvent event(context->selected_entity_id, nullptr);
                            context->event_manager->dispatch(event);
                        }
                    });
                }
                if (ImGui::BeginMenu("Collider##AddColliderMenu")) {
                    if (ImGui::MenuItem("Box Collider##AddBoxCollider")) {
                        // Defer this action to the end of the frame.
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Find the parent archetype again using the fast map to ensure the pointer is valid.
                            auto parent_it = context->current_realm_map.find(archetype_id);
                            if (parent_it == context->current_realm_map.end()) {
                                return; // Parent not found.
                            }
                            EntityArchetype* parent_archetype = parent_it->second;

                            // Create the new element.
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("BoxCollider");
                            if (new_element.id.is_valid()) {
                                new_element.owner_id = parent_archetype->id;
                                parent_archetype->elements.push_back(new_element);
                                
                                
                                // Check and update the parent's state, then update all its ancestors.
                                if (parent_archetype->state != ArchetypeState::New) {
                                    if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                        parent_archetype->state = ArchetypeState::Modified;
                                    } else {
                                        parent_archetype->state = ArchetypeState::UnModified;
                                    }
                                }
                                update_all_ancestor_states(parent_archetype->id);
                                

                                context->realm_is_dirty = true;
                                
                                // Reselect the parent entity to refresh the Inspector.
                                context->selected_entity_id = parent_archetype->id;
                                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                                context->event_manager->dispatch(event);
                            }
                        });
                    }
                    // Will add other collider types here (e.g., Sphere, Capsule) following the same pattern.
                    ImGui::EndMenu();
                }
                
                if (ImGui::MenuItem("Sprite2D##AddSprite2D")) {
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        auto parent_it = context->current_realm_map.find(archetype_id);
                        if (parent_it == context->current_realm_map.end()) return;
                        EntityArchetype* parent_archetype = parent_it->second;

                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Sprite2D");
                        if (new_element.id.is_valid()) {
                            new_element.owner_id = parent_archetype->id;
                            parent_archetype->elements.push_back(new_element);
                            
                            // Adding an element modifies the parent, so check its state and its ancestors.
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);

                            context->realm_is_dirty = true;
                            context->selected_entity_id = parent_archetype->id;
                            EntitySelectedEvent event(context->selected_entity_id, nullptr);
                            context->event_manager->dispatch(event);
                        }
                    });
                }
                
                if (ImGui::BeginMenu("Script##AddScriptMenu")) {
                    if (ImGui::MenuItem("C++ Script##AddCPPScript")) {
                        // Defer this action to the end of the frame.
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Find the parent archetype again using the fast map lookup.
                            auto parent_it = context->current_realm_map.find(archetype_id);
                            if (parent_it == context->current_realm_map.end()) return;
                            EntityArchetype* parent_archetype = parent_it->second;

                            // Create the new CppScript element.
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("CppScript");
                            if (new_element.id.is_valid()) {
                                new_element.owner_id = parent_archetype->id;
                                parent_archetype->elements.push_back(new_element);
                                
                                // Check and update the parent's state, then update all its ancestors.
                                if (parent_archetype->state != ArchetypeState::New) {
                                    if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                        parent_archetype->state = ArchetypeState::Modified;
                                    } else {
                                        parent_archetype->state = ArchetypeState::UnModified;
                                    }
                                }
                                update_all_ancestor_states(parent_archetype->id);

                                context->realm_is_dirty = true;
                                
                                // Reselect the parent entity to refresh the Inspector.
                                context->selected_entity_id = parent_archetype->id;
                                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                                context->event_manager->dispatch(event);
                            }
                        });
                    }
                    if (ImGui::MenuItem("Python Script##AddPythonScript")) {
                        // Defer this action to the end of the frame.
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Find the parent archetype again using the fast map lookup.
                            auto parent_it = context->current_realm_map.find(archetype_id);
                            if (parent_it == context->current_realm_map.end()) return;
                            EntityArchetype* parent_archetype = parent_it->second;

                            // Create the new PythonScript element.
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("PythonScript");
                            if (new_element.id.is_valid()) {
                                new_element.owner_id = parent_archetype->id;
                                parent_archetype->elements.push_back(new_element);
                                
                                // Check and update the parent's state, then update all its ancestors.
                                if (parent_archetype->state != ArchetypeState::New) {
                                    if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                        parent_archetype->state = ArchetypeState::Modified;
                                    } else {
                                        parent_archetype->state = ArchetypeState::UnModified;
                                    }
                                }
                                update_all_ancestor_states(parent_archetype->id);

                                context->realm_is_dirty = true;
                                
                                // Reselect the parent entity to refresh the Inspector.
                                context->selected_entity_id = parent_archetype->id;
                                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                                context->event_manager->dispatch(event);
                            }
                        });
                    }
                    ImGui::EndMenu();
                }
                
                
                if (ImGui::MenuItem("Camera##AddCamera")) {
                    // Defer this action to the end of the frame.
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // Find the parent archetype again using the fast map to ensure the pointer is valid.
                        auto parent_it = context->current_realm_map.find(archetype_id);
                        if (parent_it == context->current_realm_map.end()) {
                            return; // Parent not found.
                        }
                        EntityArchetype* parent_archetype = parent_it->second;

                        // Create the new element.
                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Camera");
                        if (new_element.id.is_valid()) {
                            new_element.owner_id = parent_archetype->id;
                            parent_archetype->elements.push_back(new_element);
                            
                            // Check and update the parent's state, then update all its ancestors.
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);
                            

                            context->realm_is_dirty = true;
                            
                            // Reselect the parent entity to refresh the Inspector.
                            context->selected_entity_id = parent_archetype->id;
                            EntitySelectedEvent event(context->selected_entity_id, nullptr);
                            context->event_manager->dispatch(event);
                        }
                    });
                }
                ImGui::EndMenu();
            }

            // Hierarchy Operations
            if (archetype.parent_id.get_value() != 0) {
                if (ImGui::MenuItem("Release From Parent##ReleaseFromParent")) {
                    // Defer this action to the end of the frame.
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // Find the child archetype that is being released.
                        auto child_it = context->current_realm_map.find(archetype_id);
                        if (child_it == context->current_realm_map.end()) return;
                        EntityArchetype* child_archetype = child_it->second;

                        // Get the parent's ID before we change anything.
                        SimpleGuid parent_id = child_archetype->parent_id;
                        if (!parent_id.is_valid()) return; // Already a root, nothing to do.

                        // Find the parent archetype.
                        auto parent_it = context->current_realm_map.find(parent_id);
                        if (parent_it != context->current_realm_map.end()) {
                            EntityArchetype* parent_archetype = parent_it->second;
                            
                            // Remove the child's ID from the parent's list of children.
                            parent_archetype->child_ids.erase(
                                std::remove(parent_archetype->child_ids.begin(), parent_archetype->child_ids.end(), archetype_id),
                                parent_archetype->child_ids.end()
                            );

                            // Check and update the parent's state against the snapshot
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            
                            
                            // Now, update all of the former parent's ancestors.
                            update_all_ancestor_states(parent_archetype->id);
                            
                        }
                        
                        // Clear the child's parent reference.
                        child_archetype->parent_id = SimpleGuid::invalid();
                        
                        // Mark child as modified.
                        if (child_archetype->state != ArchetypeState::New) {
                            if (context->loaded_realm_snapshot.is_entity_modified(*child_archetype)) {
                                child_archetype->state = ArchetypeState::Modified;
                            } else {
                                child_archetype->state = ArchetypeState::UnModified;
                            }
                        }
                        
                        context->realm_is_dirty = true;
                        this->build_world_tree_hierarchy(); 
                        this->print_world_tree_hierarchy();
                    });
                }
            }
            
            if (ImGui::MenuItem("Add Child Entity##AddChildEntity")) {
                // Queue the command to be executed later
                // CAPTURE BY VALUE: We capture the name and id we need, not the whole archetype.
                deferred_commands.push_back([this, archetype_name = archetype.name, archetype_id = archetype.id]() {
                    
                    // Use the captured values
                    EntityArchetype new_child_entity = ArchetypeFactory::create_entity_archetype(archetype_name + "_Child");
                    new_child_entity.parent_id = archetype_id;

                    if (new_child_entity.parent_id != new_child_entity.id && new_child_entity.id.is_valid()) {
                        // Find the parent using the captured ID and the fast map lookup
                        auto parent_it = context->current_realm_map.find(archetype_id);

                        if (parent_it != context->current_realm_map.end()) {
                            EntityArchetype* parent_archetype = parent_it->second;
                            parent_archetype->child_ids.push_back(new_child_entity.id);

                            // Adding a child modifies the parent, so check its state and its ancestors.
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);
                        
                        }
                        
                        context->current_realm.push_back(new_child_entity);
                        this->rebuild_current_realm_map_internal();
                        context->realm_is_dirty = true;

                        context->selected_entity_id = new_child_entity.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);

                        build_world_tree_hierarchy();
                        print_world_tree_hierarchy();
                    }
                });
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Rename##RenameEntity", "F2")) {
                context->is_editing_property = true;
                entity_to_rename_id = archetype.id;
                strncpy_s(rename_buffer, sizeof(rename_buffer), 
                        archetype.name.c_str(), sizeof(rename_buffer) - 1);
            }

            if (ImGui::MenuItem("Duplicate##DuplicateEntity", "Ctrl+D")) {
                // 1. Create the duplicate immediately. This is a read-only operation that
                // needs the current state of the realm to generate a unique name.
                EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(archetype, context->current_realm);

                // 2. Defer the modification of the realm to the end of the frame.
                // We capture the created duplicate by value to ensure it's safely stored.
                deferred_commands.push_back([this, duplicated_archetype]() {
                    // Add the new archetype to the realm.
                    context->current_realm.push_back(duplicated_archetype);
                    rebuild_current_realm_map_internal(); // Update the lookup map

                    // Select the new entity to give the user immediate feedback.
                    context->selected_entity_id = duplicated_archetype.id;
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);

                    // Mark the realm as dirty to trigger a preview refresh.
                    context->realm_is_dirty = true;
                });
            }

            if (ImGui::MenuItem("Duplicate As Sibling##DuplicateEntityAsSibling", "Ctrl+Alt+D")) {
                // 1. Create the duplicate immediately to generate its unique name.
                EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(
                    archetype,
                    context->current_realm
                );
                // 2. Set the parent to the SOURCE's parent to make it a sibling.
                duplicated_archetype.parent_id = archetype.parent_id;
                // 3. Defer all modifications to the realm.
                deferred_commands.push_back([this, duplicated_archetype]() {
                    // Add the new archetype to the realm.
                    context->current_realm.push_back(duplicated_archetype);
                    rebuild_current_realm_map_internal(); // Update the lookup map

                    // If it has a parent, find the parent and add the new sibling to its child list.
                    SimpleGuid parent_id = duplicated_archetype.parent_id;
                    if (parent_id.is_valid()) {
                        auto parent_it = context->current_realm_map.find(parent_id);
                        if (parent_it != context->current_realm_map.end()) {
                            EntityArchetype* parent_archetype = parent_it->second;
                            parent_archetype->child_ids.push_back(duplicated_archetype.id);

                            // Check and update the parent's state, then update all its ancestors.
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);
                            
                        }
                    }
                    
                    // Select the new entity to give the user immediate feedback.
                    context->selected_entity_id = duplicated_archetype.id;
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);

                    // Mark the realm as dirty to trigger a preview refresh.
                    context->realm_is_dirty = true;
                    build_world_tree_hierarchy();
                });
            }

            if (ImGui::MenuItem("Duplicate With Children##DuplicateEntityWithChildren", "Ctrl+Shift+D")) {
                // 1. Create the entire new family of archetypes immediately.
                // This is a read-only operation on the current realm.
                std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_and_children(
                    archetype,
                    context->current_realm
                );

                // 2. Defer all modifications to the realm.
                // We capture the created family by value.
                deferred_commands.push_back([this, new_family]() {
                    // Add all the newly created entities (parent and children) to the realm.
                    for (const auto& new_member : new_family) {
                        context->current_realm.push_back(new_member);
                    }
                    rebuild_current_realm_map_internal(); // Update the lookup map

                    // Select the top-level parent of the new family.
                    if (!new_family.empty()) {
                        context->selected_entity_id = new_family[0].id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }

                    context->realm_is_dirty = true;
                    build_world_tree_hierarchy();
                });
            }

            if (ImGui::MenuItem("Duplicate Family As Sibling##DuplicateFamilyAsSibling", "Ctrl+Alt+Shift+D")) {
                // 1. Create the entire new family of archetypes immediately.
                std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(
                    archetype,
                    context->current_realm
                );
                // 2. Defer all modifications to the realm if the duplication was successful.
                if (!new_family.empty()) {
                    deferred_commands.push_back([this, new_family]() {
                        // Add all the newly created entities to the realm.
                        for (const auto& new_member : new_family) {
                            context->current_realm.push_back(new_member);
                        }
                        rebuild_current_realm_map_internal(); // Update the lookup map

                        // Find the parent entity and add the new sibling to its child list.
                        SimpleGuid parent_id = new_family[0].parent_id;
                        if (parent_id.is_valid()) {
                            auto parent_it = context->current_realm_map.find(parent_id);
                            if (parent_it != context->current_realm_map.end()) {
                                EntityArchetype* parent_archetype = parent_it->second;
                                parent_archetype->child_ids.push_back(new_family[0].id);

                                
                                // Check and update the parent's state, then update all its ancestors.
                                if (parent_archetype->state != ArchetypeState::New) {
                                    if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                        parent_archetype->state = ArchetypeState::Modified;
                                    } else {
                                        parent_archetype->state = ArchetypeState::UnModified;
                                    }
                                }
                                update_all_ancestor_states(parent_archetype->id);
                                
                            }
                        }
                        
                        // Select the top-level parent of the new family.
                        context->selected_entity_id = new_family[0].id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);

                        context->realm_is_dirty = true;
                        build_world_tree_hierarchy();
                    });
                }
            }
            /*
            if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
                // Defer the entire purge operation.
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // --- 1. Find and Orphan All Children ---
                    auto archetype_to_purge_it = context->current_realm_map.find(archetype_id);
                    if (archetype_to_purge_it == context->current_realm_map.end()) return; // Not found
                    
                    std::vector<SimpleGuid> child_ids_copy = archetype_to_purge_it->second->child_ids;
                    for (const auto& child_id : child_ids_copy) {
                        auto child_it = context->current_realm_map.find(child_id);
                        if (child_it != context->current_realm_map.end()) {
                            child_it->second->parent_id = SimpleGuid::invalid();
                        }
                    }

                    // --- 2. Remove From Parent's Child List & Update Parent State ---
                    SimpleGuid parent_id = archetype_to_purge_it->second->parent_id;
                    
                    if (parent_id.is_valid()) {
                        auto parent_it = context->current_realm_map.find(parent_id);
                        if (parent_it != context->current_realm_map.end()) {
                            EntityArchetype* parent_archetype = parent_it->second;

                            parent_archetype->child_ids.erase(
                                std::remove(parent_archetype->child_ids.begin(), parent_archetype->child_ids.end(), archetype_id),
                                parent_archetype->child_ids.end()
                            );

                            // Check and update the parent's state against the snapshot
                            if (parent_archetype->state != ArchetypeState::New) {
                                if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                    parent_archetype->state = ArchetypeState::Modified;
                                } else {
                                    parent_archetype->state = ArchetypeState::UnModified;
                                }
                            }
                            update_all_ancestor_states(parent_archetype->id);
                        }
                    }
                    
                    
                    // --- 3. Remove the Entity Itself From the Realm ---
                    context->current_realm.erase(
                        std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                            [&](const EntityArchetype& e) { return e.id == archetype_id; }),
                        context->current_realm.end()
                    );
                    // --- 4. Update All Systems ---
                    rebuild_current_realm_map_internal();
                    context->realm_is_dirty = true;
                    
                    context->selected_entity_id = SimpleGuid::invalid();
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                    
                    build_world_tree_hierarchy();
                });
            }
            */
            // ---TEST CODE---
            if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
    deferred_commands.push_back([this, archetype_id = archetype.id]() {
        auto archetype_to_purge_it = context->current_realm_map.find(archetype_id);
        if (archetype_to_purge_it == context->current_realm_map.end()) return;

        // --- 1. Orphan All Children (in both live scene and archetypes) ---
        std::vector<SimpleGuid> child_ids_copy = archetype_to_purge_it->second->child_ids;
        for (const auto& child_id : child_ids_copy) {
            // Update the archetype data to orphan the child
            auto child_archetype_it = context->current_realm_map.find(child_id);
            if (child_archetype_it != context->current_realm_map.end()) {
                child_archetype_it->second->parent_id = SimpleGuid::invalid();
            }
            // Update the live scene by orphaning the live child
            Entity* live_child_entity = context->preview_scene->get_entity_by_id(child_id);
            if (live_child_entity) {
                live_child_entity->set_parent(nullptr);
            }
        }

        // --- 2. Remove From Parent's Archetype ---
        SimpleGuid parent_id = archetype_to_purge_it->second->parent_id;
        if (parent_id.is_valid()) {
            auto parent_it = context->current_realm_map.find(parent_id);
            if (parent_it != context->current_realm_map.end()) {
                EntityArchetype* parent_archetype = parent_it->second;
                parent_archetype->child_ids.erase(
                    std::remove(parent_archetype->child_ids.begin(), parent_archetype->child_ids.end(), archetype_id),
                    parent_archetype->child_ids.end()
                );
                if (parent_archetype->state != ArchetypeState::New) parent_archetype->state = ArchetypeState::Modified;
            }
        }

        // --- 3. Purge the Live Entity ---
        Entity* live_entity_to_purge = context->preview_scene->get_entity_by_id(archetype_id);
        if (live_entity_to_purge) {
            live_entity_to_purge->purge(); // Mark for removal from scene
        }

        // --- 4. Remove the Archetype from the Realm Data ---
        context->current_realm.erase(
            std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                [&](const EntityArchetype& e) { return e.id == archetype_id; }),
            context->current_realm.end()
        );

        // --- 5. Update All Systems ---
        rebuild_current_realm_map_internal();
        context->selected_entity_id = SimpleGuid::invalid();
        EntitySelectedEvent event(context->selected_entity_id, nullptr);
        context->event_manager->dispatch(event);
        build_world_tree_hierarchy();

        // For a purge, a full rebuild is still the cleanest way to ensure
        // the preview scene is perfectly in sync with the removed object.
        context->realm_is_dirty = true;
    });
}
            // ---END TEST CODE---
            if (ImGui::MenuItem("Purge Entity Children##PurgeEntityChildren", "Ctrl+Del")) {
                // Defer the entire purge operation.
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // Find the parent archetype whose children will be purged.
                    auto parent_it = context->current_realm_map.find(archetype_id);
                    if (parent_it == context->current_realm_map.end()) return;
                    EntityArchetype* parent_archetype = parent_it->second;

                    // 1. Make a copy of the child IDs before we start modifying the realm.
                    std::vector<SimpleGuid> children_to_purge = parent_archetype->child_ids;

                    // 2. Loop through the list of children to purge.
                    for (const auto& child_id : children_to_purge) {
                        auto child_to_purge_it = context->current_realm_map.find(child_id);
                        if (child_to_purge_it != context->current_realm_map.end()) {
                            EntityArchetype* child_to_purge = child_to_purge_it->second;
                            
                            // 3. Orphan the grandchildren: Set their parent_id to invalid.
                            for (const auto& grandchild_id : child_to_purge->child_ids) {
                                auto grandchild_it = context->current_realm_map.find(grandchild_id);
                                if (grandchild_it != context->current_realm_map.end()) {
                                    grandchild_it->second->parent_id = SimpleGuid::invalid();
                                }
                            }
                        }
                        
                        // 4. Remove the child entity itself from the main realm list.
                        context->current_realm.erase(
                            std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                                [&](const EntityArchetype& e) { return e.id == child_id; }),
                            context->current_realm.end()
                        );
                    }

                    // 5. Clear the parent's original child list and update its state.
                    parent_archetype->child_ids.clear();
                    if (parent_archetype->state != ArchetypeState::New) {
                        if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                            parent_archetype->state = ArchetypeState::Modified;
                        } else {
                            parent_archetype->state = ArchetypeState::UnModified;
                        }
                    }
                    update_all_ancestor_states(parent_archetype->id); 

                    // 6. Update all necessary systems.
                    rebuild_current_realm_map_internal();
                    context->realm_is_dirty = true;
                    build_world_tree_hierarchy();
                });
            }
            if (ImGui::MenuItem("Purge Entity Descendants##PurgeEntityDescendants", "Ctrl+Shift+Del")) {
                // Defer the entire recursive purge operation.
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // 1. A recursive helper function to find all descendant IDs.
                    std::vector<SimpleGuid> descendants_to_purge;
                    std::function<void(const SimpleGuid&)> find_all_children;
                    
                    find_all_children = 
                        [&](const SimpleGuid& current_id) {
                        auto it = context->current_realm_map.find(current_id);
                        if (it != context->current_realm_map.end()) {
                            // For each child of the current entity...
                            for (const auto& child_id : it->second->child_ids) {
                                descendants_to_purge.push_back(child_id); // ...add it to the purge list...
                                find_all_children(child_id);              // ...and then find all of its children.
                            }
                        }
                    };

                    // 2. Start the recursion from the selected archetype.
                    find_all_children(archetype_id);

                    // 3. Remove all the collected descendant entities from the main realm list.
                    if (!descendants_to_purge.empty()) {
                        context->current_realm.erase(
                            std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                                [&](const EntityArchetype& e) {
                                    // Check if the entity's ID is in our purge list.
                                    return std::find(descendants_to_purge.begin(), descendants_to_purge.end(), e.id) != descendants_to_purge.end();
                                }),
                            context->current_realm.end()
                        );
                    }

                    // 4. Clear the child list of the original parent and update its state.
                    auto parent_it = context->current_realm_map.find(archetype_id);
                    if (parent_it != context->current_realm_map.end()) {
                        parent_it->second->child_ids.clear();
                        if (parent_it->second->state != ArchetypeState::New) {
                            if (context->loaded_realm_snapshot.is_entity_modified(*parent_it->second)) {
                                parent_it->second->state = ArchetypeState::Modified;
                            } else {
                                parent_it->second->state = ArchetypeState::UnModified;
                            }
                        }
                        update_all_ancestor_states(parent_it->second->id);
                    }
                    // 5. Update all necessary systems.
                    rebuild_current_realm_map_internal();
                    context->realm_is_dirty = true;
                    build_world_tree_hierarchy();
                });
            }
            if (ImGui::MenuItem("Purge Entity And Family##PurgeEntityAndFamily", "Shift+Del")) {
                // Defer the entire recursive purge operation.
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // 1. A list to hold the ID of the top-level entity AND all its descendants.
                    std::vector<SimpleGuid> family_to_purge;
                    
                    // 2. A recursive helper function to find all descendant IDs.
                    std::function<void(const SimpleGuid&)> find_all_descendants;
                    find_all_descendants = 
                        [&](const SimpleGuid& current_id) {
                        auto it = context->current_realm_map.find(current_id);
                        if (it != context->current_realm_map.end()) {
                            // For each child of the current entity...
                            for (const auto& child_id : it->second->child_ids) {
                                family_to_purge.push_back(child_id);      // ...add it to the purge list...
                                find_all_descendants(child_id);           // ...and then find all of its children.
                            }
                        }
                    };
                    // 3. Start the process by adding the top-level entity itself to the purge list.
                    family_to_purge.push_back(archetype_id);
                    // Then find all of its descendants.
                    find_all_descendants(archetype_id);

                    // 4. Remove the top-level entity from its original parent's child list (if it has one).
                    auto top_level_it = context->current_realm_map.find(archetype_id);
                    if (top_level_it != context->current_realm_map.end()) {
                        SimpleGuid parent_id = top_level_it->second->parent_id;
                        if (parent_id.is_valid()) {
                            auto parent_it = context->current_realm_map.find(parent_id);
                            if (parent_it != context->current_realm_map.end()) {
                                EntityArchetype* parent_archetype = parent_it->second; // Get pointer to parent

                                parent_archetype->child_ids.erase(
                                    std::remove(parent_archetype->child_ids.begin(), parent_archetype->child_ids.end(), archetype_id),
                                    parent_archetype->child_ids.end()
                                );
                                
                                
                                if (parent_archetype->state != ArchetypeState::New) {
                                    if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                                        parent_archetype->state = ArchetypeState::Modified;
                                    } else {
                                        parent_archetype->state = ArchetypeState::UnModified;
                                    }
                                }
                                update_all_ancestor_states(parent_archetype->id); 
                            
                            }
                        }
                    }

                    // 5. Remove all the collected family entities from the main realm list.
                    if (!family_to_purge.empty()) {
                        context->current_realm.erase(
                            std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                                [&](const EntityArchetype& e) {
                                    return std::find(family_to_purge.begin(), family_to_purge.end(), e.id) != family_to_purge.end();
                                }),
                            context->current_realm.end()
                        );
                    }

                    // 6. Update all necessary systems.
                    rebuild_current_realm_map_internal();
                    context->realm_is_dirty = true;
                    
                    // Clear selection and notify other panels.
                    context->selected_entity_id = SimpleGuid::invalid();
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);

                    build_world_tree_hierarchy();
                });
            }
            if (ImGui::MenuItem("Purge Entity Bloodline##PurgeEntityBloodline", "Alt+Shift+Del")) {
                // Defer the entire bloodline purge operation.
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // --- 1. Find the Root of the Bloodline ---
                    SimpleGuid root_id = archetype_id;
                    EntityArchetype* current_archetype = nullptr;
                    
                    // Keep walking up the parent chain until we find the entity with no parent.
                    while (true) {
                        auto it = context->current_realm_map.find(root_id);
                        if (it == context->current_realm_map.end()) {
                            // Should not happen if the starting ID is valid, but it's a safe exit.
                            break;
                        }
                        current_archetype = it->second;
                        if (!current_archetype->parent_id.is_valid()) {
                            break; // We've found the root.
                        }
                        root_id = current_archetype->parent_id;
                    }

                    // --- 2. Collect the Entire Bloodline for Purging ---
                    std::vector<SimpleGuid> bloodline_to_purge;
                    std::function<void(const SimpleGuid&)> find_all_descendants;
                    
                    find_all_descendants = 
                        [&](const SimpleGuid& current_id) {
                        auto it = context->current_realm_map.find(current_id);
                        if (it != context->current_realm_map.end()) {
                            for (const auto& child_id : it->second->child_ids) {
                                bloodline_to_purge.push_back(child_id);
                                find_all_descendants(child_id);
                            }
                        }
                    };

                    // Start by adding the root of the family to the purge list.
                    bloodline_to_purge.push_back(root_id);
                    // Then find all of its descendants.
                    find_all_descendants(root_id);

                    // --- 3. Remove All Bloodline Members from the Realm ---
                    if (!bloodline_to_purge.empty()) {
                        context->current_realm.erase(
                            std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                                [&](const EntityArchetype& e) {
                                    return std::find(bloodline_to_purge.begin(), bloodline_to_purge.end(), e.id) != bloodline_to_purge.end();
                                }),
                            context->current_realm.end()
                        );
                    }

                    // --- 4. Update All Systems ---
                    rebuild_current_realm_map_internal();
                    context->realm_is_dirty = true;
                    
                    // Clear selection and notify other panels.
                    context->selected_entity_id = SimpleGuid::invalid();
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);

                    build_world_tree_hierarchy();
                });
            }
            ImGui::EndPopup();
        }
    }


    void WorldTreePanel::Pimpl::show_element_context_menu(EntityArchetype& parent_archetype, ElementArchetype& element_archetype) {
        if (ImGui::BeginPopupContextItem("ElementContextMenu")) {
            // Header with entity name
            ImGui::TextDisabled("%s", element_archetype.name.c_str());
            ImGui::Separator();
            
            if(ImGui::MenuItem("Rename##RenameElement", "F2")) {
                context->is_editing_property = true;
                element_to_rename_id = element_archetype.id;
                strncpy_s(rename_buffer, sizeof(rename_buffer), 
                        element_archetype.name.c_str(), sizeof(rename_buffer) - 1);
            }
            
            if (element_archetype.allows_duplication) {
                if (ImGui::MenuItem("Duplicate##DuplicateElement", "Ctrl+D")) {
                    // Defer the duplication action.
                    // We capture the IDs by value to ensure they are safe to use later.
                    deferred_commands.push_back([this, parent_id = parent_archetype.id, element_id = element_archetype.id]() {
                        // 1. Find the parent archetype again using the fast map to ensure the pointer is valid.
                        auto parent_it = context->current_realm_map.find(parent_id);
                        if (parent_it == context->current_realm_map.end()) return;
                        EntityArchetype* parent = parent_it->second;

                        // 2. Find the original element to be duplicated within the parent.
                        ElementArchetype* source_element = nullptr;
                        for (auto& elem : parent->elements) {
                            if (elem.id == element_id) {
                                source_element = &elem;
                                break;
                            }
                        }
                        if (!source_element) return; // Original element not found.

                        // 3. Create and add the new element using the upgraded factory method.
                        ElementArchetype duplicated_element = ArchetypeFactory::duplicate_element_archetype(*source_element, *parent);
                        parent->elements.push_back(duplicated_element);

                        
                        // Check and update the parent's state, then update all its ancestors.
                        if (parent->state != ArchetypeState::New) {
                            if (context->loaded_realm_snapshot.is_entity_modified(*parent)) {
                                parent->state = ArchetypeState::Modified;
                            } else {
                                parent->state = ArchetypeState::UnModified;
                            }
                        }
                        update_all_ancestor_states(parent->id);
                        

                        // 4. Select the new element for immediate user feedback.
                        context->selected_element_id = duplicated_element.id;
                        ElementSelectedEvent event(context->selected_element_id, parent->id, nullptr);
                        context->event_manager->dispatch(event);

                        // 5. Mark the realm as dirty.
                        context->realm_is_dirty = true;
                    });
                }
            }

            if (ImGui::MenuItem("Purge##PurgeElement", "Del")) {
                // Defer the purge action.
                // Capturing the parent's ID by value is safer for a deferred command.
                deferred_commands.push_back([this, parent_id = parent_archetype.id, element_id = element_archetype.id]() {
                    // Find the parent again to ensure the pointer is valid.
                    auto parent_it = context->current_realm_map.find(parent_id);
                    if (parent_it == context->current_realm_map.end()) return;
                    
                    EntityArchetype* parent_archetype = parent_it->second;

                    // Use erase-remove_if to find and delete the element from the parent's list.
                    parent_archetype->elements.erase(
                        std::remove_if(parent_archetype->elements.begin(), parent_archetype->elements.end(),
                            [&](const ElementArchetype& e) { return e.id == element_id; }),
                        parent_archetype->elements.end()
                    );
                    
                    
                    // Check and update the parent's state, then update all its ancestors.
                    if (parent_archetype->state != ArchetypeState::New) {
                        if (context->loaded_realm_snapshot.is_entity_modified(*parent_archetype)) {
                            parent_archetype->state = ArchetypeState::Modified;
                        } else {
                            parent_archetype->state = ArchetypeState::UnModified;
                        }
                    }
                    update_all_ancestor_states(parent_archetype->id);
                   

                    // Signal the 3D preview to update.
                    context->realm_is_dirty = true;
                });
            }
            ImGui::EndPopup();
        }
    }

    void WorldTreePanel::Pimpl::setup_entity_drag_source(EntityArchetype& archetype) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ENTITY_DND_GUID", &archetype.id, sizeof(SimpleGuid));
            ImGui::Text("Moving %s", archetype.name.c_str());
            ImGui::EndDragDropSource();
        }
    }

    void WorldTreePanel::Pimpl::handle_entity_drop_target(EntityArchetype& target) {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID")) {
                SimpleGuid dragged_id = *(const SimpleGuid*)payload->Data;
                if (dragged_id != target.id) {
                    deferred_commands.push_back([this, dragged_id, target_id = target.id]() {
                        // Use the captured 'target_id' instead of 'target.id'
                        process_entity_drop(dragged_id, target_id);
                    });
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    void WorldTreePanel::Pimpl::handle_inter_entity_drop_target(EntityArchetype& anchor) {
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 4.0f));
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                ImVec2 rect_min = ImGui::GetItemRectMin();
                ImVec2 rect_max = ImGui::GetItemRectMax();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), ImVec2(rect_max.x, rect_min.y), ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);
                if (ImGui::IsMouseReleased(0)) {
                    SimpleGuid dragged_id = *(SimpleGuid*)payload->Data;
                    deferred_commands.push_back([this, dragged_id, parent_id = anchor.parent_id]() {
                        // Use the captured 'parent_id' instead of 'anchor.parent_id'
                        process_entity_drop(dragged_id, parent_id);
                    });
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    /*
    void WorldTreePanel::Pimpl::process_entity_drop(SimpleGuid dragged_id, SimpleGuid target_id) {
        // Find the dragged archetype
        auto dragged_it = context->current_realm_map.find(dragged_id);
        if (dragged_it == context->current_realm_map.end()) return;
        EntityArchetype* dragged_archetype = dragged_it->second;

        // Circular parenting check
        if (target_id.is_valid()) {
            auto current_id = target_id;
            while (current_id.is_valid()) {
                if (current_id == dragged_id) return;
                auto current_it = context->current_realm_map.find(current_id);
                current_id = (current_it != context->current_realm_map.end()) ? current_it->second->parent_id : SimpleGuid::invalid();
            }
        }

        // Find and update old parent (if it exists)
        if (dragged_archetype->parent_id.is_valid()) {
            auto old_parent_it = context->current_realm_map.find(dragged_archetype->parent_id);
            if (old_parent_it != context->current_realm_map.end()) {
                EntityArchetype* old_parent = old_parent_it->second;
                old_parent->child_ids.erase(
                    std::remove(old_parent->child_ids.begin(), old_parent->child_ids.end(), dragged_id),
                    old_parent->child_ids.end());
                
                // Update old parent's state
                if (old_parent->state != ArchetypeState::New) {
                    if (context->loaded_realm_snapshot.is_entity_modified(*old_parent)) {
                        old_parent->state = ArchetypeState::Modified;
                    } else {
                        old_parent->state = ArchetypeState::UnModified;
                    }
                }
                // Update all ancestors of the old parent
                update_all_ancestor_states(old_parent->id);
                
            }
        }

        // Set the new parent
        dragged_archetype->parent_id = target_id;

        // Find and update new parent (if it exists)
        if (target_id.is_valid()) {
            auto target_it = context->current_realm_map.find(target_id);
            if (target_it != context->current_realm_map.end()) {
                EntityArchetype* target_archetype = target_it->second;
                target_archetype->child_ids.push_back(dragged_id);

                // Update new parent's state
                if (target_archetype->state != ArchetypeState::New) {
                    if (context->loaded_realm_snapshot.is_entity_modified(*target_archetype)) {
                        target_archetype->state = ArchetypeState::Modified;
                    } else {
                        target_archetype->state = ArchetypeState::UnModified;
                    }
                }
                // START ENHANCEMENT: Update all ancestors of the new parent
                update_all_ancestor_states(target_archetype->id);
                // END ENHANCEMENT
            }
        }

        // Update dragged entity's state
        if (dragged_archetype->state != ArchetypeState::New) {
            if (context->loaded_realm_snapshot.is_entity_modified(*dragged_archetype)) {
                dragged_archetype->state = ArchetypeState::Modified;
            } else {
                dragged_archetype->state = ArchetypeState::UnModified;
            }
        }
        
        context->selected_entity_id = dragged_id;
        context->selected_element_id = SimpleGuid::invalid();
        EntitySelectedEvent event(context->selected_entity_id, nullptr);
        context->event_manager->dispatch(event);
        context->realm_is_dirty = true;
        build_world_tree_hierarchy(); // Rebuild hierarchy after reparenting
    }
    */

    // ---TEST CODE---
    void WorldTreePanel::Pimpl::process_entity_drop(SimpleGuid dragged_id, SimpleGuid target_id) {
        auto dragged_it = context->current_realm_map.find(dragged_id);
        if (dragged_it == context->current_realm_map.end()) return;
        EntityArchetype* dragged_archetype = dragged_it->second;

        // Circular parenting check
        if (target_id.is_valid()) {
            auto current_id = target_id;
            while (current_id.is_valid()) {
                if (current_id == dragged_id) return; // Exit if circular
                auto current_it = context->current_realm_map.find(current_id);
                current_id = (current_it != context->current_realm_map.end()) ? current_it->second->parent_id : SimpleGuid::invalid();
            }
        }

        // --- START OF FIX ---
        // 1. Update the LIVE SCENE hierarchy first
        Entity* dragged_live_entity = context->preview_scene->get_entity_by_id(dragged_id);
        // target_live_entity will be nullptr if target_id is invalid (dropping to root), which is correct
        Entity* target_live_entity = context->preview_scene->get_entity_by_id(target_id); 
        
        if (dragged_live_entity) {
            dragged_live_entity->set_parent(target_live_entity);
        }

        // 2. Now, update the ARCHETYPE DATA to match the new state
        // Remove from old parent's archetype child list
        if (dragged_archetype->parent_id.is_valid()) {
            auto old_parent_it = context->current_realm_map.find(dragged_archetype->parent_id);
            if (old_parent_it != context->current_realm_map.end()) {
                EntityArchetype* old_parent = old_parent_it->second;
                old_parent->child_ids.erase(
                    std::remove(old_parent->child_ids.begin(), old_parent->child_ids.end(), dragged_id),
                    old_parent->child_ids.end());
                // Mark old parent as modified
                if (old_parent->state != ArchetypeState::New) old_parent->state = ArchetypeState::Modified;
            }
        }

        // Set the new parent ID in the archetype
        dragged_archetype->parent_id = target_id;
        
        // Add to new parent's archetype child list
        if (target_id.is_valid()) {
            auto target_it = context->current_realm_map.find(target_id);
            if (target_it != context->current_realm_map.end()) {
                EntityArchetype* target_archetype = target_it->second;
                target_archetype->child_ids.push_back(dragged_id);
                // Mark new parent as modified
                if (target_archetype->state != ArchetypeState::New) target_archetype->state = ArchetypeState::Modified;
            }
        }
        
        // Mark dragged entity archetype as modified
        if (dragged_archetype->state != ArchetypeState::New) dragged_archetype->state = ArchetypeState::Modified;

        // 3. Rebuild the UI tree. A full scene rebuild is no longer needed for this operation.
        build_world_tree_hierarchy();
        context->realm_is_dirty = false; // Ensure we don't trigger an unnecessary rebuild
        // --- END OF FIX ---
    }
    // --- END TEST CODE ---


    void WorldTreePanel::Pimpl::build_world_tree_hierarchy() {
        if (!context) return;

        context->world_tree_hierarchy.clear();

        // Map entity_id -> Node for easy parent lookup
        std::unordered_map<SimpleGuid, std::shared_ptr<WorldTreeNode>> node_map;

        // Step 1: create all nodes
        for (auto& entity : context->current_realm) {
            auto node = std::make_shared<WorldTreeNode>();
            node->entity_id = entity.id;
            node_map[entity.id] = node;
        }

        // Step 2: attach nodes to parents or to root
        for (auto& entity : context->current_realm) {
            auto node = node_map[entity.id];
            if (entity.parent_id.is_valid()) {
                auto parent_it = node_map.find(entity.parent_id);
                if (parent_it != node_map.end()) {
                    parent_it->second->children.push_back(node);
                } else {
                    // Parent not found; treat as root and log a warning
                    context->world_tree_hierarchy.push_back(node);
                    std::cerr << "[WorldTreePanel] WARNING: Parent "
                            << entity.parent_id.get_value()
                            << " not found for entity " << entity.id.get_value()
                            << "; adding as root.\n";
                }
            } else {
                // Root entity
                context->world_tree_hierarchy.push_back(node);
            }
        }
    }


    void WorldTreePanel::Pimpl::print_world_tree_hierarchy() const {
        if (!context) return;

        std::function<void(const std::shared_ptr<WorldTreeNode>&, int)> print_node;
        print_node = [&](const std::shared_ptr<WorldTreeNode>& node, int depth) {
            if (!node) return;

            // 4 spaces per depth level for entities
            std::string entity_indent(depth * 4, ' ');

            // Look up entity name
            auto it = std::find_if(
                context->current_realm.begin(),
                context->current_realm.end(),
                [&](const EntityArchetype& e) { return e.id == node->entity_id; }
            );

            std::string entity_name = (it != context->current_realm.end()) ? it->name : "<Unknown>";

            // Print entity ID and name
            std::cout << entity_indent << node->entity_id.get_value() << " - " << entity_name << "\n";

            // Print elements with 2-space offset
            if (it != context->current_realm.end()) {
                node->print_elements_shallow(it->elements, entity_indent + "  "); // 2 extra spaces
            }

            // Recursively print children
            for (const auto& child : node->children) {
                print_node(child, depth + 1);
            }
        };

        for (const auto& root : context->world_tree_hierarchy) {
            print_node(root, 0);
        }
    }


    ImVec4 WorldTreePanel::Pimpl::get_entity_archetype_text_color(const EntityArchetype& current) {
            switch (current.state) {
            case ArchetypeState::New:      return ImVec4(0.4f, 1.0f, 0.4f, 1.0f); // Green
            case ArchetypeState::Modified: return ImVec4(1.0f, 0.7f, 0.3f, 1.0f); // Orange
            case ArchetypeState::UnModified:
            default:                        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        }
    }

        ImVec4 WorldTreePanel::Pimpl::get_element_archetype_text_color(const ElementArchetype& current) {
        switch (current.state) {
            case ArchetypeState::New:      return ImVec4(0.4f, 1.0f, 0.4f, 1.0f); // Green
            case ArchetypeState::Modified: return ImVec4(1.0f, 0.7f, 0.3f, 1.0f); // Orange
            case ArchetypeState::UnModified:
            default:                        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        }
    }


    // --- End of Pimpl methods ---

    ImGuiWindowFlags WorldTreePanel::get_window_flags() const {
        return ImGuiWindowFlags_None;
    }

    void WorldTreePanel::on_panel_gui_update() {
        if (!pimpl->context) return;

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "Scene: (YAML Mode)");
        ImGui::TextDisabled("\tEntities: %zu", pimpl->context->current_realm.size());
        ImGui::Separator();

        if (ImGui::BeginChild("WorldTreeContent", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
                ImGui::OpenPopup("WorldTreeContextMenu");
            }
            if (!pimpl->context->current_realm.empty()) {
                for (size_t i = 0; i < pimpl->context->current_realm.size(); ++i) {
                    auto& entity_archetype = pimpl->context->current_realm[i];
                    if (!entity_archetype.parent_id.is_valid()) {
                        pimpl->render_entity_tree(entity_archetype);
                    }
                }
            }
            pimpl->show_empty_space_context_menu();
        }
        ImGui::EndChild();
        if (!pimpl->deferred_commands.empty()) {
            for (const auto& command : pimpl->deferred_commands) {
                command();
            }
            pimpl->deferred_commands.clear();
        }
    }

   
   
    void WorldTreePanel::on_event(IEvent& event) {
        // 1. The main function now just dispatches to the correct helper
        switch (event.get_event_type()) {
            case EventType::EditorEntitySelected:
                pimpl->handle_entity_selection(static_cast<EntitySelectedEvent&>(event));
                break;
            case EventType::EditorElementSelected:
                pimpl->handle_element_selection(static_cast<ElementSelectedEvent&>(event));
                break;
            case EventType::EditorPropertyValueChanged:
                pimpl->handle_property_value_change(static_cast<PropertyValueChangedEvent&>(event));
                break;
            default:
                break;
        }
    }

    // --- PRIVATE HELPER METHODS ---
    void WorldTreePanel::Pimpl::handle_entity_selection(const EntitySelectedEvent& e) {
        context->selected_entity_id = e.selected_id;
        context->selected_element_id = SimpleGuid::invalid();
        return;
    }

    void WorldTreePanel::Pimpl::handle_element_selection(const ElementSelectedEvent& e) {
        context->selected_entity_id = e.owner_id;
        context->selected_element_id = e.selected_id;
        return;
    }


    
    void WorldTreePanel::Pimpl::handle_property_value_change(const PropertyValueChangedEvent& e) {
        // 1. Find the parent EntityArchetype that was changed.
        auto entity_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
            [&](EntityArchetype& archetype) { return archetype.id == e.entity_id; });

        if (entity_it == context->current_realm.end()) {
            return; // Entity not found
        }

        // 2. Find the specific ElementArchetype that was changed within the entity.
        auto element_it = std::find_if(entity_it->elements.begin(), entity_it->elements.end(),
            [&](ElementArchetype& element) { return element.id == e.element_id; });

        if (element_it == entity_it->elements.end()) {
            return;
        }
        
        // 3. Apply the new value from the event to the archetype's YAML data node.
        element_it->data[e.property_name] = YAML::property_value_to_node(e.new_value);
        
        // --- Synchronize the Element's name property change with data value ---.
        // If the property that changed was "name", we must ALSO update the mirrored 'name' member.
        if (e.property_name == "name") {
            if (std::holds_alternative<std::string>(e.new_value)) {
                element_it->name = std::get<std::string>(e.new_value);
            }
        }
        // 4. Re-evaluate the modified element and entity against the snapshot to update their UI state.
        if (context->loaded_realm_snapshot.is_element_modified(*element_it)) {
            element_it->state = ArchetypeState::Modified;
        } else {
            element_it->state = ArchetypeState::UnModified;
        }

        if (context->loaded_realm_snapshot.is_entity_modified(*entity_it)) {
            entity_it->state = ArchetypeState::Modified;
        } else {
            entity_it->state = ArchetypeState::UnModified;
        }
    }

    // This is fine, I think..
    void WorldTreePanel::Pimpl::update_entity_archetype_state(EntityArchetype& archetype) {
        // An entity that is "New" should always remain new until it's saved.
        if (archetype.state == ArchetypeState::New) {
            std::cout << "[update_entity_archetype_state] EntityArchetype: " << 
            archetype.name <<
            " is 'NEW' and will not change state until file is saved."
            << std::endl;
            return;
        }

        // First, check if the entity's shell (name, parent, etc.) or any of its
        // elements' data has changed from the snapshot.
        if (context->loaded_realm_snapshot.is_entity_modified(archetype)) {
            std::cout << "[update_entity_archetype_state] State [MODIFIED]" << std::endl;
            archetype.state = ArchetypeState::Modified;
            return; // If anything is different, we can stop here.
        }

        // If the above check passes, it means the entity's data matches the snapshot.
        // Now, we must check if any elements are still marked as modified from previous changes.
        for (const auto& element : archetype.elements) {
            if (element.state == ArchetypeState::Modified || element.state == ArchetypeState::New) {
                // If even ONE element is still considered modified, the parent is modified.
                archetype.state = ArchetypeState::Modified;
                return;
            }
        }

        // If we've gotten this far, the entity's data matches the snapshot AND
        // none of its elements are in a modified state.
        std::cout << "[update_entity_archetype_state] State [UNMODIFIED]" << std::endl;
        archetype.state = ArchetypeState::UnModified;
    }
    

    // NEW method for handling ElementArchetype's state change.
    void WorldTreePanel::Pimpl::update_element_archetype_state(ElementArchetype& archetype) {
        // An element that is "New" should always remain new until it's saved.
        if (archetype.state == ArchetypeState::New) {
            std::cout << "[update_element_archetype_state] ElementArchetype: " << 
            archetype.name <<
            " is 'NEW' and will not change state until file is saved."
            << std::endl;
            return;
        }
        if (context->loaded_realm_snapshot.is_element_modified(archetype)) {
            std::cout << "[update_element_archetype_state] '"<< archetype.name << "' State [MODIFIED]" << std::endl;
            archetype.state = ArchetypeState::Modified;
        } else {
            // If it's not different, it means the user changed it back to its original state.
            std::cout << "[update_element_archetype_state] '"<< archetype.name << "' State [UNMODIFIED]" << std::endl;
            archetype.state = ArchetypeState::UnModified;
        }
    }

    // ---- END OF TEST CODE ----

}  // namespace Salix