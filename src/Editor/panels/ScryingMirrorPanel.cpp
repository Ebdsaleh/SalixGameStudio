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
        void draw_property(const Property& prop, Element* element);
        
    };

    ScryingMirrorPanel::ScryingMirrorPanel() : pimpl(std::make_unique<Pimpl>() ) { }
    ScryingMirrorPanel::~ScryingMirrorPanel() = default;


    void ScryingMirrorPanel::initialize(EditorContext* context) {
        if(!context) { return; } // Cannot accept a null EditorContext pointer.
        pimpl->context = context;
        pimpl->icon_manager = dynamic_cast<ImGuiIconManager*>(pimpl->context->gui->get_icon_manager());
        if (!pimpl->icon_manager) { return ;} // ImGuiIconManager failed casting.
        // Subscribe this panel to the Editor event category
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
        std::cout << "ScryingMirrorPanel Initialized and subscribed to Editor events." << std::endl;
    }


    void ScryingMirrorPanel::set_name(const std::string& new_name) {
        pimpl->name = new_name;
    }

    const std::string& ScryingMirrorPanel::get_name() {
        return pimpl->name;
    }

   
    void ScryingMirrorPanel::on_gui_update() {
        if (!pimpl->is_visible) {
            return;
        }

        if(ImGui::Begin("Scrying Mirror", &pimpl->is_visible)){
            ImGui::Separator();
            // handle icon adjustment for OpenGL rendering
                ImVec2 icon_size = ImVec2(16, 16);
                ImVec2 top_left = ImVec2(0, 0);
                ImVec2 bottom_right = ImVec2(1, 1);
                if (pimpl->context->init_context->renderer_type == RendererType::OpenGL) {
                    top_left = ImVec2(0, 1);
                    bottom_right = ImVec2 (1, 0);
                }
                // --- PANEL LOCK/UNLOCK BUTTON ---
                ImGui::AlignTextToFramePadding();
                // ImGui::SameLine(

                const IconInfo& lock_icon = pimpl->is_locked ? 
                    pimpl->icon_manager->get_icon_by_name("Panel Locked") :
                    pimpl->icon_manager->get_icon_by_name("Panel Unlocked");

                ImVec4 tint_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white (no tint)
                if (pimpl->is_locked) {
                    tint_color = ImVec4(0.50f, 0.0f, 0.0f, 1.0f); // Red tint when locked
                }

                // Only attempt to draw if the icon texture ID is valid
                if (lock_icon.texture_id != 0) {
                    if (ImGui::ImageButton("##PanelLockBtn", lock_icon.texture_id, icon_size, top_left, bottom_right, ImVec4(0,0,0,0), tint_color)) {
                        // Button was clicked, toggle the lock state
                        pimpl->is_locked = !pimpl->is_locked; 
                        std::cout << "Scrying Mirror Panel lock toggled to: " << (pimpl->is_locked ? "LOCKED" : "UNLOCKED") << std::endl;
                    }
                } 
            ImGui::Text("Properties:");
            ImGui::Separator();

            if (pimpl->is_locked) {
                ImGui::BeginDisabled(); // Disable all interactive widgets below this point
            }
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
        if (pimpl->is_locked) {
            ImGui::EndDisabled(); // End the disabled block
        }
        ImGui::End();
    }


    void ScryingMirrorPanel::on_gui_render() {}

    void ScryingMirrorPanel::on_render() {}

    void ScryingMirrorPanel::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::EditorEntitySelected) {
            EntitySelectedEvent& entity_selection = static_cast<EntitySelectedEvent&>(event);
            pimpl->selected_entity = entity_selection.entity;
            pimpl->selected_element = nullptr; // Clear the element selection
            std::cout << "Scrying Mirror received selection event for: "
                << (entity_selection.entity ? entity_selection.entity->get_name() : "None") << std::endl;
        }
        else if (event.get_event_type() == EventType::EditorElementSelected) {
            ElementSelectedEvent& element_selection = static_cast<ElementSelectedEvent&>(event);
            pimpl->selected_element = element_selection.element;
            pimpl->selected_entity = nullptr; // Clear the element selection
            std::cout << "Scrying Mirror received selection event for: "
                << (element_selection.element ? element_selection.element->get_class_name() : "None") << std::endl;
        }
    }


    bool ScryingMirrorPanel::get_visibility() const {
        return pimpl->is_visible;
    }


    void ScryingMirrorPanel::set_visibility(bool visibility) {
        pimpl->is_visible = visibility;
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

    bool ScryingMirrorPanel::is_locked() {
        return pimpl->is_locked;
    }


    void ScryingMirrorPanel::unlock() {
        pimpl->is_locked = false;
    }


    void ScryingMirrorPanel::lock() {
        pimpl->is_locked = true;
    }   

}  // namespace Salix