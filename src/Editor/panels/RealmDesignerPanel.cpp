// Editor/panels/RealmDesignerPanel.cpp
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
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Salix/math/RayCasting.h>
#include <Salix/events/EventManager.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <iostream>
#include <memory>
#include <thread>
#include <SDL.h>


namespace Salix {

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
        void draw_scene();
        void draw_test_cube();
        void draw_test_cube_only();
        void draw_lock_button();
        void draw_gizmo_toolbar();
        void update_viewport(IRenderer* renderer);
        void handle_gizmos(EditorCamera* camera, Entity* selected_entity);
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
        
    }





    void RealmDesignerPanel::on_gui_update() {
        if (!pimpl->is_visible) return;
        if (pimpl->context->init_context->engine->is_running()) {        
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("Realm Designer", &pimpl->is_visible)) {
                IRenderer* renderer = pimpl->context->init_context->renderer;
                if (!renderer) {
                    ImGui::End();
                    ImGui::PopStyleVar();
                    return;
                }

                if (pimpl->context->data_mode == EditorDataMode::Live) {

                    // --- Get the selected entity via its GUID (The Data-Driven Approach) ---
                    Entity* selected_entity_live = nullptr;
                    if (pimpl->context && pimpl->context->active_scene) {
                        selected_entity_live = pimpl->context->active_scene->get_entity_by_id(pimpl->context->selected_entity_id);
                    }

                    // --- Camera Control ---
                    bool camera_can_move = false;
                    if (!pimpl->is_locked &&  // THIS IS THE CRITICAL ADDITION
                        ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) && 
                        ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow)) {
                        camera_can_move = true;
                    }
                    pimpl->context->editor_camera->set_mouse_inside_scene(camera_can_move);

                    // --- Panel Lock Button ---
                    pimpl->draw_lock_button();

                    // --- Gizmo Toolbar ---
                    pimpl->draw_gizmo_toolbar();
                    
                    // --- Viewport ---
                    pimpl->update_viewport(renderer);

                    if (pimpl->framebuffer_id != 0) {
                        ImTextureID tex_id = renderer->get_framebuffer_texture_id(pimpl->framebuffer_id);
                        if (tex_id != 0) {
                            ImGui::Image(tex_id, pimpl->viewport_size, ImVec2(0,1), ImVec2(1,0));

                            // --- Gizmo Interaction ---
                            if (!pimpl->is_locked && selected_entity_live) {
                                pimpl->handle_gizmos( 
                                    pimpl->context->editor_camera, 
                                    selected_entity_live
                                );
                            }
                            
                            // --- MOUSE PICKING ---
                            if (ImGui::IsItemHovered() && !EntitySelectedEvent::block_selection && !pimpl->is_locked) {
                                pimpl->handle_mouse_picking(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                            }
                        }
                    }
                } else if (pimpl->context->data_mode == EditorDataMode::Yaml) {

                    // TODO: Implement YAML pathway here.

                } else {
                    // Default to Live mode if something is wrong.
                    pimpl->context->data_mode = EditorDataMode::Live;
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
            
        }
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

        if (!is_locked) {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) CurrentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E)) CurrentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) CurrentGizmoOperation = ImGuizmo::SCALE;
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


    



    void RealmDesignerPanel::Pimpl::handle_gizmos(EditorCamera* camera, Entity* selected_entity) {
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






    void RealmDesignerPanel::Pimpl::handle_mouse_picking(const ImVec2& viewport_min, const ImVec2& viewport_max) {
        if (context->data_mode == EditorDataMode::Live) {
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

            // --- Step 2: Find the closest entity with a collider that was hit ---
            Entity* selected_entity = nullptr;
            float closest_hit_distance = FLT_MAX;

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
                EntitySelectedEvent event(selected_entity);
                context->selected_entity = selected_entity;
                context->event_manager->dispatch(event);  

                // New, data-driven system (Additions)
                selected_entity_id = temp_selected_entity_id;
                context->selected_entity_id = selected_entity_id;
                
            }
        } else if (context->data_mode == EditorDataMode::Yaml)
        {
             // TODO: Implement YAML pathway here.
        }
    }



    void RealmDesignerPanel::on_gui_render() {
        
    }


    void RealmDesignerPanel::on_render() {
        if (!pimpl->is_visible || !pimpl->context || !pimpl->context->active_scene) {
            return;
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
        Scene* active_scene = context->active_scene;
        IRenderer* renderer = context->renderer; // Use the interface
        if (!renderer || !active_scene) return;

        // Loop through all entities in the scene and draw them
        for (Entity* entity : active_scene->get_entities()) {
            if (!entity || entity->is_purged()) continue;
            
            Transform* transform = entity->get_element<Transform>();
            Sprite2D* sprite = entity->get_element<Sprite2D>();

            if (transform && sprite && sprite->get_texture()) {
                // 1. Determine the correct flip state from the element's data
                SpriteFlip flip_state = SpriteFlip::None;
                if (sprite->flip_h && sprite->flip_v) {
                    flip_state = SpriteFlip::Both;
                } else if (sprite->flip_h) {
                    flip_state = SpriteFlip::Horizontal;
                } else if (sprite->flip_v) {
                    flip_state = SpriteFlip::Vertical;
                }

                // 2. Make the clean draw call
                //    The old, redundant Rect creation has been removed.
                renderer->draw_sprite(
                    sprite->get_texture(),
                    transform,
                    sprite->get_color(),
                    flip_state // Use the calculated flip state
                );
            }
        }
    }

    void RealmDesignerPanel::on_event(IEvent& event) { (void)event; }


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

        for (Entity* entity : context->active_scene->get_entities()) {
            if (!entity || entity->is_purged()) continue;
            Transform* transform = entity->get_transform(); 
            BoxCollider* collider = entity->get_element<BoxCollider>(); 

            // We only draw boxes for entities that have a transform and a collider
            if (transform && collider) { 
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