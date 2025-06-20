// =================================================================================
// Filename:    Salix/events/SDLEventPoller.cpp
// Author:      SalixGameStudio
// Description: Implements the SDLEventPoller.
// =================================================================================
#include <Salix/events/SDLEventPoller.h>
#include <Salix/events/SDLEvent.h> // Needed to create our concrete events
#include <SDL.h>

namespace Salix {
    void SDLEventPoller::poll_events(const event_callback_fn& callback) {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type)
            {
                case SDL_QUIT: {
                    WindowCloseEvent event;
                    callback(event);
                    break;
                }
                case SDL_KEYDOWN: {
                    KeyPressedEvent event(sdl_event.key.keysym.sym, sdl_event.key.repeat != 0);
                    callback(event);
                    break;
                }
                case SDL_KEYUP: {
                    KeyReleasedEvent event(sdl_event.key.keysym.sym);
                    callback(event);
                    break;
                }
                // ... add all other event translations here (mouse, etc.) ...
            }
        }
    }
}