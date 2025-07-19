// Editor/camera/EditorCamera.cpp
#include <Editor/camera/EditorCamera.h>
#include <Salix/input/ImGuiInputManager.h>
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

        glm::mat4 projection_matrix{1.0f};
        bool projection_dirty = true;

        float fov = 60.0f;
        float aspect_ratio = 16.0f / 9.0f; // This should be updated when the viewport resizes
        float near_clip = 0.1f;
        float far_clip = 1000.0f;
        
        
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
        bool first_mouse_event = true;
        ImVec2 last_mouse_pos = {0, 0};
        
        void handle_movement();
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
        projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip);
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
        pimpl->handle_movement();
        pimpl->handle_rotation();
        pimpl->handle_zoom();
        (void) delta_time;
        pimpl->view_dirty = true; 
       

    }





    void EditorCamera::Pimpl::handle_movement() {

        float current_speed = velocity;
        if (input->is_held_down(Salix::MouseButton::Right)) {
            if (input->is_held_down(Salix::KeyCode::LeftShift)) {
                current_speed *= 3.0f; // Sprint
            }

            // 1. Get the camera's current direction vectors
            glm::vec3 forward = transform.get_forward();
            glm::vec3 right = transform.get_right();


            // Move Forwards
            if (input->is_held_down(Salix::KeyCode::W)) {
                transform.translate(forward * current_speed);
                
            }

            // Move Backwards
            if (input->is_held_down(Salix::KeyCode::S)) {
                transform.translate(-forward * current_speed);
                
            }

            // Move Right
            if (input->is_held_down(Salix::KeyCode::D)) {
                transform.translate(right * current_speed);              
            }

            // Move Left
            if (input->is_held_down(Salix::KeyCode::A)) {
                transform.translate(-right * current_speed);
                
            }
        }
        

    }

    
    void EditorCamera::Pimpl::handle_zoom() {
        if (!input) { return; }
        glm::vec3 forward = transform.get_forward();
        scroll_speed *= acceleration;
        // Zoom in
        if (input->did_scroll(MouseScroll::Forward)) {
            transform.translate(forward * scroll_speed);
        }

        if (input->did_scroll(MouseScroll::Backward)){
            transform.translate(-forward *scroll_speed);
        }

    }


    void EditorCamera::Pimpl::handle_rotation() {

        if (!is_initialzed) { return; }

        // The logic should only run when this condition is true
        if (input->is_held_down(Salix::MouseButton::Right)) {
            
            // ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            ImVec2 mouse_pos = ImGui::GetMousePos();

            if (first_mouse_event) {
                // Before we start, sync our yaw/pitch with the camera's actual current rotation.
                const Salix::Vector3& current_rot_rad = transform.get_rotation();
                pitch = glm::degrees(current_rot_rad.x);
                yaw = glm::degrees(current_rot_rad.y);
                last_mouse_pos = mouse_pos;
                first_mouse_event = false;
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
            first_mouse_event = true;
        } 
    }
}  // namespace Salix