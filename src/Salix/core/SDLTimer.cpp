// Salix/core/SDLTimer.cpp
#include <Salix/core/SDLTimer.h>
#include <SDL.h>

namespace Salix {
    SDLTimer::SDLTimer() :
        last_tick_time(0),
        delta_time(0.0f),
        target_frame_duration_ms(0.0f) {
            // Initialize last_tick_time to the number of milliseconds since SDL was initialized.
            last_tick_time = SDL_GetTicks();
        }


    void SDLTimer::set_target_fps(int fps) {
        if (fps > 0) {
            target_frame_duration_ms = 1000.0f / fps;
        } else {
            target_frame_duration_ms =0.0f;
        }
    }

    void SDLTimer::tick() {
        // This took a long time.
        unsigned int current_tick_time = SDL_GetTicks();
        unsigned int frame_duration_ms = current_tick_time - last_tick_time;

        // If we have a target FPS and the frame finished too quickly...
        if (target_frame_duration_ms > frame_duration_ms) {
            // ... sleep for the remaining time to meet our target.
            SDL_Delay(static_cast<Uint32>(target_frame_duration_ms - frame_duration_ms));
        }

        // Now that the frame has officially ended (after the delay),
        // calculate the final delta_time for the *next* frame to use
        current_tick_time = SDL_GetTicks();
        delta_time = (current_tick_time - last_tick_time) / 1000.0f; // Convert ms to seconds.

        // Update the last tick time for the next frame's calculation.
        last_tick_time = current_tick_time;
    }

    float SDLTimer::get_delta_time() const{
        return delta_time;
    }
} // namespace Salix