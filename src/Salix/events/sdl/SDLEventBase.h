// Salix/events/sdl/SDLEventBase.h
// New file for an intermediate base class for SDL-originating events.
#pragma once

#include <Salix/events/IEvent.h>
#include <SDL.h> // We include SDL.h here because this class directly interacts with SDL_Event
namespace Salix {

    // SDLEventBase is an abstract base class for all Salix events
    // that originate from an SDL_Event. It provides a way to access
    // the raw SDL_Event* if needed (e.g., for ImGui).
    class SDLEventBase : public IEvent {
    public:
        // Constructor takes a reference to the raw SDL_Event.
        // It's important to store a reference or a pointer, not a copy,
        // if the event data might be large or dynamic.
        // For our purposes, a const reference is suitable as we just need to read from it.
         // The parameter name is 'sdl_event_in'.
        SDLEventBase(const SDL_Event& sdl_event_in) // This is the *parameter*
            : sdl_event(sdl_event_in) {} // This initializes the *member variable* 'sdl_event' with the *parameter* 'sdl_event_in'

        // Overrides IEvent::get_native_handle() to return a pointer to the raw SDL_Event.
        // This is crucial for ImGui_ImplSDL2_ProcessEvent().
        void* get_native_handle() override {
            // We cast away constness because ImGui_ImplSDL2_ProcessEvent expects SDL_Event*,
            // not const SDL_Event*. It doesn't modify the event, so this is generally safe in this context.
            return const_cast<SDL_Event*>(&sdl_event);
        }
        // SDLEventBase itself is still abstract because it doesn't define
        // get_event_type(), get_name(), get_category_flags(), or to_string().
        // These will be defined by its concrete derived classes (e.g., KeyPressedEvent, MouseMovedEvent).

    protected:
        const SDL_Event& sdl_event; // Reference to the underlying SDL event
    };
}  // namespace Salix