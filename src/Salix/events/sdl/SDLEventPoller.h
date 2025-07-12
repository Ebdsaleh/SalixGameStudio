// =================================================================================
// Filename:    Salix/events/sdl/SDLEventPoller.h
// Author:      SalixGameStudio
// Description: Declares the SDL implementation of the IEventPoller interface.
// =================================================================================
// Salix/events/SDLEventPoller.h
#pragma once

#include <Salix/events/IEventPoller.h> // Inherit from our interface
#include <map>        // For storing raw event callbacks with handles
#include <mutex>      // For thread safety if raw event callbacks are added/removed from different threads

// Forward declare SDL_Event for SDLEventPoller's internal use
// We only include <SDL.h> in the .cpp file to minimize include dependencies
union SDL_Event; // Note: union is correct for SDL_Event

namespace Salix {

    class SDLEventPoller : public IEventPoller {
    public:
        SDLEventPoller(); // Constructor
        ~SDLEventPoller() override; // Destructor, marked override

        // Override the IEventPoller interface methods
        void poll_events(const event_callback_fn& callback) override;

        // Implement the raw event callback registration for SDL_Event*
        RawEventCallbackHandle register_raw_event_callback(RawEventCallback callback) override;
        void unregister_raw_event_callback(RawEventCallbackHandle handle) override;

    private:
        // Internal storage for raw event callbacks, mapped by their handle.
        std::map<RawEventCallbackHandle, RawEventCallback> raw_event_callbacks;
        RawEventCallbackHandle next_callback_handle; // Simple counter for unique handles
        std::mutex callbacks_mutex; // Protects access to raw_event_callbacks
    };

} // namespace Salix