// Timer.h
#pragma once

#include <chrono>

class Timer {
    public:
        Timer();
        
        // Marks the beginning of a new frame.
        void tick();

        // Gets the elapsed time since the last tick
        float get_delta_time() const;

    private:
        // Using the high_resolution_clock for the most precision available.
        std::chrono::high_resolution_clock::time_point last_time_point;

        // the delta_time is stored here after every tick.
        float delta_time;

};