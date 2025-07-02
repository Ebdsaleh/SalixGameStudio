#pragma once
#include <Salix/core/Core.h>

namespace Salix {

    class IRenderer;
    struct InitContext;

    enum class AppStateType {
        None,
        Launch,
        Editor,
        Game,
        Options
    };

    class SALIX_API IAppState {
    public:
        virtual ~IAppState() = default;

        virtual void on_enter(const InitContext& context) = 0;
        virtual void on_exit() = 0;
        virtual void update(float delta_time) = 0;
        virtual void render(IRenderer* renderer) = 0;
    };
}
