// Timer.cpp
#include "Timer.h"

Timer::Timer() : delta_time(0.0f) {
    // Initialize the last time point to the current time when the timer was created.
    last_time_point = std::chrono::high_resolution_clock::now();
}

void Timer::tick() {
    // Get the current time point.
    auto current_time_point = std::chrono::high_resolution_clock::now();

    // Calculate the duration between the current frame and the last frame.
    std::chrono::duration<float> duration = current_time_point - last_time_point;

    // Store the result in our delta_time variable, in seconds.
    delta_time = duration.count();

    // Update the last time point to be the start of the new frame.
    last_time_point = current_time_point;
}

float Timer::get_delta_time() const {
    return delta_time;
}