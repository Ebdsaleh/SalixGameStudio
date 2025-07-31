// =================================================================================
// Filename:    Salix/events/EventManager.cpp
// Author:      SalixGameStudio
// Description: Implements the EventManager, a system for dispatching events to
//              subscribed listeners based on event categories.
// =================================================================================
#include <Salix/events/EventManager.h>
#include <algorithm> // For std::find

namespace Salix {
    struct EventManager::Pimpl {
        std::map<EventCategory, std::vector<IEventListener*>> subscribers;
    };
    
    EventManager::EventManager() : pimpl(std::make_unique<Pimpl>()) {}

    EventManager::~EventManager() = default;

    void EventManager::subscribe(EventCategory category, IEventListener* listener) {
        auto& listener_list = pimpl->subscribers[category];

        // --- THE FIX IS HERE ---
        // We check if the iterator returned by std::find is equal to the end iterator.
        // If it is, it means the listener was NOT found, and it's safe to add.
        if (std::find(listener_list.begin(), listener_list.end(), listener) == listener_list.end()) {
            // If not found, add the new listener.
            listener_list.push_back(listener);
        }
    }

    void EventManager::unsubscribe(EventCategory category, IEventListener* listener) {
        // Check is a subscriber list for this category even exists.
        if (pimpl->subscribers.count(category)) {
            auto& listener_list = pimpl->subscribers.at(category);

            // The 'erase-remove idiom' is a standard C++ way to efficiently
            // remove an element from a vector.
            listener_list.erase(std::remove(listener_list.begin(), listener_list.end(), listener), listener_list.end());

        }
    }

    void EventManager::dispatch(IEvent& event) {

         if (event.should_block()) {
            event.handled = true;
            return;
        }
        // When an event comes in, we need to notify listeners for all categories
        // that this event belongs to.

        // We can't just iterate the map, because an event like KeyPressed needs to
        // be sent to listeners of EventCategory::Keyboard AND EventCategory::Input.
        // We must check every category we have subscribers for.
        for (const auto& pair : pimpl->subscribers) {
            EventCategory category = pair.first;
            const auto& listener_list = pair.second;

            // Use our 'is_in_category' helper to check if the event matches the subscription category.
            if (event.is_in_category(category)) {
                // We create a copy of the listener list before iterating. This is a very important
                // safety measure. It prevents crashes if a listener decides to unsubscribe itself
                // (or another listener) from within its on_event method, which would otherwise
                // invalidate the iterator of the loop we are in.
                auto listeners_to_notify = listener_list;
                for (auto* listener : listeners_to_notify) {
                    listener->on_event(event);

                    // If an event is "handled" by a listener, we can stop propagating it.
                    if (event.handled) {
                        return; // Exit the dispatch function entirely.
                    }
                }
            }
        }
    }

} // namespace Salix