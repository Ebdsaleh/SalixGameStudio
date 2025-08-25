// Editor/panels/ScryingMirrorPanel.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Editor/Archetypes.h>
#include <Editor/reflection/PropertyHandleFactory.h>
#include <Salix/events/EventManager.h>
#include <imgui/imgui.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/management/FileManager.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
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
#include <algorithm>
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
        SimpleGuid selected_entity_id = SimpleGuid::invalid();
        SimpleGuid selected_element_id = SimpleGuid::invalid();
        void handle_media_file_selection(PropertyHandle& handle, ElementArchetype* element_archetype, const TypeInfo* type_info);
        void handle_box_collider_resize_button(ElementArchetype* box_collider_archetype);
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
   


    void ScryingMirrorPanel::Pimpl::handle_media_file_selection(PropertyHandle& handle, ElementArchetype* element_archetype, const TypeInfo* type_info) {
        UIHint hint = handle.get_hint();
        std::string dialog_key, filters;

        switch (hint) {
            case UIHint::ImageFile:
                dialog_key = "SelectImageFile";
                filters = "Image Files (*.png, *.jpg, *.jpeg){.png,.jpg,.jpeg},All Files (*.*){.*}";
                break;
            case UIHint::AudioFile:
                dialog_key = "SelectAudioFile";
                filters = "Audio Files (*.wav, *.mp3, *.ogg){.wav,.mp3,.ogg},All Files (*.*){.*}";
                break;
            case UIHint::SourceFile:
                dialog_key = "SelectSourceFile";
                filters = "Source Files (*.h, *.cpp){.h,.cpp},All Files (*.*){.*}";
                break;
            default:
                dialog_key = "SelectFile";
                filters = "All Files (*.*){.*}";
                break;
        }

        if (DialogBox* dialog = context->gui->get_dialog(dialog_key)) {
            dialog->set_default_path(Salix::g_project_root_path.string());
            dialog->set_filters(filters);

            std::string property_name = handle.get_name();

            dialog->set_callback([this, element_archetype, type_info, property_name](const FileDialogResult& result) {
                if (result.is_ok) {
                    std::function<void()> command = [=]() {
                        std::string relative_path = FileManager::convert_to_relative_path(
                            Salix::g_project_root_path.string(),
                            result.file_path_name
                        );
                        
                        PropertyValueChangedEvent event(
                            this->selected_entity_id, // 'this' now refers to the Pimpl struct
                            element_archetype->id,
                            type_info->name,
                            property_name,
                            relative_path,
                            true
                        );
                        this->context->event_manager->dispatch(event); // 'this' now refers to the Pimpl struct
                    };
                    this->context->deferred_type_drawer_commands.push_back(command); // 'this' now refers to the Pimpl struct
                }
            });
            context->gui->show_dialog_by_key(dialog_key);
        }
    }
    
 
    void ScryingMirrorPanel::Pimpl::handle_box_collider_resize_button(ElementArchetype* box_collider_archetype) {
        if (!box_collider_archetype) {
            return;
        }

        // 1. Get the ID of the owner entity from the collider archetype.
        const SimpleGuid& owner_id = box_collider_archetype->owner_id;

        // 2. Use the map's .find() method for a fast lookup.
        auto entity_iterator = context->current_realm_map.find(owner_id);

        // 3. Check if the entity was found in the map.
        if (entity_iterator != context->current_realm_map.end()) {
            // 4. If found, get the pointer to the entity archetype from the iterator.
            EntityArchetype* owner_entity = entity_iterator->second;

            // 5. Use your helper to find any sibling Sprite2D components.
            std::vector<ElementArchetype*> sprite_archetypes = owner_entity->get_elements_by_type_name("Sprite2D");

            // 6. If we found at least one Sprite2D, use the first one.
            if (!sprite_archetypes.empty()) {
                ElementArchetype* sprite_archetype = sprite_archetypes[0];

                int tex_width = sprite_archetype->data["width"] ? sprite_archetype->data["width"].as<int>() : 0;
                int tex_height = sprite_archetype->data["height"] ? sprite_archetype->data["height"].as<int>() : 0;

                if (tex_width > 0 && tex_height > 0) {
                    float ppu = context->renderer->get_pixels_per_unit();
                    if (ppu <= 0.0f) ppu = 100.0f;

                    // 7. Calculate the new size.
                    Vector3 new_size = Vector3(
                        (float)tex_width / ppu,
                        (float)tex_height / ppu,
                        std::max(1.0f, box_collider_archetype->data["size"].as<Vector3>().z) // Ensures Z is at least 1.0
                    );

                    // 8. Fire the event to update the BoxCollider's size property.
                    PropertyValueChangedEvent event(
                        owner_entity->id,
                        box_collider_archetype->id,
                        "BoxCollider",
                        "size",
                        new_size
                    );
                    context->event_manager->dispatch(event);
                }
            }
        }
    }

    void ScryingMirrorPanel::on_panel_gui_update() {
        if (!pimpl->is_visible || !pimpl->context) {
            return;
        }

        ImGui::Text("Properties:");
        ImGui::Separator();

        // The main logic is now nested inside this check for the data mode.
        if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            EntityArchetype* selected_archetype = nullptr;
            if (pimpl->selected_entity_id.is_valid()) {
                for (auto& archetype : pimpl->context->current_realm) {
                    if (archetype.id == pimpl->selected_entity_id) {
                        selected_archetype = &archetype;
                        break;
                    }
                }
            }

            if (selected_archetype) {
                // YAML/Archetype drawing logic.
                // It correctly handles displaying either a whole entity or a single selected element.
                std::vector<ElementArchetype*> elements_to_display;
                std::string header_name = "Entity: " + selected_archetype->name;

                if (pimpl->selected_element_id.is_valid()) {
                    for (auto& element_archetype : selected_archetype->elements) {
                        if (element_archetype.id == pimpl->selected_element_id) {
                            elements_to_display.push_back(&element_archetype);
                            header_name = "Element: " + element_archetype.name;
                            break;
                        }
                    }
                } else {
                    for (auto& element_archetype : selected_archetype->elements) {
                        elements_to_display.push_back(&element_archetype);
                    }
                }
                
                ImGui::Text("%s", header_name.c_str());
                ImGui::Separator();

                for (auto* element_archetype : elements_to_display) {
                    const TypeInfo* type_info = ByteMirror::get_type_info_by_name(element_archetype->type_name);
                    if (ImGui::CollapsingHeader(StringUtils::convert_from_pascal_case(type_info->name).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        auto handles = PropertyHandleFactory::create_handles_for_element_archetype(element_archetype);
                        if (!handles.empty() && ImGui::BeginTable(type_info->name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                            for (const auto& handle : handles) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0); 
                                std::string display_name = StringUtils::to_title_case(handle->get_name(), true);
                                ImGui::Text("%s", display_name.c_str());
                                ImGui::TableSetColumnIndex(1); ImGui::PushItemWidth(-FLT_MIN);
                                std::string widget_id = "##" + handle->get_name();
                                if(TypeDrawer::draw_property(widget_id.c_str(), *handle, pimpl->context)) {
                                    if (handle->get_hint() != UIHint::None) {
                                        // do the DialogBox stuff
                                        std::cout << "We got a button click!" << std::endl;
                                        pimpl->handle_media_file_selection(*handle, element_archetype, type_info);
                                        
                                    } else {
                                        // If it returns true, a value was changed. FIRE THE EVENT!
                                        PropertyValueChangedEvent event(
                                            pimpl->selected_entity_id,
                                            element_archetype->id,
                                            type_info->name,
                                            handle->get_name(),
                                            handle->get_value()
                                        );
                                        if (ImGui::IsItemActive()) {
                                            pimpl->context->is_editing_property = true;
                                        }
                                        pimpl->context->event_manager->dispatch(event);
                                    }
                                }
                                ImGui::PopItemWidth();
                            }
                            ImGui::EndTable();
                        }
                        // If the element we just drew was a BoxCollider, add our special button.
                        if (element_archetype->type_name == "BoxCollider") {
                            if (ImGui::Button("Fit Texture", ImVec2(-1, 0))) {
                                // Create a command to be executed at the end of the frame.
                                std::function<void()> command = [this, element_archetype]() {
                                    // The command calls our existing helper method.
                                    pimpl->handle_box_collider_resize_button(element_archetype);
                                };
                                // Add the command to the queue.
                                pimpl->context->deferred_type_drawer_commands.push_back(command);
                            }
                        }
                    }
                }

            } else {
                ImGui::Text("No object selected.");
            }
        }
        // Might add 'else if' blocks here later for other data modes
        // else if (pimpl->context->data_mode == EditorDataMode::Json) { ... }
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

           
        }

        
    }

}  // namespace Salix