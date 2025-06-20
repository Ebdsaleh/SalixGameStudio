// =================================================================================
// Filename:    Salix/events/IEventPoller.h
// Author:      SalixGameStudio
// Description: Declares the interface for an abstract event poller.
// =================================================================================
#pragma once

#include <functional>

namespace Salix {
    // Forward declare IEvent so we don't need the full header.
    class IEvent;

    class IEventPoller {
    public:
        // A function that can receive an event (a "callback").
        using event_callback_fn = std::function<void(IEvent&)>;

        virtual ~IEventPoller() = default;

        // This method will poll for all pending events and send each one
        // to the provided callback function.
        virtual void poll_events(const event_callback_fn& callback) = 0;
    };
}