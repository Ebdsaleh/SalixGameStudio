// ITimer.h
#pragma once

namespace Salix {
    enum class TimerType {
        SDL,
        Chrono,
    };

    class ITimer {
        public:
            virtual ~ITimer() = default;

            // A contract that all timers must know how to:
            virtual void set_target_fps(int fps) = 0;
            virtual void tick() = 0;
            virtual float get_delta_time() const = 0;

    };
} // namespace Salix