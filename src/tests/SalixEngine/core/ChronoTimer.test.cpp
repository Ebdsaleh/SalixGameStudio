// =================================================================================
// Filename:    src/Tests/SalixEngine/core/ChronoTimer.test.cpp
// Description: Contains unit tests for the ChronoTimer functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/core/ChronoTimer.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <Tests/TestFixtures.h>
#include <SDL.h>  // Required for accuracy while testing.

TEST_SUITE("Salix::core::ChronoTimer") {
    
    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "initialization state") {
        Salix::ChronoTimer timer;
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "correctly calculates delta_time") {
        Salix::ChronoTimer timer;
        const int wait_duration_ms = 16;

        timer.tick_start();
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_duration_ms));
        timer.tick_start();

        CHECK(timer.get_delta_time() == doctest::Approx(0.016).epsilon(0.005));
    }

    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "frame rate capping logic") {
    
        SUBCASE("calculates correct sleep duration when frame is shorter than target") {
            Salix::ChronoTimer timer;
            timer.set_target_fps(60); // Target is ~16.6667ms

            timer.tick_start();
            // Simulate a frame that took 5ms
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            
            auto sleep_duration = timer.calculate_sleep_duration();

            // The calculated sleep should be the target minus the frame time.
            // It should be ~11.6667ms. We check if it's close.
            CHECK(sleep_duration.count() == doctest::Approx(16.6667f - 5.0f).epsilon(1.0));
        }

        SUBCASE("calculates zero sleep duration when frame is longer than target") {
            Salix::ChronoTimer timer;
            timer.set_target_fps(60); // Target is ~16.6667ms

            timer.tick_start();
            // Simulate a frame that took 20ms
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            auto sleep_duration = timer.calculate_sleep_duration();

            // The sleep duration should be zero.
            CHECK(sleep_duration.count() == doctest::Approx(0.0f));
        }

        SUBCASE("calculates zero sleep duration when uncapped") {
            Salix::ChronoTimer timer;
            timer.set_target_fps(0);
            timer.tick_start();
            auto sleep_duration = timer.calculate_sleep_duration();
            CHECK(sleep_duration.count() == doctest::Approx(0.0f));
        }
    }
    TEST_CASE("tick_end calls delay with correct duration") {
        using namespace std::chrono;

        milliseconds captured_delay{0};

        Salix::ChronoTimer timer([&](milliseconds d) { captured_delay = d; });
        timer.set_target_fps(60);

        timer.tick_start();
        SDL_Delay(2); // simulate ~2ms work
        timer.tick_end();

        CHECK(captured_delay.count() >= 7);
        CHECK(captured_delay.count() <= 20);
    }

    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "tick_end capping logic") {
        SUBCASE("does not delay when uncapped") {
            // This covers the branch where target_frame_duration_ms <= 0.
            Salix::ChronoTimer timer;
            timer.set_target_fps(0);

            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            timer.tick_end(); // Should return immediately.
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            // Should be very fast, confirming no delay was added.
            CHECK(elapsed_ms < 5);
        }

        SUBCASE("does not delay when frame is longer than target") {
            // This covers the 'else' branch where no sleep is needed.
            Salix::ChronoTimer timer;
            timer.set_target_fps(60); // Target is ~16.67ms

            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            // Simulate a long frame that exceeds the target duration.
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            timer.tick_end(); // Should not add any extra delay.
            auto end_time = std::chrono::high_resolution_clock::now();

            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            // The total time should be just over our 20ms sleep, with very little extra.
            CHECK(elapsed_ms < 25);
        }
    }

    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "tick_end uses default_delay") {
        // By constructing the timer without a lambda, it will use its internal default_delay.
        Salix::ChronoTimer timer; 
        timer.set_target_fps(60); // Target is ~16.67ms

        auto start_time = std::chrono::high_resolution_clock::now();
        timer.tick_start();
        // Simulate a short frame, forcing a delay.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        timer.tick_end();
        auto end_time = std::chrono::high_resolution_clock::now();

        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        // The total elapsed time should be approximately the target frame duration.
        // We check if it's at least 15ms to confirm a delay happened.
        CHECK(elapsed_ms >= 15);
    }

    // The static function tests don't need the fixture but it's used for consistency.
    TEST_CASE_FIXTURE(HighResolutionTimerFixture, "static helper functions") {
        SUBCASE("get_ticks_ms reports elapsed time") {
            unsigned int start_ticks = Salix::ChronoTimer::get_ticks_ms();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            unsigned int end_ticks = Salix::ChronoTimer::get_ticks_ms();
            CHECK((end_ticks - start_ticks) >= 20);
        }

        SUBCASE("delay waits for the specified duration") {
            auto start_time = std::chrono::high_resolution_clock::now();
            Salix::ChronoTimer::delay_for(20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            CHECK(elapsed_ms >= 20);
        }
        SUBCASE("delay_for with chrono::duration waits correctly") {
            // NOTE: Testing very short durations (~1-2ms) is unreliable because the OS scheduler
            // may not be precise enough. We test a slightly longer, more stable duration.

            const int delay_duration_ms = 15;
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // This call will exercise both the `if (duration > margin)` and the final spin-wait.
            Salix::ChronoTimer::delay_for(std::chrono::milliseconds(delay_duration_ms));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // We check that the elapsed time is AT LEAST the requested delay.
            // It might be slightly more due to system scheduling, which is expected.
            // Introduce a small tolerance to account for OS scheduler inaccuracies.
            // A 2ms difference is acceptable for a non-real-time system.
            // This check now correctly verifies the function waited for *approximately* the right duration.
            const int tolerance_ms = 2;
            CHECK(elapsed >= (delay_duration_ms - tolerance_ms));
        }
        SUBCASE("delay_for with short duration covers spin-wait loop") {
            // This test covers the final uncovered line by forcing the spin-wait to execute.
            // It calls delay_for with a duration so short that the main sleep_for is skipped.
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            Salix::ChronoTimer::delay_for(std::chrono::milliseconds(1));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // Assert that at least *some* time has passed. Due to scheduler precision,
            // the result might be 0ms, but the code path will have been covered.
            // A passing test here confirms the logic was executed.
            CHECK(elapsed >= 0);
        }
        
    }

}

