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
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/ScriptElement.h>
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
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        begin();
        // Render all entities
        for (Entity* entity : pimpl->context->active_scene->get_entities()) {
            if (entity) render_entity_tree(entity);
        }
        end();
    }

    void WorldTreePanel::render_entity_tree(Entity* entity) {
        // Set up tree node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | 
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
        
        if (pimpl->context->selected_entity == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Render icon
        const auto& icon = pimpl->context->init_context->icon_manager->get_icon_for_entity(entity);
        if (icon.texture_id) {
            ImGui::Image(icon.texture_id, get_lock_icon_size(), get_top_left(), get_bottom_right());
            ImGui::SameLine();
        }

        // Render tree node
        bool node_open = ImGui::TreeNodeEx(entity, flags, "%s", entity->get_name().c_str());

        // Handle interactions
        if (ImGui::IsItemClicked()) {
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

        // Render children if open
        if (node_open) {
            for (auto& element : entity->get_all_elements()) {
                if (element) render_element(element);
            }
            ImGui::TreePop();
        }
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
}  // namespace Salix