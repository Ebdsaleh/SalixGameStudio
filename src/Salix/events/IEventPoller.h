// =================================================================================
// Filename:    Salix/events/IEventPoller.h
// Author:      SalixGameStudio
// Description: Declares the interface for an abstract event poller.
// =================================================================================
#pragma once

#include <functional>
#include <cstdint> // For uint64_t

namespace Salix {
    class IEvent; // Forward declare IEvent

    // A callback type for raw native events, useful for libraries like ImGui.
    // The void* allows passing any native event type (e.g., SDL_Event*, GLFWevent*).
    using RawEventCallback = std::function<void(void*)>;

    // A simple handle for registered raw event callbacks.
    using RawEventCallbackHandle = uint64_t; // Using uint64_t for a unique ID

    class IEventPoller {
    public:
        // Your existing callback type for processed IEvents.
        using event_callback_fn = std::function<void(IEvent&)>; 

        virtual ~IEventPoller() = default;

        // This method polls for native events and dispatches them as IEvents
        // via the provided callback.
        virtual void poll_events(const event_callback_fn& callback) = 0;

        // Allows a system (like SDLImGui) to register a callback that receives
        // the raw native event (e.g., SDL_Event*). This is the universal hook.
        // Returns a handle for unregistration.
        virtual RawEventCallbackHandle register_raw_event_callback(RawEventCallback callback) = 0;

        // Unregisters a previously registered raw event callback using its handle.
        virtual void unregister_raw_event_callback(RawEventCallbackHandle handle) = 0;

    protected:
        // No members declared here, as it's a pure interface.
    };
} // namespace Salix