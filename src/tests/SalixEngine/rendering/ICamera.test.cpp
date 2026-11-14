#include <doctest.h>
#include <Salix/rendering/ICamera.h>
#include <Tests/SalixEngine/mocking/rendering/MockICamera.h>
#include <glm/glm.hpp>
#include <memory>

namespace {
class Batch1347MinimalCamera : public Salix::ICamera {
public:
    const glm::mat4& get_view_matrix() override { return view; }
    const glm::mat4& get_projection_matrix() override { return projection; }
    const Salix::ProjectionMode& get_projection_mode() const override { return mode; }
    void set_projection_mode(Salix::ProjectionMode new_mode) override { mode = new_mode; }
    void set_orthographic_size(float size) override { orthographic_size = size; }

    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};
    Salix::ProjectionMode mode = Salix::ProjectionMode::Perspective;
    float orthographic_size = 10.0f;
};
}

TEST_SUITE("Salix::rendering::ICameraBatch1347") {
    TEST_CASE("camera interface default transform accessor returns null") {
        Batch1347MinimalCamera camera;
        Salix::ICamera* interface_ptr = &camera;
        CHECK(interface_ptr->get_transform() == nullptr);
    }

    TEST_CASE("camera interface default active state is false") {
        Batch1347MinimalCamera camera;
        Salix::ICamera* interface_ptr = &camera;
        CHECK_FALSE(interface_ptr->get_is_active());
    }

    TEST_CASE("camera interface default activation hooks are safe no-ops") {
        Batch1347MinimalCamera camera;
        Salix::ICamera* interface_ptr = &camera;
        interface_ptr->set_is_active(true);
        interface_ptr->activate();
        CHECK_FALSE(interface_ptr->get_is_active());
        interface_ptr->deactivate();
        CHECK_FALSE(interface_ptr->get_is_active());
    }

    TEST_CASE("camera interface projection mode dispatches through stateful mock") {
        MockICamera camera;
        Salix::ICamera* interface_ptr = &camera;
        CHECK(interface_ptr->get_projection_mode() == Salix::ProjectionMode::Perspective);
        interface_ptr->set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(interface_ptr->get_projection_mode() == Salix::ProjectionMode::Orthographic);
    }

    TEST_CASE("camera interface orthographic size reaches concrete implementation") {
        MockICamera camera;
        Salix::ICamera* interface_ptr = &camera;
        interface_ptr->set_orthographic_size(42.5f);
        CHECK(camera.get_mock_orthographic_size() == doctest::Approx(42.5f));
    }

    TEST_CASE("camera interface two dimensional mode reaches concrete implementation") {
        MockICamera camera;
        Salix::ICamera* interface_ptr = &camera;
        interface_ptr->set_2D_mode(true);
        CHECK(camera.get_mock_2d_mode());
        interface_ptr->set_2D_mode(false);
        CHECK_FALSE(camera.get_mock_2d_mode());
    }

    TEST_CASE("camera interface mock exposes owned transform without transferring ownership") {
        MockICamera camera;
        Salix::ICamera* interface_ptr = &camera;
        REQUIRE(interface_ptr->get_transform() != nullptr);
        CHECK(interface_ptr->get_transform() == camera.get_transform());
    }

    TEST_CASE("camera interface returns configured view and projection matrices") {
        MockICamera camera;
        camera.set_mock_view_matrix(glm::mat4(2.0f));
        camera.set_mock_projection_matrix(glm::mat4(3.0f));
        Salix::ICamera* interface_ptr = &camera;
        CHECK(interface_ptr->get_view_matrix()[0][0] == doctest::Approx(2.0f));
        CHECK(interface_ptr->get_projection_matrix()[0][0] == doctest::Approx(3.0f));
    }

    TEST_CASE("camera interface active state supports set activate and deactivate") {
        MockICamera camera;
        Salix::ICamera* interface_ptr = &camera;
        interface_ptr->set_is_active(true);
        CHECK(interface_ptr->get_is_active());
        interface_ptr->deactivate();
        CHECK_FALSE(interface_ptr->get_is_active());
        interface_ptr->activate();
        CHECK(interface_ptr->get_is_active());
    }

    TEST_CASE("camera interface supports polymorphic destruction") {
        std::unique_ptr<Salix::ICamera> camera = std::make_unique<MockICamera>();
        REQUIRE(camera != nullptr);
        camera->set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(camera->get_projection_mode() == Salix::ProjectionMode::Orthographic);
    }
}

