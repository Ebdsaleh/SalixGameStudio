// =================================================================================
// Filename:    Salix/core/SDLTimer.cpp
// Author:      SalixGameStudio
// Description: Implements the ITimer interface using SDL's high-performance counter.
// =================================================================================
#include <Salix/core/SDLTimer.h>
#include <SDL.h>

namespace Salix {

    // --- Helper variable to convert performance counter ticks to seconds ---
    // We get the frequency once and store it to avoid repeated division.
    static const double G_PERFORMANCE_FREQUENCY = (double)SDL_GetPerformanceFrequency();

    SDLTimer::SDLTimer() :
        last_frame_time(0),
        frame_start_time(0),
        delta_time(0.0f),
        target_frame_duration_ms(0.0f)
    {
        // Initialize the last frame time to the current high-resolution time.
        last_frame_time = SDL_GetPerformanceCounter();
    }

    void SDLTimer::set_target_fps(int fps) {
        if (fps > 0) {
            target_frame_duration_ms = 1000.0f / fps;
        }
        else {
            // A target of 0 means we run as fast as possible (uncapped).
            target_frame_duration_ms = 0.0f;
        }
    }

    void SDLTimer::tick_start() {
        // Mark the beginning of the frame.
        frame_start_time = SDL_GetPerformanceCounter();

        // Calculate the time elapsed since the last frame's start.
        // This is our true, accurate delta_time.
        unsigned long long time_diff = frame_start_time - last_frame_time;

        // Convert the difference in high-resolution ticks to seconds.
        delta_time = (float)(time_diff / G_PERFORMANCE_FREQUENCY);

        // Update the last frame time for the next frame's calculation.
        last_frame_time = frame_start_time;
    }

    void SDLTimer::tick_end() {
        // If we don't have a target FPS, there's no need to delay.
        if (target_frame_duration_ms <= 0.0f) {
            return;
        }

        // Calculate how long the game logic and rendering took for this frame.
        unsigned long long time_at_end = SDL_GetPerformanceCounter();
        unsigned long long frame_duration_ticks = time_at_end - frame_start_time;

        // Convert the duration from ticks to milliseconds.
        float frame_duration_ms = (float)((frame_duration_ticks * 1000.0) / G_PERFORMANCE_FREQUENCY);

        // If the frame finished faster than our target...
        if (frame_duration_ms < target_frame_duration_ms) {
            // ...calculate how long we need to sleep and then delay.
            Uint32 delay_ms = static_cast<Uint32>(target_frame_duration_ms - frame_duration_ms);
            SDL_Delay(delay_ms);
        }
    }

    float SDLTimer::get_delta_time() const {
        return delta_time;
    }

} // namespace Salix