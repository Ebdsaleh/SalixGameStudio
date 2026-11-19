#include <doctest.h>
#include <Salix/core/InitContext.h>
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

TEST_SUITE("Salix::core::EngineBatch3247") {
 TEST_CASE("batch3247 fresh engine combines stopped neutral unit-scale defaults") { Salix::Engine e; CHECK_FALSE(e.is_running()); CHECK(e.get_mode()==Salix::EngineMode::None); CHECK(e.get_time_scale()==doctest::Approx(1.f)); }
 TEST_CASE("batch3247 starting engine never changes mode or time scale") { Salix::Engine e; e.set_mode(Salix::EngineMode::Debug); e.set_time_scale(2.25f); CHECK(e.is_running(true)); CHECK(e.get_mode()==Salix::EngineMode::Debug); CHECK(e.get_time_scale()==doctest::Approx(2.25f)); }
 TEST_CASE("batch3247 stopping engine never changes mode or time scale") { Salix::Engine e; e.is_running(true); e.set_mode(Salix::EngineMode::Game); e.set_time_scale(0.5f); CHECK_FALSE(e.is_running(false)); CHECK(e.get_mode()==Salix::EngineMode::Game); CHECK(e.get_time_scale()==doctest::Approx(0.5f)); }
 TEST_CASE("batch3247 full mode traversal leaves running state untouched") { Salix::Engine e; e.is_running(true); for(auto m:{Salix::EngineMode::Launch,Salix::EngineMode::Editor,Salix::EngineMode::Game,Salix::EngineMode::Options,Salix::EngineMode::Debug,Salix::EngineMode::None}) e.set_mode(m); CHECK(e.is_running()); CHECK(e.get_mode()==Salix::EngineMode::None); }
 TEST_CASE("batch3247 negative zero time scale preserves sign bit") { Salix::Engine e; e.set_time_scale(-0.0f); CHECK(e.get_time_scale()==0.0f); CHECK(std::signbit(e.get_time_scale())); }
 TEST_CASE("batch3247 maximum finite time scale is stored verbatim") { Salix::Engine e; e.set_time_scale((std::numeric_limits<float>::max)()); CHECK(e.get_time_scale()==(std::numeric_limits<float>::max)()); }
 TEST_CASE("batch3247 lowest finite time scale is stored verbatim") { Salix::Engine e; e.set_time_scale((std::numeric_limits<float>::lowest)()); CHECK(e.get_time_scale()==(std::numeric_limits<float>::lowest)()); }
 TEST_CASE("batch3247 no-op state stack calls preserve a running configured engine") { Salix::Engine e; e.is_running(true); e.set_mode(Salix::EngineMode::Editor); e.set_time_scale(3.f); e.push_state(nullptr); e.change_state(nullptr); e.pop_state(); CHECK(e.is_running()); CHECK(e.get_mode()==Salix::EngineMode::Editor); CHECK(e.get_time_scale()==doctest::Approx(3.f)); }
 TEST_CASE("batch3247 EngineInterface can configure running mode and scale together") { std::unique_ptr<Salix::EngineInterface> e=std::make_unique<Salix::Engine>(); CHECK(e->is_running(true)); e->set_mode(Salix::EngineMode::Options); e->set_time_scale(-1.25f); CHECK(e->is_running()); CHECK(e->get_mode()==Salix::EngineMode::Options); CHECK(e->get_time_scale()==doctest::Approx(-1.25f)); }
 TEST_CASE("batch3247 mutating one engine never changes a fresh peer") { Salix::Engine a,b; a.is_running(true); a.set_mode(Salix::EngineMode::Debug); a.set_time_scale(99.f); CHECK_FALSE(b.is_running()); CHECK(b.get_mode()==Salix::EngineMode::None); CHECK(b.get_time_scale()==doctest::Approx(1.f)); }
}

