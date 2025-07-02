 // Salix/states/OptionsMenuState.h
#pragma once
#include <Salix/states/IAppState.h>

namespace Salix {
    struct InitContext;
    class OptionsMenuState : public IAppState {
        public:
            OptionsMenuState();
            virtual ~OptionsMenuState();

            void on_enter(const InitContext& new_context) override;
            void on_exit() override;
            void update(float delta_time) override;
            void render(class IRenderer* renderer) override;
            
        private:
            InitContext context;
    };
} // namespace Salix