TEST_SUITE("Salix::rendering::ICameraBatch6347") {
    TEST_CASE("batch6347 minimal camera begins in perspective projection") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        CHECK(interface.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch6347 minimal camera projection setter dispatches through interface") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        interface.set_projection_mode(Salix::ProjectionMode::Orthographic);
        CHECK(interface.get_projection_mode() == Salix::ProjectionMode::Orthographic);
        CHECK(camera.mode == Salix::ProjectionMode::Orthographic);
    }

    TEST_CASE("batch6347 minimal camera orthographic size setter stores exact value") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        interface.set_orthographic_size(-63.47f);
        CHECK(camera.orthographic_size == doctest::Approx(-63.47f));
    }

    TEST_CASE("batch6347 default two dimensional hook is a safe no op") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        CHECK_NOTHROW(interface.set_2D_mode(true));
        CHECK_NOTHROW(interface.set_2D_mode(false));
        CHECK(interface.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch6347 default active setter remains a no op") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        interface.set_is_active(true);
        CHECK_FALSE(interface.get_is_active());
        interface.activate();
        CHECK_FALSE(interface.get_is_active());
    }

    TEST_CASE("batch6347 minimal camera matrix references remain stable across repeated access") {
        Batch1347MinimalCamera camera;
        Salix::ICamera& interface = camera;
        const auto* view = &interface.get_view_matrix();
        const auto* projection = &interface.get_projection_matrix();
        CHECK(&interface.get_view_matrix() == view);
        CHECK(&interface.get_projection_matrix() == projection);
    }

    TEST_CASE("batch6347 mock camera two dimensional state roundtrips through interface") {
        MockICamera camera;
        Salix::ICamera& interface = camera;
        interface.set_2D_mode(true);
        REQUIRE(camera.get_mock_2d_mode());
        interface.set_2D_mode(false);
        CHECK_FALSE(camera.get_mock_2d_mode());
    }

    TEST_CASE("batch6347 mock camera projection transitions roundtrip through interface") {
        MockICamera camera;
        Salix::ICamera& interface = camera;
        interface.set_projection_mode(Salix::ProjectionMode::Orthographic);
        REQUIRE(interface.get_projection_mode() == Salix::ProjectionMode::Orthographic);
        interface.set_projection_mode(Salix::ProjectionMode::Perspective);
        CHECK(interface.get_projection_mode() == Salix::ProjectionMode::Perspective);
    }

    TEST_CASE("batch6347 mock camera active state follows helper transition sequence") {
        MockICamera camera;
        Salix::ICamera& interface = camera;
        interface.activate();
        REQUIRE(interface.get_is_active());
        interface.deactivate();
        REQUIRE_FALSE(interface.get_is_active());
        interface.set_is_active(true);
        CHECK(interface.get_is_active());
    }

    TEST_CASE("batch6347 polymorphic camera ownership preserves configured matrices") {
        auto camera = std::make_unique<MockICamera>();
        camera->set_mock_view_matrix(glm::mat4(2.0f));
        camera->set_mock_projection_matrix(glm::mat4(3.0f));
        std::unique_ptr<Salix::ICamera> interface = std::move(camera);
        REQUIRE(interface != nullptr);
        CHECK(interface->get_view_matrix()[0][0] == doctest::Approx(2.0f));
        CHECK(interface->get_projection_matrix()[0][0] == doctest::Approx(3.0f));
    }
}
