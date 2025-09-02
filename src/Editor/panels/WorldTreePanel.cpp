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
#include <Editor/events/OnRootEntityAddedEvent.h>
#include <Editor/events/OnEntityAddedEvent.h>
#include <Editor/events/OnEntityFamilyAddedEvent.h>
#include <Editor/events/OnEntityPurgedEvent.h>
#include <Editor/events/OnEntityFamilyPurgedEvent.h>
#include <Editor/events/OnMainCameraChangedEvent.h>
#include <Editor/management/EditorRealmManager.h>
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
        ImVec4 get_entity_archetype_text_color(const EntityArchetype& current);
        ImVec4 get_element_archetype_text_color(const ElementArchetype& current);
        // Event helper methods
        void handle_entity_selection(const EntitySelectedEvent& e);
        void handle_element_selection(const ElementSelectedEvent& e);
        void handle_property_value_change(const PropertyValueChangedEvent& e);
        void update_entity_archetype_state(EntityArchetype& archetype);
        void update_element_archetype_state(ElementArchetype& archetype);
        std::vector<std::function<void()>> deferred_commands;

        
    };

   

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
        
        

    }


    // --- PIMPL Methods ---

    

    void WorldTreePanel::Pimpl::render_entity_tree(EntityArchetype& archetype) {
        ImGui::PushID(static_cast<int>(archetype.id.get_value()));
        handle_inter_entity_drop_target(archetype);
        
        // This logic is unchanged, it correctly determines how the tree node should look.
        const bool is_populated = !archetype.child_ids.empty() || !archetype.elements.empty();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (context->selected_entity_id == archetype.id) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!is_populated) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
        }

        // --- Renaming Logic (Updated to use the manager) ---
        bool is_renaming = (entity_to_rename_id == archetype.id);
        if (is_renaming) {
            ImGui::SetKeyboardFocusHere(0);
            context->is_editing_property = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                entity_to_rename_id = SimpleGuid::invalid();
            }
            
            bool rename_confirmed = ImGui::InputText("##RenameBox", rename_buffer, sizeof(rename_buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
            bool rename_deactivated = ImGui::IsItemDeactivatedAfterEdit();

            if (rename_confirmed || rename_deactivated) {
                EntityArchetype temp_archetype = archetype;
                temp_archetype.name = rename_buffer;
                
                // Use the manager to get the snapshot for comparison
                if (context->editor_realm_manager->get_snapshot()->is_entity_modified(temp_archetype)) {
                    archetype.state = ArchetypeState::Modified;
                } else {
                    archetype.state = ArchetypeState::UnModified;
                }
                
                archetype.name = rename_buffer;
                entity_to_rename_id = SimpleGuid::invalid();
            }
        } else {
            // --- Standard Display Logic (Unchanged) ---
            ImVec4 entity_text_color = get_entity_archetype_text_color(archetype);
            ImGui::PushStyleColor(ImGuiCol_Text, entity_text_color);
            bool node_open = ImGui::TreeNodeEx((void*)archetype.id.get_value(), flags, "%s", archetype.name.c_str());
            ImGui::PopStyleColor();

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                context->selected_entity_id = archetype.id;
                context->selected_element_id = SimpleGuid::invalid();
                
                context->event_manager->dispatch(
                    std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                );
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                Entity* live_entity_to_focus = context->preview_scene->get_entity_by_id(archetype.id);
                if (live_entity_to_focus && live_entity_to_focus->get_transform()) {
                    context->editor_camera->focus_on(live_entity_to_focus->get_transform(), 3.0f);
                }
            }
            
            setup_entity_drag_source(archetype);
            handle_entity_drop_target(archetype);
            show_entity_context_menu(archetype);


            

            if (node_open) {
                // Element Rendering (Updated to use the manager for snapshot checks)
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
                            
                            context->event_manager->dispatch(
                                std::make_unique<PropertyValueChangedEvent>(
                                archetype.id,
                                element.id,
                                element.type_name,
                                "name",
                                new_name)
                            );
                            //  Update the ElementArchetype state.
                            if(context->editor_realm_manager->get_snapshot()->is_element_modified(element)){
                                element.state = ArchetypeState::Modified;
                            }
                            else {
                                element.state = ArchetypeState::UnModified;
                            }
                            
                            // Update the EntityArchetype state.
                            if (context->editor_realm_manager->get_snapshot()->is_entity_modified(archetype)) {
                                archetype.state = ArchetypeState::Modified;
                            }
                            else {
                                if (archetype.state == ArchetypeState::Modified) {
                                    archetype.state = ArchetypeState::UnModified;
                                }
                            }
                            element_to_rename_id = SimpleGuid::invalid(); // End renaming
                            
                        }

                    }
                    ImGuiTreeNodeFlags element_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (context->selected_element_id == element.id) {
                        element_flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    ImVec4 element_text_color = get_element_archetype_text_color(element);
                    ImGui::PushStyleColor(ImGuiCol_Text, element_text_color);
                    ImGui::TreeNodeEx((void*)element.id.get_value(), element_flags, "%s", element.name.c_str());
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemClicked()) {
                        context->selected_element_id = element.id;
                        context->selected_entity_id = archetype.id;
                        
                        context->event_manager->dispatch(
                            std::make_unique<ElementSelectedEvent>(
                            context->selected_element_id,
                            context->selected_entity_id,
                            nullptr)
                        );
                    }
                    show_element_context_menu(archetype, element);
                    ImGui::PopID();
                }
                // --- Child Rendering (THIS IS THE KEY CHANGE) ---
                // Instead of searching the raw vector, we now get the children directly
                // from the manager, which is guaranteed to be correct.
                for (const auto& child_id : archetype.child_ids) {
                    EntityArchetype* child_archetype = context->editor_realm_manager->get_archetype(child_id);
                    if (child_archetype) {
                        ImGui::Indent(child_indent);
                        render_entity_tree(*child_archetype);
                        ImGui::Unindent(child_indent);
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }

    void WorldTreePanel::Pimpl::show_empty_space_context_menu() {
        if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                deferred_commands.push_back([this]() {
                    // The panel's only job is to create the data and send the command.
                    EntityArchetype new_entity = ArchetypeFactory::create_entity_archetype("Entity");
                    if (new_entity.id.is_valid()) {
                        // Tell the manager to add the entity. The manager handles everything else.
                        context->editor_realm_manager->add_entity(new_entity);

                        // The panel is still responsible for UI state, like what is selected.
                        context->selected_entity_id = new_entity.id;
                        context->selected_element_id = SimpleGuid::invalid();
                        
                        context->event_manager->dispatch(
                            std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                        );
                    }
                });
            }
            
            ImGui::Separator();

            // This button is now a no-op because the view is always fresh.
            // We can disable it or remove it entirely.
            if (ImGui::MenuItem("Refresh View##RefreshWorldTree", nullptr, false, false)) {
                // This will never be called as the item is disabled.
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Not required with the new Realm Manager!");
            }

            ImGui::EndPopup();
        }
    }

    void WorldTreePanel::Pimpl::show_entity_context_menu(EntityArchetype& archetype) {
        if (ImGui::BeginPopupContextItem("EntityContextMenu")) {
            // Header with entity name
            ImGui::TextDisabled("%s", archetype.name.c_str());
            ImGui::Separator();

            if (archetype.has_element_of_type("Camera")) {
                if (ImGui::MenuItem("Set As Realm Camera##SetAsRealmCamera")) {
                    deferred_commands.push_back([this, owner_id = archetype.id]() {
                        
                        context->event_manager->dispatch(
                            std::make_unique<OnMainCameraChangedEvent>(owner_id)
                        );
                    });
                }
                ImGui::Separator();
            }

            // Element Creation Submenu
            if (ImGui::BeginMenu("Add Element##AddElementMenu")) {
                if (ImGui::MenuItem("Transform##AddTransform")) {
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // Create the new element data.
                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Transform");
                        new_element.owner_id = archetype_id;
                        
                        if (new_element.id.is_valid()) {
                            // Tell the manager to add the element. The manager handles everything else.
                            context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                            // Reselect the entity to refresh the Inspector panel.
                            context->selected_entity_id = archetype_id;
                            
                            context->event_manager->dispatch(
                                std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                            );
                        }
                    });
                }
                if (ImGui::BeginMenu("Collider##AddColliderMenu")) {
                    if (ImGui::MenuItem("Box Collider##AddBoxCollider")) {
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Create the new element data.
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("BoxCollider");
                            new_element.owner_id = archetype_id;
                            
                            if (new_element.id.is_valid()) {
                                // Tell the manager to add the element. The manager handles all data
                                // manipulation, state updates, and event dispatching.
                                context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                                // Reselect the entity to refresh the Inspector panel.
                                context->selected_entity_id = archetype_id;
                                
                                context->event_manager->dispatch(
                                    std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                                );
                            }
                        });
                    }
                    // Will add other collider types here (e.g., Sphere, Capsule) following the same pattern.
                    ImGui::EndMenu();
                }
                
                if (ImGui::MenuItem("Sprite2D##AddSprite2D")) {
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // 1. Create the new element data.
                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Sprite2D");
                        new_element.owner_id = archetype_id;
                        
                        if (new_element.id.is_valid()) {
                            // 2. Tell the manager to add the element. It handles all the complex logic internally.
                            context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                            // 3. Reselect the entity to refresh the Inspector panel.
                            context->selected_entity_id = archetype_id;
                            
                            context->event_manager->dispatch(
                                std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                            );
                        }
                    });
                }
                
                if (ImGui::BeginMenu("Script##AddScriptMenu")) {
                    if (ImGui::MenuItem("C++ Script##AddCPPScript")) {
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Create the new element data
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("CppScript");
                            new_element.owner_id = archetype_id;
                            
                            if (new_element.id.is_valid()) {
                                // Tell the manager to add the element
                                context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                                // Reselect the entity to refresh the Inspector
                                context->selected_entity_id = archetype_id;
                                
                                context->event_manager->dispatch(
                                    std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                                );
                            }
                        });
                    }

                    if (ImGui::MenuItem("Python Script##AddPythonScript")) {
                        deferred_commands.push_back([this, archetype_id = archetype.id]() {
                            // Create the new element data
                            ElementArchetype new_element = ArchetypeFactory::create_element_archetype("PythonScript");
                            new_element.owner_id = archetype_id;

                            if (new_element.id.is_valid()) {
                                // Tell the manager to add the element
                                context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                                // Reselect the entity to refresh the Inspector
                                context->selected_entity_id = archetype_id;
                                
                                context->event_manager->dispatch(
                                    std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                                );
                            }
                        });
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::MenuItem("Camera##AddCamera")) {
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // 1. Create the new element data.
                        ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Camera");
                        new_element.owner_id = archetype_id;
                        
                        if (new_element.id.is_valid()) {
                            // 2. Tell the manager to add the element. It handles everything else.
                            context->editor_realm_manager->add_element_to_entity(archetype_id, std::move(new_element));

                            // 3. Reselect the entity to refresh the Inspector panel.
                            context->selected_entity_id = archetype_id;
                            
                            context->event_manager->dispatch(
                                std::make_unique<EntitySelectedEvent>(context->selected_entity_id, nullptr)
                            );
                        }
                    });
                }
                // This should be followed by the existing ImGui::EndMenu() call for the "Add Element" menu
                ImGui::EndMenu();
            }

            // Hierarchy Operations
            if (archetype.parent_id.is_valid()) {
                if (ImGui::MenuItem("Release From Parent##ReleaseFromParent")) {
                    deferred_commands.push_back([this, archetype_id = archetype.id]() {
                        // Just send the command to the manager
                        context->editor_realm_manager->release_from_parent(archetype_id);
                    });
                }
            }
            
            if (ImGui::MenuItem("Add Child Entity##AddChildEntity")) {
                deferred_commands.push_back([this, parent_id = archetype.id]() {
                    EntityArchetype new_child = ArchetypeFactory::create_entity_archetype("New Child");
                    new_child.parent_id = parent_id;
                    // Just send the command to the manager
                    context->editor_realm_manager->add_child_entity(std::move(new_child));
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
                deferred_commands.push_back([this, source_id = archetype.id]() {
                    context->editor_realm_manager->duplicate_entity(source_id);
                });
            }

            if (ImGui::MenuItem("Duplicate As Sibling##DuplicateEntityAsSibling", "Ctrl+Alt+D")) {
                deferred_commands.push_back([this, source_id = archetype.id]() {
                    context->editor_realm_manager->duplicate_entity_as_sibling(source_id);
                });
            }


             if (ImGui::MenuItem("Duplicate With Children##DuplicateEntityWithChildren", "Ctrl+Shift+D")) {
                deferred_commands.push_back([this, source_id = archetype.id]() {
                    context->editor_realm_manager->duplicate_entity_with_children(source_id);
                });
            }

            if (ImGui::MenuItem("Duplicate Family As Sibling##DuplicateFamilyAsSibling", "Ctrl+Alt+Shift+D")) {
                deferred_commands.push_back([this, source_id = archetype.id]() {
                    context->editor_realm_manager->duplicate_family_as_sibling(source_id);
                });
            }
            

            if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    context->editor_realm_manager->purge_entity(archetype_id);
                });
            }

            if (ImGui::MenuItem("Purge Entity Descendants##PurgeEntityDescendants", "Ctrl+Shift+Del")) {
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // Just send the command. The manager handles all the complex work.
                    context->editor_realm_manager->purge_entity_descendants(archetype_id);
                });
            }
             
            if (ImGui::MenuItem("Purge Entity And Family##PurgeEntityAndFamily", "Shift+Del")) {
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    context->editor_realm_manager->purge_entity_and_family(archetype_id);
                });
            }

            
            if (ImGui::MenuItem("Purge Entity Bloodline##PurgeEntityBloodline", "Alt+Shift+Del")) {
                deferred_commands.push_back([this, archetype_id = archetype.id]() {
                    // Just send the command. The manager handles all the complex work.
                    context->editor_realm_manager->purge_entity_bloodline(archetype_id);

                    // If the selected entity was part of the purged family, clear the selection.
                    if (context->selected_entity_id == archetype_id) {
                        context->selected_entity_id = SimpleGuid::invalid();
                    }
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
            if (element_archetype.type_name == "Camera") {
                ImGui::Separator();
                if (ImGui::MenuItem("Set As Realm Camera##SetAsRealmCamera")) {
                    deferred_commands.push_back([this, owner_id = parent_archetype.id]() {
                        // Dispatch the new event with the owner entity's ID
                        
                        context->event_manager->dispatch(
                            std::make_unique<OnMainCameraChangedEvent>(owner_id)
                        );
                    });
                }
            } 
            if(ImGui::MenuItem("Rename##RenameElement", "F2")) {
                context->is_editing_property = true;
                element_to_rename_id = element_archetype.id;
                strncpy_s(rename_buffer, sizeof(rename_buffer), 
                        element_archetype.name.c_str(), sizeof(rename_buffer) - 1);
            }
            
            if (element_archetype.allows_duplication) {
                if (ImGui::MenuItem("Duplicate##DuplicateElement", "Ctrl+D")) {
                    // Defer the command to call the manager.
                    deferred_commands.push_back([this, parent_id = parent_archetype.id, element_id = element_archetype.id]() {
                        context->editor_realm_manager->duplicate_element(parent_id, element_id);
                    });
                }
            }
            

            if (ImGui::MenuItem("Purge##PurgeElement", "Del")) {
                // Defer the command to call the manager.
                deferred_commands.push_back([this, parent_id = parent_archetype.id, element_id = element_archetype.id]() {
                    context->editor_realm_manager->purge_element(parent_id, element_id);
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


    // ---TEST CODE---
    void WorldTreePanel::Pimpl::process_entity_drop(SimpleGuid dragged_id, SimpleGuid target_id) {
        // --- STEP 1: PRE-FLIGHT CHECKS (UI-level validation) ---
        EntityArchetype* dragged_archetype = context->editor_realm_manager->get_archetype(dragged_id);
        if (!dragged_archetype) return;

        // Check for invalid operations
        if (dragged_archetype->parent_id == target_id) return; // Dropped on current parent
        if (dragged_id == target_id) return; // Dropped on self

        // Circular dependency check (can't drop a parent on its own child)
        if (target_id.is_valid()) {
            SimpleGuid current_id = target_id;
            while (current_id.is_valid()) {
                if (current_id == dragged_id) return; 
                
                EntityArchetype* current_archetype = context->editor_realm_manager->get_archetype(current_id);
                if (!current_archetype) break;
                current_id = current_archetype->parent_id;
            }
        }

        // --- STEP 2: COMMAND THE DATA CHANGE ---
        // Tell the manager to perform the raw data update and its internal synchronization.
        context->editor_realm_manager->reparent_entity(dragged_id, target_id);

        // --- STEP 3: UPDATE ARTIFACTS (UI State, etc.) ---
        // The panel is responsible for updating the state after the fact.
        if (dragged_archetype->state != ArchetypeState::New) {
            dragged_archetype->state = ArchetypeState::Modified;
        }
        
        // Update states of old and new parents if they exist
        SimpleGuid old_parent_id = dragged_archetype->parent_id; // Note: this is the *old* parent id before reparenting
        if (old_parent_id.is_valid()) {
            EntityArchetype* old_parent = context->editor_realm_manager->get_archetype(old_parent_id);
            if (old_parent && old_parent->state != ArchetypeState::New) {
                old_parent->state = ArchetypeState::Modified;
            }
        }
        if (target_id.is_valid()) {
            EntityArchetype* new_parent = context->editor_realm_manager->get_archetype(target_id);
            if (new_parent && new_parent->state != ArchetypeState::New) {
                new_parent->state = ArchetypeState::Modified;
            }
        }
        context->editor_realm_manager->update_ancestor_states(dragged_id);

        // --- STEP 4: DISPATCH EVENT FOR LISTENERS (like RealmDesignerPanel) ---
        
        context->event_manager->dispatch(
            std::make_unique<OnHierarchyChangedEvent>(dragged_id, target_id)
        );
    }
    // --- END TEST CODE ---


   


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

    // In: src/Editor/panels/WorldTreePanel.cpp

    void WorldTreePanel::on_panel_gui_update() {
        if (!pimpl->context || !pimpl->context->editor_realm_manager) return;

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "Scene: (YAML Mode)");
        ImGui::TextDisabled("\tEntities: %zu", pimpl->context->editor_realm_manager->get_realm_size());
        ImGui::Separator();

        if (ImGui::BeginChild("WorldTreeContent", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
                ImGui::OpenPopup("WorldTreeContextMenu");
            }
            
            // --- CORRECTED LOGIC ---
            // Instead of looping through the entire realm, we now loop through the
            // pre-built hierarchy of root nodes provided by the manager. This is much more efficient.
            if (!pimpl->context->editor_realm_manager->get_hierarchy().empty()) {
                for (const auto& root_node : pimpl->context->editor_realm_manager->get_hierarchy()) {
                    // The node gives us the ID, and we use the manager to safely get the archetype data.
                    EntityArchetype* archetype = pimpl->context->editor_realm_manager->get_archetype(root_node->entity_id);
                    if (archetype) {
                        pimpl->render_entity_tree(*archetype);
                    }
                }
            }
            
            pimpl->show_empty_space_context_menu();
        }
        ImGui::EndChild();

        // The deferred command logic from your original file remains the same.
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
        // 1. Get the parent EntityArchetype directly using the manager's fast lookup.
        EntityArchetype* entity_archetype = context->editor_realm_manager->get_archetype(e.entity_id);

        if (!entity_archetype) {
            return; // Entity not found
        }

        // 2. Get the specific ElementArchetype using the helper method on the entity.
        ElementArchetype* element_archetype = entity_archetype->get_element_by_id(e.element_id);

        if (!element_archetype) {
            return; // Element not found
        }
        
        // Put a clause in here to prevent it from overwriting the camera's projection mode after its been set
        // With the ScryingMirrorPanel
        if (e.property_name != "projection_mode") {
            // 3. Apply the new value from the event to the archetype's YAML data node.
            element_archetype->data[e.property_name] = YAML::property_value_to_node(e.new_value);
        }

        // --- Synchronize the Element's name property change with its data value ---.
        // If the property that changed was "name", we must ALSO update the mirrored 'name' member.
        if (e.property_name == "name") {
            if (std::holds_alternative<std::string>(e.new_value)) {
                element_archetype->name = std::get<std::string>(e.new_value);
            }
        }

        // 4. Re-evaluate the modified element and entity against the snapshot to update their UI state.
        if (context->editor_realm_manager->get_snapshot()->is_element_modified(*element_archetype)) {
            element_archetype->state = ArchetypeState::Modified;
        } else {
            element_archetype->state = ArchetypeState::UnModified;
        }

        if (context->editor_realm_manager->get_snapshot()->is_entity_modified(*entity_archetype)) {
            entity_archetype->state = ArchetypeState::Modified;
        } else {
            entity_archetype->state = ArchetypeState::UnModified;
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
        if (context->editor_realm_manager->get_snapshot()->is_entity_modified(archetype)) {
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
        if (context->editor_realm_manager->get_snapshot()->is_element_modified(archetype)) {
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