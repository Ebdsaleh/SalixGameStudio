// =================================================================================
// Filename:    Salix/events/EventManager.h
// Author:      SalixGameStudio
// Description: Declares the EventManager, a system for dispatching events to
//              any number of subscribed listeners based on event categories.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <Salix/events/IEventListener.h>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace Salix {

    class SALIX_API EventManager {
        public:
            EventManager();
            ~EventManager();

            // A listener subscribes to a specific category of events.
            void subscribe(EventCategory category, IEventListener* listener);

            // A listener can unsubscribe from a specific category.
            void unsubscribe(EventCategory category, IEventListener* listener);

            // The Engine calls this method to push a new event into the system.
            // The EventManager will then forward it to all relevant subscribers.
            void dispatch(std::unique_ptr<IEvent> event);
            // The new overload for cloning.
            void dispatch(const IEvent& event);           
            void process_queue();
            bool is_queue_empty() const;
 
        private:
            // This is the core data structure: a map where the key is an event
            // category, and the value is a list of all listeners subscribed
            // to that category.
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
            
    };
}  // namespace Salix