// =================================================================================
// Filename:    Salix/events/IEventPoller.h
// Author:      SalixGameStudio
// Description: Declares the interface for an abstract event poller.
// =================================================================================
#pragma once

#include <functional>
#include <cstdint>
#include <vector>

namespace Salix {
    // Forward declare IEvent so we don't need the full header.
    class IEvent;

    // A callback type for raw native events, useful for libraries like ImGui.
    // The void* allows passing any native event type (e.g., SDL_Event*, GLFWevent*).
    using RawEventCallback = std::function<void(void*)>;

    // A simple handle for registered raw event callbacks.
    using RawEventCallbackHandle = uint64_t; // Using uint64_t for a unique ID

    class IEventPoller {
    public:
        // A function that can receive an event (a "callback").
        using event_callback_fn = std::function<void(IEvent&)>;

        virtual ~IEventPoller() = default;

        // This method will poll for all pending events and send each one
        // to the provided callback function.
        virtual void poll_events(const event_callback_fn& callback) = 0;
        
        // Allows a system (like SDLImGui) to register a callback that receives
        // the raw native event (e.g., SDL_Event*).
        virtual void register_raw_event_callback(RawEventCallback callback) = 0;
        virtual void unregister_raw_event_callback(RawEventCallback callback) = 0; 
    
        protected:
        // No members declared here, as it's a pure interface.// Nothing here

    };
}