TEST_SUITE("Salix::core::EngineBatch4047") {
    TEST_CASE("batch4047 fresh engine combines null subsystems with scalar defaults") {
        Salix::Engine engine;
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch4047 running setter return value always matches stored state") {
        Salix::Engine engine;
        CHECK(engine.is_running(true) == engine.is_running());
        CHECK(engine.is_running(false) == engine.is_running());
        CHECK(engine.is_running(true) == engine.is_running());
    }

    TEST_CASE("batch4047 repeated identical running assignments are idempotent") {
        Salix::Engine engine;
        CHECK(engine.is_running(true));
        CHECK(engine.is_running(true));
        CHECK(engine.is_running());
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running(false));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4047 mode overwrite retains only final requested mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Launch);
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_mode(Salix::EngineMode::Debug);
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch4047 time scale overwrite retains exact final finite value") {
        Salix::Engine engine;
        engine.set_time_scale(0.25f);
        engine.set_time_scale(8.0f);
        engine.set_time_scale(-4.5f);
        CHECK(engine.get_time_scale() == doctest::Approx(-4.5f));
    }

    TEST_CASE("batch4047 changing running state never rewrites configured mode or scale") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(3.75f);
        engine.is_running(true);
        engine.is_running(false);
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(3.75f));
    }

    TEST_CASE("batch4047 null state stack calls preserve complete configured scalar state") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(0.125f);
        engine.push_state(nullptr);
        engine.change_state(nullptr);
        engine.pop_state();
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(0.125f));
    }

    TEST_CASE("batch4047 separate engines remain independent after alternating mutations") {
        Salix::Engine first, second;
        first.is_running(true);
        second.set_mode(Salix::EngineMode::Debug);
        first.set_time_scale(2.0f);
        second.set_time_scale(-2.0f);
        CHECK(first.is_running());
        CHECK_FALSE(second.is_running());
        CHECK(first.get_mode() == Salix::EngineMode::None);
        CHECK(second.get_mode() == Salix::EngineMode::Debug);
        CHECK(first.get_time_scale() == doctest::Approx(2.0f));
        CHECK(second.get_time_scale() == doctest::Approx(-2.0f));
    }

    TEST_CASE("batch4047 EngineInterface virtual API preserves combined configuration") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        CHECK(engine->is_running(true));
        engine->set_mode(Salix::EngineMode::Editor);
        engine->set_time_scale(1.5f);
        CHECK(engine->is_running());
        CHECK(engine->get_mode() == Salix::EngineMode::Editor);
        CHECK(engine->get_time_scale() == doctest::Approx(1.5f));
    }

    TEST_CASE("batch4047 explicit destruction through EngineInterface remains safe before initialization") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        engine->set_mode(Salix::EngineMode::Launch);
        engine->set_time_scale(0.5f);
        CHECK_NOTHROW(engine.reset());
        CHECK(engine == nullptr);
    }
}

TEST_SUITE("Salix::core::EngineBatch4847") {
    TEST_CASE("batch4847 invalid engine mode value roundtrips without altering scalar defaults") {
        Salix::Engine engine;
        const auto invalid = static_cast<Salix::EngineMode>(4847);
        engine.set_mode(invalid);
        CHECK(engine.get_mode() == invalid);
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch4847 smallest positive finite time scale is preserved verbatim") {
        Salix::Engine engine;
        const float value = (std::numeric_limits<float>::denorm_min)();
        engine.set_time_scale(value);
        CHECK(engine.get_time_scale() == value);
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4847 smallest negative finite time scale is preserved verbatim") {
        Salix::Engine engine;
        const float value = -(std::numeric_limits<float>::denorm_min)();
        engine.set_time_scale(value);
        CHECK(engine.get_time_scale() == value);
        CHECK(std::signbit(engine.get_time_scale()));
    }

    TEST_CASE("batch4847 running toggles leave all preinitialization subsystem getters null") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.is_running(false);
        engine.is_running(true);
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
    }

    TEST_CASE("batch4847 null state stack operations preserve invalid mode and diagnostic scale") {
        Salix::Engine engine;
        const auto invalid = static_cast<Salix::EngineMode>(-4847);
        engine.set_mode(invalid);
        engine.set_time_scale(-48.47f);
        engine.push_state(nullptr);
        engine.change_state(nullptr);
        engine.pop_state();
        CHECK(engine.get_mode() == invalid);
        CHECK(engine.get_time_scale() == doctest::Approx(-48.47f));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4847 alternating mode and time scale writes retain latest value in each channel") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Launch);
        engine.set_time_scale(4.0f);
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(-8.0f);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(0.4847f);
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(0.4847f));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch4847 three engines retain independent running mode and scale combinations") {
        Salix::Engine a, b, c;
        a.is_running(true);
        a.set_mode(Salix::EngineMode::Launch);
        a.set_time_scale(1.25f);
        b.set_mode(Salix::EngineMode::Editor);
        b.set_time_scale(-2.5f);
        c.is_running(true);
        c.set_mode(Salix::EngineMode::Debug);
        c.set_time_scale(5.0f);
        CHECK(a.is_running()); CHECK(a.get_mode() == Salix::EngineMode::Launch); CHECK(a.get_time_scale() == doctest::Approx(1.25f));
        CHECK_FALSE(b.is_running()); CHECK(b.get_mode() == Salix::EngineMode::Editor); CHECK(b.get_time_scale() == doctest::Approx(-2.5f));
        CHECK(c.is_running()); CHECK(c.get_mode() == Salix::EngineMode::Debug); CHECK(c.get_time_scale() == doctest::Approx(5.0f));
    }

    TEST_CASE("batch4847 EngineInterface repeated running writes return the newly stored value") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        CHECK_FALSE(engine->is_running(false));
        CHECK(engine->is_running(true));
        CHECK(engine->is_running(true));
        CHECK_FALSE(engine->is_running(false));
        CHECK_FALSE(engine->is_running());
    }

    TEST_CASE("batch4847 EngineInterface arbitrary mode write does not affect configured time scale") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        engine->set_time_scale(48.47f);
        const auto invalid = static_cast<Salix::EngineMode>(484700);
        engine->set_mode(invalid);
        CHECK(engine->get_mode() == invalid);
        CHECK(engine->get_time_scale() == doctest::Approx(48.47f));
    }

    TEST_CASE("batch4847 scalar configuration remains valid after no op stack calls in every order") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(-0.25f);
        engine.pop_state();
        engine.push_state(nullptr);
        engine.pop_state();
        engine.change_state(nullptr);
        engine.push_state(nullptr);
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(-0.25f));
    }
}

