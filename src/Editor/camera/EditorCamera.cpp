// Editor/camera/EditorCamera.cpp
#include <Editor/camera/EditorCamera.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Salix {

    struct EditorCamera::Pimpl {
        bool is_initialzed = false;
        bool mouse_is_inside_scene = false;
        ImGuiInputManager* input;
        Transform transform = Transform();
        // It owns all the state and matrices itself.
        glm::mat4 view_matrix{1.0f};
        bool view_dirty = true;
        ProjectionMode projection_mode = ProjectionMode::Perspective;

        glm::mat4 projection_matrix{1.0f};
        bool projection_dirty = true;

        float fov = 60.0f;
        float aspect_ratio = 16.0f / 9.0f; // This should be updated when the viewport resizes
        float near_clip = 0.1f;
        float far_clip = 1000.0f;
        float orthographic_size = 10.0f; // Controls the "zoom" in ortho mode
        
        // --- Private Methods ---
        void recalculate_view_matrix();
        void recalculate_projection_matrix();

        // private camera movement variables and methods go here
        float velocity = 0.05f;
        float acceleration = 1.00f;
        float scroll_speed = 0.20f;
        float yaw = -90.0f; // Yaw is initialized facing -Z
        float pitch = 0.0f;

        float mouse_sensitivity = 0.1f;
        bool first_rotation_event = true; 
        bool first_panning_event = true;  
        bool is_2D_project = false;
        ImVec2 last_mouse_pos = {0, 0};
        
        void handle_movement();
        void handle_panning();
        void handle_rotation();
        void handle_zoom();

    };




    EditorCamera::EditorCamera() : pimpl(std::make_unique<Pimpl>()) {}

    EditorCamera::~EditorCamera() {
        pimpl->input = nullptr;
        pimpl->transform.shutdown();
    }

    void EditorCamera::Pimpl::recalculate_view_matrix() {
        glm::vec3 position = transform.get_position().to_glm();
        glm::vec3 front = transform.get_forward();
        glm::vec3 up = transform.get_up();
        view_matrix = glm::lookAt(position, position + front, up);
        
    }

     void EditorCamera::Pimpl::recalculate_projection_matrix() {
        if (projection_mode == ProjectionMode::Perspective) {
            projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip);
        } else { // Orthographic
            float ortho_height = orthographic_size;
            float ortho_width = ortho_height * aspect_ratio;

            projection_matrix = glm::ortho(-ortho_width * 0.5f, ortho_width * 0.5f, 
                                        -ortho_height * 0.5f, ortho_height * 0.5f, 
                                        near_clip, far_clip);
        }
        projection_dirty = false; // Clear the flag here after recalculating.
    }

    const glm::mat4& EditorCamera::get_view_matrix() {
       if (pimpl->view_dirty) {
        pimpl->recalculate_view_matrix();
        pimpl->view_dirty = false;
        }
        return pimpl->view_matrix;
    }

    
    const glm::mat4& EditorCamera::get_projection_matrix() {
        if (pimpl->projection_dirty) {
            pimpl->recalculate_projection_matrix();
            pimpl->projection_dirty = false;
        }
        return pimpl->projection_matrix;
    }




    void EditorCamera::initialize(EditorContext* context) {
        if (!context) {
            std::cerr << "EditorCamera::initialize - Failed initialization, EditorContext is NULL!" << std::endl;
            return; 
        }

        ImGuiInputManager* input_manager = dynamic_cast<ImGuiInputManager*>(context->init_context->input_manager);
        if (! input_manager) {
            std::cerr << "EditorCamera::initialize - Failed initialization, " << 
                "unable to cast input_manager to type 'ImGuiInputManager'!" << std::endl;
        }
        pimpl->input = input_manager;
        
        std::cout << "EditorCamera::initialize - Initialization complete..." << std::endl;
        pimpl->is_initialzed = true;
    }




    void EditorCamera::on_update(float delta_time) {
        if (!pimpl->is_initialzed) {return; }
        if (pimpl->mouse_is_inside_scene) {
            pimpl->handle_movement();
            pimpl->handle_panning();
            if(!pimpl->is_2D_project) {
                pimpl->handle_rotation();
            }
           
            pimpl->handle_zoom();
        }
        (void) delta_time;
        pimpl->view_dirty = true; 
       

    }


    void EditorCamera::set_projection_mode(ProjectionMode mode) {
        pimpl->projection_mode = mode;
        pimpl->projection_dirty = true;
    }

    const ProjectionMode& EditorCamera::get_projection_mode() const {
        return pimpl->projection_mode;
    }




    void EditorCamera::Pimpl::handle_movement() {

        float current_speed = velocity;
        if (input->is_held_down(Salix::MouseButton::Right)) {
            if (input->is_held_down(Salix::KeyCode::LeftShift)) {
                current_speed *= 3.0f; // Sprint
            }

            // 1. Get the camera's current direction vectors
            glm::vec3 forward = transform.get_forward();
            glm::vec3 backward = -forward;
            glm::vec3 right = transform.get_right();
            glm::vec3 left = -right;
            glm::vec3 up = transform.get_up();
            glm::vec3 down = -up;

            if (!is_2D_project) {
                // Move Forwards
                if (input->is_held_down(Salix::KeyCode::W)) {
                    transform.translate(forward * current_speed);
                    
                }

                // Move Backwards
                if (input->is_held_down(Salix::KeyCode::S)) {
                    transform.translate(backward * current_speed);
                    
                }
            } else {
                 // Move Up
                if (input->is_held_down(Salix::KeyCode::W)) {
                    transform.translate(up * current_speed);
                    
                }

                // Move Down
                if (input->is_held_down(Salix::KeyCode::S)) {
                    transform.translate(down * current_speed);
                    
                }
            }

            // Move Right
            if (input->is_held_down(Salix::KeyCode::D)) {
                transform.translate(right * current_speed);              
            }

            // Move Left
            if (input->is_held_down(Salix::KeyCode::A)) {
                transform.translate(left * current_speed);
                
            }

            // Move Up
            if (input->is_held_down(Salix::KeyCode::Space)){
                transform.translate(up * current_speed);
            }

            // Move Down
            if (input->is_held_down(Salix::KeyCode::C)) {
                transform.translate(down * current_speed);
            }
        }
        

    }

    
    void EditorCamera::Pimpl::handle_zoom() {
    if (!input) { return; }

    if (projection_mode == ProjectionMode::Perspective) {
        // Start with a base speed for zooming
        float current_zoom_speed = scroll_speed;

        // Increase speed if Shift is held down
        if (input->is_held_down(Salix::KeyCode::LeftShift)) {
            current_zoom_speed *= 5.0f; // 5x faster zoom
        }

        glm::vec3 forward = transform.get_forward();

        // Zoom in
        if (input->did_scroll(MouseScroll::Forward)) {
            transform.translate(forward * current_zoom_speed);
            view_dirty = true;
        }
        // Zoom out
        if (input->did_scroll(MouseScroll::Backward)){
            transform.translate(-forward * current_zoom_speed);
            view_dirty = true;
        }
        } else {// ORTHOGRAPHIC
            // --- ORTHOGRAPHIC ZOOM (change size) ---
            if (input->did_scroll(MouseScroll::Forward)) {
                orthographic_size *= 0.9f; // Zoom in by 10%
            }
            if (input->did_scroll(MouseScroll::Backward)) {
                orthographic_size *= 1.1f; // Zoom out by 10%
            }
            
            // Clamp the size to prevent it from becoming too small
            if (orthographic_size < 0.1f) {
                orthographic_size = 0.1f;
            }
            
            projection_dirty = true; // Mark matrix for recalculation
        }
    }


    void EditorCamera::Pimpl::handle_panning() {
        // Panning is typically done by holding the middle mouse button
        float pan_speed = velocity;

        if (input->is_held_down(Salix::MouseButton::Middle)) {
            if (input->is_held_down(Salix::KeyCode::LeftShift)) {
                pan_speed *= 3.0f; // Sprint
            }
            ImVec2 mouse_pos = ImGui::GetMousePos();
        
            // Use the same 'first_mouse_event' flag to prevent a jump on the first click
            if (first_panning_event) {
                last_mouse_pos = mouse_pos;
                first_panning_event = false;
            }

            float x_offset = mouse_pos.x - last_mouse_pos.x;
            float y_offset = mouse_pos.y - last_mouse_pos.y;
            last_mouse_pos = mouse_pos;

            // Translate the camera along its local right and up vectors
            // Note the signs to make the world feel like it's being dragged under the cursor
            transform.translate(-transform.get_right() * x_offset * pan_speed);
            transform.translate( transform.get_up()   * y_offset * pan_speed);
            view_dirty = true;
        }
        else {
            // If the middle mouse isn't held, we should also reset the flag
            // (This assumes you won't be panning and rotating at the same time)
            first_panning_event = true;
        }
    }


    void EditorCamera::Pimpl::handle_rotation() {

        if (!is_initialzed) return; 
        

        // The logic should only run when this condition is true
        if (input->is_held_down(Salix::MouseButton::Right)) {
            
            // ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            ImVec2 mouse_pos = ImGui::GetMousePos();

            if (first_rotation_event) {
                // Before we start, sync our yaw/pitch with the camera's actual current rotation.
                const Salix::Vector3& current_rot_rad = transform.get_rotation();
                pitch = glm::degrees(current_rot_rad.x);
                yaw = glm::degrees(current_rot_rad.y);
                last_mouse_pos = mouse_pos;
                first_rotation_event = false;
            }

            float x_offset = mouse_pos.x - last_mouse_pos.x;
            float y_offset = last_mouse_pos.y - mouse_pos.y;
            last_mouse_pos = mouse_pos;

            x_offset *= mouse_sensitivity;
            y_offset *= mouse_sensitivity;

            yaw -= x_offset;
            pitch += y_offset;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            // **CRITICAL: This line must be INSIDE the if-statement**
            transform.set_rotation(glm::radians(pitch), glm::radians(yaw), 0.0f);
        }
        else {
            // When the button is NOT held, we reset this flag
            first_rotation_event = true;
        } 
    }

    void EditorCamera::set_viewport_size(int width, int height) {
        if (height == 0) return; // Prevent division by zero
    
        // In your Pimpl struct, or as a direct member:
        pimpl->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        pimpl->projection_dirty = true;
    }

    void EditorCamera::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::WindowResize) {
            auto& resize_event = static_cast<WindowResizeEvent&>(event);
            set_viewport_size(resize_event.get_width(), resize_event.get_height());
        }
    }

    void EditorCamera::set_mouse_inside_scene(bool is_inside) { 
        pimpl->mouse_is_inside_scene = is_inside; 
    }

    void EditorCamera::set_orthographic_size(float size) {
        pimpl->orthographic_size = size;
        pimpl->projection_dirty = true; // Mark matrix for recalculation
    }

    void EditorCamera::set_2D_mode(bool is_2d) {
        pimpl->is_2D_project = is_2d;
    }
    
    Transform* EditorCamera::get_transform() {
        return &pimpl->transform;
    }
    
}  // namespace Salix