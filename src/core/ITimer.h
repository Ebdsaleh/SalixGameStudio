// ITimer.h
#pragma once

class ITimer {
    public:
        virtual ~ITimer() = default;

        // A contract that all timers must know how to:
        virtual void set_target_fps(int fps) = 0;
        virtual void tick() = 0;
        virtual void get_delta_time();

};