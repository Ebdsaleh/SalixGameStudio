#include <doctest.h>
#include <Salix/core/EngineInterface.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <memory>

TEST_SUITE("Salix::core::EngineInterfaceBatch1347") {
    TEST_CASE("engine interface starts with neutral mock mode and running state") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::None);
        CHECK(interface_ptr->is_running());
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine interface dispatches state switches through base pointer") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->switch_state(Salix::AppStateType::Editor);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
        CHECK(engine.switch_state_calls == 1);
    }

    TEST_CASE("engine interface accepts repeated state transitions") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->switch_state(Salix::AppStateType::Launch);
        interface_ptr->switch_state(Salix::AppStateType::Game);
        CHECK(engine.current_state == Salix::AppStateType::Game);
        CHECK(engine.switch_state_calls == 2);
    }

    TEST_CASE("engine interface mode setter and getter preserve selected mode") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_mode(Salix::EngineMode::Debug);
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.set_mode_calls == 1);
    }

    TEST_CASE("engine interface running setter returns and stores false") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        CHECK_FALSE(interface_ptr->is_running(false));
        CHECK_FALSE(interface_ptr->is_running());
    }

    TEST_CASE("engine interface running setter can restart state") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->is_running(false);
        CHECK(interface_ptr->is_running(true));
        CHECK(interface_ptr->is_running());
    }

    TEST_CASE("engine interface time scale preserves fractional values") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_time_scale(0.25f);
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(0.25f));
        CHECK(engine.set_time_scale_calls == 1);
    }

    TEST_CASE("engine interface time scale permits zero without changing mode") {
        MockEngineInterface engine;
        Salix::EngineInterface* interface_ptr = &engine;
        interface_ptr->set_mode(Salix::EngineMode::Game);
        interface_ptr->set_time_scale(0.0f);
        CHECK(interface_ptr->get_time_scale() == doctest::Approx(0.0f));
        CHECK(interface_ptr->get_mode() == Salix::EngineMode::Game);
    }

    TEST_CASE("separate engine interface implementations keep independent state") {
        MockEngineInterface first;
        MockEngineInterface second;
        Salix::EngineInterface* a = &first;
        Salix::EngineInterface* b = &second;
        a->set_mode(Salix::EngineMode::Editor);
        b->set_mode(Salix::EngineMode::Game);
        a->set_time_scale(0.5f);
        CHECK(a->get_mode() == Salix::EngineMode::Editor);
        CHECK(b->get_mode() == Salix::EngineMode::Game);
        CHECK(a->get_time_scale() == doctest::Approx(0.5f));
        CHECK(b->get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine interface supports polymorphic destruction") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<MockEngineInterface>();
        REQUIRE(engine != nullptr);
        engine->set_mode(Salix::EngineMode::Launch);
        CHECK(engine->get_mode() == Salix::EngineMode::Launch);
    }
}
