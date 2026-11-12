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
    ~SDLTimerFixture() { /*SDL_Quit();*/ }
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
            // Use the RELIABLE SDL_Delay to simulate the long frame
            SDL_Delay(20);
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

#include <cmath>
TEST_SUITE("Salix::core::SDLTimerBatch947") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "fresh SDL timer starts with finite zero delta") {
        Salix::SDLTimer timer;
        CHECK(timer.get_delta_time() == 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "first SDL tick produces a finite nonnegative delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "two SDL ticks separated by delay report positive elapsed seconds") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(timer.get_delta_time() < 1.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "uncapped tick end preserves the most recent delta measurement") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "negative target FPS preserves the most recent delta measurement") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(-60);
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "changing target FPS does not reset accumulated delta state") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(30);
        CHECK(timer.get_delta_time() == measured);
        timer.set_target_fps(0);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "two SDL timer instances keep independent delta state") {
        Salix::SDLTimer first;
        Salix::SDLTimer second;
        first.tick_start();
        SDL_Delay(2);
        first.tick_start();
        CHECK(first.get_delta_time() > 0.0f);
        CHECK(second.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "SDL timer dispatches correctly through ITimer interface") {
        Salix::SDLTimer timer;
        Salix::ITimer* base = &timer;
        base->set_target_fps(0);
        base->tick_start();
        CHECK(base->get_delta_time() >= 0.0f);
        CHECK(std::isfinite(base->get_delta_time()));
        CHECK_NOTHROW(base->tick_end());
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "short capped frame waits for approximately its target budget") {
        Salix::SDLTimer timer;
        timer.set_target_fps(100);
        const auto begin = std::chrono::steady_clock::now();
        timer.tick_start();
        timer.tick_end();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - begin).count();
        CHECK(elapsed >= 8);
        CHECK(elapsed < 100);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "long frame tick end never rewrites measured delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps(500);
        timer.tick_start();
        SDL_Delay(3);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        SDL_Delay(3);
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
        CHECK(measured > 0.0f);
    }
}

TEST_SUITE("Salix::core::SDLTimerBatch2247") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 fresh timer exposes finite zero delta") {
        Salix::SDLTimer timer; CHECK(timer.get_delta_time()==0.0f); CHECK(std::isfinite(timer.get_delta_time()));
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 zero target FPS leaves fresh delta unchanged") {
        Salix::SDLTimer timer; timer.set_target_fps(0); CHECK(timer.get_delta_time()==0.0f);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 negative target FPS leaves fresh delta unchanged") {
        Salix::SDLTimer timer; timer.set_target_fps(-144); CHECK(timer.get_delta_time()==0.0f);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 positive target FPS configuration does not synthesize delta time") {
        Salix::SDLTimer timer; timer.set_target_fps(240); CHECK(timer.get_delta_time()==0.0f);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 first tick start produces finite nonnegative elapsed time") {
        Salix::SDLTimer timer; timer.tick_start(); CHECK(timer.get_delta_time()>=0.0f); CHECK(std::isfinite(timer.get_delta_time()));
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 delayed second tick reports positive elapsed time") {
        Salix::SDLTimer timer; timer.tick_start(); SDL_Delay(2); timer.tick_start(); CHECK(timer.get_delta_time()>0.0f); CHECK(timer.get_delta_time()<1.0f);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 switching positive cap to zero makes tick end return without altering delta") {
        Salix::SDLTimer timer; timer.tick_start(); SDL_Delay(2); timer.tick_start(); const float measured=timer.get_delta_time(); timer.set_target_fps(60); timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end()); CHECK(timer.get_delta_time()==measured);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 switching positive cap to negative makes tick end uncapped") {
        Salix::SDLTimer timer; timer.tick_start(); SDL_Delay(2); timer.tick_start(); const float measured=timer.get_delta_time(); timer.set_target_fps(60); timer.set_target_fps(-1);
        CHECK_NOTHROW(timer.tick_end()); CHECK(timer.get_delta_time()==measured);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 ITimer dispatch preserves SDL timer delta state") {
        Salix::SDLTimer concrete; Salix::ITimer& timer=concrete; timer.set_target_fps(0); timer.tick_start(); const float measured=timer.get_delta_time(); CHECK(std::isfinite(measured)); CHECK(measured>=0.0f); CHECK_NOTHROW(timer.tick_end()); CHECK(timer.get_delta_time()==measured);
    }
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch2247 separate SDL timers never share measured delta state") {
        Salix::SDLTimer first,second; first.tick_start(); SDL_Delay(2); first.tick_start(); CHECK(first.get_delta_time()>0.0f); CHECK(second.get_delta_time()==0.0f);
    }
}


#include <limits>

