// Salix/states/IAppState
#pragma once

namespace Salix {

    // Forward declations
    class Engine;
    class IRenderer;

    // A type-safe enum to indentify the application states.
    enum class AppStateType {
        None,
        Launch,
        Editor,
        Game,
        Options
    };

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
} // namespace Salix