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

TEST_SUITE("Salix::rendering::DummyCameraBatch2147") {
    TEST_CASE("batch2147 fresh dummy camera begins inactive with default orthographic size") {
        Salix::DummyCamera c; CHECK_FALSE(c.get_is_active()); CHECK(c.get_orthographic_size()==doctest::Approx(10.0f));
    }
    TEST_CASE("batch2147 fresh dummy camera matrices are identity") {
        Salix::DummyCamera c; CHECK(c.get_view_matrix()==glm::mat4(1.0f)); CHECK(c.get_projection_matrix()==glm::mat4(1.0f));
    }
    TEST_CASE("batch2147 setting orthographic size does not activate camera") {
        Salix::DummyCamera c; c.set_orthographic_size(42.0f); CHECK(c.get_orthographic_size()==42.0f); CHECK_FALSE(c.get_is_active());
    }
    TEST_CASE("batch2147 activation does not alter orthographic size") {
        Salix::DummyCamera c; c.set_orthographic_size(-12.5f); c.activate(); CHECK(c.get_is_active()); CHECK(c.get_orthographic_size()==-12.5f);
    }
    TEST_CASE("batch2147 deactivation does not alter matrices") {
        Salix::DummyCamera c; c.activate(); c.deactivate(); CHECK(c.get_view_matrix()==glm::mat4(1.0f)); CHECK(c.get_projection_matrix()==glm::mat4(1.0f));
    }
    TEST_CASE("batch2147 direct active setter can toggle repeatedly") {
        Salix::DummyCamera c; c.set_is_active(true); CHECK(c.get_is_active()); c.set_is_active(false); CHECK_FALSE(c.get_is_active()); c.set_is_active(true); CHECK(c.get_is_active());
    }
    TEST_CASE("batch2147 projection setter remains intentional no-op for orthographic request") {
        Salix::DummyCamera c; c.set_projection_mode(Salix::ProjectionMode::Orthographic); CHECK(c.get_projection_mode()==Salix::ProjectionMode::Perspective);
    }
    TEST_CASE("batch2147 matrix references remain stable across state mutations") {
        Salix::DummyCamera c; const auto* view=&c.get_view_matrix(); const auto* projection=&c.get_projection_matrix(); c.activate(); c.set_orthographic_size(2.0f); CHECK(view==&c.get_view_matrix()); CHECK(projection==&c.get_projection_matrix());
    }
    TEST_CASE("batch2147 two cameras retain fully independent active state") {
        Salix::DummyCamera a,b; a.activate(); CHECK(a.get_is_active()); CHECK_FALSE(b.get_is_active()); b.activate(); a.deactivate(); CHECK_FALSE(a.get_is_active()); CHECK(b.get_is_active());
    }
    TEST_CASE("batch2147 interface dispatch keeps matrices identity after all supported mutations") {
        Salix::DummyCamera concrete; Salix::ICamera& c=concrete; c.set_projection_mode(Salix::ProjectionMode::Orthographic); c.set_orthographic_size(3.0f); c.set_is_active(true); CHECK(c.get_view_matrix()==glm::mat4(1.0f)); CHECK(c.get_projection_matrix()==glm::mat4(1.0f)); CHECK(c.get_is_active());
    }
}


#include <cmath>
#include <limits>

