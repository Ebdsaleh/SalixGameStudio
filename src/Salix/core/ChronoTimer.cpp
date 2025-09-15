// =================================================================================
// Filename:    Salix/core/ChronoTimer.cpp
// Author:      SalixGameStudio
// Description: Implements the ITimer interface using std::chrono.
// =================================================================================
// Salix/core/ChronoTimer.cpp
#include <Salix/core/ChronoTimer.h>
#include <thread>
#include <iostream>

namespace Salix {

    // --- Static epoch for get_ticks_ms ---
    static const auto G_CHRONO_EPOCH = ChronoTimer::clock::now();

    unsigned int ChronoTimer::get_ticks_ms() {
        auto now = clock::now();
        auto duration_since_epoch = now - G_CHRONO_EPOCH;
        return static_cast<unsigned int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count()
        );
    }

   
    ChronoTimer::ChronoTimer(delay_fn_t delay_fn): delay(delay_fn), delta_time(0.0f),
        target_frame_duration_ms(std::chrono::duration<float, std::milli>::zero()) {
        last_frame_time = clock::now();
        frame_start_time = last_frame_time;
    }

    void ChronoTimer::default_delay(std::chrono::milliseconds d) {
        std::this_thread::sleep_for(d);
    }
    void ChronoTimer::delay_for(unsigned int ms) {
        if (ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
    }

    void ChronoTimer::delay_for(std::chrono::duration<float, std::milli> duration) {
        if (duration.count() <= 0.0f) return;

        const auto margin = std::chrono::milliseconds(2);
        auto end = clock::now() + duration;

        if (duration > margin) {
            std::this_thread::sleep_for(duration - margin);
        }

        while (clock::now() < end) {
            // spin
        }
    }

    

    void ChronoTimer::set_target_fps(int fps) {
        if (fps > 0) {
            target_frame_duration_ms =
                std::chrono::duration<float, std::milli>(1000.0f / fps);
        } else {
            target_frame_duration_ms = std::chrono::duration<float, std::milli>::zero();
        }
    }

    void ChronoTimer::tick_start() {
        frame_start_time = clock::now();
        std::chrono::duration<float> time_diff_seconds = frame_start_time - last_frame_time;
        delta_time = time_diff_seconds.count();
        last_frame_time = frame_start_time;
    }

    std::chrono::duration<float, std::milli> ChronoTimer::calculate_sleep_duration() {
        if (target_frame_duration_ms.count() <= 0.0f) {
            return std::chrono::duration<float, std::milli>::zero();
        }

        auto now = clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - frame_start_time);

        if (frame_duration < target_frame_duration_ms) {
            return target_frame_duration_ms - frame_duration;
        }

        return std::chrono::duration<float, std::milli>::zero();
    }

    void ChronoTimer::tick_end() {
        if (target_frame_duration_ms.count() <= 0) {
            std::cout << "[tick_end] no target FPS, skipping\n";
            return;
        }

        auto frame_end_time = clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end_time - frame_start_time);

        std::cout << "[tick_end] frame_duration=" << frame_duration.count()
                << "ms, target=" << target_frame_duration_ms.count() << "ms\n";

        if (frame_duration < target_frame_duration_ms) {
            auto sleep_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                target_frame_duration_ms - frame_duration
            );

            std::cout << "[tick_end] calling delay(" << sleep_time.count() << "ms)\n";
            delay(sleep_time);
        } else {
            std::cout << "[tick_end] no sleep needed\n";
        }
    }

    float ChronoTimer::get_delta_time() const {
        return delta_time;
    }

} // namespace Salix
