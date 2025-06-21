// =================================================================================
// Filename:    Salix/core/ITimer.h
// Author:      SalixGameStudio
// Description: Declares the ITimer interface for time management.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>

namespace Salix {

    enum class TimerType {
        SDL,
        Chrono
    };

    class SALIX_API ITimer {
    public:
        virtual ~ITimer() = default;

        virtual void set_target_fps(int fps) = 0;

        // --- NEW, COMBINED METHODS ---
        // Marks the beginning of a frame, calculating and storing delta time.
        virtual void tick_start() = 0;

        // Delays if necessary to meet the target FPS.
        virtual void tick_end() = 0;

        // Returns the delta time calculated by the most recent tick_start().
        virtual float get_delta_time() const = 0;
    };

} // namespace Salix