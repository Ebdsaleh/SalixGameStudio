 // Salix/states/OptionsMenuState.h
#pragma once
#include <IAppState.h>

namespace Salix {

    class OptionsMenuState : public IAppState {
        public:
        OptionsMenuState();
        virtual ~OptionsMenuState();

        void on_enter(class Engine* engine) override;
        void on_exit() override;
        void update(float delta_time) override;
        void render(class IRenderer* renderer) override;

    };
} // namespace Salix