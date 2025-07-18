// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================
#include <Editor/panels/WorldTreePanel.h>
#include <imgui/imgui.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/management/SceneManager.h>
#include <Editor/EditorContext.h>
#include <memory>
#include <iostream>

namespace Salix {

    struct WorldTreePanel::Pimpl { 
        EditorContext* context = nullptr;
        bool is_visible = true;

    };


    WorldTreePanel::WorldTreePanel() : pimpl(std::make_unique<Pimpl>()) {}
    WorldTreePanel::~WorldTreePanel() {}

    void WorldTreePanel::initialize(EditorContext* context) {
        if(!context) { return; } // Cannot accept a null EditorContext pointer.
        pimpl->context = context;
    }

    void WorldTreePanel::on_gui_render() {
        // If the panel isn't visible, do nothing.
        if (!pimpl->is_visible) {
            return;
        }

        // Create a new ImGui window for our panel.
        if (ImGui::Begin("World Tree", &pimpl->is_visible)) {
            if (pimpl->context && pimpl->context->active_scene) {
                // Loop through all entities in the active scene
                for (Entity* entity_ptr : pimpl->context->active_scene->get_entities()) {
                    if (!entity_ptr) continue;

                    // --- Entity Node ---
                    ImGuiTreeNodeFlags entity_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                    // Highlight the entity if it or one of its children is selected
                    if (pimpl->context->selected_entity == entity_ptr) {
                        entity_flags |= ImGuiTreeNodeFlags_Selected;
                    }

                    // Use the entity pointer as a unique ID for the TreeNode
                    bool entity_node_open = ImGui::TreeNodeEx((void*)entity_ptr, entity_flags, "%s", entity_ptr->get_name().c_str());

                    // Handle selection for the entity itself
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                        pimpl->context->selected_entity = entity_ptr;
                        pimpl->context->selected_element = nullptr; // Deselect any element when an entity is clicked
                        EntitySelectedEvent event(entity_ptr);
                        pimpl->context->event_manager->dispatch(event);
                    }

                    // If the entity node is open, draw its elements
                    if (entity_node_open) {
                        for (const auto& element_ptr : entity_ptr->get_all_elements()) {
                            if (!element_ptr) continue;
                            
                            // --- Element Node (as a leaf) ---
                            ImGuiTreeNodeFlags element_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                            if (pimpl->context->selected_element == element_ptr) {
                                element_flags |= ImGuiTreeNodeFlags_Selected;
                            }

                            // Display the element as a selectable leaf node
                            ImGui::TreeNodeEx((void*)element_ptr, element_flags, "  - %s", element_ptr->get_class_name());

                            // Handle selection for the element
                            if (ImGui::IsItemClicked()) {
                                // Update the context to select this element AND its parent entity
                                pimpl->context->selected_element = element_ptr;
                                pimpl->context->selected_entity = entity_ptr;

                                // Dispatch the specific event for element selection
                                ElementSelectedEvent event(element_ptr);
                                pimpl->context->event_manager->dispatch(event);
                            }
                        }
                        ImGui::TreePop(); // Must be called after TreeNodeEx if it returns true
                    }
                }
            } else {
                ImGui::Text("No active scene.");
            }
        }
        ImGui::End();
    }


    void WorldTreePanel::on_event(IEvent& event) {
        // This panel dispatches events, but doesn't need to handle any for now.
        // We can leave this empty.
        (void)event; // This prevents the "unreferenced formal parameter" warning.
    }

    void WorldTreePanel::set_visibility(bool visibility) {
        pimpl->is_visible = visibility;
    }


    bool WorldTreePanel::get_visibility() const  { return pimpl->is_visible; }

} // namespace Salix