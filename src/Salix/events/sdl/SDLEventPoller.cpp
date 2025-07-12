// Salix/events/sdl/SDLEventPoller.cpp
#include <Salix/events/sdl/SDLEventPoller.h>
#include <Salix/events/sdl/SDLEvent.h> // Needed to create our concrete events (KeyPressedEvent, etc.)
#include <SDL.h> // Full SDL include needed here for SDL_Event definition and polling
// #include <Salix/core/Log.h> // Commented out, as you don't have logging yet

namespace Salix {

    SDLEventPoller::SDLEventPoller()
        : next_callback_handle(1) // Start handles from 1
    {
        // SALIX_CORE_INFO("SDLEventPoller created."); // Removed logging for now
    }

    SDLEventPoller::~SDLEventPoller() {
        // SALIX_CORE_INFO("SDLEventPoller destroyed."); // Removed logging for now
    }

    RawEventCallbackHandle SDLEventPoller::register_raw_event_callback(RawEventCallback callback) {
        std::lock_guard<std::mutex> lock(callbacks_mutex); // Protect access
        RawEventCallbackHandle handle = next_callback_handle++;
        raw_event_callbacks[handle] = callback;
        // SALIX_CORE_INFO("Registered raw event callback with handle: {0}", handle); // Removed logging for now
        return handle;
    }

    void SDLEventPoller::unregister_raw_event_callback(RawEventCallbackHandle handle) {
        std::lock_guard<std::mutex> lock(callbacks_mutex); // Protect access
        if (raw_event_callbacks.count(handle)) {
            raw_event_callbacks.erase(handle);
            // SALIX_CORE_INFO("Unregistered raw event callback with handle: {0}", handle); // Removed logging for now
        } else {
            // SALIX_CORE_WARN("Attempted to unregister unknown raw event callback handle: {0}", handle); // Removed logging for now
        }
    }

    void SDLEventPoller::poll_events(const event_callback_fn& event_dispatcher) {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            // 1. Pass the raw SDL_Event to all registered raw event callbacks first.
            //    This is where ImGui_ImplSDL2_ProcessEvent will be called.
            {
                std::lock_guard<std::mutex> lock(callbacks_mutex); // Protect access during iteration
                for (auto const& [handle, callback_fn] : raw_event_callbacks) {
                    // Pass the raw SDL_Event* (as void*) to the callback.
                    callback_fn(&sdl_event); 
                }
            }

            // 2. Then, process the SDL_Event into our Salix IEvent system
            //    Crucially, pass the raw 'sdl_event' to the constructors of SDLEventBase derived classes.
            switch (sdl_event.type)
            {
                case SDL_QUIT: {
                    // WindowCloseEvent does NOT take SDL_Event&, as it's an application event.
                    WindowCloseEvent event; 
                    event_dispatcher(event);
                    break;
                }
                case SDL_WINDOWEVENT: {
                    switch (sdl_event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            // WindowResizeEvent does NOT take SDL_Event&, as it's an application event.
                            WindowResizeEvent event(
                                (unsigned int)sdl_event.window.data1,
                                (unsigned int)sdl_event.window.data2
                            );
                            event_dispatcher(event);
                            break;
                        }
                        // Add other window events if you have them (e.g., SDL_WINDOWEVENT_FOCUS_GAINED/LOST)
                        // If you add focus events, remember they also inherit from IEvent, not SDLEventBase
                        // unless you modify them to directly pass the SDL_Event.
                    }
                    break;
                }
                case SDL_KEYDOWN: {
                    // KeyPressedEvent now takes SDL_Event& as the last parameter
                    KeyPressedEvent event(sdl_event.key.keysym.sym, sdl_event.key.repeat != 0, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                case SDL_KEYUP: {
                    // KeyReleasedEvent now takes SDL_Event& as the last parameter
                    KeyReleasedEvent event(sdl_event.key.keysym.sym, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                case SDL_TEXTINPUT: {
                    // SDL_TEXTINPUT is important for ImGui for text input characters.
                    // If you have a KeyTypedEvent or similar for raw character input,
                    // you would create and dispatch it here. Otherwise, ImGui handles it.
                    // Example (if you have KeyTypedEvent):
                    // KeyTypedEvent event(sdl_event.text.text[0], sdl_event); // Assuming single char and SDL_Event& constructor
                    // event_dispatcher(event);
                    break; // No Salix IEvent created by default for TEXTINPUT, ImGui handles it via raw callback
                }
                case SDL_MOUSEMOTION: {
                    // MouseMovedEvent now takes SDL_Event& as the last parameter
                    MouseMovedEvent event((float)sdl_event.motion.x, (float)sdl_event.motion.y, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    // MouseButtonPressedEvent now takes SDL_Event& as the last parameter
                    MouseButtonPressedEvent event(sdl_event.button.button, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    // MouseButtonReleasedEvent now takes SDL_Event& as the last parameter
                    MouseButtonReleasedEvent event(sdl_event.button.button, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                case SDL_MOUSEWHEEL: {
                    // MouseScrolledEvent now takes SDL_Event& as the last parameter
                    MouseScrolledEvent event((float)sdl_event.wheel.x, (float)sdl_event.wheel.y, sdl_event);
                    event_dispatcher(event);
                    break;
                }
                // Add other SDL event types as needed that you want to convert to Salix IEvents.
            }
        }
    }

} // namespace Salix