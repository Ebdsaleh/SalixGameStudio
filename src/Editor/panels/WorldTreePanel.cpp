// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================

#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/ArchetypeFactory.h>
#include <imgui/imgui.h>
#include <Salix/math/Color.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>
#include <Salix/core/InitContext.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Editor/EditorContext.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/gui/IGui.h>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>


namespace Salix {
    struct WorldTreePanel::Pimpl {
        EditorContext* context = nullptr;
        Entity* entity_to_rename = nullptr;
        SimpleGuid entity_to_rename_id = SimpleGuid::invalid();
        SimpleGuid element_to_rename_id = SimpleGuid::invalid();
        char rename_buffer[256];
        float child_indent = 22.0f;
        // --- YAML PATHWAY HELPERS ---
        // Using Archetypes
        void render_entity_tree_ARCHETYPE(EntityArchetype& archetype);
        void show_empty_space_context_menu_ARCHETYPE();
        void show_entity_context_menu_ARCHETYPE(EntityArchetype& archetype);
        void show_element_context_menu_ARCHETYPE(EntityArchetype& parent_archetype, ElementArchetype& element_archtype);
        void setup_entity_drag_source_ARCHETYPE(EntityArchetype& archetype);
        void process_entity_drop_ARCHETYPE(SimpleGuid dragged_id, SimpleGuid target_id);
        void handle_entity_drop_target_ARCHETYPE(EntityArchetype& archetype);
        void handle_inter_entity_drop_target_ARCHETYPE(EntityArchetype& archetype);
    };

    


    // --- START OF YAML HELPER IMPLEMENTATIONS ---

    // -- render_entity_tree_ARCHETYPE ---
    void WorldTreePanel::Pimpl::render_entity_tree_ARCHETYPE(EntityArchetype& archetype) {
        // Use string ID format that ImGui expects
        ImGui::PushID(static_cast<int>(archetype.id.get_value()));

        // 1. Drop zone above entity
        handle_inter_entity_drop_target_ARCHETYPE(archetype);
        const bool has_elements = !archetype.elements.empty();
        const bool is_child = archetype.parent_id.is_valid();
        const bool has_children = !archetype.child_ids.empty();
        const bool is_populated = has_children || has_elements;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed;
         
        if (context->selected_entity_id == archetype.id) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (!is_populated) {
            flags |=  ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_Bullet;
        } else {
           flags |= ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow;
        }

        // 2. Node setup - CORRECTED TreeNodeEx usage
        
        if (is_child) {
            
            if (is_populated) {
                // This flag draws a visible border around the tree node, visually grouping it.
                flags |=  ImGuiTreeNodeFlags_None | ImGuiTableColumnFlags_IndentEnable;
                
            } else {
                
                flags |=  ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf;
            }
        }
        

        // Renaming logic
        bool is_renaming = (entity_to_rename_id == archetype.id);
        if (is_renaming) {
            ImGui::SameLine();
            ImGui::SetKeyboardFocusHere(0);
            if (ImGui::InputText("##RenameBox", rename_buffer, sizeof(rename_buffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                archetype.name = rename_buffer;
                entity_to_rename_id = SimpleGuid::invalid();
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                entity_to_rename_id = SimpleGuid::invalid();
            }
        } else {
            // CORRECTED: Use string format for TreeNodeEx
            bool node_open = ImGui::TreeNodeEx(archetype.name.c_str(), flags);
            // --- NEW: We must pop the style variable if we pushed it ---
            
            setup_entity_drag_source_ARCHETYPE(archetype);
            handle_entity_drop_target_ARCHETYPE(archetype);

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                context->selected_entity_id = archetype.id;
                context->selected_element_id = SimpleGuid::invalid();
                // ADD THIS LINE FOR DEBUGGING
                std::cout << "[DEBUG:] [WorldTreePanel] SENDING event for Archetype ID: " << archetype.id.get_value() << std::endl;

                EntitySelectedEvent event(context->selected_entity_id , nullptr);
                context->event_manager->dispatch(event);

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // Find the transform data within the archetype
                    Transform focus_transform;
                    for (const auto& element : archetype.elements) {
                        if (element.type_name == "Transform") {
                            // Read the position directly from the YAML data
                            focus_transform.set_position(element.data["position"].as<Salix::Vector3>());
                            focus_transform.set_rotation(element.data["rotation"].as<Salix::Vector3>());
                            focus_transform.set_scale(element.data["scale"].as<Salix::Vector3>());
                            context->editor_camera->focus_on(&focus_transform, 3.0f);
                            break; // Stop searching once we've found it
                        }
                    }
                    
                }
            }
            

            show_entity_context_menu_ARCHETYPE(archetype);

            if (node_open) {
                for (auto& element : archetype.elements) {
                    ImGui::PushID(static_cast<int>(element.id.get_value())); // Push a unique ID for the element

                    // Element Renaming Section
                    bool is_renaming_element = (element_to_rename_id == element.id);
                    if (is_renaming_element) {
                        // If we are renaming this element, draw an input text box
                        ImGui::SetKeyboardFocusHere(0);
                        if (ImGui::InputText("##RenameBox", rename_buffer, sizeof(rename_buffer),
                            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                            std::string new_name = rename_buffer;
                            element.name = new_name;
                            element.data["name"] = new_name;  
                            element_to_rename_id = SimpleGuid::invalid(); // End renaming
                            context->realm_is_dirty = true;
                            
                        }
                        if (ImGui::IsItemDeactivatedAfterEdit()) {
                            element_to_rename_id = SimpleGuid::invalid(); // End renaming if user clicks away
                        }
                    } else {
                        // Otherwise, draw the element's name as a leaf node
                        ImGuiTreeNodeFlags element_flags = ImGuiTreeNodeFlags_Leaf | 
                                                        ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                                        ImGuiTreeNodeFlags_SpanAvailWidth;
                        
                        if (context->selected_element_id == element.id) {
                            element_flags |= ImGuiTreeNodeFlags_Selected;
                        }

                        // CORRECTED: Use string format for element TreeNodeEx
                        ImGui::TreeNodeEx(element.name.c_str(), element_flags);
                        
                        if (ImGui::IsItemClicked()) {
                            context->selected_element_id = element.id;
                            context->selected_entity_id = archetype.id;
                            ElementSelectedEvent event(context->selected_element_id, context->selected_entity_id, nullptr);
                            context->event_manager->dispatch(event);
                        }
                        
                    }
                    // Call the new context menu function for the element
                    show_element_context_menu_ARCHETYPE(archetype, element);

                    ImGui::PopID(); // Pop the element's unique ID
                }
                // --- NEW: Recursively render child entities ---
                for (const auto& child_id : archetype.child_ids) {
                    // Find the child archetype in the main realm vector
                    auto child_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                        [&](const EntityArchetype& e) { return e.id == child_id; });
                    
                    if (child_it != context->current_realm.end()) {
                        ImGui::Indent(child_indent);
                        // Call this function again for the child
                        render_entity_tree_ARCHETYPE(*child_it);
                        ImGui::Unindent(child_indent);
                    }
                }
                
                ImGui::TreePop();
                
            }
        }
        ImGui::PopID();
       
    }
    


