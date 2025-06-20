// ChronoTimer.h
#pragma once
#include "Salix/core/ITimer.h"
#include <chrono>

namespace Salix {
    class ChronoTimer : public ITimer {
    public:
        ChronoTimer();

        void set_target_fps(int fps);
        
        // The tick method now handles all logic at the end of a frame.
        void tick();

        // This returns the delta_time that was calculated in the *previous* tick.
        float get_delta_time() const;
        
        // Get the number of miliseconds since an arbitrary starting point.
        static unsigned int get_ticks_ms();  // We should also make a version for floats and doubles.

        

        // A high precision delay function
        static void delay(unsigned int ms);  // We should also make a version for floats and doubles.


    private:
        std::chrono::high_resolution_clock::time_point last_frame_start_time;
        float delta_time;
        std::chrono::duration<float> target_frame_duration;
    };
} // namespace Salix