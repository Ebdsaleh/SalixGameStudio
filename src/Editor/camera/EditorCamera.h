// Editor/camera/EditorCamera.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/EditorContext.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/events/IEventListener.h>
#include <Salix/math/Vector3.h>
#include <glm/glm.hpp>
#include <memory>

namespace Salix {
    struct EditorContext;
    class Transform;

    class EDITOR_API EditorCamera : public ICamera, public IEventListener {
    public:
        EditorCamera();
        ~EditorCamera() override;
        void initialize(EditorContext* context);
        // The main update loop for handling input
        void on_update(float delta_time);

        float get_far_plane() const;
        void set_far_plane(float far_plane);
        float get_near_plane() const;
        void set_near_plane(float near_plane);
        const glm::mat4& get_view_matrix() override;
        const glm::mat4& get_projection_matrix() override;
        void set_projection_mode(ProjectionMode mode) override;
        const ProjectionMode& get_projection_mode() const override;
        void on_event(IEvent& event) override;
        void set_orthographic_size(float size) override;
        void set_viewport_size(int width, int height);
        void set_mouse_inside_scene(bool is_inside);
        void set_2D_mode(bool is_2d) override;
        Transform* get_transform() override;
        void focus_on(glm::vec3 focus_position);
        void focus_on(Transform* target_transform, float distance);
        void simple_focus_on(Transform* target_transform, float distance);
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}