// Salix/core/ChronoTimer.h
#pragma once

#include <Salix/core/ITimer.h>
#include <chrono>

namespace Salix {

    class ChronoTimer : public ITimer {
    public:
        ChronoTimer();

        // --- UPDATED METHODS ---
        // These now correctly match the ITimer interface.
        void set_target_fps(int fps) override;
        void tick_start() override;
        void tick_end() override;
        float get_delta_time() const override;

        // --- STATIC HELPER FUNCTIONS ---
        // These are useful utilities and can remain.
        static unsigned int get_ticks_ms();
        static void delay(unsigned int ms);

    private:
        // Use a consistent name for clarity. This tracks the start of the previous frame.
        std::chrono::high_resolution_clock::time_point last_frame_time;
        
        // <<< NEW >>> This tracks the start of the *current* frame for the delay calculation.
        std::chrono::high_resolution_clock::time_point frame_start_time;

        float delta_time;
        std::chrono::duration<float, std::milli> target_frame_duration_ms;
    };

} // namespace Salix