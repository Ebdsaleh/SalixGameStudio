#include <doctest.h>
#include <Salix/core/ITimer.h>
#include <Tests/SalixEngine/mocking/core/MockITimer.h>
#include <chrono>
#include <memory>

namespace {
class Batch1347DefaultSleepTimer : public Salix::ITimer {
public:
    void set_target_fps(int fps) override { target_fps = fps; }
    void tick_start() override { ++start_calls; }
    void tick_end() override { ++end_calls; }
    float get_delta_time() const override { return delta_time; }

    int target_fps = 0;
    int start_calls = 0;
    int end_calls = 0;
    float delta_time = 0.0f;
};
}

TEST_SUITE("Salix::core::ITimerBatch1347") {
    TEST_CASE("timer interface default sleep duration is zero milliseconds") {
        Batch1347DefaultSleepTimer timer;
        Salix::ITimer* interface_ptr = &timer;
        CHECK(interface_ptr->calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("timer interface dispatches target FPS through base pointer") {
        MockITimer timer;
        Salix::ITimer* interface_ptr = &timer;
        interface_ptr->set_target_fps(60);
        CHECK(timer.target_fps == 60);
        CHECK(timer.set_target_fps_calls == 1);
    }

    TEST_CASE("timer interface preserves uncapped zero target FPS") {
        MockITimer timer;
        Salix::ITimer* interface_ptr = &timer;
        interface_ptr->set_target_fps(0);
        CHECK(timer.target_fps == 0);
    }

    TEST_CASE("timer interface dispatches frame start exactly once per call") {
        MockITimer timer;
        Salix::ITimer* interface_ptr = &timer;
        interface_ptr->tick_start();
        interface_ptr->tick_start();
        CHECK(timer.tick_start_calls == 2);
        CHECK(timer.tick_end_calls == 0);
    }

    TEST_CASE("timer interface dispatches frame end independently from frame start") {
        MockITimer timer;
        Salix::ITimer* interface_ptr = &timer;
        interface_ptr->tick_end();
        CHECK(timer.tick_end_calls == 1);
        CHECK(timer.tick_start_calls == 0);
    }

    TEST_CASE("timer interface exposes mock delta time without conversion") {
        MockITimer timer;
        timer.delta_time = 0.0166667f;
        Salix::ITimer* interface_ptr = &timer;
        CHECK(interface_ptr->get_delta_time() == doctest::Approx(0.0166667f));
    }

    TEST_CASE("timer sleep duration override preserves fractional milliseconds") {
        MockITimer timer;
        timer.sleep_duration = std::chrono::duration<float, std::milli>(8.25f);
        Salix::ITimer* interface_ptr = &timer;
        CHECK(interface_ptr->calculate_sleep_duration().count() == doctest::Approx(8.25f));
        CHECK(timer.sleep_duration_calls == 1);
    }

    TEST_CASE("timer target changes do not implicitly tick frame lifecycle") {
        MockITimer timer;
        Salix::ITimer* interface_ptr = &timer;
        interface_ptr->set_target_fps(144);
        interface_ptr->set_target_fps(30);
        CHECK(timer.target_fps == 30);
        CHECK(timer.set_target_fps_calls == 2);
        CHECK(timer.tick_start_calls == 0);
        CHECK(timer.tick_end_calls == 0);
    }

    TEST_CASE("two timer implementations maintain independent frame accounting") {
        MockITimer first;
        MockITimer second;
        Salix::ITimer* a = &first;
        Salix::ITimer* b = &second;
        a->tick_start();
        a->tick_end();
        b->tick_start();
        CHECK(first.tick_start_calls == 1);
        CHECK(first.tick_end_calls == 1);
        CHECK(second.tick_start_calls == 1);
        CHECK(second.tick_end_calls == 0);
    }

    TEST_CASE("timer interface supports polymorphic destruction") {
        std::unique_ptr<Salix::ITimer> timer = std::make_unique<MockITimer>();
        REQUIRE(timer != nullptr);
        timer->set_target_fps(120);
        timer->tick_start();
        CHECK(timer->get_delta_time() == doctest::Approx(0.0f));
    }
}

TEST_SUITE("Salix::core::ITimerBatch6447") {
    TEST_CASE("batch6447 negative target FPS is transported exactly through interface") {
        MockITimer timer;
        Salix::ITimer& interface = timer;
        interface.set_target_fps(-6447);
        CHECK(timer.target_fps == -6447);
        CHECK(timer.set_target_fps_calls == 1);
    }

    TEST_CASE("batch6447 repeated target assignment increments call count") {
        MockITimer timer;
        Salix::ITimer& interface = timer;
        interface.set_target_fps(60);
        interface.set_target_fps(60);
        CHECK(timer.target_fps == 60);
        CHECK(timer.set_target_fps_calls == 2);
    }

    TEST_CASE("batch6447 frame end may precede frame start in mock accounting") {
        MockITimer timer;
        Salix::ITimer& interface = timer;
        interface.tick_end();
        interface.tick_start();
        CHECK(timer.tick_end_calls == 1);
        CHECK(timer.tick_start_calls == 1);
    }

    TEST_CASE("batch6447 multiple frame pairs preserve independent start and end counts") {
        MockITimer timer;
        Salix::ITimer& interface = timer;
        interface.tick_start();
        interface.tick_end();
        interface.tick_start();
        interface.tick_end();
        CHECK(timer.tick_start_calls == 2);
        CHECK(timer.tick_end_calls == 2);
    }

    TEST_CASE("batch6447 negative delta value is exposed without normalization") {
        MockITimer timer;
        timer.delta_time = -6.447f;
        Salix::ITimer& interface = timer;
        CHECK(interface.get_delta_time() == doctest::Approx(-6.447f));
    }

    TEST_CASE("batch6447 negative sleep duration override is transported exactly") {
        MockITimer timer;
        timer.sleep_duration = std::chrono::duration<float, std::milli>(-6.447f);
        Salix::ITimer& interface = timer;
        CHECK(interface.calculate_sleep_duration().count() == doctest::Approx(-6.447f));
        CHECK(timer.sleep_duration_calls == 1);
    }

    TEST_CASE("batch6447 repeated sleep queries increment accounting without changing value") {
        MockITimer timer;
        timer.sleep_duration = std::chrono::duration<float, std::milli>(4.25f);
        Salix::ITimer& interface = timer;
        CHECK(interface.calculate_sleep_duration().count() == doctest::Approx(4.25f));
        CHECK(interface.calculate_sleep_duration().count() == doctest::Approx(4.25f));
        CHECK(timer.sleep_duration_calls == 2);
    }

    TEST_CASE("batch6447 default sleep implementation remains zero after lifecycle calls") {
        Batch1347DefaultSleepTimer timer;
        Salix::ITimer& interface = timer;
        interface.tick_start();
        interface.tick_end();
        CHECK(interface.calculate_sleep_duration().count() == doctest::Approx(0.0f));
        CHECK(timer.start_calls == 1);
        CHECK(timer.end_calls == 1);
    }

    TEST_CASE("batch6447 target assignment does not alter configured mock sleep duration") {
        MockITimer timer;
        timer.sleep_duration = std::chrono::duration<float, std::milli>(8.5f);
        timer.set_target_fps(120);
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(8.5f));
    }

    TEST_CASE("batch6447 polymorphic timer ownership preserves configured delta") {
        auto concrete = std::make_unique<MockITimer>();
        concrete->delta_time = 0.6447f;
        std::unique_ptr<Salix::ITimer> timer = std::move(concrete);
        CHECK(timer->get_delta_time() == doctest::Approx(0.6447f));
    }
}
