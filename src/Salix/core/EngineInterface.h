// Salix/core/EngineInterface.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/states/IAppState.h>
#include <Salix/core/EngineMode.h>

namespace Salix {
    
    class SALIX_API EngineInterface {
    public:
        virtual void switch_state(AppStateType state) = 0;
        virtual void set_mode(EngineMode mode) = 0;
        virtual EngineMode get_mode() const = 0;
        virtual ~EngineInterface() = default;
        virtual bool is_running() const = 0;
        virtual const bool is_running(bool keep_running) = 0;
        virtual float get_time_scale() const = 0;
        virtual void set_time_scale(float new_time_scale) = 0;
    };
} // namespace Salix