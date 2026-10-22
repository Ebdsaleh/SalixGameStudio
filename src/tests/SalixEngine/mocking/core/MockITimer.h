#pragma once

#include <Salix/core/ITimer.h>
#include <chrono>

class MockITimer : public Salix::ITimer {
public:
    void set_target_fps(int fps) override {
        target_fps = fps;
        ++set_target_fps_calls;
    }

    void tick_start() override {
        ++tick_start_calls;
    }

    void tick_end() override {
        ++tick_end_calls;
    }

    float get_delta_time() const override {
        return delta_time;
    }

    std::chrono::duration<float, std::milli> calculate_sleep_duration() override {
        ++sleep_duration_calls;
        return sleep_duration;
    }

    int target_fps = 0;
    float delta_time = 0.0f;
    std::chrono::duration<float, std::milli> sleep_duration{0.0f};
    int set_target_fps_calls = 0;
    int tick_start_calls = 0;
    int tick_end_calls = 0;
    int sleep_duration_calls = 0;
};
