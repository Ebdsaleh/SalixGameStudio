// Salix/core/ChronoTimer.h
#pragma once

#include <Salix/core/ITimer.h>
#include <chrono>
#include <functional>

namespace Salix {

    class SALIX_API ChronoTimer : public ITimer {
    public:
        using clock = std::chrono::steady_clock;
        using delay_fn_t = std::function<void(std::chrono::milliseconds)>;
        
        ChronoTimer(delay_fn_t delay_fn = default_delay);

        void set_target_fps(int fps) override;
        void tick_start() override;
        void tick_end() override;
        float get_delta_time() const override;

        // Exposed for testing
        std::chrono::duration<float, std::milli> calculate_sleep_duration();

        static unsigned int get_ticks_ms();
        static void delay_for(unsigned int ms);
        static void delay_for(std::chrono::duration<float, std::milli> duration);

    private:
        static void default_delay(std::chrono::milliseconds d);
        delay_fn_t delay;
        clock::time_point last_frame_time;
        clock::time_point frame_start_time;

        float delta_time;
        std::chrono::duration<float, std::milli> target_frame_duration_ms;
    };

} // namespace Salix