TEST_SUITE("Salix::core::EngineBatch5247") {
    TEST_CASE("batch5247 fresh engine exposes complete scalar and subsystem defaults together") {
        Salix::Engine engine;
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch5247 running transitions preserve configured mode and time scale") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(5.247f);
        engine.is_running(true);
        engine.is_running(false);
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(5.247f));
    }

    TEST_CASE("batch5247 mode transitions preserve running state and diagnostic scale") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_time_scale(-5.247f);
        engine.set_mode(Salix::EngineMode::Launch);
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_mode(Salix::EngineMode::Debug);
        CHECK(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(-5.247f));
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch5247 time scale replacements preserve running state and selected mode") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(10.0f);
        engine.set_time_scale(0.0f);
        engine.set_time_scale(-2.5f);
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(-2.5f));
    }

    TEST_CASE("batch5247 invalid engine mode value is stored without altering other scalar channels") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_time_scale(2.0f);
        const auto invalid = static_cast<Salix::EngineMode>(5247);
        engine.set_mode(invalid);
        CHECK(engine.get_mode() == invalid);
        CHECK(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(2.0f));
    }

    TEST_CASE("batch5247 negative zero time scale preserves sign and mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(-0.0f);
        CHECK(std::signbit(engine.get_time_scale()));
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch5247 null state operations preserve running configured engine") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(0.5247f);
        engine.push_state(nullptr);
        engine.change_state(nullptr);
        engine.pop_state();
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(0.5247f));
    }

    TEST_CASE("batch5247 scalar mutations never materialize preinitialized subsystems") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Editor);
        engine.set_time_scale(3.0f);
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch5247 EngineInterface dispatch preserves independent scalar channels") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        engine->set_mode(Salix::EngineMode::Launch);
        engine->set_time_scale(7.25f);
        CHECK(engine->is_running(true));
        CHECK(engine->get_mode() == Salix::EngineMode::Launch);
        CHECK(engine->get_time_scale() == doctest::Approx(7.25f));
        CHECK(engine->is_running());
    }

    TEST_CASE("batch5247 two fresh engines remain independent after mutating only one") {
        Salix::Engine first, second;
        first.is_running(true);
        first.set_mode(Salix::EngineMode::Debug);
        first.set_time_scale(-7.0f);
        CHECK(first.is_running());
        CHECK(first.get_mode() == Salix::EngineMode::Debug);
        CHECK(first.get_time_scale() == doctest::Approx(-7.0f));
        CHECK_FALSE(second.is_running());
        CHECK(second.get_mode() == Salix::EngineMode::None);
        CHECK(second.get_time_scale() == doctest::Approx(1.0f));
    }
}

