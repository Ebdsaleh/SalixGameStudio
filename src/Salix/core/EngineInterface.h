// Salix/core/EngineInterface.h
#pragma once
#include <Salix/states/IAppState.h>
#include <Salix/core/EngineMode.h>

namespace Salix {

    class EngineInterface {
    public:
        virtual void switch_state(AppStateType state) = 0;
        virtual void set_mode(EngineMode mode) = 0;
        virtual EngineMode get_mode() const = 0;
        virtual ~EngineInterface() = default;
    };
} // namespace Salix