TEST_SUITE("Salix::rendering::DummyCameraBatch3047") {
    TEST_CASE("batch3047 orthographic size preserves positive infinity exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size((std::numeric_limits<float>::infinity)());
        CHECK(std::isinf(camera.get_orthographic_size()));
        CHECK(camera.get_orthographic_size() > 0.0f);
    }

    TEST_CASE("batch3047 orthographic size preserves negative infinity exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-(std::numeric_limits<float>::infinity)());
        CHECK(std::isinf(camera.get_orthographic_size()));
        CHECK(camera.get_orthographic_size() < 0.0f);
    }

    TEST_CASE("batch3047 orthographic size preserves NaN diagnostic values") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size((std::numeric_limits<float>::quiet_NaN)());
        CHECK(std::isnan(camera.get_orthographic_size()));
    }

    TEST_CASE("batch3047 negative zero orthographic size retains its sign bit") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-0.0f);
        CHECK(camera.get_orthographic_size() == 0.0f);
        CHECK(std::signbit(camera.get_orthographic_size()));
    }

    TEST_CASE("batch3047 matrix references remain stable through repeated activation transitions") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.activate();
        camera.deactivate();
        camera.activate();
        CHECK(view == &camera.get_view_matrix());
        CHECK(projection == &camera.get_projection_matrix());
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch3047 camera interface can change size while inactive without activating camera") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.deactivate();
        camera.set_orthographic_size(3047.5f);
        CHECK_FALSE(camera.get_is_active());
        CHECK(concrete.get_orthographic_size() == doctest::Approx(3047.5f));
    }

    TEST_CASE("batch3047 camera interface can toggle active state without changing extreme size") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_orthographic_size(-9999.25f);
        camera.activate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
        CHECK(concrete.get_orthographic_size() == doctest::Approx(-9999.25f));
    }

    TEST_CASE("batch3047 identical size assignments remain isolated between separate cameras") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(12.25f);
        second.set_orthographic_size(12.25f);
        first.activate();
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
        CHECK(first.get_orthographic_size() == second.get_orthographic_size());
    }

    TEST_CASE("batch3047 projection mode remains perspective after extreme orthographic size assignments") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size((std::numeric_limits<float>::max)());
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == (std::numeric_limits<float>::max)());
    }

    TEST_CASE("batch3047 mutating size and activity never changes identity matrices") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-1234.5f);
        camera.set_is_active(true);
        camera.set_is_active(false);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }
}
TEST_SUITE("Salix::rendering::DummyCameraBatch3747") {
    TEST_CASE("batch3747 projection mode reference is process stable across instances") {
        Salix::DummyCamera first, second;
        const auto* a = &first.get_projection_mode();
        const auto* b = &second.get_projection_mode();
        CHECK(a == b);
        CHECK(*a == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch3747 invalid projection mode request remains an intentional no op") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(static_cast<Salix::ProjectionMode>(999));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch3747 maximum finite orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        const float value = (std::numeric_limits<float>::max)();
        camera.set_orthographic_size(value);
        CHECK(camera.get_orthographic_size() == value);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch3747 lowest finite orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        const float value = (std::numeric_limits<float>::lowest)();
        camera.set_orthographic_size(value);
        CHECK(camera.get_orthographic_size() == value);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch3747 finite size can recover after NaN diagnostic assignment") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(std::numeric_limits<float>::quiet_NaN());
        REQUIRE(std::isnan(camera.get_orthographic_size()));
        camera.set_orthographic_size(37.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(37.47f));
    }

    TEST_CASE("batch3747 direct active setter leaves maximum finite size unchanged") {
        Salix::DummyCamera camera;
        const float value = (std::numeric_limits<float>::max)();
        camera.set_orthographic_size(value);
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
        CHECK(camera.get_orthographic_size() == value);
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
        CHECK(camera.get_orthographic_size() == value);
    }

    TEST_CASE("batch3747 interface dispatch keeps invalid projection request as no op") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        camera.set_projection_mode(static_cast<Salix::ProjectionMode>(-1));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch3747 all identity matrix components survive extreme state mutations") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-std::numeric_limits<float>::infinity());
        camera.activate();
        const auto& view = camera.get_view_matrix();
        const auto& projection = camera.get_projection_matrix();
        const glm::mat4 identity(1.0f);
        for (int column = 0; column < 4; ++column) {
            for (int row = 0; row < 4; ++row) {
                CHECK(view[column][row] == identity[column][row]);
                CHECK(projection[column][row] == identity[column][row]);
            }
        }
    }

    TEST_CASE("batch3747 destroying one camera cannot affect another retained camera state") {
        Salix::DummyCamera survivor;
        survivor.set_orthographic_size(74.37f);
        survivor.activate();
        {
            Salix::DummyCamera temporary;
            temporary.set_orthographic_size(-3747.0f);
            temporary.deactivate();
        }
        CHECK(survivor.get_is_active());
        CHECK(survivor.get_orthographic_size() == doctest::Approx(74.37f));
    }

    TEST_CASE("batch3747 copying scalar size value between cameras never aliases later mutations") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(12.5f);
        second.set_orthographic_size(first.get_orthographic_size());
        first.set_orthographic_size(99.0f);
        CHECK(first.get_orthographic_size() == doctest::Approx(99.0f));
        CHECK(second.get_orthographic_size() == doctest::Approx(12.5f));
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch4147") {
    TEST_CASE("batch4147 fresh dummy camera combines identity matrices default size and inactive state") {
        Salix::DummyCamera camera;
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_orthographic_size() == doctest::Approx(10.0f));
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch4147 direct active setter toggles without changing orthographic size") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(14.147f);
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
        CHECK(camera.get_orthographic_size() == doctest::Approx(14.147f));
    }

    TEST_CASE("batch4147 projection mode requests remain intentional no ops") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        camera.set_projection_mode(Salix::ProjectionMode::Perspective);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch4147 matrix reference addresses remain stable across scalar mutations") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(-4.147f);
        camera.activate();
        camera.deactivate();
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
    }

    TEST_CASE("batch4147 separate cameras keep activation and size state independent") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(1.0f);
        second.set_orthographic_size(2.0f);
        first.activate();
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
        CHECK(first.get_orthographic_size() == doctest::Approx(1.0f));
        CHECK(second.get_orthographic_size() == doctest::Approx(2.0f));
    }

    TEST_CASE("batch4147 zero orthographic size leaves matrices identity") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(0.0f);
        CHECK(camera.get_orthographic_size() == 0.0f);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch4147 negative orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-4147.25f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(-4147.25f));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch4147 repeated activate calls remain active until explicit deactivate") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.activate();
        camera.activate();
        CHECK(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch4147 ICamera virtual dispatch preserves dummy guarantees") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_orthographic_size(41.47f);
        camera.activate();
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_is_active());
        CHECK(concrete.get_orthographic_size() == doctest::Approx(41.47f));
    }

    TEST_CASE("batch4147 temporary camera destruction cannot change survivor state") {
        Salix::DummyCamera survivor;
        survivor.set_orthographic_size(7.5f);
        survivor.activate();
        {
            Salix::DummyCamera temporary;
            temporary.set_orthographic_size(-99.0f);
            temporary.deactivate();
        }
        CHECK(survivor.get_is_active());
        CHECK(survivor.get_orthographic_size() == doctest::Approx(7.5f));
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch4547") {
    TEST_CASE("batch4547 projection mode reference remains stable across repeated calls") {
        Salix::DummyCamera camera;
        const auto* first = &camera.get_projection_mode();
        const auto* second = &camera.get_projection_mode();
        CHECK(first == second);
        CHECK(*first == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch4547 negative zero orthographic size preserves sign bit") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-0.0f);
        const float value = camera.get_orthographic_size();
        CHECK(value == 0.0f);
        CHECK(std::signbit(value));
    }

    TEST_CASE("batch4547 positive infinity orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(std::numeric_limits<float>::infinity());
        CHECK(std::isinf(camera.get_orthographic_size()));
        CHECK(camera.get_orthographic_size() > 0.0f);
    }

    TEST_CASE("batch4547 negative infinity orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-std::numeric_limits<float>::infinity());
        CHECK(std::isinf(camera.get_orthographic_size()));
        CHECK(camera.get_orthographic_size() < 0.0f);
    }

    TEST_CASE("batch4547 NaN orthographic size survives activation transitions") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(std::numeric_limits<float>::quiet_NaN());
        camera.activate();
        REQUIRE(camera.get_is_active());
        CHECK(std::isnan(camera.get_orthographic_size()));
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
        CHECK(std::isnan(camera.get_orthographic_size()));
    }

    TEST_CASE("batch4547 repeated deactivate calls remain idempotently inactive") {
        Salix::DummyCamera camera;
        camera.deactivate();
        camera.deactivate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch4547 direct active setter and convenience methods operate on same state") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        REQUIRE(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch4547 matrix references remain stable through nonfinite size assignment") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(std::numeric_limits<float>::quiet_NaN());
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
        CHECK(*view == glm::mat4(1.0f));
        CHECK(*projection == glm::mat4(1.0f));
    }

    TEST_CASE("batch4547 separate cameras retain opposite activity and infinity size state") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(std::numeric_limits<float>::infinity());
        second.set_orthographic_size(-std::numeric_limits<float>::infinity());
        first.activate();
        second.deactivate();
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
        CHECK(first.get_orthographic_size() > 0.0f);
        CHECK(second.get_orthographic_size() < 0.0f);
    }

    TEST_CASE("batch4547 ICamera invalid projection request remains a no op") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_projection_mode(static_cast<Salix::ProjectionMode>(4547));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch4747") {
    TEST_CASE("batch4747 maximum finite orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        const float value = (std::numeric_limits<float>::max)();
        camera.set_orthographic_size(value);
        CHECK(camera.get_orthographic_size() == value);
    }

    TEST_CASE("batch4747 lowest finite orthographic size is stored verbatim") {
        Salix::DummyCamera camera;
        const float value = (std::numeric_limits<float>::lowest)();
        camera.set_orthographic_size(value);
        CHECK(camera.get_orthographic_size() == value);
    }

    TEST_CASE("batch4747 repeated size assignments retain only final requested scalar") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(1.0f);
        camera.set_orthographic_size(-2.0f);
        camera.set_orthographic_size(47.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(47.47f));
    }

    TEST_CASE("batch4747 changing size never changes active state") {
        Salix::DummyCamera camera;
        camera.activate();
        REQUIRE(camera.get_is_active());
        camera.set_orthographic_size(-4747.0f);
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch4747 changing active state never changes configured size") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(74.47f);
        camera.set_is_active(true);
        camera.set_is_active(false);
        CHECK(camera.get_orthographic_size() == doctest::Approx(74.47f));
    }

    TEST_CASE("batch4747 separate cameras own distinct view matrix storage") {
        Salix::DummyCamera first, second;
        CHECK(&first.get_view_matrix() != &second.get_view_matrix());
        CHECK(first.get_view_matrix() == glm::mat4(1.0f));
        CHECK(second.get_view_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch4747 separate cameras own distinct projection matrix storage") {
        Salix::DummyCamera first, second;
        CHECK(&first.get_projection_matrix() != &second.get_projection_matrix());
        CHECK(first.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(second.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch4747 projection mode reference is shared immutable dummy perspective value") {
        Salix::DummyCamera first, second;
        const auto* first_mode = &first.get_projection_mode();
        const auto* second_mode = &second.get_projection_mode();
        CHECK(first_mode == second_mode);
        CHECK(*first_mode == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch4747 invalid projection mode request leaves size activity and matrices untouched") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(4.747f);
        camera.activate();
        camera.set_projection_mode(static_cast<Salix::ProjectionMode>(-4747));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == doctest::Approx(4.747f));
        CHECK(camera.get_is_active());
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch4747 ICamera active setter and convenience methods share one underlying flag") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_is_active(true);
        REQUIRE(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
        camera.activate();
        CHECK(camera.get_is_active());
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch5147") {
    TEST_CASE("batch5147 constructor exposes complete immutable camera defaults") {
        Salix::DummyCamera camera;
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == doctest::Approx(10.0f));
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5147 zero orthographic size is stored exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(0.0f);
        CHECK(camera.get_orthographic_size() == 0.0f);
    }

    TEST_CASE("batch5147 negative orthographic size is stored exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-51.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(-51.47f));
    }

    TEST_CASE("batch5147 repeated activation is idempotent") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.activate();
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch5147 repeated deactivation is idempotent after activation") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.deactivate();
        camera.deactivate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5147 explicit active setter sequence retains final state only") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        camera.set_is_active(false);
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch5147 ignored projection mode setter never changes size or activity") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(51.47f);
        camera.activate();
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == doctest::Approx(51.47f));
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch5147 matrix reference addresses remain stable through scalar and activity changes") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(14.7f);
        camera.activate();
        camera.deactivate();
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
        CHECK(*view == glm::mat4(1.0f));
        CHECK(*projection == glm::mat4(1.0f));
    }

    TEST_CASE("batch5147 separate cameras own distinct matrices and independent activity flags") {
        Salix::DummyCamera first, second;
        first.activate();
        second.deactivate();
        CHECK(&first.get_view_matrix() != &second.get_view_matrix());
        CHECK(&first.get_projection_matrix() != &second.get_projection_matrix());
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
    }

    TEST_CASE("batch5147 ICamera virtual configuration reaches size and active implementations") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_orthographic_size(5.147f);
        camera.set_is_active(true);
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(concrete.get_orthographic_size() == doctest::Approx(5.147f));
        CHECK(camera.get_is_active());
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch5547") {
    TEST_CASE("batch5547 constructor retains perspective mode and identity matrices") {
        Salix::DummyCamera camera;
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch5547 positive orthographic size replacement stores exact value") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(55.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(55.47f));
    }

    TEST_CASE("batch5547 repeated orthographic size replacements retain final value") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(1.0f);
        camera.set_orthographic_size(-2.0f);
        camera.set_orthographic_size(55.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(55.47f));
    }

    TEST_CASE("batch5547 active setter can alternate repeatedly") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        camera.set_is_active(false);
        camera.set_is_active(true);
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5547 activate after explicit false sets active") {
        Salix::DummyCamera camera;
        camera.set_is_active(false);
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch5547 deactivate after explicit true clears active") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5547 ignored projection setter preserves identity matrices") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch5547 scalar changes preserve matrix reference identity") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(55.47f);
        camera.set_is_active(true);
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
    }

    TEST_CASE("batch5547 separate cameras retain independent size and activity") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(55.0f);
        first.activate();
        second.set_orthographic_size(47.0f);
        second.deactivate();
        CHECK(first.get_orthographic_size() == doctest::Approx(55.0f));
        CHECK(second.get_orthographic_size() == doctest::Approx(47.0f));
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
    }

    TEST_CASE("batch5547 ICamera dispatch preserves dummy projection contract") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_orthographic_size(5.547f);
        camera.activate();
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(concrete.get_orthographic_size() == doctest::Approx(5.547f));
        CHECK(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch5947") {
    TEST_CASE("batch5947 constructor exposes identity matrices perspective mode and inactive state") {
        Salix::DummyCamera camera;
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == doctest::Approx(10.0f));
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5947 zero orthographic size is stored exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(0.0f);
        CHECK(camera.get_orthographic_size() == 0.0f);
    }

    TEST_CASE("batch5947 negative orthographic size is stored exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-59.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(-59.47f));
    }

    TEST_CASE("batch5947 repeated size replacement retains final value") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(1.0f);
        camera.set_orthographic_size(-2.0f);
        camera.set_orthographic_size(59.47f);
        CHECK(camera.get_orthographic_size() == doctest::Approx(59.47f));
    }

    TEST_CASE("batch5947 explicit active setter alternates and retains final state") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        camera.set_is_active(false);
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch5947 activate and deactivate remain idempotent") {
        Salix::DummyCamera camera;
        camera.activate();
        camera.activate();
        REQUIRE(camera.get_is_active());
        camera.deactivate();
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch5947 projection mode setter preserves dummy perspective contract") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch5947 matrix references remain stable through scalar and activity changes") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(59.47f);
        camera.activate();
        camera.deactivate();
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
        CHECK(*view == glm::mat4(1.0f));
        CHECK(*projection == glm::mat4(1.0f));
    }

    TEST_CASE("batch5947 separate cameras keep size and activity independent") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(59.0f);
        first.activate();
        second.set_orthographic_size(47.0f);
        second.deactivate();
        CHECK(first.get_orthographic_size() == doctest::Approx(59.0f));
        CHECK(second.get_orthographic_size() == doctest::Approx(47.0f));
        CHECK(first.get_is_active());
        CHECK_FALSE(second.get_is_active());
    }

    TEST_CASE("batch5947 ICamera dispatch preserves dummy projection and activity behavior") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_orthographic_size(5.947f);
        camera.activate();
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(concrete.get_orthographic_size() == doctest::Approx(5.947f));
        CHECK(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch6047") {
    TEST_CASE("batch6047 signed size replacements retain final zero exactly") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(60.47f);
        camera.set_orthographic_size(-47.06f);
        camera.set_orthographic_size(0.0f);
        CHECK(camera.get_orthographic_size() == 0.0f);
    }

    TEST_CASE("batch6047 explicit inactive setter overrides prior activation") {
        Salix::DummyCamera camera;
        camera.activate();
        REQUIRE(camera.get_is_active());
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch6047 activate overrides prior explicit inactive state") {
        Salix::DummyCamera camera;
        camera.set_is_active(false);
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch6047 deactivate overrides prior explicit active state") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        REQUIRE(camera.get_is_active());
        camera.deactivate();
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch6047 repeated ignored orthographic projection requests remain perspective") {
        Salix::DummyCamera camera;
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch6047 size changes never modify identity matrices") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(6.047f);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
        camera.set_orthographic_size(-60.47f);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch6047 activity changes never modify configured orthographic size") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(47.06f);
        camera.activate();
        camera.deactivate();
        camera.set_is_active(true);
        CHECK(camera.get_orthographic_size() == doctest::Approx(47.06f));
    }

    TEST_CASE("batch6047 separate cameras own distinct matrix references with equal identity values") {
        Salix::DummyCamera first, second;
        CHECK(&first.get_view_matrix() != &second.get_view_matrix());
        CHECK(&first.get_projection_matrix() != &second.get_projection_matrix());
        CHECK(first.get_view_matrix() == second.get_view_matrix());
        CHECK(first.get_projection_matrix() == second.get_projection_matrix());
    }

    TEST_CASE("batch6047 ICamera dispatch stores size while preserving dummy perspective behavior") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_orthographic_size(-6.047f);
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(concrete.get_orthographic_size() == doctest::Approx(-6.047f));
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_view_matrix() == glm::mat4(1.0f));
        CHECK(camera.get_projection_matrix() == glm::mat4(1.0f));
    }

    TEST_CASE("batch6047 fresh instances remain independent after one is fully mutated") {
        Salix::DummyCamera mutated, fresh;
        mutated.set_orthographic_size(-60.47f);
        mutated.activate();
        mutated.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(mutated.get_orthographic_size() == doctest::Approx(-60.47f));
        CHECK(mutated.get_is_active());
        CHECK(fresh.get_orthographic_size() == doctest::Approx(10.0f));
        CHECK_FALSE(fresh.get_is_active());
        CHECK(fresh.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }
}