TEST_SUITE("Salix::core::EngineBatch5747") {
    TEST_CASE("batch5747 fresh engine exposes stopped neutral unit scale state") {
        Salix::Engine engine;
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK(engine.get_time_scale() == doctest::Approx(1.0f));
    }

    TEST_CASE("batch5747 preinitialized subsystem getters remain null") {
        Salix::Engine engine;
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch5747 running state follows repeated explicit writes") {
        Salix::Engine engine;
        CHECK(engine.is_running(true));
        CHECK(engine.is_running(true));
        CHECK_FALSE(engine.is_running(false));
        CHECK(engine.is_running(true));
        CHECK(engine.is_running());
    }

    TEST_CASE("batch5747 mode transitions can return to neutral") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Launch);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_mode(Salix::EngineMode::None);
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("batch5747 mode transitions preserve running state and time scale") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_time_scale(-5.747f);
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_mode(Salix::EngineMode::Editor);
        CHECK(engine.is_running());
        CHECK(engine.get_time_scale() == doctest::Approx(-5.747f));
    }

    TEST_CASE("batch5747 time scale replacements preserve selected mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(0.0f);
        engine.set_time_scale(0.5747f);
        engine.set_time_scale(-57.47f);
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.get_time_scale() == doctest::Approx(-57.47f));
    }

    TEST_CASE("batch5747 state stack no ops preserve preinitialized scalar state") {
        Salix::Engine engine;
        engine.is_running(true);
        engine.set_mode(Salix::EngineMode::Game);
        engine.set_time_scale(5.747f);
        engine.push_state(nullptr);
        engine.change_state(nullptr);
        engine.pop_state();
        CHECK(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(5.747f));
    }

    TEST_CASE("batch5747 state stack no ops never materialize core subsystems") {
        Salix::Engine engine;
        engine.push_state(nullptr);
        engine.pop_state();
        engine.change_state(nullptr);
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch5747 separate engines retain independent scalar state") {
        Salix::Engine first, second;
        first.is_running(true);
        first.set_mode(Salix::EngineMode::Launch);
        first.set_time_scale(2.0f);
        second.set_mode(Salix::EngineMode::Options);
        second.set_time_scale(-3.0f);
        CHECK(first.is_running());
        CHECK(first.get_mode() == Salix::EngineMode::Launch);
        CHECK(first.get_time_scale() == doctest::Approx(2.0f));
        CHECK_FALSE(second.is_running());
        CHECK(second.get_mode() == Salix::EngineMode::Options);
        CHECK(second.get_time_scale() == doctest::Approx(-3.0f));
    }

    TEST_CASE("batch5747 EngineInterface dispatch preserves scalar independence") {
        std::unique_ptr<Salix::EngineInterface> engine = std::make_unique<Salix::Engine>();
        REQUIRE(engine != nullptr);
        engine->is_running(true);
        engine->set_mode(Salix::EngineMode::Editor);
        engine->set_time_scale(1.5747f);
        CHECK(engine->is_running());
        CHECK(engine->get_mode() == Salix::EngineMode::Editor);
        CHECK(engine->get_time_scale() == doctest::Approx(1.5747f));
    }
}

#include <Salix/core/ApplicationConfig.h>
#include <SDL.h>
#include <cstdlib>
#include <string>

namespace {
struct Batch7147InvalidRendererRun {
    Salix::Engine engine;
    Salix::ApplicationConfig config;
    bool initialized = true;

    explicit Batch7147InvalidRendererRun(Salix::RendererType type) {
        config.renderer_type = type;
        config.gui_type = Salix::GuiType::None;
        initialized = engine.initialize(config);
    }

    ~Batch7147InvalidRendererRun() {
        engine.shutdown();
    }
};

struct Batch7147VideoDriverGuard {
    bool had_old = false;
    std::string old_value;

    Batch7147VideoDriverGuard() {
        if (const char* existing = SDL_getenv("SDL_VIDEODRIVER")) {
            had_old = true;
            old_value = existing;
        }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "__batch7147_invalid_driver__", 1);
    }

    ~Batch7147VideoDriverGuard() {
        SDL_Quit();
        if (had_old) {
            SDL_setenv("SDL_VIDEODRIVER", old_value.c_str(), 1);
        } else {
            _putenv_s("SDL_VIDEODRIVER", "");
        }
    }
};
}

