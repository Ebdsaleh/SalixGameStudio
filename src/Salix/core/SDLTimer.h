// Salix/core/SDLTimer.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/ITimer.h>

namespace Salix {

    class SALIX_API SDLTimer : public ITimer {
    public:
        SDLTimer();
        
        // --- METHODS MATCHING THE NEW INTERFACE ---
        void set_target_fps(int fps) override;
        void tick_start() override;
        void tick_end() override;
        float get_delta_time() const override;

    private:
        unsigned long long last_frame_time;
        unsigned long long frame_start_time;

        float delta_time;
        float target_frame_duration_ms;
    };
} // namespace Salix