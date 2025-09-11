// =================================================================================
// Filename:    src/Tests/SalixEngine/core/SDLTimer.test.cpp
// Description: Contains unit tests for the SDLTimer functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/core/SDLTimer.h>
#include <thread>
#include <chrono>
#include <SDL.h>
// It's good practice to initialize/quit SDL for each test suite
// to ensure a clean state, though you could also use setup/teardown fixtures.
struct SDLTimerFixture {
    SDLTimerFixture() { SDL_Init(SDL_INIT_TIMER); }
    ~SDLTimerFixture() { SDL_Quit(); }
};

TEST_SUITE("Salix::core::SDLTimer") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "initialization state") {
        Salix::SDLTimer timer;
        // The timer's delta_time should be 0.0f immediately after construction.
        CHECK(timer.get_delta_time() == 0.0f);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "correctly calculates delta_time") {
        // ARRANGE
        Salix::SDLTimer timer;
        const int wait_duration_ms = 16;

        // ACT
        timer.tick_start();  // Set initial time
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_duration_ms));
        timer.tick_start();   // Calculate delta time based on sleep

        // ASSERT
        // The delta time should be approximately 0.016 seconds.
        // Epsilon is used to allow for slight timing inaccuracies.
        CHECK(timer.get_delta_time() == doctest::Approx(0.016).epsilon(0.005));
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "frame rate capping") {
        
        SUBCASE("delays when frame is shorter than target") {
            // ARRANGE
            // This test covers the if-branch inside tick_end where a delay is needed.
            Salix::SDLTimer timer;
            const int target_fps = 60;
            const float target_duration_ms = 1000.0f / target_fps;
            timer.set_target_fps(target_fps);
            // ACT
            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            timer.tick_end(); // Frame takes almost 0ms, so it must delay.
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            // Calculate the expected delay *as an integer*, just like SDLTimer does.
            Uint32 expected_delay_ms = static_cast<Uint32>(target_duration_ms); // This will be 16
            
            // ASSERT
            // Check that the actual elapsed time is at least the expected integer delay.
            // This makes the test robust against floating point precision issues.
            CHECK(elapsed_ms >= expected_delay_ms);
        }
        SUBCASE("does not delay when frame is longer than target") {
            // ARRANGE
            // This is the new test case to cover the implicit else-branch in tick_end.
            Salix::SDLTimer timer;
            timer.set_target_fps(60); // Target is ~16.67ms
            
            // ACT
            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            // Simulate a long frame by sleeping for 20ms.
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            timer.tick_end(); // Should not add any extra delay.
            auto end_time = std::chrono::high_resolution_clock::now();

            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // ASSERT
            // The total time should be just over our 20ms sleep, with very little extra.
            CHECK(elapsed_ms < 25);
        }
        SUBCASE("does not delay when uncapped (fps <= 0)") {
            // ARRANGE
            // This test covers the first 'if' condition in tick_end.
            Salix::SDLTimer timer;
            timer.set_target_fps(0); // Set to uncapped

            // ACT
            auto start_time = std::chrono::high_resolution_clock::now();
            timer.tick_start();
            timer.tick_end(); // Should return immediately.
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // ASSERT
            CHECK(elapsed_ms < 2); // Should be very fast.
        }
    }
}