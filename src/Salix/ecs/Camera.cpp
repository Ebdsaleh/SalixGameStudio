// Salix/ecs/Camera.cpp
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <cassert>

namespace Salix {

    struct Camera::Pimpl {
        ProjectionMode projection_mode = ProjectionMode::Perspective;

        // Is this set as the active camera?.
        bool is_active = false;
        // Matrices
        glm::mat4 view_matrix{1.0f};
        glm::mat4 projection_matrix{1.0f};

        // Viewport dimensions
        float aspect_ratio = 16.0f / 9.0f;

        // Perspective properties
        float field_of_view = 45.0f;

        // Orthographic properties
        float orthographic_size = 10.0f;

        // Shared properties
        float near_clip = 0.1f;
        float far_clip = 1000.0f;

        // Dirty flags to avoid recalculating matrices every frame
        bool view_dirty = true;
        bool projection_dirty = true;
        bool is_2D_project = false;
     };


     
    Camera::Camera() : pimpl(std::make_unique<Pimpl>()) { 
        set_name(get_class_name()); 
        
    }
    
    Camera::~Camera() = default;


    void Camera::initialize() {
        // When initialized, mark matrices as dirty to force a recalculation on the first frame.
        pimpl->view_dirty = true;
        pimpl->projection_dirty = true;
    }

    void Camera::update(float /*delta_time*/) {
        if (!pimpl->is_active) return;
        // In a real engine, you'd check if the owner's transform has changed.
        // For now, we'll just assume it might have and mark the view matrix as dirty.
        pimpl->view_dirty = true;
    }

    void Camera::set_projection_mode(ProjectionMode mode) {
        pimpl->projection_mode = mode;
        pimpl->projection_dirty = true;
    }

    void Camera::set_is_active(bool active) { pimpl->is_active = active; }

    bool Camera::get_is_active() { return pimpl->is_active; }

    void Camera::activate() {
        if (pimpl->is_active) return;
        set_is_active(true);
    }

    void Camera::deactivate() {
        if (!pimpl->is_active) return;
        set_is_active(false);
    }

    const ProjectionMode& Camera::get_projection_mode() const {
        std::cout << "[Camera::Getter] READING value: " << static_cast<int>(pimpl->projection_mode) << std::endl;
        return pimpl->projection_mode;
    }

    void Camera::set_viewport_size(int width, int height) {
        if (height == 0) return;
        pimpl->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        pimpl->projection_dirty = true;
    }

    const glm::mat4& Camera::get_view_matrix() {
        // Get the owner entity's transform component.
        if (owner) {
            Transform* transform = owner->get_transform();
            if (transform) {
                // Recalculate the view matrix based on the transform's current state.
                glm::vec3 position = transform->get_position().to_glm();
                glm::vec3 front    = transform->get_forward();
                glm::vec3 up       = transform->get_up();

                pimpl->view_matrix = glm::lookAt(position, position + front, up);
            }
        }
        return pimpl->view_matrix;
    }

    const glm::mat4& Camera::get_projection_matrix() {
        if (pimpl->projection_dirty) {
            recalculate_projection_matrix();
            pimpl->projection_dirty = false;
        }
        return pimpl->projection_matrix;
    }

    void Camera::recalculate_view_matrix() {
        // Get the transform from the entity that owns this camera
        Transform* transform = owner->get_transform();
        if (!transform) {
            pimpl->view_matrix = glm::mat4(1.0f); // Default to identity if no transform
            return;
        }

        glm::vec3 position = transform->get_position().to_glm();  // need to cast from a Vector3
        glm::vec3 front = transform->get_forward();
        glm::vec3 up = transform->get_up();

        pimpl->view_matrix = glm::lookAt(position, position + front, up);
    }

    void Camera::recalculate_projection_matrix() {
        if (pimpl->projection_mode == ProjectionMode::Perspective) {
            pimpl->projection_matrix = glm::perspective(
                glm::radians(pimpl->field_of_view),
                pimpl->aspect_ratio,
                pimpl->near_clip,
                pimpl->far_clip
            );
        } else { // Orthographic
            float ortho_left = -pimpl->orthographic_size * pimpl->aspect_ratio * 0.5f;
            float ortho_right = pimpl->orthographic_size * pimpl->aspect_ratio * 0.5f;
            float ortho_bottom = -pimpl->orthographic_size * 0.5f;
            float ortho_top = pimpl->orthographic_size * 0.5f;

            pimpl->projection_matrix = glm::ortho(
                ortho_left, ortho_right,
                ortho_bottom, ortho_top,
                pimpl->near_clip,
                pimpl->far_clip
            );
        }
    }

    void Camera::set_2D_mode(bool is_2d) {
        pimpl->is_2D_project = is_2d;
    }


    Transform* Camera::get_transform() {
        return owner->get_transform();
    }
    // --- Property Getters/Setters ---
    void Camera::set_field_of_view(float fov) { pimpl->field_of_view = fov; pimpl->projection_dirty = true; }
    float& Camera::get_field_of_view() const { return pimpl->field_of_view; }
    void Camera::set_orthographic_size(float size) { pimpl->orthographic_size = size; pimpl->projection_dirty = true; }
    float& Camera::get_orthographic_size() const { return pimpl->orthographic_size; }
    void Camera::set_near_clip(float near_clip) { pimpl->near_clip = near_clip; pimpl->projection_dirty = true; }
    float& Camera::get_near_clip() const { return pimpl->near_clip; }
    void Camera::set_far_clip(float far_clip) { pimpl->far_clip = far_clip; pimpl->projection_dirty = true; }
    float& Camera::get_far_clip() const { return pimpl->far_clip; }

}