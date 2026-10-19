// =================================================================================
// Filename:    src/Tests/SalixEngine/rendering/DummyCamera.test.cpp
// Description: Contains unit tests for the DummyCamera functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/rendering/DummyCamera.h>
#include <glm/glm.hpp>
#include <memory>


TEST_SUITE("Salix::rendering::DummyCamera") {
   
        TEST_CASE("ensure default behaviour") {
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
        TEST_CASE("toggle activation") {
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
        TEST_CASE("correctly set the orthographic size") {
            std::unique_ptr<Salix::DummyCamera> dummy_camera;
            dummy_camera = std::make_unique<Salix::DummyCamera>();

            float new_size = 12.0f;
            dummy_camera->set_orthographic_size(new_size);
            CHECK(dummy_camera->get_orthographic_size() == 12.0f);
        }
}

// Verify stable null-renderer camera contracts through direct and interface dispatch.
TEST_SUITE("Salix::rendering::DummyCameraExtended") {
    TEST_CASE("dummy camera projection remains perspective after repeated mode changes") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        camera.set_projection_mode(Salix::ProjectionMode::Perspective);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("dummy camera viewport-independent view remains identity when camera is activated") {
        Salix::DummyCamera camera;
        camera.activate();
        CHECK(camera.get_is_active());
        CHECK(camera.get_view_matrix() == glm::mat4(1.f));
    }

    TEST_CASE("dummy camera viewport-independent projection remains identity when deactivated") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
        CHECK(camera.get_projection_matrix() == glm::mat4(1.f));
    }

    TEST_CASE("dummy camera setting active state false is idempotent") {
        Salix::DummyCamera camera;
        camera.set_is_active(false);
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("dummy camera calling activate twice remains active until explicit deactivation") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.activate();
        CHECK(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("dummy camera orthographic size can be changed independently of activation") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(32.f);
        camera.activate();
        CHECK(camera.get_orthographic_size() == 32.f);
        camera.deactivate();
        CHECK(camera.get_orthographic_size() == 32.f);
    }

    TEST_CASE("dummy camera size setter retains signed values without modifying matrices") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-6.f);
        CHECK(camera.get_orthographic_size() == -6.f);
        CHECK(camera.get_view_matrix() == glm::mat4(1.f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.f));
    }

    TEST_CASE("dummy camera view and projection references have independent stable storage") {
        Salix::DummyCamera camera;
        const glm::mat4* view = &camera.get_view_matrix();
        const glm::mat4* projection = &camera.get_projection_matrix();
        REQUIRE(view != nullptr);
        REQUIRE(projection != nullptr);
        CHECK(view != projection);
        CHECK(view == &camera.get_view_matrix());
        CHECK(projection == &camera.get_projection_matrix());
    }

    TEST_CASE("dummy camera separate cameras maintain isolated activation and orthographic settings") {
        Salix::DummyCamera a, b;
        a.set_orthographic_size(5.f);
        b.set_orthographic_size(20.f);
        a.activate();
        CHECK(a.get_is_active());
        CHECK_FALSE(b.get_is_active());
        CHECK(a.get_orthographic_size() == 5.f);
        CHECK(b.get_orthographic_size() == 20.f);
    }

    TEST_CASE("dummy camera camera interface virtual dispatch preserves dummy camera guarantees") {
        Salix::DummyCamera camera;
        Salix::ICamera* interface = &camera;
        interface->set_projection_mode(Salix::ProjectionMode::Orthographic);
        interface->set_orthographic_size(7.75f);
        interface->activate();
        CHECK(interface->get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(interface->get_view_matrix() == glm::mat4(1.f));
        CHECK(interface->get_projection_matrix() == glm::mat4(1.f));
        CHECK(interface->get_is_active());
        CHECK(camera.get_orthographic_size() == 7.75f);
    }
}

// Isolated virtual-dispatch and GUI-input state behavioral contracts.
TEST_SUITE("Salix::Batch747::DummyCameraBatch747") {
    TEST_CASE("dummy camera can toggle active state repeatedly without altering projection") {
        Salix::DummyCamera c;for(int i=0;i<5;++i){c.activate();CHECK(c.get_is_active());c.deactivate();CHECK_FALSE(c.get_is_active());}CHECK(c.get_projection_mode()==Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("dummy camera set active false overrides an earlier activate call") {
        Salix::DummyCamera c;c.activate();c.set_is_active(false);CHECK_FALSE(c.get_is_active());CHECK(c.get_view_matrix()==glm::mat4(1.f));
    }

    TEST_CASE("dummy camera set active true overrides an earlier deactivate call") {
        Salix::DummyCamera c;c.deactivate();c.set_is_active(true);CHECK(c.get_is_active());CHECK(c.get_projection_matrix()==glm::mat4(1.f));
    }

    TEST_CASE("dummy camera orthographic size accepts zero without changing perspective mode") {
        Salix::DummyCamera c;c.set_orthographic_size(0.f);CHECK(c.get_orthographic_size()==0.f);CHECK(c.get_projection_mode()==Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("dummy camera last orthographic assignment wins across multiple updates") {
        Salix::DummyCamera c;c.set_orthographic_size(1.25f);c.set_orthographic_size(2.5f);c.set_orthographic_size(7.75f);CHECK(c.get_orthographic_size()==7.75f);
    }

    TEST_CASE("dummy camera instance destruction cannot reset the next instance size") {
        {Salix::DummyCamera temporary;temporary.set_orthographic_size(123.f);temporary.activate();}Salix::DummyCamera fresh;CHECK(fresh.get_orthographic_size()==10.f);CHECK_FALSE(fresh.get_is_active());
    }

    TEST_CASE("dummy camera two instances expose distinct internal view matrix storage") {
        Salix::DummyCamera a,b;CHECK(&a.get_view_matrix()!=&b.get_view_matrix());CHECK(a.get_view_matrix()==b.get_view_matrix());
    }

    TEST_CASE("dummy camera two instances expose distinct internal projection matrix storage") {
        Salix::DummyCamera a,b;CHECK(&a.get_projection_matrix()!=&b.get_projection_matrix());CHECK(a.get_projection_matrix()==b.get_projection_matrix());
    }

    TEST_CASE("dummy camera mode cannot be changed through its camera interface") {
        Salix::DummyCamera concrete;Salix::ICamera& c=concrete;c.set_projection_mode(Salix::ProjectionMode::Orthographic);CHECK(c.get_projection_mode()==Salix::ProjectionMode::Perspective);c.set_projection_mode(Salix::ProjectionMode::Perspective);CHECK(c.get_projection_mode()==Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("dummy camera orthographic and active settings are independent across camera interface dispatch") {
        Salix::DummyCamera concrete;Salix::ICamera& c=concrete;c.set_orthographic_size(0.25f);c.set_is_active(true);CHECK(concrete.get_orthographic_size()==0.25f);CHECK(c.get_is_active());c.deactivate();CHECK_FALSE(c.get_is_active());CHECK(concrete.get_orthographic_size()==0.25f);
    }
}
