// Editor/panels/ScryingMirrorPanel.cpp
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Salix/events/EventManager.h>
#include <imgui/imgui.h>
#include <map>
#include <string>
#include <iostream>
#include <vector>


namespace Salix {

    struct ScryingMirrorPanel::Pimpl { 
        EditorContext* context = nullptr;
        std::map<std::string, std::string> properties;
        bool is_visible = true;
        
    };

    ScryingMirrorPanel::ScryingMirrorPanel() : pimpl(std::make_unique<Pimpl>() ) { }
    ScryingMirrorPanel::~ScryingMirrorPanel() = default;


    void ScryingMirrorPanel::initialize(EditorContext* context) {
        if(!context) { return; } // Cannot accept a null EditorContext pointer.
        pimpl->context = context;
        
        // Subscribe this panel to the Editor event category
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
        std::cout << "ScryingMirrorPanel Initialized and subscribed to Editor events." << std::endl;
    }


    void ScryingMirrorPanel::on_gui_render() {
        if (!pimpl->is_visible) {
            return; // If the panel isn't visible, do nothing.
        }

        if(ImGui::Begin("Scrying Mirror", &pimpl->is_visible)){
            ImGui::Text("Properties:");
            ImGui::Separator();
            // If not properties retrieved from selected object don't populate this space.
            if (pimpl->properties.empty()) { 
                ImGui::Text("No object selected.");
            } 
            else {
                // Use a two-column table for a clean, aligned layout.
                // The ImGuiTableFlags_Resizable flag lets the user drag the column divider.
                if (ImGui::BeginTable("properties_table", 2, ImGuiTableFlags_Resizable)) {
                    // Set up the columns. The second column will stretch to fill available space.
                    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                char text_buffer[256];
                // iterate through the properties map
                // Iterate through all properties in the map
                    for (auto& pair : pimpl->properties) {
                        // Start a new row in the table
                        ImGui::TableNextRow();

                        // --- Column 1: Property Name (the label) ---
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", pair.first.c_str());

                        // --- Column 2: Property Value (the editable text box) ---
                        ImGui::TableSetColumnIndex(1);

                        // We need to give each InputText a unique ID so ImGui doesn't get confused.
                        // Pushing the property name to the ID stack is a good way to do this.
                        ImGui::PushID(pair.first.c_str());
                        
                        // Copy the map's value into our temporary buffer for editing.
                        strncpy_s(text_buffer, sizeof(text_buffer), pair.second.c_str(), sizeof(text_buffer) - 1);

                        // Make the InputText widget fill the entire width of the column.
                        ImGui::PushItemWidth(-FLT_MIN);
                        if (ImGui::InputText("##Value", text_buffer, sizeof(text_buffer))) {
                            // If the user changed the text, update our map.
                            pair.second = text_buffer;
                        }
                        ImGui::PopItemWidth();

                        // Pop the unique ID to keep the stack clean.
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }

    bool ScryingMirrorPanel::get_visibility() const {
        return pimpl->is_visible;
    }

    void ScryingMirrorPanel::set_visibility(bool visibility) {
        pimpl->is_visible = visibility;
    }

    void ScryingMirrorPanel::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::EditorEntitySelected) {
            // Need to cast the event to the specific type
            EntitySelectedEvent& e = static_cast<EntitySelectedEvent&>(event);

            // Log that we received the event to confirm it's working
            std::cout << "Scrying Mirror received selection event for: "
                      << (e.entity ? e.entity->get_name() : "None") << std::endl;
            // The panel's on_gui_render will automatically pick up the
            // selected_entity from the context, so we don't need to do
            // anything else here. This event is more for triggering
            // one-time actions when a selection changes.
        }
    }


}  // namespace Salix