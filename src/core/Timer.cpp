// Timer.cpp
// Will revisit this at another time.
/*
#include "Timer.h"
#include <thread> // Needed for std::this_thread:sleep_for function.

Timer::Timer() : 
    delta_time(0.0f),
    target_frame_duration(0) {
        // Initialize the last tick time to the moment the timer is created.
        last_frame_start_time = std::chrono::high_resolution_clock::now(); 
}

void Timer::tick() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto frame_duration = current_time - last_frame_start_time;

    if (target_frame_duration.count() > 0.0f && frame_duration < target_frame_duration) {
        auto sleep_duration = target_frame_duration - frame_duration;
        std::this_thread::sleep_for(sleep_duration);
    }

    // --- THE CRUCIAL FIX IS HERE ---
    // Recalculate the current time after sleeping to get the true frame time.
    current_time = std::chrono::high_resolution_clock::now();
    
    // Explicitly cast the duration to a float representing seconds.
    delta_time = std::chrono::duration<float>(current_time - last_frame_start_time).count();
    
    // Update the start time for the next frame.
    last_frame_start_time = current_time;
}


void Timer::set_target_fps(int fps) {
    if (fps > 0) {
        // Calculate the target duration for the one frame in seconds.
        target_frame_duration = std::chrono::duration<float>(1.0f / fps);
    } else {
        // A target of 0 means the framerate is uncapped.
        target_frame_duration = std::chrono::duration<float>(0);
    }
}

float Timer::get_delta_time() const {
    return delta_time;
}

*/