// SDLEvent.cpp
// src/Salix/events/SDLEvent.cpp

#include "SDLEvent.h"

namespace Salix {

    // Constructor implementation
    EventHandler::EventHandler(const event_callback_fn& callback)
        : m_event_callback(callback)
    {
        // Ensure the callback is valid.
        if (!m_event_callback) {
            // In a real engine, you might log a fatal error here.
            // For now, we'll assume the user provides a valid one.
        }
    }

    // This is the core function that translates SDL events to Salix events.
    void EventHandler::poll_events()
    {
        SDL_Event sdl_event;

        // Process all events in SDL's queue.
        while (SDL_PollEvent(&sdl_event))
        {
            switch (sdl_event.type)
            {
                // --- APPLICATION EVENTS ---
                case SDL_QUIT:
                {
                    WindowCloseEvent event;
                    m_event_callback(event);
                    break;
                }
                case SDL_WINDOWEVENT:
                {
                    if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        WindowResizeEvent event(sdl_event.window.data1, sdl_event.window.data2);
                        m_event_callback(event);
                    }
                    // Other window events like focus gain/loss could be handled here.
                    break;
                }

                // --- KEYBOARD EVENTS ---
                case SDL_KEYDOWN:
                {
                    // Note: sdl_event.key.keysym.sym is the keycode
                    // Note: sdl_event.key.repeat is non-zero if this is a repeat event
                    KeyPressedEvent event(sdl_event.key.keysym.sym, sdl_event.key.repeat != 0);
                    m_event_callback(event);
                    break;
                }
                case SDL_KEYUP:
                {
                    KeyReleasedEvent event(sdl_event.key.keysym.sym);
                    m_event_callback(event);
                    break;
                }

                // --- MOUSE EVENTS ---
                case SDL_MOUSEMOTION:
                {
                    MouseMovedEvent event((float)sdl_event.motion.x, (float)sdl_event.motion.y);
                    m_event_callback(event);
                    break;
                }
                case SDL_MOUSEWHEEL:
                {
                    MouseScrolledEvent event((float)sdl_event.wheel.x, (float)sdl_event.wheel.y);
                    m_event_callback(event);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    MouseButtonPressedEvent event(sdl_event.button.button);
                    m_event_callback(event);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                {
                    MouseButtonReleasedEvent event(sdl_event.button.button);
                    m_event_callback(event);
                    break;
                }
            }
        }
    }

} // namespace Salix