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

TEST_SUITE("Salix::core::EngineInterfaceBatch6447") {
    TEST_CASE("batch6447 state switch count increments for every requested transition") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.switch_state(Salix::AppStateType::Launch);
        interface.switch_state(Salix::AppStateType::Editor);
        interface.switch_state(Salix::AppStateType::Game);
        CHECK(engine.switch_state_calls == 3);
        CHECK(engine.current_state == Salix::AppStateType::Game);
    }

    TEST_CASE("batch6447 repeated identical state transitions remain observable") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.switch_state(Salix::AppStateType::Options);
        interface.switch_state(Salix::AppStateType::Options);
        CHECK(engine.switch_state_calls == 2);
        CHECK(engine.current_state == Salix::AppStateType::Options);
    }

    TEST_CASE("batch6447 repeated mode assignment increments setter accounting") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.set_mode(Salix::EngineMode::Debug);
        interface.set_mode(Salix::EngineMode::Debug);
        CHECK(engine.set_mode_calls == 2);
        CHECK(interface.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch6447 running toggle returns exact newly stored state") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        CHECK_FALSE(interface.is_running(false));
        CHECK(interface.is_running(true));
        CHECK(interface.is_running());
    }

    TEST_CASE("batch6447 negative time scale roundtrips without changing running state") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.set_time_scale(-6.447f);
        CHECK(interface.get_time_scale() == doctest::Approx(-6.447f));
        CHECK(interface.is_running());
    }

    TEST_CASE("batch6447 mode changes do not increment time scale setter accounting") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.set_mode(Salix::EngineMode::Editor);
        interface.set_mode(Salix::EngineMode::Game);
        CHECK(engine.set_time_scale_calls == 0);
        CHECK(engine.set_mode_calls == 2);
    }

    TEST_CASE("batch6447 time scale changes do not increment mode setter accounting") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.set_time_scale(0.5f);
        interface.set_time_scale(2.0f);
        CHECK(engine.set_mode_calls == 0);
        CHECK(engine.set_time_scale_calls == 2);
    }

    TEST_CASE("batch6447 state switches do not rewrite selected mode") {
        MockEngineInterface engine;
        Salix::EngineInterface& interface = engine;
        interface.set_mode(Salix::EngineMode::Options);
        interface.switch_state(Salix::AppStateType::Game);
        CHECK(interface.get_mode() == Salix::EngineMode::Options);
    }

    TEST_CASE("batch6447 two engines retain independent lifecycle counters") {
        MockEngineInterface first;
        MockEngineInterface second;
        first.switch_state(Salix::AppStateType::Editor);
        first.set_mode(Salix::EngineMode::Editor);
        second.set_time_scale(3.0f);
        CHECK(first.switch_state_calls == 1);
        CHECK(first.set_mode_calls == 1);
        CHECK(first.set_time_scale_calls == 0);
        CHECK(second.switch_state_calls == 0);
        CHECK(second.set_mode_calls == 0);
        CHECK(second.set_time_scale_calls == 1);
    }

    TEST_CASE("batch6447 polymorphic ownership preserves negative time scale") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<MockEngineInterface>();
        engine->set_time_scale(-1.25f);
        CHECK(engine->get_time_scale() == doctest::Approx(-1.25f));
    }
}
