// Editor/panels/ScryingMirrorPanel.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/Archetypes.h>
#include <Editor/reflection/PropertyHandleFactory.h>
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
#include <Salix/core/StringUtils.h>
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
        if (!pimpl->is_visible || !pimpl->context) {
            return;
        }

        ImGui::Text("Properties:");
        ImGui::Separator();

        // First, find the selected entity (live or archetype) based on the stored ID.
        Entity* selected_entity_live = nullptr;
        EntityArchetype* selected_archetype = nullptr;

        if (pimpl->selected_entity_id.is_valid()) {
            if (pimpl->context->data_mode == EditorDataMode::Live) {
                if (pimpl->context->active_scene) {
                    selected_entity_live = pimpl->context->active_scene->get_entity_by_id(pimpl->selected_entity_id);
                }
            } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
                for (auto& archetype : pimpl->context->current_realm) {
                    if (archetype.id == pimpl->selected_entity_id) {
                        selected_archetype = &archetype;
                        break;
                    }
                }
            }
        }

        // --- DRAWING LOGIC STARTS HERE ---

        if (selected_entity_live) {
            // --- LIVE MODE DRAWING ---
            std::vector<Element*> elements_to_display;
            
            // NEW LOGIC: Check if a specific element is selected.
            if (pimpl->selected_element_id.is_valid()) {
                Element* selected_element = selected_entity_live->get_element_by_id(pimpl->selected_element_id);
                if (selected_element) {
                    elements_to_display.push_back(selected_element);
                }
            } else {
                // Otherwise, display all elements for the entity.
                elements_to_display = selected_entity_live->get_all_elements();
                ImGui::Text("Entity: %s", selected_entity_live->get_name().c_str());
                ImGui::Separator();
            }

            for (auto* element : elements_to_display) {
                const TypeInfo* type_info = ByteMirror::get_type_info(typeid(*element));
                if (ImGui::CollapsingHeader(type_info->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto handles = ByteMirror::create_handles_for(element);
                    if (!handles.empty() && ImGui::BeginTable(type_info->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                        for (const auto& handle : handles) {
                            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); 
                            std::string display_name = StringUtils::to_title_case(handle->get_name(), true);
                            ImGui::Text("%s", display_name.c_str());
                            ImGui::TableSetColumnIndex(1); ImGui::PushItemWidth(-FLT_MIN);
                            std::string widget_id = "##" + handle->get_name();
                            TypeDrawer::draw_property(widget_id.c_str(), *handle);
                            ImGui::PopItemWidth();
                        }
                        ImGui::EndTable();
                    }
                }
            }

        } else if (selected_archetype) {
            // --- YAML MODE DRAWING ---
            std::vector<ElementArchetype*> elements_to_display;

            // NEW LOGIC: Check if a specific element archetype is selected.
            if (pimpl->selected_element_id.is_valid()) {
                for (auto& element_archetype : selected_archetype->elements) {
                    if (element_archetype.id == pimpl->selected_element_id) {
                        elements_to_display.push_back(&element_archetype);
                        break;
                    }
                }
            } else {
                // Otherwise, display all element archetypes for the entity archetype.
                for (auto& element_archetype : selected_archetype->elements) {
                    elements_to_display.push_back(&element_archetype);
                }
                ImGui::Text("Entity: %s", selected_archetype->name.c_str());
                ImGui::Separator();
            }

            for (auto* element_archetype : elements_to_display) {
                const TypeInfo* type_info = ByteMirror::get_type_info_by_name(element_archetype->type_name);
                if (ImGui::CollapsingHeader(type_info->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto handles = PropertyHandleFactory::create_handles_for_element_archetype(element_archetype);
                    if (!handles.empty() && ImGui::BeginTable(type_info->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                        for (const auto& handle : handles) {
                            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); 
                            std::string display_name = StringUtils::to_title_case(handle->get_name(), true);
                            ImGui::Text("%s", display_name.c_str());
                            ImGui::TableSetColumnIndex(1); ImGui::PushItemWidth(-FLT_MIN);
                            std::string widget_id = "##" + handle->get_name();
                            TypeDrawer::draw_property(widget_id.c_str(), *handle);
                            ImGui::PopItemWidth();
                        }
                        ImGui::EndTable();
                    }
                }
            }

        } else {
            ImGui::Text("No object selected.");
        }
    }



    void ScryingMirrorPanel::on_event(IEvent& event) {
        // ADD THIS LINE FOR DEBUGGING
        std::cout << "[DEBUG:] [ScryingMirrorPanel] RECEIVED an event of type: " << static_cast<int>(event.get_event_type()) << std::endl;

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