TEST_SUITE("Salix::core::EngineBatch7147") {
    TEST_CASE("batch7147 renderer None reaches unsupported renderer initialization failure") {
        Batch7147InvalidRendererRun run(Salix::RendererType::None);
        CHECK_FALSE(run.initialized);
        CHECK_FALSE(run.engine.is_running());
    }

    TEST_CASE("batch7147 Vulkan renderer reaches unsupported renderer initialization failure") {
        Batch7147InvalidRendererRun run(Salix::RendererType::Vulkan);
        CHECK_FALSE(run.initialized);
        CHECK(run.engine.get_renderer() == nullptr);
    }

    TEST_CASE("batch7147 DirectX renderer reaches unsupported renderer initialization failure") {
        Batch7147InvalidRendererRun run(Salix::RendererType::DirectX);
        CHECK_FALSE(run.initialized);
        CHECK(run.engine.get_asset_manager() == nullptr);
    }

    TEST_CASE("batch7147 arbitrary renderer value reaches unsupported renderer failure") {
        Batch7147InvalidRendererRun run(static_cast<Salix::RendererType>(7147));
        CHECK_FALSE(run.initialized);
        CHECK(run.engine.get_event_manager() == nullptr);
    }

    TEST_CASE("batch7147 unsupported renderer never materializes the input manager") {
        Batch7147InvalidRendererRun run(Salix::RendererType::None);
        CHECK_FALSE(run.initialized);
        CHECK(run.engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch7147 unsupported renderer leaves engine stopped after failed initialize") {
        Batch7147InvalidRendererRun run(Salix::RendererType::Vulkan);
        CHECK_FALSE(run.initialized);
        CHECK_FALSE(run.engine.is_running());
    }

    TEST_CASE("batch7147 unsupported renderer preserves caller renderer selection") {
        Batch7147InvalidRendererRun run(Salix::RendererType::DirectX);
        CHECK(run.config.renderer_type == Salix::RendererType::DirectX);
        CHECK_FALSE(run.initialized);
    }

    TEST_CASE("batch7147 failed unsupported renderer initialization supports explicit shutdown") {
        Batch7147InvalidRendererRun run(Salix::RendererType::None);
        REQUIRE_FALSE(run.initialized);
        CHECK_NOTHROW(run.engine.shutdown());
        CHECK(run.engine.get_renderer() == nullptr);
    }

    TEST_CASE("batch7147 failed unsupported renderer initialization preserves preconfigured time scale") {
        Salix::Engine engine;
        engine.set_time_scale(7.147f);
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_time_scale() == doctest::Approx(7.147f));
        engine.shutdown();
    }

    TEST_CASE("batch7147 failed unsupported renderer initialization preserves preconfigured engine mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        engine.shutdown();
    }

    TEST_CASE("batch7147 invalid SDL video driver reaches SDL initialization failure") {
        Batch7147VideoDriverGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        CHECK_FALSE(engine.initialize(config));
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch7147 invalid SDL video driver leaves renderer null") {
        Batch7147VideoDriverGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_renderer() == nullptr);
    }

    TEST_CASE("batch7147 invalid SDL video driver leaves asset manager null") {
        Batch7147VideoDriverGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_asset_manager() == nullptr);
    }

    TEST_CASE("batch7147 invalid SDL video driver leaves event manager null") {
        Batch7147VideoDriverGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_event_manager() == nullptr);
    }

    TEST_CASE("batch7147 invalid SDL video driver preserves scalar engine state") {
        Batch7147VideoDriverGuard driver;
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Options);
        engine.set_time_scale(-7.147f);
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        REQUIRE_FALSE(engine.initialize(config));
        CHECK(engine.get_mode() == Salix::EngineMode::Options);
        CHECK(engine.get_time_scale() == doctest::Approx(-7.147f));
    }

    TEST_CASE("batch7147 no factory switch with None state fails creation without changing mode") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        CHECK_NOTHROW(engine.switch_state(Salix::AppStateType::None));
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
    }

    TEST_CASE("batch7147 no factory switch with Game state fails creation safely before initialization") {
        Salix::Engine engine;
        CHECK_NOTHROW(engine.switch_state(Salix::AppStateType::Game));
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch7147 arbitrary app state takes no factory default branch safely") {
        Salix::Engine engine;
        const auto invalid = static_cast<Salix::AppStateType>(7147);
        CHECK_NOTHROW(engine.switch_state(invalid));
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("batch7147 no factory state switch preserves configured time scale") {
        Salix::Engine engine;
        engine.set_time_scale(71.47f);
        engine.switch_state(Salix::AppStateType::Game);
        CHECK(engine.get_time_scale() == doctest::Approx(71.47f));
    }

    TEST_CASE("batch7147 repeated no factory state switches remain safe") {
        Salix::Engine engine;
        engine.switch_state(Salix::AppStateType::None);
        engine.switch_state(Salix::AppStateType::Game);
        engine.switch_state(static_cast<Salix::AppStateType>(-7147));
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_mode() == Salix::EngineMode::None);
    }

    TEST_CASE("batch7147 shutdown is safe before initialization") {
        Salix::Engine engine;
        CHECK_NOTHROW(engine.shutdown());
        CHECK_FALSE(engine.is_running());
    }

    TEST_CASE("batch7147 repeated shutdown remains safe before initialization") {
        Salix::Engine engine;
        CHECK_NOTHROW(engine.shutdown());
        CHECK_NOTHROW(engine.shutdown());
        CHECK(engine.get_renderer() == nullptr);
    }

    TEST_CASE("batch7147 preinitialization shutdown preserves diagnostic mode and scale") {
        Salix::Engine engine;
        engine.set_mode(Salix::EngineMode::Debug);
        engine.set_time_scale(0.7147f);
        engine.shutdown();
        CHECK(engine.get_mode() == Salix::EngineMode::Debug);
        CHECK(engine.get_time_scale() == doctest::Approx(0.7147f));
    }

    TEST_CASE("batch7147 preinitialization shutdown leaves all exposed subsystems null") {
        Salix::Engine engine;
        engine.shutdown();
        CHECK(engine.get_renderer() == nullptr);
        CHECK(engine.get_asset_manager() == nullptr);
        CHECK(engine.get_event_manager() == nullptr);
        CHECK(engine.get_input_manager() == nullptr);
    }

    TEST_CASE("batch7147 no factory switch remains safe after preinitialization shutdown") {
        Salix::Engine engine;
        engine.shutdown();
        CHECK_NOTHROW(engine.switch_state(Salix::AppStateType::Game));
        CHECK(engine.get_mode() == Salix::EngineMode::None);
        CHECK_FALSE(engine.is_running());
    }
}

