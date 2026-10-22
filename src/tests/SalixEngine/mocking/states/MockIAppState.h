#pragma once

#include <Salix/states/IAppState.h>
#include <Salix/core/InitContext.h>

class MockIAppState : public Salix::IAppState {
public:
    void on_enter(const Salix::InitContext& context) override {
        last_context = &context;
        ++enter_calls;
    }

    void on_exit() override {
        ++exit_calls;
    }

    void update(float delta_time) override {
        last_delta_time = delta_time;
        ++update_calls;
    }

    void render(Salix::IRenderer* renderer) override {
        last_renderer = renderer;
        ++render_calls;
    }

    const Salix::InitContext* last_context = nullptr;
    Salix::IRenderer* last_renderer = nullptr;
    float last_delta_time = 0.0f;
    int enter_calls = 0;
    int exit_calls = 0;
    int update_calls = 0;
    int render_calls = 0;
};
