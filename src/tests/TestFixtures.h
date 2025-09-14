// Tests/TestFixtures.h
#pragma once
#include <SDL.h>
#include <iostream>

// Add Windows headers to query and set the timer resolution directly
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

struct HighResolutionTimerFixture {
    HighResolutionTimerFixture() {
        std::cout << "\n--- [FIXTURE SETUP] ---\n";
        
        // Explicitly request 1ms timer resolution. This is the key change.
        if (timeBeginPeriod(1) == TIMERR_NOERROR) {
            std::cout << "[FIXTURE INFO] High-resolution timer (1ms) requested successfully.\n";
        } else {
            std::cerr << "[FIXTURE WARNING] Could not set high-resolution timer.\n";
        }

        if (SDL_Init(SDL_INIT_TIMER) != 0) {
            std::cerr << "[FIXTURE ERROR] SDL_Init(SDL_INIT_TIMER) failed: " << SDL_GetError() << std::endl;
        }
    }

    ~HighResolutionTimerFixture() {
        SDL_Quit();
        
        // Explicitly release the high-resolution timer.
        timeEndPeriod(1);
        
        std::cout << "--- [FIXTURE TEARDOWN] ---\n";
    }
};