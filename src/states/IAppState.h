// IAppState
#pragma once

// Forward declations
class Engine;
class IRenderer;

class IAppState {
    public:
        virtual ~IAppState() = default;

        // Called once when the state is entered.
        virtual void on_enter(Engine* engine) = 0;
        // Called once when the state is exited.
        virtual void on_exit() = 0;

        // Called every frame.
        virtual void update(float delta_time) = 0;
        virtual void render(IRenderer* renderer) = 0;
};