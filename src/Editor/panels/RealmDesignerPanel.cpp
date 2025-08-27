// Editor/panels/RealmDesignerPanel.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/panels/RealmDesignerPanel.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IconInfo.h>
#include <Editor/EditorContext.h>
#include <Salix/core/SDLTimer.h>
#include <Salix/core/SimpleGuid.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/core/InitContext.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/RenderableElement3D.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/math/Color.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Salix/math/RayCasting.h>
#include <Salix/events/EventManager.h>
#include <yaml-cpp/yaml.h>
#include <Editor/Archetypes.h>
#include <Editor/ArchetypeInstantiator.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <cassert>
#include <SDL.h>


namespace Salix {

    // A small struct to hold all the info needed for a single draw call
    struct RenderJob {
        const Sprite2D* sprite;
        const Transform* transform;
        int sorting_layer;
    };

    struct RealmDesignerPanel::Pimpl { 
        EditorContext* context = nullptr;
        ImGuiIconManager* icon_manager = nullptr;
        bool is_visible = true;
        bool is_locked = false;
        std::string name = "Realm Designer";
        uint32_t framebuffer_id = 0;
        ImVec2 viewport_size = { 1280, 720 };
        bool is_panel_focused_this_frame = false;
        SimpleGuid selected_entity_id = SimpleGuid::invalid();
        Ray last_picking_ray;
        ImGuizmo::OPERATION CurrentGizmoOperation = ImGuizmo::TRANSLATE;

        
        GLint render_pass_begin();
        void render_pass_end(GLint last_fbo);
        // YAML update Methods
        void update_camera_and_buttons();
        EntityArchetype* get_selected_archetype();
        ElementArchetype* get_transform_archetype(EntityArchetype* entity_archetype);
        void handle_gizmos_for_archetype(EditorCamera* camera);
        glm::mat4 get_world_matrix(const EntityArchetype* archetype);
        void draw_scene();
        void draw_test_cube();
        void draw_test_cube_only();
        void draw_lock_button();
        void draw_gizmo_toolbar();
        void update_viewport(IRenderer* renderer);
        // void handle_gizmos_for_live_entity(EditorCamera* camera, Entity* selected_entity);
        void handle_input();
        void handle_mouse_picking(const ImVec2& viewport_min, const ImVec2& viewport_max);
        void draw_bounding_boxes();
        void draw_grid();
        
    };

    RealmDesignerPanel::RealmDesignerPanel() : pimpl(std::make_unique<Pimpl>()) {

    }


    RealmDesignerPanel::~RealmDesignerPanel() = default;


    void RealmDesignerPanel::initialize(EditorContext* context) {

        pimpl->context = context;

        if (!pimpl->context || !pimpl->context->init_context || !pimpl->context->init_context->renderer) {
            std::cerr << "RealmDesignerPanel Error: Renderer is not available, cannot create framebuffer!" << std::endl;
            return;
        }
        
        pimpl->icon_manager = dynamic_cast<ImGuiIconManager*>(pimpl->context->gui->get_icon_manager());
        if (!pimpl->icon_manager) { return; }
        

        // Get the renderer from the context
        IRenderer* renderer = pimpl->context->init_context->renderer;
        
        // Create the framebuffer with the panel's default size
        pimpl->framebuffer_id = renderer->create_framebuffer(
            static_cast<int>(pimpl->viewport_size.x),
            static_cast<int>(pimpl->viewport_size.y)
        );

        if (pimpl->framebuffer_id == 0) {
            std::cerr << "RealmDesignerPanel Error: Failed to create framebuffer!" << std::endl;
        } else {
            std::cout << "RealmDesignerPanel Initialized and created framebuffer with ID: " << 
                pimpl->framebuffer_id << std::endl;
        }
         // The preview scene also needs a context to function properly

        // --- Create the scene and give it to the context ---
        pimpl->context->preview_scene = std::make_unique<Scene>("Preview");
        if (pimpl->context->preview_scene) {
            pimpl->context->preview_scene->set_context(*pimpl->context->init_context);
        }
        
        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
        std::cout << "RealmDesignerPanel Initialized and subscribed to Editor events." << std::endl;
    }

    // YAML PATHWAY Update helpers


    glm::mat4 RealmDesignerPanel::Pimpl::get_world_matrix(const EntityArchetype* archetype) {
        if (!archetype) {
            return glm::mat4(1.0f);
        }

        ElementArchetype* transform_archetype = get_transform_archetype(const_cast<EntityArchetype*>(archetype));
        if (!transform_archetype) {
            return glm::mat4(1.0f);
        }

        Vector3 p = transform_archetype->data["position"].as<Vector3>();
        Vector3 r = transform_archetype->data["rotation"].as<Vector3>();
        Vector3 s = transform_archetype->data["scale"].as<Vector3>();

        glm::mat4 local_matrix;
        ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(p.to_glm()), glm::value_ptr(r.to_glm()), glm::value_ptr(s.to_glm()), glm::value_ptr(local_matrix));

