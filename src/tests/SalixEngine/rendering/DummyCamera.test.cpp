// Tests/SalixEngine/rendering/DummyCamera.test.cpp
#include <doctest.h>
#include <Salix/rendering/DummyCamera.h>
#include <glm/glm.hpp>
#include <memory>


TEST_SUITE("Salix::rendering::DummyCamera") {
   
        TEST_CASE("Default Behavior") {
            std::unique_ptr<Salix::DummyCamera> dummy_camera;
            dummy_camera = std::make_unique<Salix::DummyCamera>();

            // A default constructed glm::mat4 is the view matrix.
            glm::mat4 view_matrix{1.0f};
            // Check that the view matrix returns the identity matrix by default.
            CHECK(dummy_camera->get_view_matrix() == view_matrix);

            // Check that the projection matrix returns the identity matrix by default.
            CHECK(dummy_camera->get_projection_matrix() == view_matrix);
            
            // Ensure that calling setters does not change anything.
            dummy_camera->set_projection_mode(Salix::ProjectionMode::Orthographic);
            CHECK(dummy_camera->get_projection_mode() == Salix::ProjectionMode::Perspective);
            
            // Check that the orthographic size is set to the default value of 10.0f;
            CHECK(dummy_camera->get_orthographic_size() == 10.0f);

            // The default return value for a non-implemented virtual method should be predictable.
            CHECK(dummy_camera->get_is_active() == false);
        }
        TEST_CASE("Toggle Activation") {
            std::unique_ptr<Salix::DummyCamera> dummy_camera;
            dummy_camera = std::make_unique<Salix::DummyCamera>();
            // Try setting as active.
            dummy_camera->set_is_active(true);
            CHECK(dummy_camera->get_is_active() == true);
            // Try de-activating the DummyCamera.
            dummy_camera->deactivate();
            CHECK(dummy_camera->get_is_active() == false);

            dummy_camera->activate();
            CHECK(dummy_camera->get_is_active() == true);
        }
        TEST_CASE("Set Orthographic Size") {
            std::unique_ptr<Salix::DummyCamera> dummy_camera;
            dummy_camera = std::make_unique<Salix::DummyCamera>();

            float new_size = 12.0f;
            dummy_camera->set_orthographic_size(new_size);
            CHECK(dummy_camera->get_orthographic_size() == 12.0f);
        }
}