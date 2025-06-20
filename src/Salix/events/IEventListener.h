// =================================================================================
// Filename:    Salix/events/IEventListener.h
// Author:      SalixGameStudio
// Description: Declares the IEventListener interface, the contract for any
//              object that wishes to subscribe to and receive engine events.
// =================================================================================
#pragma once

// We need to know about the base IEvent class to receive events.
#include <Salix/events/IEvent.h>

namespace Salix {

    class IEventListener {
        public:
        // Virtual dectructors are essential to all interfaces.
        virtual ~IEventListener() = default;

        // This is the core method of the pattern.
        // The EventManager will call this function on every listener when a
        // relevant event occurs, passing the event details.
        virtual void on_event(IEvent& event) = 0;
    };

} // namespace Salix