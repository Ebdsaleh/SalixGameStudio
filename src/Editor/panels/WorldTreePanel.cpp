// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================
#include <Editor/panels/WorldTreePanel.h>
#include <imgui/imgui.h>
#include <Editor/events/EntitySelectedEvent.h>
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
        // Create a new ImGui window for our panel.
        // This will automatically be dockable because of the dockspace we will
        // create in the EditorState.

         if (!pimpl->is_visible) {
            return; // If the panel isn't visible, do nothing.
        }

        if (ImGui::Begin("World Tree", &pimpl->is_visible))
        {
            if (pimpl->context->active_scene) {
                for (const auto& entity_ptr : pimpl->context->active_scene->get_entities()) {
                    if (!entity_ptr) continue;
                    bool is_selected = (pimpl->context->selected_entity == entity_ptr);

                    if (ImGui::Selectable(entity_ptr->get_name().c_str(), is_selected)) {
                        // 1. Update the context directly, as before.
                        pimpl->context->selected_entity = entity_ptr;

                        // 2. Create and dispatch the event.
                        EntitySelectedEvent event(entity_ptr);

                        pimpl->context->event_manager->dispatch(event);
                    }
                }
            } else {
                ImGui::Text("No active scene.");
            }
            
        }
        // It's crucial to always call End(), even if the window is collapsed.
        ImGui::End();
    }

    bool WorldTreePanel::get_visibility() const {
        return pimpl->is_visible;
    }

    void WorldTreePanel::set_visibility(bool visibility) {
        pimpl->is_visible = visibility;
    }

} // namespace Salix