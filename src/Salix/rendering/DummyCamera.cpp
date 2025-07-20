// Salix/rendering/DummyCamera.cpp
#include <Salix/rendering/DummyCamera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>


namespace Salix {
    struct DummyCamera::Pimpl{
       glm::mat4 view_matrix = 1.0f;
       glm::mat4 projection_matrix = 1.0f;

    };
    
    DummyCamera::DummyCamera() : pimpl(std::make_unique<Pimpl>()) {}
    DummyCamera::~DummyCamera() = default;

    const glm::mat4& DummyCamera::get_view_matrix() {
        return pimpl->view_matrix;
    }

    const glm::mat4& DummyCamera::get_projection_matrix() {
        return pimpl->projection_matrix;
    }
}