    // --- setup_entity_drag_source_ARCHETYPE ---
    void WorldTreePanel::Pimpl::setup_entity_drag_source_ARCHETYPE(EntityArchetype& archetype) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // DEBUG: Match Live mode's logging
            std::cout << "DRAG START (ARCHETYPE): " << archetype.name << std::endl;
            
            // Use GUID instead of pointer (key difference)
            ImGui::SetDragDropPayload("ENTITY_DND_GUID", &archetype.id, sizeof(SimpleGuid));
            
            // Visual feedback identical to Live mode
            ImGui::Text("Moving %s", archetype.name.c_str());
            ImGui::EndDragDropSource();
        }
    }



    //  --- handle_entity_drop_target_ARCHETYPE ---
    void WorldTreePanel::Pimpl::handle_entity_drop_target_ARCHETYPE(EntityArchetype& target) {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID")) {
                SimpleGuid dragged_id = *(const SimpleGuid*)payload->Data;
                
                // Skip if dropping onto self
                if (dragged_id == target.id) {
                    ImGui::EndDragDropTarget();
                    return;
                }
                
                // Immediate processing (no AcceptBeforeDelivery)
                process_entity_drop_ARCHETYPE(dragged_id, target.id);
            }
            ImGui::EndDragDropTarget();
        }
    }




    // --- handle_inter_entity_drop_target_ARCHETYPE ---
    void WorldTreePanel::Pimpl::handle_inter_entity_drop_target_ARCHETYPE(EntityArchetype& anchor) {
        // Same visual setup as Live mode
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 4.0f));

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID", 
                ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                
                // Visual feedback identical to Live mode
                ImVec2 rect_min = ImGui::GetItemRectMin();
                ImVec2 rect_max = ImGui::GetItemRectMax();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), 
                                ImVec2(rect_max.x, rect_min.y), 
                                ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);

                if (ImGui::IsMouseReleased(0)) {
                    SimpleGuid dragged_id = *(SimpleGuid*)payload->Data;
                    
                    // ACTUAL IMPLEMENTATION using anchor's parent_id
                    SimpleGuid new_parent_id = anchor.parent_id; // Use the anchor's parent
                    
                    process_entity_drop_ARCHETYPE(dragged_id, new_parent_id);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    

    // --- process_entity_drop_ARCHETYPE ---
    void WorldTreePanel::Pimpl::process_entity_drop_ARCHETYPE(SimpleGuid dragged_id, SimpleGuid target_id) {
        // Find the dragged archetype
        auto dragged_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
            [&](const EntityArchetype& e) { return e.id == dragged_id; });
        
        if (dragged_it == context->current_realm.end()) return;

        // Check for circular parenting
        if (target_id.is_valid()) {
            auto current_id = target_id;
            while (current_id.is_valid()) {
                if (current_id == dragged_id) {
                    std::cerr << "Circular parenting detected!" << std::endl;
                    return;
                }
                
                // Find parent of current
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

        // Add to new parent or make root
        dragged_it->parent_id = target_id;
        if (target_id.is_valid()) {
            auto target_it = std::find_if(context->current_realm.begin(), context->current_realm.end(),
                [&](const EntityArchetype& e) { return e.id == target_id; });
            
            if (target_it != context->current_realm.end()) {
                target_it->child_ids.push_back(dragged_id);
            }
        }

        // Update selection
        context->selected_entity_id = dragged_id;
        context->selected_element_id = SimpleGuid::invalid();
        EntitySelectedEvent event(context->selected_entity_id, nullptr);
        context->event_manager->dispatch(event);
    }



    // show_empty_space_context_menu_ARCHETYPE
    void WorldTreePanel::Pimpl::show_empty_space_context_menu_ARCHETYPE() {
        if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            
            if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                EntityArchetype new_entity = ArchetypeFactory::create_entity_archetype("Entity");
                
                if (new_entity.id.is_valid()) {
                    // Add to current realm
                    context->current_realm.push_back(new_entity);
                    
                    // Mirror Live mode selection behavior
                    context->selected_entity_id = new_entity.id;
                    context->selected_element_id = SimpleGuid::invalid();
                    
                    EntitySelectedEvent event(context->selected_entity_id, nullptr);
                    context->event_manager->dispatch(event);
                    context->realm_is_dirty = true;
                }
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Refresh View##RefreshWorldTree")) {
                // Same empty refresh logic as Live mode
            }

            ImGui::EndPopup();
        }
    }


    



    //  --- show_element_context_menu_ARCHETYPE ---
    void WorldTreePanel::Pimpl::show_element_context_menu_ARCHETYPE(EntityArchetype& parent_archetype, ElementArchetype& element_archetype) {
        if (ImGui::BeginPopupContextItem("ElementContextMenu")) {
            // Header with entity name
            ImGui::TextDisabled("%s", element_archetype.name.c_str());
            ImGui::Separator();
            
            if(ImGui::MenuItem("Rename##RenameElement", "F2")) {
                element_to_rename_id = element_archetype.id;
                strncpy_s(rename_buffer, sizeof(rename_buffer), 
                        element_archetype.name.c_str(), sizeof(rename_buffer) - 1);
            }

            if (ImGui::MenuItem("Duplicate##DuplicateElement", "Ctrl+D")) {

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

    //  --- show_entity_context_menu_ARCHETYPE ---
    void WorldTreePanel::Pimpl::show_entity_context_menu_ARCHETYPE(EntityArchetype& archetype) {
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

            if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
                // --- FIX 1: Release all children before purging the parent ---
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

                // --- FIX 2: Signal the RealmDesignerPanel to update ---
                context->realm_is_dirty = true;
                
                // Clear selection and notify other panels
                context->selected_entity_id = SimpleGuid::invalid();
                EntitySelectedEvent event(context->selected_entity_id, nullptr);
                context->event_manager->dispatch(event);
            }

            ImGui::EndPopup();
        }
    }

    // 

    // --- END OF YAML HELPER IMPLEMENTATIONS ---


    WorldTreePanel::WorldTreePanel() : pimpl(std::make_unique<Pimpl>()) {
        set_name("World Tree Panel");
        set_title("World Tree");
        set_lock_icon_size(ImVec2(16, 16)); 
    }

    WorldTreePanel::~WorldTreePanel() = default;

    void WorldTreePanel::initialize(EditorContext* context) {
        if (!context) return;
        
        LockablePanel::initialize(context); // CRUCIAL: Base init first
        pimpl->context = context;
        set_visibility(true);
        
        // Configure lock visuals
        set_locked_state_tint_color(ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        set_unlocked_state_tint_color(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        set_lock_icon("Panel Unlocked");
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
    }



    ImGuiWindowFlags WorldTreePanel::get_window_flags() const {
        return ImGuiWindowFlags_None; // Add any custom flags here
    }





    void WorldTreePanel::on_panel_gui_update() {
        if (!pimpl->context) {
            std::cerr << "Editor Context not initialized!" << std::endl;
            return;
        }

        // --- The conditional logic for the two pathways ---
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            // --- All of the existing rendering logic goes in here for the Live pathway ---
            if (!pimpl->context->active_scene) {
                ImGui::Text("No active scene.");
                return;
            }
            // Scene header
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Scene: %s ", 
                            pimpl->context->active_scene->get_name().c_str());
            // Status bar
            // ImGui::Separator();
            ImGui::TextDisabled("\tEntities: %d", pimpl->context->active_scene->get_entities().size());
            ImGui::Separator();
            
            // Content area with scrolling
            if (ImGui::BeginChild("WorldTreeContent", ImVec2(0, 0), true, 
                                ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                
                // Check for right-click in empty space
                if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    if (!ImGui::IsAnyItemHovered()) {
                        ImGui::OpenPopup("WorldTreeContextMenu");
                    }
                }

                // Render all root entities
                for (Entity* entity : pimpl->context->active_scene->get_entities()) {
                    if (entity && entity->get_transform() && !entity->get_transform()->get_parent()) {
                        render_entity_tree(entity);
                    }
                }

                // Show empty space context menu.
                show_empty_space_context_menu();

                // Handle Keyboard Shortcuts.
                if (ImGui::IsWindowFocused()) {
                    handle_keyboard_shortcuts();
                }                     
            }

            ImGui::EndChild();

        } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            if (pimpl->context->current_realm.empty()) {
                ImGui::Text("No Entities in current realm.");
                
            }
            // --- YAML PATHWAY IMPLEMENTATION ---
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "Scene: (YAML Mode)");
            ImGui::TextDisabled("\tEntities: %d", pimpl->context->current_realm.size());
            ImGui::Separator();

            if (ImGui::BeginChild("WorldTreeContent", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && 
                    !ImGui::IsAnyItemHovered()) {
                    ImGui::OpenPopup("WorldTreeContextMenu");
                }
                if (pimpl->context->current_realm.size() != 0) {
                    for (auto& entity_archetype : pimpl->context->current_realm) {
                        // We now call our new, cleaner function!
                        if (!entity_archetype.parent_id.is_valid()) {
                            pimpl->render_entity_tree_ARCHETYPE(entity_archetype);
                        }
                    }
                }
                pimpl->show_empty_space_context_menu_ARCHETYPE();
            }
            
                ImGui::EndChild();
            }
        }
    



    
    void WorldTreePanel::render_entity_tree(Entity* entity) {
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (!entity || entity->is_purged()) return;

            // Use the entity's memory address for a unique ID scope
            int entity_id = static_cast<int>(entity->get_id().get_value());
            if (entity_id == 0) return;
            ImGui::PushID(entity_id);

            // 1. Handle the drop zone ABOVE this entity (for re-ordering/un-parenting)
            handle_inter_entity_drop_target(entity);

            
            // 2. Draw the actual entity node (icon, text, etc.)
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

            // Add the new check for the SimpleGuid
            if (pimpl->context->selected_entity_id == entity->get_id()) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            else if (pimpl->context->selected_entity == entity) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool is_renaming_this_entity = (pimpl->entity_to_rename == entity);

            if (is_renaming_this_entity) {
                // Draw the tree node part (the arrow) without a label
                // We use "##" to give it an ID without displaying any text.
                ImGui::SameLine();

                // Give the input text widget focus when it first appears.
                ImGui::SetKeyboardFocusHere(0);
                
                // Draw the input text field.
                if (ImGui::InputText("##RenameBox", pimpl->rename_buffer, sizeof(pimpl->rename_buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    // If the user presses Enter, set the new name and end the rename state.
                    entity->set_name(pimpl->rename_buffer);
                    pimpl->entity_to_rename = nullptr;
                }

                // If the user clicks away from the input box, also end the rename state.
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pimpl->entity_to_rename = nullptr;
                }

                
            }
            else {

                const auto& icon = pimpl->context->init_context->icon_manager->get_icon_for_entity(entity);
                if (icon.texture_id) {
                    ImGui::Image(icon.texture_id, get_lock_icon_size(), get_top_left(), get_bottom_right());
                    ImGui::SameLine();
                }
                bool node_open = ImGui::TreeNodeEx(entity, flags, "%s", entity->get_name().c_str());

                // 3. Handle dragging FROM this entity and dropping ONTO this entity
                setup_entity_drag_source(entity);
                handle_entity_drop_target(entity);

                // 4. Handle standard interactions like clicking and context menus
                // --- LIVE OBJECT USAGE ---
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    pimpl->context->selected_entity = entity;
                    pimpl->context->selected_element = nullptr;
                    EntitySelectedEvent event(entity->get_id(), entity);
                    pimpl->context->event_manager->dispatch(event);
                    // --- END LIVE OBJECT USAGE ---
                    // --- The New, Data-Driven System (Additions) ---
                    pimpl->context->selected_entity_id = entity->get_id();
                    pimpl->context->selected_element_id = SimpleGuid::invalid();
                    
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (auto transform = entity->get_transform()) {
                            pimpl->context->editor_camera->focus_on(transform, 3.0f);
                        }
                    }
                }
                show_entity_context_menu(entity);

                // 5. If the node is open, render all of its children recursively
                if (node_open) {
                    if (auto transform = entity->get_transform()) {
                        for (auto child_transform : transform->get_children()) {
                            if (auto child_entity = dynamic_cast<Entity*>(child_transform->get_owner())) {
                                render_entity_tree(child_entity);
                            }
                        }
                    }
                    for (auto& element : entity->get_all_elements()) {
                        if (element) render_element(element);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::PopID();
        }  
    }





    void WorldTreePanel::render_element(Element* element) {
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (!element) return;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | 
                                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                    ImGuiTreeNodeFlags_SpanAvailWidth;
            
            // Add the new check for the SimpleGuid
            if (pimpl->context->selected_element_id == element->get_id()) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            else if (pimpl->context->selected_element == element) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            // Render icon
            const auto& icon = pimpl->context->init_context->icon_manager->get_icon_for_element(element);
            if (icon.texture_id) {
                ImGui::Indent(16.0f);
                ImGui::Image(icon.texture_id, get_lock_icon_size(), get_top_left(), get_bottom_right());
                ImGui::SameLine();
                ImGui::Unindent(16.0f);
            }

            // Render element
            ImGui::TreeNodeEx(element, flags, "  - %s", element->get_class_name());

            if (ImGui::IsItemClicked()) {
                // --- The Old, Pointer-Based System ---
                pimpl->context->selected_element = element;
                pimpl->context->selected_entity = element->get_owner();
                ElementSelectedEvent event(element->get_id(),element->get_owner()->get_id(), element);
                pimpl->context->event_manager->dispatch(event);
                // --- End Of The Old, Pointer-Based System ---

                // --- The New, Data-Driven System (Additions) ---
                pimpl->context->selected_element_id = element->get_id();
                pimpl->context->selected_entity_id = element->get_owner()->get_id();

            }
        } 
    }


    void WorldTreePanel::show_empty_space_context_menu() {
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu",
                ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
                
                if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                    if (pimpl->context && pimpl->context->active_scene) {
                        Entity* new_entity = pimpl->context->active_scene->create_entity();
                        if (new_entity) {
                            pimpl->context->selected_entity = new_entity;
                            EntitySelectedEvent event(new_entity->get_id(), new_entity);
                            pimpl->context->event_manager->dispatch(event);

                            // --- New, Data-Driven System (Additions) ---
                            pimpl->context->selected_entity_id = new_entity->get_id();
                        }
                    }
                }

                ImGui::Separator();
                
                if (ImGui::MenuItem("Refresh View##RefreshWorldTree")) {
                    // Refresh logic here
                }

                ImGui::EndPopup();
            }
        } 
    } 





    void WorldTreePanel::show_entity_context_menu(Entity* entity) {
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (!entity) return;

            if (ImGui::BeginPopupContextItem("EntityContextMenu")) {
                // Header with unique ID based on entity pointer
                ImGui::Separator();

                // Element Creation Submenu with unique IDs
                if (ImGui::BeginMenu("Add Element##AddElementMenu")) {
                    if (ImGui::MenuItem("Sprite2D##AddSprite2D")) {
                        entity->add_element<Sprite2D>();
                        EntitySelectedEvent event(entity->get_id(), entity);
                        pimpl->context->event_manager->dispatch(event);

                        // New: GUID-based context update
                        pimpl->context->selected_entity_id = entity->get_id();
                    }
                    
                    if (ImGui::MenuItem("Script##AddScript")) {
                        entity->add_element<ScriptElement>();
                        EntitySelectedEvent event(entity->get_id(), entity);
                        pimpl->context->event_manager->dispatch(event);

                        // New: GUID-based context update
                        pimpl->context->selected_entity_id = entity->get_id();

                    }
                    
                    if (ImGui::MenuItem("Camera##AddCamera")) {
                        entity->add_element<Camera>();
                        EntitySelectedEvent event(entity->get_id(), entity);
                        pimpl->context->event_manager->dispatch(event);

                        // New: GUID-based context update
                        pimpl->context->selected_entity_id = entity->get_id();
                    }
                    ImGui::EndMenu();
                }
                // Release from Parent
                if (entity->get_parent()) {
                    if(ImGui::MenuItem("Release From Parent##ReleaseFromParent")) {
                        if(pimpl->context && pimpl->context->active_scene) {
                            entity->release_from_parent();
                        }
                    }
                }

                // Hierarchy Operations with unique IDs
                if (ImGui::MenuItem("Add Child Entity##AddChildEntity")) {
                    if (pimpl->context && pimpl->context->active_scene) {
                        Entity* new_entity = pimpl->context->active_scene->create_entity();

                        if (new_entity && entity) {
                            // This single, high-level call handles everything for us.
                            new_entity->set_parent(entity);
                            pimpl->context->selected_entity = new_entity;
                            EntitySelectedEvent event(new_entity->get_id(), new_entity);
                            pimpl->context->event_manager->dispatch(event);

                            // New: GUID-based context update
                            pimpl->context->selected_entity_id = new_entity->get_id();
                        }
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Rename##RenameEntity", "F2")) {
                    pimpl->entity_to_rename = entity; // Store which entity we are renaming
                    
                    // New: GUID-based context update
                    pimpl->entity_to_rename_id = entity->get_id();
                    
                    // And copy its current name into our buffer to start editing
                    strncpy_s(pimpl->rename_buffer, sizeof(
                    pimpl->rename_buffer), entity->get_name().c_str(),
                    sizeof(pimpl->rename_buffer) - 1);
                    
                    

                }

                if (ImGui::MenuItem("Duplicate##DuplicateEntity", "Ctrl+D")) {
                    // Future duplicate functionality
                }

                if (ImGui::MenuItem("Purge##PurgeEntity", "Del")) {
                    if (pimpl->context && pimpl->context->active_scene) {
                        if (entity && !entity->is_purged()){
                            if (entity->get_parent()) {
                                entity->release_from_parent();
                            }
                        }
                        entity->purge();
                        EntitySelectedEvent event(SimpleGuid::invalid(), nullptr);
                        pimpl->context->event_manager->dispatch(event);

                        // New: GUID-based context update
                        pimpl->context->selected_entity_id = SimpleGuid::invalid();
                    }
                }

                ImGui::EndPopup();
            }
        } 
    }
    


    void WorldTreePanel::handle_keyboard_shortcuts() {
            if (pimpl->context->data_mode == EditorDataMode::Live) {

            // --- New Data-Driven Lookup Logic ---
            // Get the selected entity using its GUID, which is a stable ID.
            Entity* selected_entity = nullptr;
            if (pimpl->context && pimpl->context->active_scene) {
                selected_entity = pimpl->context->active_scene->get_entity_by_id(pimpl->context->selected_entity_id);

            }

            // Do nothing if no entity is selected.
            if (!selected_entity) {
                return;
            }

            // --- Purge Shortcut (Del key) ---
            // We check if the 'Delete' key was pressed this frame.
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                // This is the same logic from your MenuItem.
                if (selected_entity->get_parent()) {
                    selected_entity->release_from_parent();
                }
                selected_entity->purge();
                
                // We set selection to null here, but the event will also do this.
                pimpl->context->selected_entity = nullptr; 
                EntitySelectedEvent event(SimpleGuid::invalid(), nullptr);
                pimpl->context->event_manager->dispatch(event);
            }

            // --- Rename Shortcut (F2 key) ---
            if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
                // This is the same logic from your MenuItem.
                pimpl->entity_to_rename = selected_entity;
                strncpy_s(pimpl->rename_buffer, sizeof(pimpl->rename_buffer), 
                        selected_entity->get_name().c_str(), sizeof(pimpl->rename_buffer) - 1);
            }

            // --- Duplicate Shortcut (Ctrl+D) ---
            // ImGui::GetIO().KeyCtrl checks if the Ctrl key is held down.
            if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) {
                // TODO: Implement duplication logic here.
                std::cout << "Ctrl+D pressed for entity: " << selected_entity->get_name() << std::endl;
            }
        } 
    }





    void WorldTreePanel::create_new_entity(Entity* parent) {
            if (pimpl->context->data_mode == EditorDataMode::Live) {
            // Safety checks
            if (!pimpl->context || !pimpl->context->active_scene) {
                std::cerr << "Cannot create entity - no active scene" << std::endl;
                return;
            }

            // Create the new entity
            Entity* new_entity = pimpl->context->active_scene->create_entity();
            if (!new_entity) {
                std::cerr << "Failed to create new entity" << std::endl;
                return;
            }

            // Set default name based on hierarchy
            std::string entity_name = "Entity";
            if (parent) {
                entity_name = parent->get_name() + "_Child";
                // Find unique name by checking existing children
                int counter = 1;
                bool name_exists = true;
                while (name_exists) {
                    name_exists = false;
                    if (parent->get_transform()) {
                        for (auto childTransform : parent->get_transform()->get_children()) {
                            if (auto childEntity = dynamic_cast<Entity*>(childTransform->get_owner())) {
                                if (childEntity->get_name() == entity_name + std::to_string(counter)) {
                                    name_exists = true;
                                    counter++;
                                    break;
                                }
                            }
                        }
                    }
                }
                entity_name += std::to_string(counter);
            }
            new_entity->set_name(entity_name);

            // Set up parent-child relationship if specified
            if (parent && parent->get_transform()) {
                new_entity->get_transform()->set_parent(parent->get_transform());
            }

            // Select and focus the new entity
            pimpl->context->selected_entity = new_entity;
            
            // Dispatch selection event
            EntitySelectedEvent event(new_entity->get_id(), new_entity);
            if (pimpl->context->event_manager) {
                pimpl->context->event_manager->dispatch(event);
            }

            // Optionally focus camera on new entity
            if (pimpl->context->editor_camera && new_entity->get_transform()) {
                pimpl->context->editor_camera->focus_on(new_entity->get_transform(), 2.0f);
            }

            std::cout <<"Created new entity: " << entity_name <<
                        (parent ? " (child of " + parent->get_name() + ")" : " (root)") << std::endl;

        } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            // TODO: Implement YAML pathway here.
        }
    }




    void WorldTreePanel::add_element_to_entity(Entity* entity, const std::string& element_type) {
            if (pimpl->context->data_mode == EditorDataMode::Live) {
            // Validate inputs
            if (!entity) {
                std::cerr << "Cannot add element - null entity" << std::endl;
                return;
            }

            if (!pimpl->context || !pimpl->context->event_manager) {
                std::cerr << "Cannot add element - missing context or event manager" << std::endl;
                return;
            }

            // Element creation logic
            bool elementAdded = false;
            std::string elementName = element_type;

            try {
                if (element_type == "Sprite2D") {
                    entity->add_element<Sprite2D>();
                    elementAdded = true;
                }
                else if (element_type == "ScriptElement") {
                    entity->add_element<ScriptElement>();
                    elementAdded = true;
                }
                else if (element_type == "Camera") {
                    entity->add_element<Camera>();
                    elementAdded = true;
                }

                // Add more element types as needed
                else {
                    std::cerr << "Unknown element type: " + element_type << std::endl;
                    return;
                }
            }
            catch (const std::exception& e) {
                std::cerr <<"Failed to add element: " + std::string(e.what()) << std::endl;
                return;
            }

            // Post-creation handling
            if (elementAdded) {
                // Refresh selection to show new element
                EntitySelectedEvent event(entity->get_id(), entity);
                pimpl->context->event_manager->dispatch(event);

                // Log successful creation
                std::cout << "Added " << elementName << " to entity: " << entity->get_name() << std::endl;

                // Special handling for specific elements
                if (element_type == "Camera" && pimpl->context->editor_camera) {
                    // Auto-focus when adding a camera
                    if (auto transform = entity->get_transform()) {
                        pimpl->context->editor_camera->focus_on(transform, 3.0f);
                    }
                }
            }
        } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            // TODO: Implement YAML pathway here.
        }
    }


    void WorldTreePanel::setup_entity_drag_source(Entity* entity) {
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                std::cout << "DRAG START: " << entity->get_name() << std::endl; // DEBUG
                ImGui::SetDragDropPayload("ENTITY_DND", &entity, sizeof(Entity*));
                ImGui::Text("Moving %s", entity->get_name().c_str());
                ImGui::EndDragDropSource();
            }
        } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::cout << "DRAG START: " << entity->get_name() << std::endl; // DEBUG
            // This is the new, data-driven payload with a unique ID
            ImGui::SetDragDropPayload("ENTITY_DND_GUID", &entity->get_id(), sizeof(SimpleGuid));
            ImGui::Text("Moving %s", entity->get_name().c_str());
            ImGui::EndDragDropSource();
            }
        }
    }


    void WorldTreePanel::process_entity_drop(Entity* dragged_entity, Entity* target_entity)
    {
        // This function is called AFTER a payload has been accepted.

        // All validation logic now lives in one place.
        if (!dragged_entity || dragged_entity->is_purged()) {
            std::cerr << "Null or Purged dragged entity in drop!" << std::endl;
            return;
        }
        // Note: 'target_entity' CAN be null, which means we are dropping on the background.

        // Defensive checks for the target entity
        if (target_entity && target_entity->is_purged()) {
            target_entity = nullptr;
        }

        if (dragged_entity == target_entity) {
            // Self-parenting blocked
            return;
        }

        if (target_entity && target_entity->is_child_of(dragged_entity)) {
            // Circular parenting blocked
            return;
        }

        // --- The Core Action ---
        if (target_entity) {

            // If there IS a target, we are re-parenting ONTO it.
            dragged_entity->set_parent(target_entity);

        } else {

            // If the target is NULL, we are releasing to become a root object.
            dragged_entity->release_from_parent();

        }
        
        // --- Add the new SimpleGuid assignments ---

        // Update the old pointer-based context
        pimpl->context->selected_entity = dragged_entity;

        // Update the new GUID-based context
        pimpl->context->selected_entity_id = dragged_entity->get_id();

        // Dispatch the old event
        EntitySelectedEvent event(pimpl->context->selected_entity_id, pimpl->context->selected_entity);

        if (pimpl->context->event_manager) {
            pimpl->context->event_manager->dispatch(event);
        }
    }




    void WorldTreePanel::handle_inter_entity_drop_target(Entity* anchor_entity) {
        // Create a small, invisible region to act as our drop target hitbox
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 4.0f));

        if (ImGui::BeginDragDropTarget()) {
            if (pimpl->context->data_mode == EditorDataMode::Live) {
                // Use a special flag to get feedback before the drop happens
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND", ImGuiDragDropFlags_AcceptBeforeDelivery);

                // If a valid payload is hovering over our dummy hitbox...
                if (payload) {
                    // Get the rectangle for the dummy item we just created
                    ImVec2 rect_min = ImGui::GetItemRectMin();
                    ImVec2 rect_max = ImGui::GetItemRectMax();
                    
                    // Draw our visible feedback line over that rectangle
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), ImVec2(rect_max.x, rect_min.y), ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);
                    
                }

                // If the user actually releases the mouse...
                if (payload && ImGui::IsMouseReleased(0)) {
                    Entity* dragged_entity = *(Entity**)payload->Data;
                    // The logic is to parent the dragged item to the ANCHOR's parent.
                    if (anchor_entity) {
                        process_entity_drop(dragged_entity, anchor_entity->get_parent());
                    }
                }
            } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID", ImGuiDragDropFlags_AcceptBeforeDelivery);

                if (payload) {
                    ImVec2 rect_min = ImGui::GetItemRectMin();
                    ImVec2 rect_max = ImGui::GetItemRectMax();
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), ImVec2(rect_max.x, rect_min.y), ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);
                }

                if (payload && ImGui::IsMouseReleased(0)) {
                    SimpleGuid dragged_guid = *(SimpleGuid*)payload->Data;
                    Entity* dragged_entity = pimpl->context->active_scene->get_entity_by_id(dragged_guid);
                    if (dragged_entity && anchor_entity) {
                        process_entity_drop(dragged_entity, anchor_entity->get_parent());
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
    }


    void WorldTreePanel::handle_entity_drop_target(Entity* target) {
        if (ImGui::BeginDragDropTarget()) {
                if (pimpl->context->data_mode == EditorDataMode::Live) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND")) {
                        Entity* dragged_entity = *(Entity**)payload->Data;
                        // Just call the central processor with the target entity
                        process_entity_drop(dragged_entity, target);
                    }
                } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND_GUID")) {
                    SimpleGuid dragged_guid = *(SimpleGuid*)payload->Data;
                    Entity* dragged_entity = pimpl->context->active_scene->get_entity_by_id(dragged_guid);
                    if (dragged_entity) {
                        process_entity_drop(dragged_entity, target);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
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

}  // namespace Salix