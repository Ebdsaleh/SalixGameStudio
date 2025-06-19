// LaunchState.h
#pragma once
#include "IAppState.h"
class Engine;
class IInputManager;
class LaunchState : public IAppState{
    public:
        LaunchState();
        virtual ~LaunchState();

        // Implement IAppState interface
        void on_enter(class Engine* engine) override;
        void on_exit() override;
        void update(float delta_time) override;
        void render(class IRenderer* renderer) override;
    
    private:
        private:
    // Non-owning pointers to the systems we need to talk to.
    Engine* engine_ptr;
    IInputManager* input_manager_ptr;
};
