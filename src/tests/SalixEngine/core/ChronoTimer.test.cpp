// =================================================================================
// Filename:    src/Tests/SalixEngine/core/ChronoTimer.test.cpp
// Description: Contains unit tests for the ChronoTimer functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/core/ChronoTimer.h>
#include <thread>
#include <chrono>

// INCLUDES FOR THE FIXTURE
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib") // Link against the required library.

// This fixture sets a high timer resolution for the duration of each test case.
struct TimerResolutionFixture {
    TimerResolutionFixture() {
        // Request 1ms timer resolution before the test runs.
        timeBeginPeriod(1);
    }
    ~TimerResolutionFixture() {
        // Release the high-resolution timer after the test finishes.
        timeEndPeriod(1);
    }
};

TEST_SUITE("Salix::core::ChronoTimer") {
    
    TEST_CASE_FIXTURE(TimerResolutionFixture, "initialization state") {
        Salix::ChronoTimer timer;
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(TimerResolutionFixture, "correctly calculates delta_time") {
        Salix::ChronoTimer timer;
        const int wait_duration_ms = 16;

        timer.tick_start();
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_duration_ms));
        timer.tick_start();

        CHECK(timer.get_delta_time() == doctest::Approx(0.016).epsilon(0.005));
    }

    TEST_CASE_FIXTURE(TimerResolutionFixture, "frame rate capping") {
        
        SUBCASE("delays when frame is shorter than target") {
            Salix::ChronoTimer timer;
            const int target_fps = 60;
            const float target_duration_ms = 1000.0f / target_fps;
            timer.set_target_fps(target_fps);

            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            timer.tick_end();
            auto end_time = std::chrono::high_resolution_clock::now();

            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // With high-resolution timers, this check is now reliable.
            CHECK(elapsed_ms >= (target_duration_ms - 1));
        }
        SUBCASE("does not delay when frame is longer than target") {
            Salix::ChronoTimer timer;
            timer.set_target_fps(60);

            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            timer.tick_end();
            auto end_time = std::chrono::high_resolution_clock::now();

            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            CHECK(elapsed_ms < 25);
        }
        SUBCASE("does not delay when uncapped (fps <= 0)") {
            Salix::ChronoTimer timer;
            timer.set_target_fps(0);

            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            timer.tick_end();
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            CHECK(elapsed_ms < 2);
        }
    }
    
    // The static function tests don't need the fixture but it's used for consistency.
    TEST_CASE_FIXTURE(TimerResolutionFixture, "static helper functions") {
        SUBCASE("get_ticks_ms reports elapsed time") {
            unsigned int start_ticks = Salix::ChronoTimer::get_ticks_ms();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            unsigned int end_ticks = Salix::ChronoTimer::get_ticks_ms();
            CHECK((end_ticks - start_ticks) >= 20);
        }

        SUBCASE("delay waits for the specified duration") {
            auto start_time = std::chrono::high_resolution_clock::now();
            Salix::ChronoTimer::delay(20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            CHECK(elapsed_ms >= 20);
        }
    }
}