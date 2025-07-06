#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitEnums.h>
namespace Salix {

    class IRenderer;
    struct InitContext;

    class SALIX_API IAppState {
    public:
        virtual ~IAppState() = default;

        virtual void on_enter(const InitContext& context) = 0;
        virtual void on_exit() = 0;
        virtual void update(float delta_time) = 0;
        virtual void render(IRenderer* renderer) = 0;
    };
}
