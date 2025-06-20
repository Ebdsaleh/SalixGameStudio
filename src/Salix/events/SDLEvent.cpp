// Salix/events/SDLEvent.cpp

#include <Salix/events/SDLEvent.h>

namespace Salix {

    EventHandler::EventHandler(const event_callback_fn& callback_in)
        : event_callback(callback_in)
    {
        // Ensure the callback is valid.
        if (!event_callback) {
            // In a real engine, you might log a fatal error here.
        }
    }

    void EventHandler::poll_events()
    {
        SDL_Event sdl_event;

        while (SDL_PollEvent(&sdl_event))
        {
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                {
                    WindowCloseEvent event;
                    event_callback(event);
                    break;
                }
                case SDL_WINDOWEVENT:
                {
                    if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        WindowResizeEvent event(sdl_event.window.data1, sdl_event.window.data2);
                        event_callback(event);
                    }
                    break;
                }
                case SDL_KEYDOWN:
                {
                    KeyPressedEvent event(sdl_event.key.keysym.sym, sdl_event.key.repeat != 0);
                    event_callback(event);
                    break;
                }
                case SDL_KEYUP:
                {
                    KeyReleasedEvent event(sdl_event.key.keysym.sym);
                    event_callback(event);
                    break;
                }
                case SDL_MOUSEMOTION:
                {
                    MouseMovedEvent event((float)sdl_event.motion.x, (float)sdl_event.motion.y);
                    event_callback(event);
                    break;
                }
                case SDL_MOUSEWHEEL:
                {
                    MouseScrolledEvent event((float)sdl_event.wheel.x, (float)sdl_event.wheel.y);
                    event_callback(event);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    MouseButtonPressedEvent event(sdl_event.button.button);
                    event_callback(event);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                {
                    MouseButtonReleasedEvent event(sdl_event.button.button);
                    event_callback(event);
                    break;
                }
            }
        }
    }

} // namespace Salix