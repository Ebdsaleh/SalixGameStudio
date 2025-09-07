// Salix/rendering/DummyCamera/h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/rendering/ICamera.h>
#include <glm/glm.hpp>
#include <memory>

namespace Salix {

    class SALIX_API DummyCamera : public ICamera {
    public:
        DummyCamera();
        ~DummyCamera() override;
        const glm::mat4& get_view_matrix() override;
        const glm::mat4& get_projection_matrix() override;
        void set_projection_mode(ProjectionMode mode) override { (void) mode;}
        const ProjectionMode& get_projection_mode() const override {
             static const ProjectionMode mode = ProjectionMode::Perspective;
            return mode;
        }
        void set_orthographic_size(float size) override;
        float get_orthographic_size() const;
        void set_is_active(bool is_active) override;
        bool get_is_active() override;
        void activate()override;
        void deactivate() override;
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}   // namespace Salix