// Salix/rendering/DummyCamera/h
#pragma once
#include <Salix/rendering/ICamera.h>
#include <glm/glm.hpp>
#include <memory>

namespace Salix {

    class DummyCamera : public ICamera {
    public:
        DummyCamera();
        ~DummyCamera() override;
        const glm::mat4& get_view_matrix() override;
        const glm::mat4& get_projection_matrix() override;
    
    private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
    };
}   // namespace Salix