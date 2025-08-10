// Editor/panels/ScryingMirrorPanel.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/Archetypes.h>
#include <Salix/events/EventManager.h>
#include <imgui/imgui.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Editor/reflection/ui/TypeDrawer.h>
#include <Salix/reflection/ui/TypeDrawerLive.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <map>
#include <string>
#include <iostream>
#include <vector>


namespace Salix {

    struct ScryingMirrorPanel::Pimpl {
        std::string name;
        EditorContext* context = nullptr;
        bool is_visible = true;
        bool is_locked = false;
        ImGuiIconManager* icon_manager = nullptr;
        Entity* selected_entity = nullptr;
        Element* selected_element = nullptr;
        SimpleGuid selected_entity_id = SimpleGuid::invalid();
        SimpleGuid selected_element_id = SimpleGuid::invalid();
    };

    ScryingMirrorPanel::ScryingMirrorPanel() : pimpl(std::make_unique<Pimpl>() ) {
        set_name("Scrying Mirror Panel");
        set_title("Scrying Mirror");
        set_lock_icon_size(ImVec2(16, 16));
    }
    ScryingMirrorPanel::~ScryingMirrorPanel() = default;


    void ScryingMirrorPanel::initialize(EditorContext* context) {
        if(!context) { return; } // Cannot accept a null EditorContext pointer.
        LockablePanel::initialize(context); // CRUCIAL: Base init first
        pimpl->context = context;
        set_visibility(true);
        
        // Configure lock visuals
        set_locked_state_tint_color(ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        set_unlocked_state_tint_color(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        set_lock_icon("Panel Unlocked");
        // Subscribe this panel to the Editor event category
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
        std::cout << "ScryingMirrorPanel Initialized and subscribed to Editor events." << std::endl;
    }

    ImGuiWindowFlags ScryingMirrorPanel::get_window_flags() const {
        return ImGuiWindowFlags_None; // Add any custom flags here
    }
   


    void ScryingMirrorPanel::on_panel_gui_update() {
        if (!pimpl->is_visible) {
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Text("Properties:");
        ImGui::Separator();

        if (!pimpl->context) {
            ImGui::Text("No context.");
            return;
        }

        // =================================================================================
        // --- LIVE MODE PATHWAY ---
        // =================================================================================
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (!pimpl->context->active_scene) {
                ImGui::Text("No active scene.");
                return;
            }

            Entity* selected_entity_live = pimpl->context->active_scene->get_entity_by_id(pimpl->selected_entity_id);
            Element* selected_element_live = selected_entity_live ? selected_entity_live->get_element_by_id(pimpl->selected_element_id) : nullptr;

            std::vector<Salix::Element*> elements_to_display;
            if (selected_element_live) {
                elements_to_display.push_back(selected_element_live);
            }
            else if (selected_entity_live) {
                ImGui::Text("Entity: %s", selected_entity_live->get_name().c_str());
                ImGui::Separator();
                elements_to_display = selected_entity_live->get_all_elements();
            }

            if (!elements_to_display.empty()) {
                for (auto* element : elements_to_display) {
                    const TypeInfo* typeInfo = ByteMirror::get_type_info(typeid(*element));
                    if (!typeInfo) continue;

                    if (ImGui::CollapsingHeader(typeInfo->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        auto handles = ByteMirror::create_handles_for(element);
                        if (!handles.empty() && ImGui::BeginTable(typeInfo->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {

                            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                            for (const auto& handle : handles) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%s", handle->get_name().c_str());
                                ImGui::TableSetColumnIndex(1);
                                ImGui::PushItemWidth(-FLT_MIN);
                                TypeDrawerLive::draw_property(*handle);
                                ImGui::PopItemWidth();
                            }
                            ImGui::EndTable();
                        }
                    }
                }
            }
            else {
                ImGui::Text("No object selected.");
            }
        }
        // =================================================================================
        // --- YAML MODE PATHWAY ---
        // =================================================================================
        else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            
            ImGui::Text("Properties:");
            ImGui::Separator();

            // 1. Check if an entity is selected
            if (pimpl->selected_entity_id.get_value() == 0) {
                ImGui::Text("No object selected.");
                return;
            }

           
            EntityArchetype* selected_archetype = nullptr;
            std::vector<ElementArchetype*> elements_to_display;

            // 2. Loop through the realm to find the archetype with the matching ID.
            for (auto& archetype : pimpl->context->current_realm) {
                if (archetype.id == pimpl->selected_entity_id) {
                    selected_archetype = &archetype;
                    break;
                }
            }

            // 3. Make sure you actually found it before proceeding!
            if (!selected_archetype) {
                pimpl->selected_entity_id = SimpleGuid::invalid(); // Clear bad selection
                return;
            }
            
            // CASE 1: A specific element is selected
            if (pimpl->selected_element_id.is_valid()) {
                for (auto& elementArchetype : selected_archetype->elements) {
                    if (elementArchetype.id == pimpl->selected_element_id) {
                        elements_to_display.push_back(&elementArchetype);
                        break;
                    }
                }
            } 
            // CASE 2: Only an entity is selected, so display all its elements
            else {
                for (auto& elementArchetype : selected_archetype->elements) {
                    elements_to_display.push_back(&elementArchetype);
                }
            }
            
            

            // The rest of the drawing code now loops over our prepared list
            if (!elements_to_display.empty()) {
                for (auto* elementArchetype : elements_to_display) {
                    const TypeInfo* type_info = ByteMirror::get_type_info_by_name(elementArchetype->type_name);
                    if (!type_info) continue;

                    if (ImGui::CollapsingHeader(type_info->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::BeginTable(type_info->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                            for (const Property& prop : type_info->properties) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%s", prop.name.c_str());
                                ImGui::TableSetColumnIndex(1);
                                ImGui::PushItemWidth(-FLT_MIN);
                                TypeDrawer::draw_yaml_property(prop, elementArchetype->data);
                                ImGui::PopItemWidth();
                            }
                            ImGui::EndTable();
                        }
                    }
                }
            }
        }
    }




    void ScryingMirrorPanel::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::EditorEntitySelected) {
            EntitySelectedEvent& e = static_cast<EntitySelectedEvent&>(event);

            // --- CORRECTED LOGIC ---
            // Check the ID, not the pointer. If the ID is valid, it's a selection.
            if (e.selected_id.is_valid()) {
                pimpl->selected_entity_id = e.selected_id;
                pimpl->selected_element_id = SimpleGuid::invalid(); // Clear element selection
                std::cout << "Scrying Mirror received selection for entity ID: " << e.selected_id.get_value() << std::endl;
            } else {
                // This is a deselection event
                pimpl->selected_entity_id = SimpleGuid::invalid();
                pimpl->selected_element_id = SimpleGuid::invalid();
                std::cout << "Scrying Mirror received deselection event" << std::endl;
            }

            // We can still store the live pointer for Live Mode's convenience
            pimpl->selected_entity = e.entity;
            pimpl->selected_element = nullptr;
        }
        else if (event.get_event_type() == EventType::EditorElementSelected) {
            ElementSelectedEvent& e = static_cast<ElementSelectedEvent&>(event);

            // --- CORRECTED LOGIC ---
            // Check the ID, not the pointer.
            if (e.selected_id.is_valid()) {
                pimpl->selected_entity_id = e.owner_id;
                pimpl->selected_element_id = e.selected_id;
                std::cout << "Scrying Mirror received selection for element ID: " << e.selected_id.get_value() << std::endl;
            } else {
                // This is a deselection event
                pimpl->selected_entity_id = SimpleGuid::invalid();
                pimpl->selected_element_id = SimpleGuid::invalid();
                std::cout << "Scrying Mirror received deselection event" << std::endl;
            }

            // Store the live pointer for Live Mode
            pimpl->selected_element = e.element;
        }
    }

}  // namespace Salix