        if (archetype->parent_id.is_valid()) {
            auto parent_it = context->current_realm_map.find(archetype->parent_id);
            if (parent_it != context->current_realm_map.end()) {
                return get_world_matrix(parent_it->second) * local_matrix;
            }
        }

        return local_matrix;
    }

    // --- Implementation of Archetype Helper Functions ---
    EntityArchetype* RealmDesignerPanel::Pimpl::get_selected_archetype() {
        if (!context || !context->selected_entity_id.is_valid()) {
            return nullptr;
        }
        for (auto& archetype : context->current_realm) {
            if (archetype.id == context->selected_entity_id) {
                return &archetype;
            }
        }
        return nullptr;
    }


    ElementArchetype* RealmDesignerPanel::Pimpl::get_transform_archetype(EntityArchetype* entity_archetype) {
        if (!entity_archetype) {
            return nullptr;
        }
        for (auto& element : entity_archetype->elements) {
            if (element.type_name == "Transform") {
                return &element;
            }
        }
        return nullptr;
    }

    // --- Implementation of Archetype Helper Functions End ---



    void RealmDesignerPanel::Pimpl::update_camera_and_buttons() {
        // --- Camera Control Logic ---
        bool camera_can_move = false;
        // This logic determines if the viewport is focused and not locked
        if (!is_locked &&
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) && 
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow)) {
            camera_can_move = true;
        }
        context->editor_camera->set_mouse_inside_scene(camera_can_move);

        // --- Panel UI Button Logic ---
        draw_lock_button();
        draw_gizmo_toolbar();
    }



    void on_gui_update_early_exit() {
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    // End of YAML PATHWAYS Helper methods


    // MODIFIED: This is now the single entry point for interactions.
    void RealmDesignerPanel::Pimpl::handle_input() {
        ImVec2 min_bound = ImGui::GetItemRectMin();
        ImVec2 max_bound = ImGui::GetItemRectMax();

        if (!is_locked) {
            /* if (context->data_mode == EditorDataMode::Live) {
                Entity* selected_entity = context->active_scene ? context->active_scene->get_entity_by_id(context->selected_entity_id) : nullptr;
                if (selected_entity) {
                    // handle_gizmos_for_live_entity(context->editor_camera, selected_entity);
                }
            }
            else */
            if (context->data_mode == EditorDataMode::Yaml) {
                // In YAML mode, we don't need to pass an entity pointer.
                handle_gizmos_for_archetype(context->editor_camera);                
            }
        }

        if (ImGui::IsItemHovered() && !EntitySelectedEvent::block_selection && !is_locked) {
            handle_mouse_picking(min_bound, max_bound);
        }
        
        // --- NEW: Handle the 'F' to Focus keyboard shortcut ---
        // We check if the window is focused so this shortcut only applies to this panel.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
            if (!context->is_editing_property && ImGui::IsKeyPressed(ImGuiKey_F)) {
                // Check if there is a selected entity
                if (context->selected_entity_id.is_valid()) {
                    Entity* selected_entity = nullptr;

                    // Find the selected entity in the correct scene (Preview or Live)
                    if (context->data_mode == EditorDataMode::Yaml) {
                        selected_entity = context->preview_scene->get_entity_by_id(context->selected_entity_id);
                    }
                    /*
                    else if (context->data_mode == EditorDataMode::Live) {
                        if (context->active_scene) {
                            selected_entity = context->active_scene->get_entity_by_id(context->selected_entity_id);
                        }
                    }
                    */


                    if (selected_entity) {
                        if (Transform* transform = selected_entity->get_transform()) {
                            context->editor_camera->focus_on(transform, 3.0f);
                        }
                    }
                }
            }
        }
    }

    


  

    // MODIFIED: The main update loop now calls the new handle_input function.
    void RealmDesignerPanel::on_gui_update() {
        if (!pimpl->is_visible) return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Realm Designer", &pimpl->is_visible)) {
            IRenderer* renderer = pimpl->context->init_context->renderer;
            if (!renderer) {
                ImGui::End(); ImGui::PopStyleVar(); return;
            }

            // --- Scene Preparation ---
            if (pimpl->context->data_mode == EditorDataMode::Yaml) {
                // --- Process the sync queue ---
                if (!pimpl->context->sync_queue.empty()) {
                    for (const auto& command : pimpl->context->sync_queue) {
                        command(); // Execute the re-instantiation command
                    }
                    pimpl->context->sync_queue.clear();
                }
                // 1. Check if the realm data has changed.
                if (pimpl->context->realm_is_dirty) {
                    Scene* preview_scene = pimpl->context->preview_scene.get();
                    auto& realm_archetypes = pimpl->context->current_realm;
                    preview_scene->clear_all_entities();
                    if (!realm_archetypes.empty()) {
                        /*
                        for (const auto& entity_archetype : realm_archetypes) {
                            ArchetypeInstantiator::instantiate(entity_archetype, preview_scene, *pimpl->context->init_context);
                        }
                        */
                        ArchetypeInstantiator::instantiate_realm(realm_archetypes, preview_scene, *pimpl->context->init_context);
                    }
                    // 3. Clear the flag so this doesn't run again.
                    pimpl->context->realm_is_dirty = false;
                }
            }

            pimpl->update_camera_and_buttons();
            pimpl->update_viewport(renderer);

            if (pimpl->framebuffer_id != 0) {
                ImTextureID tex_id = renderer->get_framebuffer_texture_id(pimpl->framebuffer_id);
                if (tex_id != 0) {
                    ImGui::Image(tex_id, pimpl->viewport_size, ImVec2(0, 1), ImVec2(1, 0));
                    // This single call now handles everything.
                    pimpl->handle_input();
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }



    

    void RealmDesignerPanel::Pimpl::draw_lock_button() {
        const IconInfo& lock_icon = is_locked ? 
            icon_manager->get_icon_by_name("Panel Locked") :
            icon_manager->get_icon_by_name("Panel Unlocked");

        ImVec4 tint_color = is_locked ? ImVec4(0.50f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (ImGui::ImageButton("##PanelLockBtn", lock_icon.texture_id, ImVec2(16, 16), 
                            ImVec2(0,0), ImVec2(1,1), ImVec4(0,0,0,0), tint_color)) {
            is_locked = !is_locked;
        }
    }


    void RealmDesignerPanel::Pimpl::draw_gizmo_toolbar() {
        if (ImGui::RadioButton("Translate", CurrentGizmoOperation == ImGuizmo::TRANSLATE))
            CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", CurrentGizmoOperation == ImGuizmo::ROTATE))
            CurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", CurrentGizmoOperation == ImGuizmo::SCALE))
            CurrentGizmoOperation = ImGuizmo::SCALE;

        if (!is_locked && !context->is_editing_property) {
            if (ImGui::IsKeyPressed(ImGuiKey_W) && ImGui::IsWindowHovered()) CurrentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E) && ImGui::IsWindowHovered()) CurrentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R) && ImGui::IsWindowHovered()) CurrentGizmoOperation = ImGuizmo::SCALE;
        }
    }


    void RealmDesignerPanel::Pimpl::update_viewport(IRenderer* renderer) {
        ImVec2 new_size = ImGui::GetContentRegionAvail();
        if (new_size.x > 0 && new_size.y > 0 && 
                (new_size.x != viewport_size.x || new_size.y != viewport_size.y)) {
            if (framebuffer_id != 0)
                renderer->delete_framebuffer(framebuffer_id);
            framebuffer_id = renderer->create_framebuffer((int)new_size.x, (int)new_size.y);
            viewport_size = new_size;
            if (context->editor_camera)
                context->editor_camera->set_viewport_size((int)new_size.x, (int)new_size.y);
        }
    }


    


    /*
    void RealmDesignerPanel::Pimpl::handle_gizmos_for_live_entity(EditorCamera* camera, Entity* selected_entity) {
        if (is_locked || !selected_entity || selected_entity->is_purged()) return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        
        // Set viewport rect
        ImVec2 viewport_min = ImGui::GetItemRectMin();
        ImVec2 viewport_max = ImGui::GetItemRectMax();
        ImGuizmo::SetRect(viewport_min.x, viewport_min.y, 
                        viewport_max.x - viewport_min.x, 
                        viewport_max.y - viewport_min.y);

        // Get matrices
        const glm::mat4& camera_view = camera->get_view_matrix();
        const glm::mat4& camera_projection = camera->get_projection_matrix();
        Transform* transform = selected_entity->get_transform();
        
        // Get the world matrix of the entity
        glm::mat4 entity_matrix = transform->get_model_matrix();
        
        // Handle interaction
        bool manipulated = ImGuizmo::Manipulate(
            glm::value_ptr(camera_view),
            glm::value_ptr(camera_projection),
            CurrentGizmoOperation,
            ImGuizmo::LOCAL, // Always use local space for the gizmo
            glm::value_ptr(entity_matrix)
        );

        // Update transform if manipulated
        if (manipulated) {
            // For child entities, we need to convert the world matrix back to local space
            if (transform->get_parent()) {
                // Get parent's world matrix and its inverse
                glm::mat4 parent_matrix = transform->get_parent()->get_model_matrix();
                glm::mat4 parent_inverse = glm::inverse(parent_matrix);
                
                // Calculate the new local matrix
                glm::mat4 new_local_matrix = parent_inverse * entity_matrix;
                
                // Decompose the new local matrix
                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(new_local_matrix),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation),
                    glm::value_ptr(scale)
                );
                
                // Apply the local transform
                transform->set_position(Vector3(translation.x, translation.y, translation.z));
                transform->set_scale(Vector3(scale.x, scale.y, scale.z));
                
                // Handle rotation based on entity type
                if (selected_entity->get_element<RenderableElement3D>()) {
                    transform->set_rotation(rotation.x, rotation.y, rotation.z);
                } else if (selected_entity->get_element<RenderableElement2D>()) {
                    transform->set_rotation(0.0f, 0.0f, rotation.z);
                } else {
                    transform->set_rotation(rotation.x, rotation.y, rotation.z);
                }
            } else {
                // For root entities, decompose directly
                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(entity_matrix),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation),
                    glm::value_ptr(scale)
                );
                
                transform->set_position(Vector3(translation.x, translation.y, translation.z));
                transform->set_scale(Vector3(scale.x, scale.y, scale.z));
                
                if (selected_entity->get_element<RenderableElement3D>()) {
                    transform->set_rotation(rotation.x, rotation.y, rotation.z);
                } else if (selected_entity->get_element<RenderableElement2D>()) {
                    transform->set_rotation(0.0f, 0.0f, rotation.z);
                } else {
                    transform->set_rotation(rotation.x, rotation.y, rotation.z);
                }
            }
        }

        // Block selection when gizmo is active
        EntitySelectedEvent::block_selection = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
        
        // Set cursor
        if (ImGuizmo::IsOver()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }
    */

    // NEW: The complete implementation for the Archetype gizmo handler.
    void RealmDesignerPanel::Pimpl::handle_gizmos_for_archetype(EditorCamera* camera) {
        EntityArchetype* selected_archetype = get_selected_archetype();
        if (is_locked || !selected_archetype) return;

        ElementArchetype* transform_archetype = get_transform_archetype(selected_archetype);
        if (!transform_archetype) return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImVec2 viewport_min = ImGui::GetItemRectMin();
        ImVec2 viewport_max = ImGui::GetItemRectMax();
        ImGuizmo::SetRect(viewport_min.x, viewport_min.y, viewport_max.x - viewport_min.x, viewport_max.y - viewport_min.y);

        const glm::mat4& camera_view = camera->get_view_matrix();
        const glm::mat4& camera_projection = camera->get_projection_matrix();

        // 1. GET WORLD MATRIX: Calculate the entity's full world matrix
        glm::mat4 entity_world_matrix = get_world_matrix(selected_archetype);

        // 2. MANIPULATE the world matrix
        if (ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection), CurrentGizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(entity_world_matrix))) {
            
            glm::mat4 new_local_matrix = entity_world_matrix;

            // 3. CONVERT BACK TO LOCAL SPACE if the entity has a parent
            if (selected_archetype->parent_id.is_valid()) {
                auto parent_it = context->current_realm_map.find(selected_archetype->parent_id);
                if (parent_it != context->current_realm_map.end()) {
                    glm::mat4 parent_world_matrix = get_world_matrix(parent_it->second);
                    new_local_matrix = glm::inverse(parent_world_matrix) * entity_world_matrix;
                }
            }

            // 4. DECOMPOSE the final local matrix and save the values
            glm::vec3 new_translation, new_rotation_deg, new_scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(new_local_matrix), glm::value_ptr(new_translation), glm::value_ptr(new_rotation_deg), glm::value_ptr(new_scale));

            Vector3 new_position_vector(new_translation.x, new_translation.y, new_translation.z);
            Vector3 new_rotation_vector(new_rotation_deg.x, new_rotation_deg.y, new_rotation_deg.z);
            Vector3 new_scale_vector(new_scale.x, new_scale.y, new_scale.z);

            // WRITE the new local data back to the YAML::Node
            transform_archetype->data["position"] = new_position_vector;
            transform_archetype->data["rotation"] = new_rotation_vector;
            transform_archetype->data["scale"]    = new_scale_vector;

            // 5. FIRE EVENTS to notify other systems of the change
            PropertyValueChangedEvent position_changed_event(selected_archetype->id, transform_archetype->id, "Transform", "position", new_position_vector);
            PropertyValueChangedEvent rotation_changed_event(selected_archetype->id, transform_archetype->id, "Transform", "rotation", new_rotation_vector);
            PropertyValueChangedEvent scale_changed_event(selected_archetype->id, transform_archetype->id, "Transform", "scale",    new_scale_vector);
            context->event_manager->dispatch(position_changed_event);
            context->event_manager->dispatch(rotation_changed_event);
            context->event_manager->dispatch(scale_changed_event);
        }

        EntitySelectedEvent::block_selection = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
        if (ImGuizmo::IsOver()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
    }




    void RealmDesignerPanel::Pimpl::handle_mouse_picking(const ImVec2& viewport_min, const ImVec2& viewport_max) {
        
        // We only proceed if the mouse was actually clicked within the hovered window
        if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            return;
        }
        // --- Step 1: Create the world-space ray from mouse ---
        // Calculate viewport size from the min/max points passed as arguments.
        viewport_size = { (viewport_max.x - viewport_min.x), (viewport_max.y - viewport_min.y) };
        ImVec2 mouse_pos = ImGui::GetMousePos();

        // Create the ray using the reliable viewport data 
        Ray world_ray = Raycast::CreateRayFromScreen(context->editor_camera, mouse_pos, viewport_min, viewport_size);
        last_picking_ray = world_ray; // Save the ray for visualization
        float closest_hit_distance = FLT_MAX;
        // Declare closest_hit_id here so it's accessible by both modes.
        SimpleGuid closest_hit_id = SimpleGuid::invalid();
        if (context->data_mode == EditorDataMode::Live) {    
                   

            // --- Step 2: Find the closest entity with a collider that was hit ---
            Entity* selected_entity = nullptr;
            

            // Add the SimpleGuid variable to store the new ID
            SimpleGuid temp_selected_entity_id = SimpleGuid::invalid();

            for (Entity* entity : context->active_scene->get_entities()) {
                if (!entity || entity->is_purged()) continue;

                Transform* transform = entity->get_transform();
                BoxCollider* collider = entity->get_element<BoxCollider>();

                // We can only pick objects that have both a transform and a collider  
                if (transform && collider) {
                    glm::vec3 half_extents = collider->get_size().to_glm() * 0.5f;
                    glm::mat4 model_matrix = transform->get_model_matrix();  
                    float distance = 0.0f;

                    if (Raycast::IntersectsOBB(world_ray, model_matrix, half_extents, distance)) {
                        glm::vec3 world_hit_point = world_ray.origin + world_ray.direction * distance;  
                        float world_distance = glm::distance(world_ray.origin, world_hit_point);  

                        if (world_distance < closest_hit_distance) {
                            closest_hit_distance = world_distance;
                            selected_entity = entity; 
                            temp_selected_entity_id = entity->get_id(); 
                        }
                    }
                }
            }

            // --- Step 3: Fire the selection event ---
            if (context->event_manager) {
                EntitySelectedEvent event(selected_entity_id,selected_entity);
                context->selected_entity = selected_entity;
                context->event_manager->dispatch(event);  

                // New, data-driven system (Additions)
                selected_entity_id = temp_selected_entity_id;
                context->selected_entity_id = selected_entity_id;
                
            }
        } else if (context->data_mode == EditorDataMode::Yaml) {
            SimpleGuid temp_selected_archetype_id = SimpleGuid::invalid();
            for (auto& archetype : context->current_realm) {
                // Find the transform and collider data for this archetype
                const ElementArchetype* transform_archetype = nullptr;
                const ElementArchetype* collider_archetype = nullptr;
                for (const auto& element : archetype.elements) {
                    if (element.type_name == "Transform") transform_archetype = &element;
                    if (element.type_name == "BoxCollider") collider_archetype = &element;
                }
                if (transform_archetype && collider_archetype) {
                    // Read the data from the YAML nodes
                    /*
                    Vector3 pos = transform_archetype->data["position"].as<Vector3>();
                    Vector3 rot = transform_archetype->data["rotation"].as<Vector3>();
                    Vector3 scl = transform_archetype->data["scale"].as<Vector3>();
                    Vector3 size = collider_archetype->data["size"].as<Vector3>();
                    
                    // Construct the model matrix (same as gizmo logic)
                    glm::mat4 model_matrix;
                    ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(pos.to_glm()), glm::value_ptr(rot.to_glm()), glm::value_ptr(scl.to_glm()), glm::value_ptr(model_matrix));
                    */
                    Vector3 size = collider_archetype->data["size"].as<Vector3>();
                    glm::mat4 model_matrix = get_world_matrix(&archetype);
                    glm::vec3 half_extents = size.to_glm() * 0.5f;
                    float distance = 0.0f;
                    if (Raycast::IntersectsOBB(world_ray, model_matrix, half_extents, distance)) {
                        if (distance < closest_hit_distance) {
                            closest_hit_distance = distance;
                            closest_hit_id = archetype.id;
                        }
                    }
                }
            }
        
            // --- Step 3: Fire the selection event (This is now the same for both modes) ---
            if (context->event_manager) {

                // Update the context with the new selection (or deselection if invalid)
                context->selected_entity_id = closest_hit_id;
                context->selected_entity = context->preview_scene->get_entity_by_id(context->selected_entity_id);
                EntitySelectedEvent event(closest_hit_id, context->selected_entity);
                // Dispatch the event. In YAML mode, the pointer will correctly be nullptr.
                context->event_manager->dispatch(event);
            }
            // 2. Check for a double-click on the entity that was just selected.
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && closest_hit_id.is_valid()) {
                Entity* entity_to_focus = context->preview_scene->get_entity_by_id(closest_hit_id);
                if (entity_to_focus) {
                    Transform* transform = entity_to_focus->get_transform();
                    if (transform) {
                        // Call the focus_on method, just like in the WorldTreePanel.
                        context->editor_camera->focus_on(transform, 3.0f);
                    }
                }
            }
        }
    }



    void RealmDesignerPanel::on_gui_render() {
        
    }


    void RealmDesignerPanel::on_render() {
        if (!pimpl->is_visible || !pimpl->context) {
            return;
        }
        if (pimpl->context->data_mode == EditorDataMode::Live) {
            if (!pimpl->context->active_scene) return;
        }
        else if (pimpl->context->data_mode == EditorDataMode::Yaml) {
            assert(pimpl->context->preview_scene != nullptr && "Cannot render, preview_scene is nullptr");
            
        }
        
        // 1. Prepare the render pass (state management)
        pimpl->context->renderer->begin_render_pass(pimpl->framebuffer_id);
        pimpl->context->renderer->set_viewport(0, 0, (int)pimpl->viewport_size.x, (int)pimpl->viewport_size.y);
        pimpl->context->renderer->set_active_camera(pimpl->context->editor_camera);       
        pimpl->context->renderer->clear();

         
        pimpl->draw_test_cube(); // draw 3D first.
        pimpl->draw_scene(); // draw 2D (in game gui/HUD elements).
        pimpl->draw_bounding_boxes();
        pimpl->context->renderer->draw_line(
            pimpl->last_picking_ray.origin,
            pimpl->last_picking_ray.origin + pimpl->last_picking_ray.direction * 1000.0f, // A long line
            {1.0f, 1.0f, 0.0f, 1.0f} // Yellow
    );
        pimpl->draw_grid();
        pimpl->context->renderer->end_render_pass();
    }

    GLint RealmDesignerPanel::Pimpl::render_pass_begin() {
        OpenGLRenderer* renderer = context->renderer->as_opengl_renderer();
        if (!renderer) return 0;

        // --- 1. Prepare the Render Pass (The Boilerplate) ---
        GLint last_bound_fbo = renderer->get_current_framebuffer_binding();
        renderer->bind_framebuffer(framebuffer_id);
        renderer->set_viewport(0, 0, (int)viewport_size.x, (int)viewport_size.y);
        renderer->set_active_camera(context->editor_camera);

        // --- 2. Clear the canvas ONCE ---
        renderer->clear();

        return last_bound_fbo;
    }

    void RealmDesignerPanel::Pimpl::render_pass_end(GLint last_fbo) {
        OpenGLRenderer* renderer = context->renderer->as_opengl_renderer();
        if (!renderer) return;

        // --- 4. Restore the Previous State (The Boilerplate) ---
        renderer->restore_framebuffer_binding(last_fbo);
    }


    void RealmDesignerPanel::Pimpl::draw_scene() {
        Scene* active_scene = nullptr;
        if (context->data_mode == EditorDataMode::Yaml) { 
            active_scene = context->preview_scene.get();
        }
        

        IRenderer* renderer = context->renderer;
        if (!renderer || !active_scene) return;

        // --- STEP 1: COLLECT all visible sprites into a render queue ---
        std::vector<RenderJob> render_queue;
        for (Entity* entity : active_scene->get_entities()) {
            if (!entity || entity->is_purged() || !entity->is_visible()) continue;
            
            Transform* transform = entity->get_transform();
            if (!transform) continue;

            std::vector<Element*> sprites = entity->get_elements_by_type_name("Sprite2D");
            for (auto* element : sprites) {
                Sprite2D* sprite = dynamic_cast<Sprite2D*>(element);
                if (sprite && sprite->is_visible() && sprite->get_texture()) {
                    render_queue.push_back({sprite, transform, sprite->get_sorting_layer()});
                }
            }
        }

        // --- STEP 2: SORT the render queue by sorting_layer ---
        std::sort(render_queue.begin(), render_queue.end(), [](const RenderJob& a, const RenderJob& b) {
            return a.sorting_layer < b.sorting_layer;
        });

        // --- STEP 3: RENDER the sorted queue ---
        for (const auto& job : render_queue) {
            const Sprite2D* sprite = job.sprite;
            const Transform* transform = job.transform;

            // Get the entity's base world matrix
            glm::mat4 entity_model_matrix = transform->get_model_matrix();

            if (sprite->use_entity_rotation) {
                // If TRUE, use the entity's full world matrix, including its rotation and scale.
                entity_model_matrix = transform->get_model_matrix();
            } else {
                // If FALSE, create a matrix from ONLY the entity's world position.
                // This makes the sprite a "billboard" that ignores the parent's rotation.
                entity_model_matrix = glm::translate(glm::mat4(1.0f), transform->get_world_position().to_glm());
            }

            // Get sprite's local properties
            Vector2 offset = sprite->offset;
            Vector2 pivot = sprite->pivot;
            const float PIXELS_PER_UNIT = renderer->get_pixels_per_unit();
            float world_width = (float)sprite->get_texture_width() / PIXELS_PER_UNIT;
            float world_height = (float)sprite->get_texture_height() / PIXELS_PER_UNIT;

            // Apply flip logic to the scale
            float scale_x = world_width;
            float scale_y = world_height;
            if (sprite->flip_h) scale_x *= -1.0f;
            if (sprite->flip_v) scale_y *= -1.0f;

            // Build the sprite's complete local transformation matrix
            // The order is important: Pivot Correction -> Scale -> Final Offset
            glm::mat4 local_sprite_matrix = glm::mat4(1.0f);
            local_sprite_matrix = glm::translate(local_sprite_matrix, glm::vec3(offset.x, offset.y, 0.0f));
            local_sprite_matrix = glm::scale(local_sprite_matrix, glm::vec3(scale_x, scale_y, 1.0f));
            // The pivot correction shifts the quad so the pivot point is at the origin before scaling
            local_sprite_matrix = local_sprite_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f - pivot.x, 0.5f - pivot.y, 0.0f));

            // Combine with the entity's world matrix to get the final matrix for this sprite
            glm::mat4 final_model_matrix = entity_model_matrix * local_sprite_matrix;
            
            // Make the clean draw call with the FINAL calculated matrix
            renderer->draw_sprite(
                sprite->get_texture(),
                final_model_matrix,
                sprite->color
            );
        }
    }
 

    void RealmDesignerPanel::on_event(IEvent& event) {
    if (event.get_event_type() != EventType::EditorPropertyValueChanged) {
        return;
    }
    if (pimpl->context->data_mode != EditorDataMode::Yaml || !pimpl->context->preview_scene) {
        return;
    }

    PropertyValueChangedEvent& e = static_cast<PropertyValueChangedEvent&>(event);

    // --- SANITY CHECK: Print every property change event received ---
    std::cout << "\n--- Received PropertyValueChangedEvent ---" << std::endl;
    std::cout << "  Entity ID: " << e.entity_id.get_value() << ", Element ID: " << e.element_id.get_value() << std::endl;
    std::cout << "  Property: '" << e.property_name << "', New Value: " << e.new_value << std::endl;

    Entity* entity_to_update = pimpl->context->preview_scene->get_entity_by_id(e.entity_id);
    if (!entity_to_update) {
        std::cout << "  [FAIL] Sanity Check: Live entity NOT found in preview_scene." << std::endl;
        return;
    }

    Element* element_to_update = nullptr;
    for (auto* element : entity_to_update->get_all_elements()) {
        if (element->get_id() == e.element_id) {
            element_to_update = element;
            break;
        }
    }

    if (!element_to_update) {
        std::cout << "  [FAIL] Sanity Check: Live element NOT found on entity." << std::endl;
        return;
    }

    // Attempt to cast to Transform to check if this is the component we're interested in
    Transform* live_transform = dynamic_cast<Transform*>(element_to_update);
    if (!live_transform) {
        std::cout << "  [INFO] Sanity Check: Event was for a non-Transform element (" << element_to_update->get_class_name() << "). Skipping position check." << std::endl;
    } else {
        std::cout << "  ✅ [BEFORE]: Live Transform Position: " << live_transform->get_position() << std::endl;
    }

    // --- This is your original, correct update logic ---
    const TypeInfo* type_info = ByteMirror::get_type_info(typeid(*element_to_update));
    if (!type_info) return;

    std::optional<Property> found_property;
    for (const auto& prop : ByteMirror::get_all_properties_for_type(type_info)) {
        if (prop.name == e.property_name) {
            found_property = prop;
            break;
        }
    }

    if (!found_property.has_value()) return;

    std::visit([&](auto&& arg) {
        auto value_copy = arg;
        found_property->set_data(element_to_update, &value_copy);
    }, e.new_value);
    
    // --- Final check after update ---
    if (live_transform) {
        std::cout << "  ✅  [AFTER]: Live Transform Position: " << live_transform->get_position() << std::endl;
    }
        
        element_to_update->on_load(*pimpl->context->init_context);

        // ---Write derived data (width/height) back to the archetype ---
        if (auto* live_sprite = dynamic_cast<Sprite2D*>(element_to_update)) {
            auto entity_it = std::find_if(pimpl->context->current_realm.begin(), pimpl->context->current_realm.end(),
                [&](EntityArchetype& archetype) { return archetype.id == e.entity_id; });

            if (entity_it != pimpl->context->current_realm.end()) {
                auto element_it = std::find_if(entity_it->elements.begin(), entity_it->elements.end(),
                    [&](ElementArchetype& element) { return element.id == e.element_id; });

                if (element_it != entity_it->elements.end()) {
                    element_it->data["width"] = live_sprite->get_texture_width();
                    element_it->data["height"] = live_sprite->get_texture_height();
                }
            }
        }
    }
    void RealmDesignerPanel::set_visibility(bool visibility) {
         pimpl->is_visible = visibility; 
    }


    bool RealmDesignerPanel::get_visibility() const {
        return pimpl->is_visible; 
    }


    void RealmDesignerPanel::set_name(const std::string& new_name) { pimpl->name = new_name; }


    const std::string& RealmDesignerPanel::get_name() { return pimpl->name; }


    uint32_t RealmDesignerPanel::get_framebuffer_id() const {
        
        return pimpl->framebuffer_id;
    }


    ImVec2 RealmDesignerPanel::get_viewport_size() const {
        return pimpl->viewport_size;
    }

    void RealmDesignerPanel::Pimpl::draw_test_cube() {
        OpenGLRenderer* cube_renderer = context->renderer->as_opengl_renderer();
        if (!cube_renderer) return;

        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        Color cube_color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        cube_renderer->draw_cube(model_matrix, cube_color);
    }

    void RealmDesignerPanel::Pimpl::draw_test_cube_only() {
        // Designed to be used inside an 'on_render' method with no explict framebuffering setup.
        // This will not work, if framebuffering and render calls are inside the 'on_render' method.
        if (!context) {
            std::cerr << "[RealmDesignerPanel] context is nullptr!\n";
        return;
        }

        if (!context->renderer) {
            std::cerr << "[RealmDesignerPanel] context->renderer is nullptr!\n";
            return;
        }
     
        IRenderer* base_renderer = context->renderer;
        if (!base_renderer) { /* error */ return; }
        OpenGLRenderer* cube_renderer = base_renderer->as_opengl_renderer();
        if (!cube_renderer) { /* error */ return; }

        int fbo_width = static_cast<int>(viewport_size.x);
        int fbo_height = static_cast<int>(viewport_size.y);
        if (fbo_width <= 0 || fbo_height <= 0) {
            fbo_width = 1280;
            fbo_height = 720;
            std::cerr << "[RealmDesignerPanel] on_render: Using fallback FBO dimensions for test.\n";
        }
        if (framebuffer_id == 0) { /* error */ return; }

        // Save the currently bound framebuffer
        GLint last_bound_fbo = cube_renderer->get_current_framebuffer_binding();

        cube_renderer->bind_framebuffer(framebuffer_id);
        cube_renderer->set_viewport(0, 0, fbo_width, fbo_height);
        cube_renderer->set_clear_color({1.0f, 0.0f, 0.0f, 1.0f}); // Make FBO 1 RED!
        cube_renderer->clear();

        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        Color cube_color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        cube_renderer->draw_cube(model_matrix, cube_color);

        // Restore the previous framebuffer binding (using the new method!)
        cube_renderer->restore_framebuffer_binding(last_bound_fbo); 

        cube_renderer->set_clear_color({0.1f, 0.1f, 0.2f, 1.0f}); 
    }

    bool RealmDesignerPanel::is_locked() {
        return pimpl->is_locked;
    }


    void RealmDesignerPanel::unlock() {
        pimpl->is_locked = false;
    }


    void RealmDesignerPanel::lock() {
        pimpl->is_locked = true;
    }   

    void RealmDesignerPanel::Pimpl::draw_bounding_boxes() {
        OpenGLRenderer* renderer = context->renderer->as_opengl_renderer();
        if (!renderer) return;
        // Define a color for the bounding boxes
        Color box_color = {0.0f, 1.0f, 0.0f, 1.0f}; // Green
        Scene* active_scene = nullptr;
        
        if (context->data_mode == EditorDataMode::Yaml) {
            active_scene = context->preview_scene.get();
        }

        for (Entity* entity : active_scene->get_entities()) {
            if (!entity || entity->is_purged()) continue;
            Transform* transform = entity->get_transform(); 
            BoxCollider* collider = entity->get_element<BoxCollider>(); 

            // We only draw boxes for entities that have a transform and a collider
            if (transform && collider && collider->is_visible()) { 
                // Get the base model matrix from the entity's transform
                glm::mat4 model_matrix = transform->get_model_matrix();

                // Get the size from the collider and scale the model matrix
                glm::vec3 collider_size = collider->get_size().to_glm();
                model_matrix = glm::scale(model_matrix, collider_size);

                // Draw the wireframe box
                renderer->draw_wire_box(model_matrix, box_color);
            }
        }
    }





    void RealmDesignerPanel::Pimpl::draw_grid() {
        if (!context || !context->grid_settings.snap_enabled) 
            return;

        IRenderer* renderer = context->renderer;
        const auto& grid = context->grid_settings;
        
        // Get camera information
        Vector3 camera_pos = context->editor_camera->get_transform()->get_position();
        float visible_range = context->editor_camera->get_far_plane(); // Assuming this is now available
        
        // Calculate grid bounds based on camera position and visible range
        float min_x = std::floor((camera_pos.x - visible_range) / grid.major_division) * grid.major_division;
        float max_x = std::ceil((camera_pos.x + visible_range) / grid.major_division) * grid.major_division;
        float min_z = std::floor((camera_pos.z - visible_range) / grid.major_division) * grid.major_division;
        float max_z = std::ceil((camera_pos.z + visible_range) / grid.major_division) * grid.major_division;
        
        // Clamp to grid size limits
        min_x = std::max(min_x, -grid.size);
        max_x = std::min(max_x, grid.size);
        min_z = std::max(min_z, -grid.size);
        max_z = std::min(max_z, grid.size);
        
        // Draw grid lines
        for (float x = min_x; x <= max_x; x += grid.major_division) {
            renderer->draw_line(
                {x, 0, min_z}, 
                {x, 0, max_z},
                grid.color
            );
        }
        
        for (float z = min_z; z <= max_z; z += grid.major_division) {
            renderer->draw_line(
                {min_x, 0, z}, 
                {max_x, 0, z},
                grid.color
            );
        }
        
        // Draw minor grid lines if enabled
        if (grid.minor_division > 0 && grid.minor_division < grid.major_division) {
            Color minor_color = grid.color * 0.7f; // Slightly dimmer color for minor lines
            
            for (float x = min_x; x <= max_x; x += grid.minor_division) {
                // Skip major division lines
                if (std::fmod(x, grid.major_division) < 0.001f) continue;
                
                renderer->draw_line(
                    {x, 0, min_z}, 
                    {x, 0, max_z},
                    minor_color
                );
            }
            
            for (float z = min_z; z <= max_z; z += grid.minor_division) {
                // Skip major division lines
                if (std::fmod(z, grid.major_division) < 0.001f) continue;
                
                renderer->draw_line(
                    {min_x, 0, z}, 
                    {max_x, 0, z},
                    minor_color
                );
            }
        }
    }
}  // namespace Salix