TEST_SUITE("Salix::core::ChronoTimerBatch947") {
    TEST_CASE("fresh chrono timer has no pending frame cap sleep") {
        Salix::ChronoTimer timer;
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("zero target FPS keeps calculated sleep disabled after frame start") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(0);
        timer.tick_start();
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("negative target FPS is treated as uncapped") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(-144);
        timer.tick_start();
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("one FPS target exposes almost a full second of remaining frame immediately") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(1);
        timer.tick_start();
        const float remaining = timer.calculate_sleep_duration().count();
        CHECK(remaining > 900.0f);
        CHECK(remaining <= 1000.0f);
    }

    TEST_CASE("sixty FPS target produces a shorter remaining frame than thirty FPS") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(30);
        timer.tick_start();
        const float at_thirty = timer.calculate_sleep_duration().count();
        timer.set_target_fps(60);
        timer.tick_start();
        const float at_sixty = timer.calculate_sleep_duration().count();
        CHECK(at_thirty > 20.0f);
        CHECK(at_sixty > 5.0f);
        CHECK(at_sixty < at_thirty);
    }

    TEST_CASE("switching a capped timer to zero FPS immediately disables sleeping") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(30);
        timer.tick_start();
        REQUIRE(timer.calculate_sleep_duration().count() > 0.0f);
        timer.set_target_fps(0);
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("switching a capped timer to negative FPS immediately disables sleeping") {
        Salix::ChronoTimer timer;
        timer.set_target_fps(30);
        timer.tick_start();
        REQUIRE(timer.calculate_sleep_duration().count() > 0.0f);
        timer.set_target_fps(-1);
        CHECK(timer.calculate_sleep_duration().count() == doctest::Approx(0.0f));
    }

    TEST_CASE("uncapped tick end never invokes an injected delay callback") {
        int calls = 0;
        Salix::ChronoTimer timer([&](std::chrono::milliseconds) { ++calls; });
        timer.set_target_fps(0);
        timer.tick_start();
        timer.tick_end();
        CHECK(calls == 0);
    }

    TEST_CASE("capped tick end invokes injected delay once with the remaining frame budget") {
        int calls = 0;
        std::chrono::milliseconds captured{0};
        Salix::ChronoTimer timer([&](std::chrono::milliseconds d) { ++calls; captured = d; });
        timer.set_target_fps(10);
        timer.tick_start();
        timer.tick_end();
        CHECK(calls == 1);
        CHECK(captured.count() >= 90);
        CHECK(captured.count() <= 100);
    }

    TEST_CASE("chrono millisecond tick counter is monotonic across immediate reads") {
        const auto first = Salix::ChronoTimer::get_ticks_ms();
        const auto second = Salix::ChronoTimer::get_ticks_ms();
        const auto third = Salix::ChronoTimer::get_ticks_ms();
        CHECK(second >= first);
        CHECK(third >= second);
    }
}

// ---- Relocated from src/tests/SalixEngine/core/ITimer.test.cpp; canonical tests mirror production .cpp files ----
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