namespace {
struct Batch7478DummyVideoDriverGuard {
    bool had_video = false;
    bool had_render = false;
    std::string old_video;
    std::string old_render;

    Batch7478DummyVideoDriverGuard() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        if (const char* value = SDL_getenv("SDL_RENDER_DRIVER")) { had_render = true; old_render = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
    }

    ~Batch7478DummyVideoDriverGuard() {
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
        if (had_render) SDL_setenv("SDL_RENDER_DRIVER", old_render.c_str(), 1); else _putenv_s("SDL_RENDER_DRIVER", "");
    }
};

void batch7478_engine_renderer_attempt(
    Salix::RendererType renderer, int width, int height,
    const std::string& title, Salix::TimerType timer, int target_fps) {
    Batch7478DummyVideoDriverGuard driver;
    Salix::Engine engine;
    Salix::ApplicationConfig config;
    config.renderer_type = renderer;
    config.gui_type = Salix::GuiType::None;
    config.initial_state = Salix::AppStateType::None;
    config.timer_type = timer;
    config.target_fps = target_fps;
    config.window_config.width = width;
    config.window_config.height = height;
    config.window_config.title = title;
    const bool initialized = engine.initialize(config);
    CHECK_FALSE(initialized);
    CHECK(engine.get_renderer() != nullptr);
    CHECK_FALSE(engine.is_running());
    CHECK_NOTHROW(engine.shutdown());
}
}