TEST_SUITE("Salix::core::SDLTimerBatch3047") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 tick end before first tick is safe when uncapped") {
        Salix::SDLTimer timer;
        timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 negative target after positive target restores uncapped behavior") {
        Salix::SDLTimer timer;
        timer.set_target_fps(60);
        timer.set_target_fps(-60);
        timer.tick_start();
        const float before = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == before);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 maximum positive FPS remains a valid finite timer configuration") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::max)());
        timer.tick_start();
        CHECK(std::isfinite(timer.get_delta_time()));
        CHECK_NOTHROW(timer.tick_end());
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 minimum negative FPS behaves as uncapped") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::min)());
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 repeated zero target assignments never synthesize delta time") {
        Salix::SDLTimer timer;
        timer.set_target_fps(0);
        timer.set_target_fps(0);
        timer.set_target_fps(0);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 three delayed ticks keep reporting finite positive elapsed time") {
        Salix::SDLTimer timer;
        timer.tick_start();
        for (int i = 0; i < 3; ++i) {
            SDL_Delay(2);
            timer.tick_start();
            CHECK(timer.get_delta_time() > 0.0f);
            CHECK(std::isfinite(timer.get_delta_time()));
        }
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 changing cap after a measured tick never rewrites that delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(30);
        timer.set_target_fps(240);
        timer.set_target_fps(0);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 separate timers keep uncapped and capped configuration independent") {
        Salix::SDLTimer capped;
        Salix::SDLTimer uncapped;
        capped.set_target_fps(1000000);
        uncapped.set_target_fps(0);
        capped.tick_start();
        uncapped.tick_start();
        CHECK_NOTHROW(capped.tick_end());
        CHECK_NOTHROW(uncapped.tick_end());
        CHECK(std::isfinite(capped.get_delta_time()));
        CHECK(std::isfinite(uncapped.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 ITimer dispatch accepts positive zero and negative target transitions") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.set_target_fps(120);
        timer.set_target_fps(0);
        timer.set_target_fps(-1);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3047 very high cap can execute capped tick end without changing delta state") {
        Salix::SDLTimer timer;
        timer.set_target_fps(1000000);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }
}
TEST_SUITE("Salix::core::SDLTimerBatch3447") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 negative target keeps tick end safe before first frame") {
        Salix::SDLTimer timer;
        timer.set_target_fps(-30);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 positive target assignment never rewrites an existing measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(240);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 repeated uncapped tick end calls preserve measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 advancing one timer never changes a fresh peer delta") {
        Salix::SDLTimer first;
        Salix::SDLTimer second;
        first.tick_start();
        SDL_Delay(2);
        first.tick_start();
        CHECK(first.get_delta_time() > 0.0f);
        CHECK(second.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 immediate consecutive ticks remain finite and nonnegative") {
        Salix::SDLTimer timer;
        timer.tick_start();
        timer.tick_start();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 disabling a cap after a delayed tick preserves that delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps(120);
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 ITimer base dispatch supports repeated uncapped frame completion") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.set_target_fps(0);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 maximum FPS can transition back to uncapped without changing delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::max)());
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 configuring a positive cap on a fresh timer does not synthesize time") {
        Salix::SDLTimer timer;
        timer.set_target_fps(75);
        CHECK(timer.get_delta_time() == 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch3447 newly constructed timer resets delta after a measured timer is destroyed") {
        {
            Salix::SDLTimer measured;
            measured.tick_start();
            SDL_Delay(2);
            measured.tick_start();
            CHECK(measured.get_delta_time() > 0.0f);
        }
        Salix::SDLTimer fresh;
        CHECK(fresh.get_delta_time() == 0.0f);
    }
}

TEST_SUITE("Salix::core::SDLTimerBatch4147") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 fresh timer remains zero after positive cap configuration") {
        Salix::SDLTimer timer;
        timer.set_target_fps(144);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 immediate first tick remains finite and nonnegative") {
        Salix::SDLTimer timer;
        timer.tick_start();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 delayed consecutive tick measures positive elapsed time") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 negative target behaves uncapped without rewriting delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(-4147);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 repeated uncapped frame completion preserves delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 positive cap assignment never synthesizes new delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(75);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 positive to zero target transition preserves measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(120);
        timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 measured timer does not alter fresh peer") {
        Salix::SDLTimer measured;
        Salix::SDLTimer fresh;
        measured.tick_start();
        SDL_Delay(2);
        measured.tick_start();
        CHECK(measured.get_delta_time() > 0.0f);
        CHECK(fresh.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 ITimer virtual API preserves latest measurement") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4147 extremely high cap executes safely without changing delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::max)());
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }
}

