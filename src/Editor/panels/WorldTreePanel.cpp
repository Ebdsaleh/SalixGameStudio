// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================

#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/ArchetypeFactory.h>
#include <imgui/imgui.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/core/SimpleGuid.h>
#include <Editor/EditorContext.h>
#include <Editor/camera/EditorCamera.h>
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
                for (auto& entity_archetype : pimpl->context->current_realm) {
                    if (!entity_archetype.parent_id.is_valid()) {
                        pimpl->render_entity_tree(entity_archetype);
                    }
                }
            }
            pimpl->show_empty_space_context_menu();
        }
        ImGui::EndChild();
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
                archetype.name = rename_buffer;
                entity_to_rename_id = SimpleGuid::invalid(); // End renaming
            }
        } else {
            bool node_open = ImGui::TreeNodeEx(archetype.name.c_str(), flags);
            setup_entity_drag_source(archetype);
            handle_entity_drop_target(archetype);

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                context->selected_entity_id = archetype.id;
                context->selected_element_id = SimpleGuid::invalid();
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);

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
                            std::string new_name = rename_buffer;
                            element.name = new_name;
                            element.data["name"] = new_name;
                            element_to_rename_id = SimpleGuid::invalid(); // End renaming
                            context->realm_is_dirty = true;
                        }

                    }
                    ImGuiTreeNodeFlags element_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (context->selected_element_id == element.id) {
                        element_flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    ImGui::TreeNodeEx(element.name.c_str(), element_flags);
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
        ImGui::PopID();
    }

    void WorldTreePanel::Pimpl::show_empty_space_context_menu() {
        if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                EntityArchetype new_entity = ArchetypeFactory::create_entity_archetype("Entity");
                if (new_entity.id.is_valid()) {
                    context->current_realm.push_back(new_entity);
                    context->selected_entity_id = new_entity.id;
                    context->selected_element_id = SimpleGuid::invalid();
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                    context->realm_is_dirty = true;
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Refresh View##RefreshWorldTree")) {}
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
                    ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Transform");
                    if (new_element.id.is_valid()) {
                        archetype.elements.push_back(new_element);
                        context->realm_is_dirty = true;
                        context->selected_entity_id = archetype.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                }
                if (ImGui::BeginMenu("Collider##AddColliderMenu")) {
                    ElementArchetype new_element;
                    if(ImGui::MenuItem("Box Collider##AddBoxCollider")) {
                        new_element = ArchetypeFactory::create_element_archetype("BoxCollider");
                    }
                    if (new_element.id.is_valid()) {
                        archetype.elements.push_back(new_element);
                        context->realm_is_dirty = true;
                        // Mirror Live mode selection
                        context->selected_entity_id = archetype.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Sprite2D##AddSprite2D")) {
                    ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Sprite2D");
                    if (new_element.id.is_valid()) { // Check if the factory was successful
                        archetype.elements.push_back(new_element);
                        context->realm_is_dirty = true;
                        // Mirror Live mode selection
                        context->selected_entity_id = archetype.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                }
                
                if (ImGui::BeginMenu("Script##AddScriptMenu")) {
                    ElementArchetype new_element;
                    if (ImGui::MenuItem("C++ Script##AddCPPScript")) {
                        new_element = ArchetypeFactory::create_element_archetype("CppScript");
                    }
                    if (ImGui::MenuItem("Python Script##AddPythonScript")) {
                        new_element = ArchetypeFactory::create_element_archetype("PythonScript");
                    }
                    if (new_element.id.is_valid()) { // Check if the factory was successful
                        archetype.elements.push_back(new_element);
                        context->realm_is_dirty = true;
                        // Mirror Live mode selection
                        context->selected_entity_id = archetype.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                   ImGui::EndMenu(); 
                }
                
                
                if (ImGui::MenuItem("Camera##AddCamera")) {
                    ElementArchetype new_element = ArchetypeFactory::create_element_archetype("Camera");
                    if (new_element.id.is_valid()) { // Check if the factory was successful
                        archetype.elements.push_back(new_element);
                        context->realm_is_dirty = true;
                        // Mirror Live mode selection
                        context->selected_entity_id = archetype.id;
                        EntitySelectedEvent event(context->selected_entity_id, nullptr);
                        context->event_manager->dispatch(event);
                    }
                }
                ImGui::EndMenu();
            }

            // Hierarchy Operations
            if (archetype.parent_id.get_value() != 0) {
                if(ImGui::MenuItem("Release From Parent##ReleaseFromParent")) {
                    auto parent_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == archetype.parent_id; });
                    
                    if (parent_it != context->current_realm.end()) {
                        // Remove from parent's child_ids
                        parent_it->child_ids.erase(
                            std::remove(parent_it->child_ids.begin(), parent_it->child_ids.end(), archetype.id),
                            parent_it->child_ids.end());
                        
                        // Clear our parent reference
                        archetype.parent_id = SimpleGuid::invalid();
                    }
                }
            }

            if (ImGui::MenuItem("Add Child Entity##AddChildEntity")) {
                EntityArchetype new_entity = ArchetypeFactory::create_entity_archetype(archetype.name + "_Child");

                new_entity.parent_id = archetype.id;
                if (new_entity.id.is_valid()){
                    archetype.child_ids.push_back(new_entity.id);
                    context->current_realm.push_back(new_entity);
                    context->realm_is_dirty = true;
                    // Mirror Live selection
                    context->selected_entity_id = new_entity.id;
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Rename##RenameEntity", "F2")) {
                context->is_editing_property = true;
                entity_to_rename_id = archetype.id;
                strncpy_s(rename_buffer, sizeof(rename_buffer), 
                        archetype.name.c_str(), sizeof(rename_buffer) - 1);
            }

            if (ImGui::MenuItem("Duplicate##DuplicateEntity", "Ctrl+D")) {
                // Use the factory to create a clean, deep copy with new IDs.
                EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(archetype, context->current_realm);

                                
                // Add the new archetype to the realm.
                context->current_realm.push_back(duplicated_archetype);

                
                
                // Select the new entity to give the user immediate feedback.
                context->selected_entity_id = duplicated_archetype.id;
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);

                // Mark the realm as dirty to trigger a preview refresh.
                context->realm_is_dirty = true;
            }

            if (ImGui::MenuItem("Duplicate As Sibling##DuplicateEntityAsSibling", "Ctrl+Alt+D")) {
                // Use the simple duplicate method. It correctly creates a single copy.
                EntityArchetype duplicated_archetype = ArchetypeFactory::duplicate_entity_archetype(
                    archetype, 
                    context->current_realm
                );

                // Set the parent to the SOURCE's parent to make it a sibling.
                duplicated_archetype.parent_id = archetype.parent_id;

                // Add the new archetype to the realm first, so the parent can find it.
                context->current_realm.push_back(duplicated_archetype);

                // Find the parent and add this new entity to its child_ids list.
                SimpleGuid parent_id = duplicated_archetype.parent_id;
                if (parent_id.is_valid()) {
                    auto parent_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](EntityArchetype& e) { return e.id == parent_id; });
                    
                    if (parent_it != context->current_realm.end()) {
                        parent_it->child_ids.push_back(duplicated_archetype.id);
                    }
                }
                
                // Select the new entity to give the user immediate feedback.
                context->selected_entity_id = duplicated_archetype.id;
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);

                // Mark the realm as dirty to trigger a preview refresh.
                context->realm_is_dirty = true;
            }
            if (ImGui::MenuItem("Duplicate With Children##DuplicateEntityWithChildren", "Ctrl+Shift+D")) {
                // This calls the new function that returns the entire family.
                std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_and_children(
                    archetype, 
                    context->current_realm
                );

                // Add all the newly created entities (parent and children) to the realm.
                for (const auto& new_member : new_family) {
                    context->current_realm.push_back(new_member);
                }
                
                // Select the top-level parent of the new family.
                if (!new_family.empty()) {
                    context->selected_entity_id = new_family[0].id;
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                }

                context->realm_is_dirty = true;
            }

            if (ImGui::MenuItem("Duplicate Family As Sibling##DuplicateFamilyAsSibling", "Ctrl+Alt+Shift+D")) {

                // This calls the new function that returns the entire family and adds it as a sibling to the source Entity.
                std::vector<EntityArchetype> new_family = ArchetypeFactory::duplicate_entity_archetype_family_as_sibling(
                    archetype, 
                    context->current_realm
                );

                // Safety check in case duplication fails
                if (new_family.empty()) {
                    
                } else {
                    
                    for (const auto& new_member : new_family) {
                        context->current_realm.push_back(new_member);
                    }
                    
                   
                    // Find the parent entity and add the new sibling to its child list.
                    SimpleGuid parent_id = new_family[0].parent_id;
                    if (parent_id.is_valid()) {
                        // Find the parent in the main realm list. Note: it must be mutable to change its child_ids.
                        auto parent_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                            [&](EntityArchetype& e) { return e.id == parent_id; });
                        
                        if (parent_it != context->current_realm.end()) {
                            // Add the new entity's ID to its parent's list of children.
                            parent_it->child_ids.push_back(new_family[0].id);
                        }
                    }
                    
                    
                    context->selected_entity_id = new_family[0].id;
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                    context->realm_is_dirty = true;
                }
            }

            if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
                // --- Release all children before purging the parent ---
                for (const auto& child_id : archetype.child_ids) {
                    auto child_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == child_id; });
                    
                    if (child_it != context->current_realm.end()) {
                        // Set the child's parent to invalid, making it a root entity.
                        child_it->parent_id = SimpleGuid::invalid();
                    }
                }
                // Remove from old parent's child list (if it has a parent)
                if (archetype.parent_id.is_valid()) {
                    auto parent_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == archetype.parent_id; });
                    if (parent_it != context->current_realm.end()) {
                        parent_it->child_ids.erase(
                            std::remove(parent_it->child_ids.begin(), parent_it->child_ids.end(), archetype.id),
                            parent_it->child_ids.end());
                    }
                }
                
                // Remove the entity itself from the realm
                context->current_realm.erase(
                    std::remove_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == archetype.id; }),
                    context->current_realm.end());

                // --- Signal the RealmDesignerPanel to update ---
                context->realm_is_dirty = true;
                
                // Clear selection and notify other panels
                context->selected_entity_id = SimpleGuid::invalid();
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);
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

                    // Use the factory to create a clean copy of the element.
                    ElementArchetype duplicated_element = ArchetypeFactory::duplicate_element_archetype(element_archetype);
                    
                    // Add the new element to its parent entity's list.
                    parent_archetype.elements.push_back(duplicated_element);

                    // Select the new element for immediate user feedback.
                    context->selected_element_id = duplicated_element.id;
                    
                    // Dispatch an event so other panels (like the Inspector) can update.
                    ElementSelectedEvent event(context->selected_element_id, context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);

                    // Mark the realm as dirty to trigger a preview refresh.
                    context->realm_is_dirty = true;
                }
            }

            if (ImGui::MenuItem("Purge##PurgeElement", "Del")) {
                 // Use erase-remove_if to find and delete the element from the parent's list
                parent_archetype.elements.erase(
                std::remove_if(parent_archetype.elements.begin(), parent_archetype.elements.end(),
                    [&](const ElementArchetype& e) { return e.id == element_archetype.id; }),
                parent_archetype.elements.end());
            
                // Signal the 3D preview to update
                context->realm_is_dirty = true;
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
                    process_entity_drop(dragged_id, target.id);
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
                    process_entity_drop(dragged_id, anchor.parent_id);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    void WorldTreePanel::Pimpl::process_entity_drop(SimpleGuid dragged_id, SimpleGuid target_id) {
        auto dragged_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
            [&](const EntityArchetype& e) { return e.id == dragged_id; });
        if (dragged_it == context->current_realm.end()) return;

        // Circular parenting check
        if (target_id.is_valid()) {
            auto current_id = target_id;
            while (current_id.is_valid()) {
                if (current_id == dragged_id) return; // Circular dependency
                auto current_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                    [&](const EntityArchetype& e) { return e.id == current_id; });
                current_id = (current_it != context->current_realm.end()) ? current_it->parent_id : SimpleGuid::invalid();
            }
        }

        // Remove from old parent
        if (dragged_it->parent_id.is_valid()) {
            auto old_parent_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                [&](const EntityArchetype& e) { return e.id == dragged_it->parent_id; });
            if (old_parent_it != context->current_realm.end()) {
                old_parent_it->child_ids.erase(
                    std::remove(old_parent_it->child_ids.begin(), old_parent_it->child_ids.end(), dragged_id),
                    old_parent_it->child_ids.end());
            }
        }

        // Add to new parent
        dragged_it->parent_id = target_id;
        if (target_id.is_valid()) {
            auto target_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                [&](const EntityArchetype& e) { return e.id == target_id; });
            if (target_it != context->current_realm.end()) {
                target_it->child_ids.push_back(dragged_id);
            }
        }
        
        context->selected_entity_id = dragged_id;
        context->selected_element_id = SimpleGuid::invalid();
        EntitySelectedEvent event(context->selected_entity_id, nullptr);
        context->event_manager->dispatch(event);
        context->realm_is_dirty = true;
    }


    void WorldTreePanel::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::EditorPropertyValueChanged) {
            PropertyValueChangedEvent& e = static_cast<PropertyValueChangedEvent&>(event);

            // We only care about changes to the "name" property for this sync issue.
            if (e.property_name == "name") {
                // Find the entity archetype that was changed.
                auto entity_it = std::find_if(pimpl->context->current_realm.begin(), pimpl->context->current_realm.end(),
                    [&](const EntityArchetype& archetype) { return archetype.id == e.entity_id; });

                if (entity_it != pimpl->context->current_realm.end()) {
                    // Find the specific element within that entity by its type name.
                    for (auto& element : entity_it->elements) {
                        if (element.type_name == e.element_type_name) {
                            // Found it. Update the name member from the event's data.
                            element.name = std::get<std::string>(e.new_value);
                            break; // Assume only one element of this type per entity
                        }
                    }
                }
            }
        }
    }
}