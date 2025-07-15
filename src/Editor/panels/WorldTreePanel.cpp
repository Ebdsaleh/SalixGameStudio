// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.cpp
// Author:      SalixGameStudio
// Description: Implements the WorldTreePanel class.
// =================================================================================
#include <Editor/panels/WorldTreePanel.h>
#include <imgui/imgui.h> // We need this to call ImGui functions
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
        if (ImGui::Begin("World Tree"))
        {
            // For now, we'll just put some placeholder text here.
            // In the future, we will loop through the scene's entities
            // and display them in a tree view.
            ImGui::Text("Scene_0");
            ImGui::Indent();
            ImGui::Text("MainCamera");
            ImGui::Text("Player");
            ImGui::Unindent();
        }
        // It's crucial to always call End(), even if the window is collapsed.
        ImGui::End();
    }

    const bool WorldTreePanel::get_visibility() const {
        return pimpl->is_visible;
    }

    void WorldTreePanel::set_visibility(bool visibility) {
        pimpl->is_visible = visibility;
    }

} // namespace Salix