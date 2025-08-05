// Editor/panels/ScryingMirrorPanel.cpp
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Salix/events/EventManager.h>
#include <imgui/imgui.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/ui/TypeDrawer.h>
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
        void draw_property(const Property& prop, Element* element);
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


        // --- New Data-Driven Lookup Logic ---

        Entity* selected_entity_live = nullptr;
        Element* selected_element_live = nullptr;

        if (pimpl->context && pimpl->context->active_scene) {
            selected_entity_live = pimpl->context->active_scene->get_entity_by_id(pimpl->selected_entity_id);
            if (selected_entity_live) {
                selected_element_live = selected_entity_live->get_element_by_id(pimpl->selected_element_id);
            }
        }
        // --- End New Data-Driven Lookup Logic ---



        if (pimpl->selected_element || (pimpl->context && pimpl->context->selected_entity)) {
            if (pimpl->context->selected_entity) {
                ImGui::Text("Entity: %s", pimpl->context->selected_entity->get_name().c_str());
                ImGui::Separator();
            }

            std::vector<Salix::Element*> elements_to_display;

            if (pimpl->selected_element) {
                elements_to_display.push_back(pimpl->selected_element);
            }
            else if (pimpl->context && pimpl->context->selected_entity) {
                elements_to_display = pimpl->context->selected_entity->get_all_elements();
            }

            for (auto* element : elements_to_display) {
                const std::type_index typeIndex = typeid(*element);
                const TypeInfo* typeInfo = ByteMirror::get_type_info(typeIndex);

                if (!typeInfo) {
                    ImGui::Text("Unreflected Element: %s", typeIndex.name());
                    continue;
                }
                
                if (ImGui::CollapsingHeader(typeInfo->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    // This is where we add the table.
                    if (ImGui::BeginTable(typeInfo->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                        for (const auto& prop : typeInfo->properties) {
                            pimpl->draw_property(prop, element);
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
        if (event.get_event_type() == EventType::EditorEntitySelected) {
            EntitySelectedEvent& entity_selection = static_cast<EntitySelectedEvent&>(event);
            if (pimpl->selected_entity == nullptr) {
                pimpl->selected_entity = nullptr;
                pimpl->selected_entity_id = SimpleGuid::invalid();
                pimpl->selected_element = nullptr;
                pimpl->selected_element_id = SimpleGuid::invalid();
                std::cout << "Scrying Mirror received selection event for: None" << std::endl;
                return; // Exit the method to prevent further use of the null pointer.
            }
            pimpl->selected_entity = entity_selection.entity; 
            pimpl->selected_entity_id = entity_selection.entity->get_id();
            pimpl->selected_element = nullptr; // Clear the element selection
            pimpl->selected_element_id = SimpleGuid::invalid();
            std::cout << "Scrying Mirror received selection event for: "
                << (entity_selection.entity ? entity_selection.entity->get_name() : "None") << std::endl;
        }
        else if (event.get_event_type() == EventType::EditorElementSelected) {
            ElementSelectedEvent& element_selection = static_cast<ElementSelectedEvent&>(event);
            if (element_selection.element == nullptr) {
                pimpl->selected_element = nullptr;
                pimpl->selected_element_id = SimpleGuid::invalid();
                pimpl->selected_entity = nullptr;
                pimpl->selected_entity_id = SimpleGuid::invalid();
                std::cout << "Scrying Mirror received selection event for: None" << std::endl;
                return;
            }
            pimpl->selected_element = element_selection.element;
            pimpl->selected_element_id = element_selection.element->get_id();
            pimpl->selected_entity = nullptr; // Clear the element selection
            pimpl->selected_entity_id = SimpleGuid::invalid();
            std::cout << "Scrying Mirror received selection event for: "
                << (element_selection.element ? element_selection.element->get_class_name() : "None") << std::endl;
        }
    }





    // --- Private methods ---

    



     void ScryingMirrorPanel::Pimpl::draw_property(const Property& prop, Element* element) {
        // We now draw the property across two columns.
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); // Column 1 for the label
        ImGui::Text("%s", prop.name.c_str());

        ImGui::TableSetColumnIndex(1); // Column 2 for the widget
        ImGui::PushItemWidth(-FLT_MIN);
        
        TypeDrawer::get_drawer(prop.type)(prop, element);

        ImGui::PopItemWidth();
    }

}  // namespace Salix