TEST_SUITE("Salix::rendering::DummyCameraBatch6147") {
    TEST_CASE("batch6147 repeated true active setter remains active") {
        Salix::DummyCamera camera;
        camera.set_is_active(true);
        camera.set_is_active(true);
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch6147 repeated false active setter remains inactive") {
        Salix::DummyCamera camera;
        camera.set_is_active(false);
        camera.set_is_active(false);
        CHECK_FALSE(camera.get_is_active());
    }

    TEST_CASE("batch6147 size mutation preserves projection matrix reference identity") {
        Salix::DummyCamera camera;
        const auto* projection = &camera.get_projection_matrix();
        camera.set_orthographic_size(61.47f);
        CHECK(&camera.get_projection_matrix() == projection);
        CHECK(*projection == glm::mat4(1.0f));
    }

    TEST_CASE("batch6147 ignored projection mutation preserves view matrix reference identity") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(&camera.get_view_matrix() == view);
        CHECK(*view == glm::mat4(1.0f));
    }

    TEST_CASE("batch6147 activity mutations preserve both matrix reference identities") {
        Salix::DummyCamera camera;
        const auto* view = &camera.get_view_matrix();
        const auto* projection = &camera.get_projection_matrix();
        camera.activate();
        camera.deactivate();
        camera.set_is_active(true);
        CHECK(&camera.get_view_matrix() == view);
        CHECK(&camera.get_projection_matrix() == projection);
    }

    TEST_CASE("batch6147 separate cameras keep orthographic sizes independent after replacements") {
        Salix::DummyCamera first, second;
        first.set_orthographic_size(61.0f);
        second.set_orthographic_size(47.0f);
        first.set_orthographic_size(-14.0f);
        CHECK(first.get_orthographic_size() == doctest::Approx(-14.0f));
        CHECK(second.get_orthographic_size() == doctest::Approx(47.0f));
    }

    TEST_CASE("batch6147 separate cameras keep activity independent through opposite transitions") {
        Salix::DummyCamera first, second;
        first.activate();
        second.deactivate();
        REQUIRE(first.get_is_active());
        REQUIRE_FALSE(second.get_is_active());
        first.deactivate();
        second.activate();
        CHECK_FALSE(first.get_is_active());
        CHECK(second.get_is_active());
    }

    TEST_CASE("batch6147 ICamera active setters and helpers reach dummy state") {
        Salix::DummyCamera concrete;
        Salix::ICamera& camera = concrete;
        camera.set_is_active(true);
        REQUIRE(camera.get_is_active());
        camera.deactivate();
        REQUIRE_FALSE(camera.get_is_active());
        camera.activate();
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch6147 dummy remains perspective after scalar and activity mutation") {
        Salix::DummyCamera camera;
        camera.set_orthographic_size(-61.47f);
        camera.activate();
        camera.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(camera.get_orthographic_size() == doctest::Approx(-61.47f));
        CHECK(camera.get_is_active());
    }

    TEST_CASE("batch6147 fresh peer matrices and defaults survive other instance mutation") {
        Salix::DummyCamera mutated, fresh;
        mutated.set_orthographic_size(61.47f);
        mutated.activate();
        mutated.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(fresh.get_view_matrix() == glm::mat4(1.0f));
        CHECK(fresh.get_projection_matrix() == glm::mat4(1.0f));
        CHECK(fresh.get_projection_mode() == Salix::ProjectionMode::Perspective);
        CHECK(fresh.get_orthographic_size() == doctest::Approx(10.0f));
        CHECK_FALSE(fresh.get_is_active());
    }
}
