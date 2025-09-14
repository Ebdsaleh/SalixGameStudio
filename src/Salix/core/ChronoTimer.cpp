// =================================================================================
// Filename:    Salix/core/ChronoTimer.cpp
// Author:      SalixGameStudio
// Description: Implements the ITimer interface using std::chrono.
// =================================================================================
#include <Salix/core/ChronoTimer.h>
#include <thread>

namespace Salix {

    // Use a steady clock for the epoch to ensure it's monotonic
    static const auto G_CHRONO_EPOCH = std::chrono::steady_clock::now();

    unsigned int ChronoTimer::get_ticks_ms() {
        auto now = std::chrono::steady_clock::now();
        auto duration_since_epoch = now - G_CHRONO_EPOCH;
        return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count());
    }

    void ChronoTimer::delay(unsigned int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    ChronoTimer::ChronoTimer() :
        delta_time(0.0f),
        target_frame_duration_ms{}
    {
        // Initialize times using the steady_clock
        last_frame_time = std::chrono::steady_clock::now();
        frame_start_time = last_frame_time;
    }

    void ChronoTimer::set_target_fps(int fps) {
        if (fps > 0) {
            using f_seconds = std::chrono::duration<float>;
            target_frame_duration_ms = f_seconds{1.0f / fps};
        } else {
            target_frame_duration_ms = std::chrono::duration<float, std::milli>::zero();
        }
    }

    void ChronoTimer::tick_start() {
        // Use steady_clock for all measurements
        frame_start_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> time_diff_seconds = frame_start_time - last_frame_time;
        delta_time = time_diff_seconds.count();
        last_frame_time = frame_start_time;
    }

    // This is the new function that contains all the logic
    std::chrono::duration<float, std::milli> ChronoTimer::calculate_sleep_duration() {
        if (target_frame_duration_ms.count() <= 0.0f) {
            return std::chrono::duration<float, std::milli>::zero();
        }

        auto time_at_end = std::chrono::steady_clock::now();
        auto frame_duration_native = time_at_end - frame_start_time;

        auto frame_duration_ms = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(frame_duration_native);

        if (frame_duration_ms < target_frame_duration_ms) {
            return target_frame_duration_ms - frame_duration_ms;
        }

        return std::chrono::duration<float, std::milli>::zero();
    }

    // tick_end is now much simpler. It just performs the action.
    void ChronoTimer::tick_end() {
        auto sleep_duration = calculate_sleep_duration();
        if (sleep_duration.count() > 0) {
            std::this_thread::sleep_for(sleep_duration);
        }
    }

    float ChronoTimer::get_delta_time() const {
        return delta_time;
    }

} // namespace Salix