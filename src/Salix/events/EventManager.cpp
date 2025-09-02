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
        std::vector<std::unique_ptr<IEvent>> event_queue; 
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

    void EventManager::dispatch(std::unique_ptr<IEvent> event) {
        // Check the block flag here. If it's true, we
        // mimic the old behavior by simply returning and not adding
        // the event to the queue. It's effectively discarded.
        if (event->should_block()) {
            event->handled = true;
            return;
        }

        // If the event is not blocked, add it to the queue for later processing.
        pimpl->event_queue.push_back(std::move(event));
    }


    // Overloaded dispatch method.
    void EventManager::dispatch(const IEvent& event) {
        // Call the unique_ptr overload with a clone of the event.
        dispatch(event.clone());
    }


    void EventManager::process_queue() {
        // Process all events currently in the queue for this frame.
        for (const auto& event_ptr : pimpl->event_queue) {
            IEvent& event = *event_ptr;

            // This is the full logic from your old dispatch method.
            for (const auto& pair : pimpl->subscribers) {
                EventCategory category = pair.first;
                const auto& listener_list = pair.second;

                if (event.is_in_category(category)) {
                    // Create a copy to prevent iterator invalidation if a listener unsubscribes.
                    auto listeners_to_notify = listener_list;
                    for (auto* listener : listeners_to_notify) {
                        listener->on_event(event);

                        // If an event is handled, stop sending it to other listeners.
                        if (event.handled) {
                            break; // Exit this inner loop for this event.
                        }
                    }
                }
                // If the event was handled, we can also stop checking other categories for it.
                if (event.handled) {
                    break;
                }
            }
        }
        // Clear the queue after processing all events for this frame.
        pimpl->event_queue.clear();
    }

} // namespace Salix