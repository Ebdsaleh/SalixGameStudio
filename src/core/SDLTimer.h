// SDLTimer.h
#pragma once
#include "ITimer.h"

class SDLTimer : public ITimer {
    public:
        SDLTimer();
        virtual ~SDLTimer();
        
        void set_target_fps(int fps) override;
        void tick() override;
        float get_delta_time() const override;

    private:
    unsigned int last_tick_time;
    float delta_time;
    float target_frame_duration_ms;
};