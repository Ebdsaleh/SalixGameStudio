// ChronoTimer.cpp

#include "ChronoTimer.h"
#include <thread> // Needed for std::this_thread:sleep_for function.

// --- Static Initialization for get_ticks_ms ---
// We need a single, application-wide start time to measure against.
// A static variable is the perfect tool for this.
static const auto app_start_time = std::chrono::high_resolution_clock::now();

ChronoTimer::ChronoTimer() : 
    delta_time(0.0f), 
    target_frame_duration(0) 
{
    last_frame_start_time = std::chrono::high_resolution_clock::now();
}

void ChronoTimer::set_target_fps(int fps) {
    if (fps > 0) {
        target_frame_duration = std::chrono::duration<float>(1.0f / fps);
    } else {
        target_frame_duration = std::chrono::duration<float>(0);
    }
}

void ChronoTimer::tick() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto frame_duration = current_time - last_frame_start_time;

    if (target_frame_duration.count() > 0.0f && frame_duration < target_frame_duration) {
        auto sleep_duration = target_frame_duration - frame_duration;
        std::this_thread::sleep_for(sleep_duration);
    }

    current_time = std::chrono::high_resolution_clock::now();
    delta_time = std::chrono::duration<float>(current_time - last_frame_start_time).count();
    last_frame_start_time = current_time;
}

float ChronoTimer::get_delta_time() const {
    return delta_time;
}

// --- Static Helper Method Implementations ---

unsigned int ChronoTimer::get_ticks_ms() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = current_time - app_start_time;
    // Cast the high-resolution duration to milliseconds.
    return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

void ChronoTimer::delay(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}