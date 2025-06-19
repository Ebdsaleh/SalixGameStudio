// Salix/core/ChronoTimer.cpp
#include <ChronoTimer.h>
#include <thread> // For std::this_thread::sleep_for
#include <iostream>
// --- ATTENTION: THIS NEEDS A RE-WORK! ---
// --- I WILL COME BACK TO THIS LATER,  ---
// --- WHEN I START EXPERIMENTING WITH  ---
// --- DIFFERENT RENDERING API'S        ---

namespace Salix {
    // Static Initialization for get_ticks_ms
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

        // --- THE CRUCIAL FIX IS HERE ---
        // Recalculate the current time after sleeping to get the true frame time.
        current_time = std::chrono::high_resolution_clock::now();
        
        // Explicitly cast the high-precision duration into a float representing SECONDS.
        std::chrono::duration<float> final_duration = current_time - last_frame_start_time;
        delta_time = final_duration.count();
        
        // Update the start time for the next frame.
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
} // namespace Salix