TEST_SUITE("Salix::core::SDLTimerBatch4547") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 maximum FPS assignment leaves fresh delta at zero") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::max)());
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 minimum integer FPS is treated as uncapped before first frame") {
        Salix::SDLTimer timer;
        timer.set_target_fps((std::numeric_limits<int>::min)());
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 cap transitions preserve an existing measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(30);
        timer.set_target_fps(0);
        timer.set_target_fps(-30);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 repeated delayed ticks keep the latest delta positive and finite") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float first = timer.get_delta_time();
        SDL_Delay(2);
        timer.tick_start();
        const float second = timer.get_delta_time();
        CHECK(first > 0.0f);
        CHECK(second > 0.0f);
        CHECK(std::isfinite(second));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 uncapped tick end leaves latest measured delta untouched") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 ITimer virtual uncapped completion preserves measured delta") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(-1);
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 measuring first timer leaves second fresh timer at zero") {
        Salix::SDLTimer first;
        Salix::SDLTimer second;
        first.tick_start();
        SDL_Delay(2);
        first.tick_start();
        CHECK(first.get_delta_time() > 0.0f);
        CHECK(second.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 newly constructed timer starts clean after another measured timer") {
        Salix::SDLTimer measured;
        measured.tick_start();
        SDL_Delay(2);
        measured.tick_start();
        REQUIRE(measured.get_delta_time() > 0.0f);
        Salix::SDLTimer fresh;
        CHECK(fresh.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 positive zero positive target sequence does not synthesize delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps(60);
        timer.set_target_fps(0);
        timer.set_target_fps(120);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch4547 very high positive cap completes safely without changing delta") {
        Salix::SDLTimer timer;
        timer.set_target_fps(500000);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == measured);
    }
}

TEST_SUITE("Salix::core::SDLTimerBatch5047") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 target assignments never synthesize delta on a fresh timer") {
        Salix::SDLTimer timer;
        timer.set_target_fps(60);
        timer.set_target_fps(0);
        timer.set_target_fps(-60);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 delayed frame measurement remains positive and finite") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 repeated uncapped tick end calls preserve measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 repeated negative cap completion preserves measured delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(-5047);
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 positive zero positive cap transitions preserve fresh state") {
        Salix::SDLTimer timer;
        timer.set_target_fps(30);
        timer.set_target_fps(0);
        timer.set_target_fps(120);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 ITimer target transitions do not rewrite measured delta") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(60);
        timer.set_target_fps(0);
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 measuring one timer leaves an independently constructed peer fresh") {
        Salix::SDLTimer first;
        Salix::SDLTimer second;
        first.tick_start();
        SDL_Delay(2);
        first.tick_start();
        REQUIRE(first.get_delta_time() > 0.0f);
        CHECK(second.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 successive delayed starts replace delta with latest finite measurement") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        REQUIRE(timer.get_delta_time() > 0.0f);
        SDL_Delay(3);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 positive cap completion leaves measured delta untouched") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(2);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(500000);
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5047 virtual uncapped lifecycle remains independently repeatable") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.set_target_fps(0);
        timer.tick_start();
        timer.tick_end();
        timer.tick_start();
        timer.tick_end();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }
}

TEST_SUITE("Salix::core::SDLTimerBatch5447") {
    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 fresh timer starts with exact zero delta") {
        Salix::SDLTimer timer;
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 immediate tick start produces finite nonnegative delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 one millisecond delayed start produces positive finite delta") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(1);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 target changes never synthesize delta before first tick") {
        Salix::SDLTimer timer;
        timer.set_target_fps(15);
        timer.set_target_fps(120);
        timer.set_target_fps(0);
        timer.set_target_fps(-15);
        CHECK(timer.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 zero cap preserves measured delta through repeated completion") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(1);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(0);
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 negative cap preserves measured delta through repeated completion") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(1);
        timer.tick_start();
        const float measured = timer.get_delta_time();
        timer.set_target_fps(-5447);
        timer.tick_end();
        timer.tick_end();
        CHECK(timer.get_delta_time() == measured);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 very high positive cap completes safely") {
        Salix::SDLTimer timer;
        timer.set_target_fps(1000000);
        timer.tick_start();
        const float before = timer.get_delta_time();
        CHECK_NOTHROW(timer.tick_end());
        CHECK(timer.get_delta_time() == before);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 measuring first timer leaves second timer fresh") {
        Salix::SDLTimer first;
        Salix::SDLTimer second;
        first.tick_start();
        SDL_Delay(1);
        first.tick_start();
        REQUIRE(first.get_delta_time() > 0.0f);
        CHECK(second.get_delta_time() == 0.0f);
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 successive delayed starts keep replacing delta with finite values") {
        Salix::SDLTimer timer;
        timer.tick_start();
        SDL_Delay(1);
        timer.tick_start();
        REQUIRE(timer.get_delta_time() > 0.0f);
        SDL_Delay(1);
        timer.tick_start();
        CHECK(timer.get_delta_time() > 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }

    TEST_CASE_FIXTURE(SDLTimerFixture, "batch5447 ITimer uncapped lifecycle remains valid after positive cap transition") {
        Salix::SDLTimer concrete;
        Salix::ITimer& timer = concrete;
        timer.set_target_fps(60);
        timer.set_target_fps(0);
        timer.tick_start();
        timer.tick_end();
        CHECK(timer.get_delta_time() >= 0.0f);
        CHECK(std::isfinite(timer.get_delta_time()));
    }
}

