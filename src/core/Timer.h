// Timer.h
// Will revist this at another time.
/*
#pragma once

#include <chrono>

class Timer {
public:
    Timer();

    void set_target_fps(int fps);
    
    // The tick method now handles all logic at the end of a frame.
    void tick();

    // This returns the delta_time that was calculated in the *previous* tick.
    float get_delta_time() const;

private:
    std::chrono::high_resolution_clock::time_point last_frame_start_time;
    float delta_time;
    std::chrono::duration<float> target_frame_duration;
};
*/