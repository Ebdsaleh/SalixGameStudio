// Tests/SalixEngine/mocking/rendering/MockICamera.h
#pragma once
#include <doctest.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <memory>


class MockICamera : public Salix::ICamera {
    public:
        MockICamera() {
        // Correctly initialize the unique_ptr in the constructor.
        // This ensures the 'transform' object exists and is managed correctly.
        transform = std::make_unique<Salix::Transform>();
        }

        const glm::mat4& get_view_matrix() override { return view_matrix; }
        const glm::mat4& get_projection_matrix() override { return projection_matrix; }
        const Salix::ProjectionMode& get_projection_mode() const override { return projection_mode; }
        void set_projection_mode(Salix::ProjectionMode mode) override {}
        void set_orthographic_size(float size) override { orthographic_size = size; }
        void set_2D_mode(bool is_2d) override { is_2d_mode  = is_2d;}
        Salix::Transform* get_transform() override { return transform.get(); }
        void set_is_active(bool is_active) override { active = is_active; }
        bool get_is_active() {return active; }
        void activate() override { active = true;}
        void deactivate() override { active = false;}
    private:
        bool active = false;
        bool is_2d_mode = false;
        glm::mat4 view_matrix = {1.0f};
        float orthographic_size = 10.f;
        glm::mat4 projection_matrix = {1.0f};
        Salix::ProjectionMode projection_mode = Salix::ProjectionMode::Perspective;
        std::unique_ptr<Salix::Transform> transform;
    };