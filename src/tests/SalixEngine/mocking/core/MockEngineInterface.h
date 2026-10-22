#pragma once

#include <Salix/core/EngineInterface.h>

class MockEngineInterface : public Salix::EngineInterface {
public:
    void switch_state(Salix::AppStateType state) override {
        current_state = state;
        ++switch_state_calls;
    }

    void set_mode(Salix::EngineMode mode) override {
        current_mode = mode;
        ++set_mode_calls;
    }

    Salix::EngineMode get_mode() const override { return current_mode; }

    bool is_running() const override { return running; }

    const bool is_running(bool keep_running) override {
        running = keep_running;
        return running;
    }

    float get_time_scale() const override { return time_scale; }

    void set_time_scale(float new_time_scale) override {
        time_scale = new_time_scale;
        ++set_time_scale_calls;
    }

    Salix::AppStateType current_state = Salix::AppStateType::None;
    Salix::EngineMode current_mode = Salix::EngineMode::None;
    bool running = true;
    float time_scale = 1.0f;
    int switch_state_calls = 0;
    int set_mode_calls = 0;
    int set_time_scale_calls = 0;
};
