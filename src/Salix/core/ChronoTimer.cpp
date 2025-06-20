// =================================================================================
// Filename:    Salix/core/ChronoTimer.cpp
// Author:      SalixGameStudio
// Description: Implements the ITimer interface using std::chrono.
// =================================================================================
#include <Salix/core/ChronoTimer.h>
#include <thread> // Required for std::this_thread::sleep_for

namespace Salix {

    // --- Static helper function implementation ---
    // This needs a static start time to measure against.
    static const auto G_CHRONO_EPOCH = std::chrono::high_resolution_clock::now();

    unsigned int ChronoTimer::get_ticks_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration_since_epoch = now - G_CHRONO_EPOCH;
        return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count());
    }

    void ChronoTimer::delay(unsigned int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    // -----------------------------------------


    ChronoTimer::ChronoTimer() :
        delta_time(0.0f),
        target_frame_duration_ms(0.0)
    {
        // Initialize the last frame time to the current time.
        last_frame_time = std::chrono::high_resolution_clock::now();
    }

    void ChronoTimer::set_target_fps(int fps) {
        if (fps > 0) {
            // Store the target duration in milliseconds.
            target_frame_duration_ms = std::chrono::duration<float, std::milli>(1000.0f / fps);
        } else {
            // A target of 0 means we run as fast as possible (uncapped).
            target_frame_duration_ms = std::chrono::duration<float, std::milli>(0.0);
        }
    }

    void ChronoTimer::tick_start() {
        // Mark the beginning of the frame.
        frame_start_time = std::chrono::high_resolution_clock::now();

        // Calculate the time elapsed since the last frame's start.
        std::chrono::duration<float> time_diff_seconds = frame_start_time - last_frame_time;
        delta_time = time_diff_seconds.count();

        // Update the last frame time for the next frame's calculation.
        last_frame_time = frame_start_time;
    }

    void ChronoTimer::tick_end() {
        // If we don't have a target FPS, there's no need to delay.
        if (target_frame_duration_ms.count() <= 0.0f) {
            return;
        }

        // Calculate how long the game logic and rendering took for this frame.
        auto time_at_end = std::chrono::high_resolution_clock::now();
        auto frame_duration = time_at_end - frame_start_time;

        // If the frame finished faster than our target...
        if (frame_duration < target_frame_duration_ms) {
            // ...sleep for the remaining time to meet our target.
            std::this_thread::sleep_for(target_frame_duration_ms - frame_duration);
        }
    }

    float ChronoTimer::get_delta_time() const {
        return delta_time;
    }

} // namespace Salix