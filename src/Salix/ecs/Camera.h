// Salix/ecs/Camera.h
#pragma once

#include <Salix/ecs/Element.h> 
#include <Salix/core/Core.h>
#include <glm/glm.hpp>


namespace Salix {

class SALIX_API Camera : public Element {
public:
    // Defines whether the camera renders with 3D perspective or flat 2D.
    enum class ProjectionMode {
        Perspective,
        Orthographic
    };

    Camera();
    ~Camera() override;

    // --- Core Camera Methods ---

    // Sets the camera's projection mode.
    void set_projection_mode(ProjectionMode mode);
    ProjectionMode get_projection_mode() const;

    // Calculates and returns the final view matrix based on the owner Entity's transform.
    const glm::mat4& get_view_matrix();

    // Calculates and returns the final projection matrix based on the current mode and properties.
    const glm::mat4& get_projection_matrix();

    // Sets the viewport dimensions, needed to calculate the aspect ratio.
    void set_viewport_size(int width, int height);

    // --- Property Getters/Setters ---

    void set_field_of_view(float fov);
    float get_field_of_view() const;

    void set_orthographic_size(float size);
    float get_orthographic_size() const;

    void set_near_clip(float near_clip);
    float get_near_clip() const;

    void set_far_clip(float far_clip);
    float get_far_clip() const;

    // --- Element Lifecycle Methods (from your base class) ---
    // We can override these if the camera needs specific logic.
    void initialize() override;
    void update(float delta_time) override;


private:
    // Recalculates the projection matrix when properties change.
    void recalculate_projection_matrix();
    void recalculate_view_matrix();
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};

} // namespace Salix