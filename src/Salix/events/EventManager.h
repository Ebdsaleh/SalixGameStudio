// =================================================================================
// Filename:    Salix/events/EventManager.h
// Author:      SalixGameStudio
// Description: Declares the EventManager, a system for dispatching events to
//              any number of subscribed listeners based on event categories.
// =================================================================================
#pragma once

#include <Salix/events/IEventListener.h>
#include <vector>
#include <map>
#include <functional>

namespace Salix {

    class EventManager : public IEventListener {
        public:
            EventManager() = default;
            ~EventManager() = default;

            // A listener subscribes to a specific category of events.
            void subscribe(EventCategory category, IEventListener* listener);

            // A listener can unsubscribe from a specific category.
            void unsubscribe(EventCategory category, IEventListener* listener);

            // The Engine calls this method to push a new event into the system.
            // The EventManager will then forward it to all relevant subscribers.
            void dispatch(IEvent& event);
        
        private:
            // This is the core data structure: a map where the key is an event
            // category, and the value is a list of all listeners subscribed
            // to that category.
            std::map<EventCategory, std::vector<IEventListener*>> subscribers;
    };
}  // namespace Salix