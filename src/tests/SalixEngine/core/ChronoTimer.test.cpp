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
    }
}