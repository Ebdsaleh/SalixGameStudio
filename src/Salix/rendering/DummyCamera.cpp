// Salix/rendering/DummyCamera.cpp
#include <Salix/rendering/DummyCamera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>


namespace Salix {
    struct DummyCamera::Pimpl{
        glm::mat4 view_matrix = 1.0f;
        glm::mat4 projection_matrix = 1.0f;
        float orthographic_size = 10.0f;
        bool is_active = false;    
    };
    
    DummyCamera::DummyCamera() : pimpl(std::make_unique<Pimpl>()) {}
    DummyCamera::~DummyCamera() = default;

    const glm::mat4& DummyCamera::get_view_matrix() {
        return pimpl->view_matrix;
    }

    const glm::mat4& DummyCamera::get_projection_matrix() {
        return pimpl->projection_matrix;
    }

    float DummyCamera::get_orthographic_size() const { return pimpl->orthographic_size; }
    void DummyCamera::set_orthographic_size(float size) { pimpl->orthographic_size = size; }
    void DummyCamera::set_is_active(bool is_active) { pimpl->is_active = is_active; }
    bool DummyCamera::get_is_active() {return pimpl->is_active;}

    void DummyCamera::activate() { set_is_active(true); }
    void DummyCamera::deactivate() { set_is_active(false); }
}