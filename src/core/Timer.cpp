// Timer.cpp
#include "Timer.h"
#include <thread> // Needed for std::this_thread:sleep_for function.

Timer::Timer() : 
    delta_time(0.0f),
    target_frame_duration(0) {
        // Initialize the start time point when the timer is created
        start_time_point = std::chrono::high_resolution_clock::now();
}

void Timer::tick() {
    // Record the end time of the previous time of the previous frame.
    auto end_time_point = std::chrono::high_resolution_clock::now();

    // The duration is the difference between the start of the last frame and the start of this one.
    std::chrono::duration<float> duration = end_time_point - start_time_point;
    delta_time = duration.count();

    // The start of the new frame is now.
    start_time_point = end_time_point;
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

void Timer::delay_if_needed() {
    // If the target is 0, do nothing, it means it's uncapped.
    if (target_frame_duration.count() == 0.0f) {
        return;
    }

    // See how long the frame has taken so far.
    auto frame_end_time = std::chrono::high_resolution_clock::now();
    auto frame_duration = frame_end_time - start_time_point;

    // if the frame has finished faster than our target, we need to wait.
    if (frame_duration < target_frame_duration) {
        // Calculate the exact duration we need to sleep for.
        auto sleep_duration = target_frame_duration - frame_duration;
        // Tell the current thread to sleep for that duration.
        // This yields CPU time back to the OS.
        std::this_thread::sleep_for(sleep_duration);
    }

}

float Timer::get_delta_time() const {
    return delta_time;
}