TEST_SUITE("Salix::core::EngineBatch7478") {
    TEST_CASE("batch7478 SDL renderer attempt with default sized dummy window remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::SDL,1280,720,"Batch7478 SDL A",Salix::TimerType::SDL,60); }
    TEST_CASE("batch7478 SDL renderer attempt with tiny dummy window remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::SDL,1,1,"Batch7478 SDL B",Salix::TimerType::SDL,60); }
    TEST_CASE("batch7478 SDL renderer attempt with zero sized dummy window remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::SDL,0,0,"Batch7478 SDL C",Salix::TimerType::SDL,0); }
    TEST_CASE("batch7478 SDL renderer attempt with empty title remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::SDL,640,480,"",Salix::TimerType::Chrono,30); }
    TEST_CASE("batch7478 SDL renderer attempt preserves negative target fps failure safety") { batch7478_engine_renderer_attempt(Salix::RendererType::SDL,320,240,"Batch7478 SDL E",Salix::TimerType::Chrono,-1); }
    TEST_CASE("batch7478 OpenGL renderer attempt with default sized dummy window remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::OpenGL,1280,720,"Batch7478 GL A",Salix::TimerType::SDL,60); }
    TEST_CASE("batch7478 OpenGL renderer attempt with tiny dummy window remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::OpenGL,1,1,"Batch7478 GL B",Salix::TimerType::Chrono,120); }
    TEST_CASE("batch7478 OpenGL renderer attempt with zero width remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::OpenGL,0,480,"Batch7478 GL C",Salix::TimerType::SDL,60); }
    TEST_CASE("batch7478 OpenGL renderer attempt with zero height remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::OpenGL,640,0,"Batch7478 GL D",Salix::TimerType::Chrono,60); }
    TEST_CASE("batch7478 OpenGL renderer attempt with empty title and uncapped fps remains controlled") { batch7478_engine_renderer_attempt(Salix::RendererType::OpenGL,640,480,"",Salix::TimerType::Chrono,0); }
}

namespace {
struct Batch7578EngineDummyRun {
    Batch7478DummyVideoDriverGuard driver;
    Salix::Engine engine;
    bool initialized = true;

    Batch7578EngineDummyRun(Salix::RendererType renderer_type, int width = 640, int height = 480) {
        Salix::ApplicationConfig config;
        config.renderer_type = renderer_type;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::None;
        config.window_config.width = width;
        config.window_config.height = height;
        config.window_config.title = "Batch7578";
        initialized = engine.initialize(config);
    }
    ~Batch7578EngineDummyRun() { engine.shutdown(); }
};
}

TEST_SUITE("Salix::core::EngineBatch7578") {
    TEST_CASE("batch7578 failed SDL renderer exposes null owned window") {
        Batch7578EngineDummyRun run(Salix::RendererType::SDL);
        REQUIRE_FALSE(run.initialized); REQUIRE(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_renderer()->get_window() == nullptr);
    }
    TEST_CASE("batch7578 failed SDL renderer exposes null native handle") {
        Batch7578EngineDummyRun run(Salix::RendererType::SDL);
        REQUIRE(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_renderer()->get_native_handle() == nullptr);
    }
    TEST_CASE("batch7578 failed SDL renderer exposes null SDL GL context") {
        Batch7578EngineDummyRun run(Salix::RendererType::SDL);
        REQUIRE(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_renderer()->get_sdl_gl_context() == nullptr);
    }
    TEST_CASE("batch7578 failed SDL renderer exposes null SDL window") {
        Batch7578EngineDummyRun run(Salix::RendererType::SDL);
        REQUIRE(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_renderer()->get_sdl_window() == nullptr);
    }
    TEST_CASE("batch7578 failed SDL renderer clear color remains readable") {
        Batch7578EngineDummyRun run(Salix::RendererType::SDL);
        REQUIRE(run.engine.get_renderer() != nullptr);
        const auto color = run.engine.get_renderer()->get_clear_color();
        CHECK(std::isfinite(color.r));
    }
    TEST_CASE("batch7578 failed OpenGL renderer retains failed window wrapper") {
        Batch7578EngineDummyRun run(Salix::RendererType::OpenGL, 757, 578);
        REQUIRE_FALSE(run.initialized); REQUIRE(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_renderer()->get_window() != nullptr);
    }
    TEST_CASE("batch7578 failed OpenGL window retains requested width") {
        Batch7578EngineDummyRun run(Salix::RendererType::OpenGL, 757, 578);
        auto* window = run.engine.get_renderer()->get_window(); REQUIRE(window != nullptr);
        CHECK(window->get_width() == 757);
    }
    TEST_CASE("batch7578 failed OpenGL window retains requested height") {
        Batch7578EngineDummyRun run(Salix::RendererType::OpenGL, 757, 578);
        auto* window = run.engine.get_renderer()->get_window(); REQUIRE(window != nullptr);
        CHECK(window->get_height() == 578);
    }
    TEST_CASE("batch7578 failed OpenGL window query reports no native dimensions") {
        Batch7578EngineDummyRun run(Salix::RendererType::OpenGL, 757, 578);
        auto* window = run.engine.get_renderer()->get_window(); REQUIRE(window != nullptr);
        int width = -1, height = -1; window->query_dimensions(width, height);
        CHECK(width == 0); CHECK(height == 0);
    }
    TEST_CASE("batch7578 failed OpenGL window reports unit DPI scale") {
        Batch7578EngineDummyRun run(Salix::RendererType::OpenGL, 757, 578);
        auto* window = run.engine.get_renderer()->get_window(); REQUIRE(window != nullptr);
        CHECK(window->get_dpi_scale() == doctest::Approx(1.0f));
    }
}

namespace {
struct Batch7678WindowsVideoGuard {
    bool had_video = false;
    bool had_render = false;
    std::string old_video;
    std::string old_render;

    Batch7678WindowsVideoGuard() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        if (const char* value = SDL_getenv("SDL_RENDER_DRIVER")) { had_render = true; old_render = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "windows", 1);
        _putenv_s("SDL_RENDER_DRIVER", "");
    }

    ~Batch7678WindowsVideoGuard() {
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
        if (had_render) SDL_setenv("SDL_RENDER_DRIVER", old_render.c_str(), 1); else _putenv_s("SDL_RENDER_DRIVER", "");
    }
};

struct Batch7678NativeEngineRun {
    Batch7678WindowsVideoGuard driver;
    Salix::Engine engine;
    Salix::ApplicationConfig config;
    bool initialized = false;

    Batch7678NativeEngineRun(
        Salix::AppStateType state = Salix::AppStateType::Launch,
        Salix::TimerType timer = Salix::TimerType::SDL,
        Salix::GuiType gui = Salix::GuiType::None) {
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = gui;
        config.initial_state = state;
        config.timer_type = timer;
        config.target_fps = 60;
        config.window_config.title = "Batch7678 Native SDL";
        config.window_config.width = 96;
        config.window_config.height = 64;
        initialized = engine.initialize(config);
    }

    ~Batch7678NativeEngineRun() { engine.shutdown(); }
};
}

TEST_SUITE("Salix::core::EngineBatch7678") {
    TEST_CASE("batch7678 native SDL launch initialization reaches running engine") {
        Batch7678NativeEngineRun run;
        REQUIRE(run.initialized);
        CHECK(run.engine.is_running());
        CHECK(run.engine.get_mode() == Salix::EngineMode::Launch);
    }
    TEST_CASE("batch7678 native SDL initialization materializes core subsystems") {
        Batch7678NativeEngineRun run;
        REQUIRE(run.initialized);
        CHECK(run.engine.get_renderer() != nullptr);
        CHECK(run.engine.get_asset_manager() != nullptr);
        CHECK(run.engine.get_event_manager() != nullptr);
        CHECK(run.engine.get_input_manager() != nullptr);
    }
    TEST_CASE("batch7678 native SDL chrono timer initialization reaches launch state") {
        Batch7678NativeEngineRun run(Salix::AppStateType::Launch, Salix::TimerType::Chrono);
        REQUIRE(run.initialized);
        CHECK(run.engine.get_mode() == Salix::EngineMode::Launch);
    }
    TEST_CASE("batch7678 native SDL options initialization reaches options mode") {
        Batch7678NativeEngineRun run(Salix::AppStateType::Options, Salix::TimerType::SDL);
        REQUIRE(run.initialized);
        CHECK(run.engine.get_mode() == Salix::EngineMode::Options);
    }
    TEST_CASE("batch7678 native SDL make context exposes initialized renderer and window") {
        Batch7678NativeEngineRun run;
        REQUIRE(run.initialized);
        const Salix::InitContext context = run.engine.make_context();
        CHECK(context.engine == &run.engine);
        CHECK(context.renderer != nullptr);
        CHECK(context.window != nullptr);
        CHECK(context.event_manager != nullptr);
        CHECK(context.project_manager != nullptr);
    }
    TEST_CASE("batch7678 native SDL state switch from launch to options remains live") {
        Batch7678NativeEngineRun run;
        REQUIRE(run.initialized);
        run.engine.switch_state(Salix::AppStateType::Options);
        CHECK(run.engine.get_mode() == Salix::EngineMode::Options);
        CHECK(run.engine.is_running());
    }
    TEST_CASE("batch7678 native SDL state switch back to launch remains live") {
        Batch7678NativeEngineRun run(Salix::AppStateType::Options);
        REQUIRE(run.initialized);
        run.engine.switch_state(Salix::AppStateType::Launch);
        CHECK(run.engine.get_mode() == Salix::EngineMode::Launch);
        CHECK(run.engine.is_running());
    }
    TEST_CASE("batch7678 native SDL invalid timer reaches post renderer timer rejection") {
        Batch7678WindowsVideoGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::Launch;
        config.timer_type = static_cast<Salix::TimerType>(7678);
        config.window_config.width = 96;
        config.window_config.height = 64;
        CHECK_FALSE(engine.initialize(config));
        CHECK(engine.get_renderer() != nullptr);
        engine.shutdown();
    }
    TEST_CASE("batch7678 native SDL unsupported GUI reaches post DLL GUI rejection") {
        Batch7678WindowsVideoGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = static_cast<Salix::GuiType>(7678);
        config.initial_state = Salix::AppStateType::Launch;
        config.window_config.width = 96;
        config.window_config.height = 64;
        CHECK_FALSE(engine.initialize(config));
        CHECK(engine.get_renderer() != nullptr);
        engine.shutdown();
    }
    TEST_CASE("batch7678 native SDL None initial state reaches final state creation rejection") {
        Batch7678WindowsVideoGuard driver;
        Salix::Engine engine;
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        config.gui_type = Salix::GuiType::None;
        config.initial_state = Salix::AppStateType::None;
        config.window_config.width = 96;
        config.window_config.height = 64;
        CHECK_FALSE(engine.initialize(config));
        CHECK_FALSE(engine.is_running());
        CHECK(engine.get_renderer() != nullptr);
        engine.shutdown();
    }
}

