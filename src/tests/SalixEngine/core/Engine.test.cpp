#include <doctest.h>
#include <Salix/core/Engine.h>
#include <Salix/core/EngineMode.h>
#include <memory>

TEST_SUITE("Salix::core::EngineBatch1647") {
    TEST_CASE("fresh engine begins stopped") {
        Salix::Engine engine;
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("fresh engine begins in neutral engine mode") {
        Salix::Engine engine;
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("fresh engine begins with unit time scale") {
        Salix::Engine engine;
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("preinitialized engine exposes null owned subsystem pointers") {
        Salix::Engine engine;
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("engine running setter toggles stopped and running states explicitly") {
        Salix::Engine engine;
        CHECK(engine.is_running(true));
        CHECK(engine.is_running());
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("engine mode setter roundtrips launch editor game options and debug") {
        Salix::Engine engine;
        const Salix::EngineMode modes[] = {
            Salix::EngineMode::Launch,
            Salix::EngineMode::Editor,
            Salix::EngineMode::Game,
            Salix::EngineMode::Options,
            Salix::EngineMode::Debug
        };
        for (auto mode : modes) {
            engine.set_mode(mode);
            CHECK(engine.get_mode() == mode);
        }
    }

    TEST_CASE("engine time scale preserves fractional zero and negative assignments") {
        Salix::Engine engine;
        engine.set_time_scale(0.25f);
        CHECK(engine.get_time_scale() == doctest::Approx(0.25f));
        engine.set_time_scale(0.0f);
        CHECK(engine.get_time_scale() == doctest::Approx(0.0f));
        engine.set_time_scale(-2.0f);
        CHECK(engine.get_time_scale() == doctest::Approx(-2.0f));
    }

    TEST_CASE("engine no-op state stack methods preserve preinitialization state") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(0.5f);
        engine.push_state(nullptr);
        engine.pop_state();
        engine.change_state(nullptr);
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.get_time_scale() == doctest::Approx(0.5f));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("separate engine instances keep independent running mode and time scale state") {
        Salix::Engine first;
        Salix::Engine second;
        first.is_running(true);
        first.set_mode(Salix::EngineMode::Game);
        first.set_time_scale(2.0f);
        CHECK(first.is_running());
        CHECK(first.get_mode() == Salix::EngineMode::Game);
        CHECK(first.get_time_scale() == doctest::Approx(2.0f));
        CHECK_FALSE(second.is_running());
        CHECK(second.get_mode() == Salix::EngineMode::None);
        CHECK(second.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("engine supports polymorphic ownership through EngineInterface") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        REQUIRE(engine != nullptr);
        CHECK_FALSE(engine->is_running());
        engine->set_mode(Salix::EngineMode::Editor);
        engine->set_time_scale(1.5f);
        CHECK(engine->get_mode() == Salix::EngineMode::Editor);
        CHECK(engine->get_time_scale() == doctest::Approx(1.5f));
    }
}

#include <limits>
#include <cmath>

TEST_SUITE("Salix::core::EngineBatch2447") {
    TEST_CASE("batch2447 running true setter is idempotent") {
        Salix::Engine engine;
        CHECK(engine.is_running(true));
        CHECK(engine.is_running(true));
        CHECK(engine.is_running());
    }

    TEST_CASE("batch2447 running false setter is idempotent") {
        Salix::Engine engine;
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch2447 mode can return to neutral after several transitions") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_mode(Salix::EngineMode::None);
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("batch2447 mode changes never alter running state") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Launch);
        engine.set_mode(Salix::EngineMode::Debug);
        CHECK(engine.is_running());
    }

    TEST_CASE("batch2447 mode changes never alter time scale") {
        Salix::Engine engine;
        engine.set_time_scale(3.25f);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_mode(Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(3.25f));
    }

    TEST_CASE("batch2447 time scale changes never alter selected mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_time_scale(0.0f);
        engine.set_time_scale(-9.5f);
        CHECK(engine.get_mode() == Salix::EngineMode::Editor);
    }

    TEST_CASE("batch2447 no-op state stack operations preserve all uninitialized subsystem pointers") {
        Salix::Engine engine;
        engine.push_state(nullptr);
        engine.pop_state();
        engine.change_state(nullptr);
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch2447 EngineInterface dispatch toggles running state through virtual API") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        CHECK(engine->is_running(true));
        CHECK(engine->is_running());
        CHECK_FALSE(engine->is_running(false));
        CHECK_FALSE(engine->is_running());
    }

    TEST_CASE("batch2447 time scale preserves positive infinity exactly") {
        Salix::Engine engine;
        engine.set_time_scale(std::numeric_limits<float>::infinity());
        CHECK(std::isinf(engine.get_time_scale()));
        CHECK(engine.get_time_scale() > 0.0f);
    }

    TEST_CASE("batch2447 time scale preserves NaN diagnostic values") {
        Salix::Engine engine;
        engine.set_time_scale(std::numeric_limits<float>::quiet_NaN());
        CHECK(std::isnan(engine.get_time_scale()));
    }
}

TEST_SUITE("Salix::core::EngineBatch2847") {
    TEST_CASE("batch2847 running state survives mode and time-scale changes") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(2.5f);
        CHECK(engine.is_running());
    }
    TEST_CASE("batch2847 stopping engine preserves mode and time scale") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_time_scale(0.75f);
        engine.is_running(false);
        CHECK(engine.get_mode()==Salix::EngineMode::Editor);
        CHECK(engine.get_time_scale()==doctest::Approx(0.75f));
    }
    TEST_CASE("batch2847 repeated neutral mode assignment is idempotent") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::None);
        engine.set_mode(Salix::EngineMode::None);
        CHECK(engine.get_mode()==Salix::EngineMode::None);
    }
    TEST_CASE("batch2847 time scale accepts negative infinity without touching mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(-std::numeric_limits<float>::infinity());
        CHECK(std::isinf(engine.get_time_scale()));
        CHECK(engine.get_time_scale()<0.0f);
        CHECK(engine.get_mode()==Salix::EngineMode::Debug);
    }
    TEST_CASE("batch2847 subsystem getters remain null after state-stack no-op calls") {
        Salix::Engine engine;
        engine.push_state(nullptr);
        engine.change_state(nullptr);
        engine.pop_state();
        CHECK(engine.get_renderer()==nullptr);
        CHECK(engine.get_asset_manager()==nullptr);
        CHECK(engine.get_input_manager()==nullptr);
        CHECK(engine.get_event_manager()==nullptr);
    }
    TEST_CASE("batch2847 two engines can hold opposite running states independently") {
        Salix::Engine first,second;
        first.is_running(true);
        second.is_running(false);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
    }
    TEST_CASE("batch2847 two engines can hold different modes independently") {
        Salix::Engine first,second;
        first.set_mode(Salix::EngineMode::Launch);
        second.set_mode(Salix::EngineMode::Options);
        CHECK(first.get_mode()==Salix::EngineMode::Launch);
        CHECK(second.get_mode()==Salix::EngineMode::Options);
    }
    TEST_CASE("batch2847 two engines can hold different time scales independently") {
        Salix::Engine first,second;
        first.set_time_scale(0.25f);
        second.set_time_scale(4.0f);
        CHECK(first.get_time_scale()==doctest::Approx(0.25f));
        CHECK(second.get_time_scale()==doctest::Approx(4.0f));
    }
    TEST_CASE("batch2847 EngineInterface dispatch preserves selected debug mode") {
        std::unique_ptr<Salix::EngineInterface> engine=std::make_unique<Salix::Engine>();
        engine->set_mode(Salix::EngineMode::Debug);
        CHECK(engine->get_mode()==Salix::EngineMode::Debug);
    }
    TEST_CASE("batch2847 EngineInterface dispatch preserves negative time scale") {
        std::unique_ptr<Salix::EngineInterface> engine=std::make_unique<Salix::Engine>();
        engine->set_time_scale(-3.0f);
        CHECK(engine->get_time_scale()==doctest::Approx(-3.0f));
    }
}

