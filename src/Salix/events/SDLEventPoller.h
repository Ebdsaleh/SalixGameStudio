// =================================================================================
// Filename:    Salix/events/SDLEventPoller.h
// Author:      SalixGameStudio
// Description: Declares the SDL implementation of the IEventPoller interface.
// =================================================================================
#pragma once

#include <Salix/events/IEventPoller.h>

namespace Salix {
    class SDLEventPoller : public IEventPoller {
    public:
        void poll_events(const event_callback_fn& callback) override;
    };
}