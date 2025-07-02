// Salix/states/LaunchState.h
#pragma once
#include <Salix/states/IAppState.h>

namespace Salix {

    struct InitContext;
    class IInputManager;
    class LaunchState : public IAppState{
        public:
            LaunchState();
            virtual ~LaunchState();

            // Implement IAppState interface
            void on_enter(const InitContext& new_context) override;
            void on_exit() override;
            void update(float delta_time) override;
            void render(class IRenderer* renderer) override;
        
        private:
            private:
        // Non-owning pointers to the systems we need to talk to.
        InitContext context;
        IInputManager* input_manager_ptr;
    };
} // namespace Salix