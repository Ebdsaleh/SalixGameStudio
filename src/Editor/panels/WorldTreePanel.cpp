// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================
#include <Editor/panels/WorldTreePanel.h>
#include <imgui/imgui.h>
#include <Salix/math/Color.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <Salix/management/SceneManager.h>
#include <Salix/core/InitContext.h>
#include <Editor/EditorContext.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/gui/IGui.h>
#include <memory>
#include <iostream>

namespace Salix {
    struct WorldTreePanel::Pimpl {
        EditorContext* context = nullptr;
        
    };

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
    }



    ImGuiWindowFlags WorldTreePanel::get_window_flags() const {
        return ImGuiWindowFlags_None; // Add any custom flags here
    }




    void WorldTreePanel::on_panel_gui_update() {
        if (!pimpl->context || !pimpl->context->active_scene) {
            ImGui::Text("No active scene.");
            return;
        }

        // Scene header
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Scene: %s", 
                        pimpl->context->active_scene->get_name().c_str());
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

            // Show empty space context menu
            show_empty_space_context_menu();
        }
        ImGui::EndChild();

        // Status bar
        ImGui::Separator();
        ImGui::TextDisabled("%d entities", 
                        pimpl->context->active_scene->get_entities().size());
    }



    
    void WorldTreePanel::render_entity_tree(Entity* entity) {
        if (!entity) return;

        // Use the entity's memory address for a unique ID scope
        ImGui::PushID(entity);

        // 1. Handle the drop zone ABOVE this entity (for re-ordering/un-parenting)
        handle_inter_entity_drop_target(entity);

        // 2. Draw the actual entity node (icon, text, etc.)
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (pimpl->context->selected_entity == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
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
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            pimpl->context->selected_entity = entity;
            pimpl->context->selected_element = nullptr;
            EntitySelectedEvent event(entity);
            pimpl->context->event_manager->dispatch(event);

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

        ImGui::PopID();
    }





    void WorldTreePanel::render_element(Element* element) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | 
                                  ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
        
        if (pimpl->context->selected_element == element) {
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
            pimpl->context->selected_element = element;
            pimpl->context->selected_entity = element->get_owner();
            ElementSelectedEvent event(element);
            pimpl->context->event_manager->dispatch(event);
        }
        
    }


    void WorldTreePanel::show_empty_space_context_menu() {
        if (ImGui::BeginPopupContextWindow("WorldTreeContextMenu", 
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            
            if (ImGui::MenuItem("Add Entity##AddRootEntity")) {
                if (pimpl->context && pimpl->context->active_scene) {
                    Entity* newEntity = pimpl->context->active_scene->create_entity();
                    if (newEntity) {
                        pimpl->context->selected_entity = newEntity;
                        EntitySelectedEvent event(newEntity);
                        pimpl->context->event_manager->dispatch(event);
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





    void WorldTreePanel::show_entity_context_menu(Entity* entity) {
        if (!entity) return;

        if (ImGui::BeginPopupContextItem("EntityContextMenu")) {
            // Header with unique ID based on entity pointer
            ImGui::Separator();

            // Element Creation Submenu with unique IDs
            if (ImGui::BeginMenu("Add Element##AddElementMenu")) {
                if (ImGui::MenuItem("Sprite2D##AddSprite2D")) {
                    entity->add_element<Sprite2D>();
                    EntitySelectedEvent event(entity);
                    pimpl->context->event_manager->dispatch(event);
                }
                
                if (ImGui::MenuItem("Script##AddScript")) {
                    entity->add_element<ScriptElement>();
                    EntitySelectedEvent event(entity);
                    pimpl->context->event_manager->dispatch(event);
                }
                
                if (ImGui::MenuItem("Camera##AddCamera")) {
                    entity->add_element<Camera>();
                    EntitySelectedEvent event(entity);
                    pimpl->context->event_manager->dispatch(event);
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
                    Entity* newEntity = pimpl->context->active_scene->create_entity();
                    if (newEntity && entity->get_transform()) {
                        newEntity->get_transform()->set_parent(entity->get_transform());
                        pimpl->context->selected_entity = newEntity;
                        EntitySelectedEvent event(newEntity);
                        pimpl->context->event_manager->dispatch(event);
                    }
                }
            }

            ImGui::Separator();

            // Entity Operations with unique IDs
            if (ImGui::MenuItem("Rename##RenameEntity", "F2")) {
                // Future rename functionality
            }

            if (ImGui::MenuItem("Duplicate##DuplicateEntity", "Ctrl+D")) {
                // Future duplicate functionality
            }

            if (ImGui::MenuItem("Delete##DeleteEntity", "Del")) {
                if (pimpl->context && pimpl->context->active_scene) {
                    if (entity->get_transform() && entity->get_transform()->get_parent()) {
                        if (auto parentEntity = dynamic_cast<Entity*>(entity->get_transform()->get_parent()->get_owner())) {
                            pimpl->context->selected_entity = parentEntity;
                        }
                    }
                    entity->purge();
                    EntitySelectedEvent event(nullptr);
                    pimpl->context->event_manager->dispatch(event);
                }
            }

            ImGui::EndPopup();
        }
    }
    








    void WorldTreePanel::create_new_entity(Entity* parent) {
        // Safety checks
        if (!pimpl->context || !pimpl->context->active_scene) {
            std::cerr << "Cannot create entity - no active scene" << std::endl;
            return;
        }

        // Create the new entity
        Entity* newEntity = pimpl->context->active_scene->create_entity();
        if (!newEntity) {
            std::cerr << "Failed to create new entity" << std::endl;
            return;
        }

        // Set default name based on hierarchy
        std::string entityName = "Entity";
        if (parent) {
            entityName = parent->get_name() + "_Child";
            // Find unique name by checking existing children
            int counter = 1;
            bool nameExists = true;
            while (nameExists) {
                nameExists = false;
                if (parent->get_transform()) {
                    for (auto childTransform : parent->get_transform()->get_children()) {
                        if (auto childEntity = dynamic_cast<Entity*>(childTransform->get_owner())) {
                            if (childEntity->get_name() == entityName + std::to_string(counter)) {
                                nameExists = true;
                                counter++;
                                break;
                            }
                        }
                    }
                }
            }
            entityName += std::to_string(counter);
        }
        newEntity->set_name(entityName);

        // Set up parent-child relationship if specified
        if (parent && parent->get_transform()) {
            newEntity->get_transform()->set_parent(parent->get_transform());
        }

        // Select and focus the new entity
        pimpl->context->selected_entity = newEntity;
        
        // Dispatch selection event
        EntitySelectedEvent event(newEntity);
        if (pimpl->context->event_manager) {
            pimpl->context->event_manager->dispatch(event);
        }

        // Optionally focus camera on new entity
        if (pimpl->context->editor_camera && newEntity->get_transform()) {
            pimpl->context->editor_camera->focus_on(newEntity->get_transform(), 2.0f);
        }

        std::cout <<"Created new entity: " << entityName <<
                    (parent ? " (child of " + parent->get_name() + ")" : " (root)") << std::endl;
    }




    void WorldTreePanel::add_element_to_entity(Entity* entity, const std::string& element_type) {
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
            EntitySelectedEvent event(entity);
            pimpl->context->event_manager->dispatch(event);

            // Log successful creation
            std::cout << "Added " << elementName << " to entity: " << entity->get_name() << std::endl;

            // Special handling for specific components
            if (element_type == "Camera" && pimpl->context->editor_camera) {
                // Auto-focus when adding a camera
                if (auto transform = entity->get_transform()) {
                    pimpl->context->editor_camera->focus_on(transform, 3.0f);
                }
            }
        }
    }


    void WorldTreePanel::setup_entity_drag_source(Entity* entity) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::cout << "DRAG START: " << entity->get_name() << std::endl; // DEBUG
            ImGui::SetDragDropPayload("ENTITY_DND", &entity, sizeof(Entity*));
            ImGui::Text("Moving %s", entity->get_name().c_str());
            ImGui::EndDragDropSource();
        }
    }


    void WorldTreePanel::process_entity_drop(Entity* dragged_entity, Entity* target_entity)
    {
        // This function is called AFTER a payload has been accepted.

        // All validation logic now lives in one place.
        if (!dragged_entity) {
            std::cerr << "Null dragged entity in drop!" << std::endl;
            return;
        }
        // Note: 'target_entity' CAN be null, which means we are dropping on the background.
        
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
        
        // Select the entity that was moved and dispatch an event
        pimpl->context->selected_entity = dragged_entity;
        EntitySelectedEvent event(dragged_entity);
        if (pimpl->context->event_manager) {
            pimpl->context->event_manager->dispatch(event);
        }
    }




    void WorldTreePanel::handle_inter_entity_drop_target(Entity* anchor_entity) {
        // Create a small, invisible region to act as our drop target hitbox
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 4.0f));

        if (ImGui::BeginDragDropTarget())
        {
            // Use a special flag to get feedback before the drop happens
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND", ImGuiDragDropFlags_AcceptBeforeDelivery);

            // If a valid payload is hovering over our dummy hitbox...
            if (payload) {
                // --- THIS IS THE CORRECTED SECTION ---
                // Get the rectangle for the dummy item we just created
                ImVec2 rect_min = ImGui::GetItemRectMin();
                ImVec2 rect_max = ImGui::GetItemRectMax();
                
                // Draw our visible feedback line over that rectangle
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), ImVec2(rect_max.x, rect_min.y), ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);
                // --- END CORRECTION ---
            }

            // If the user actually releases the mouse...
            if (payload && ImGui::IsMouseReleased(0)) {
                Entity* dragged_entity = *(Entity**)payload->Data;
                // The logic is to parent the dragged item to the ANCHOR's parent.
                if (anchor_entity) {
                    process_entity_drop(dragged_entity, anchor_entity->get_parent());
                }
            }

            ImGui::EndDragDropTarget();
        }
    }


    void WorldTreePanel::handle_entity_drop_target(Entity* target) {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND")) {
                Entity* dragged_entity = *(Entity**)payload->Data;
                // Just call the central processor with the target entity
                process_entity_drop(dragged_entity, target);
            }
            ImGui::EndDragDropTarget();
        }
    }


}  // namespace Salix