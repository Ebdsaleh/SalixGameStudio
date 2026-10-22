#pragma once

#include <Salix/events/IEventPoller.h>
#include <Salix/events/IEvent.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

class MockIEventPoller : public Salix::IEventPoller {
public:
    void poll_events(const event_callback_fn& callback) override {
        ++poll_calls;
        for (auto& event : queued_events) {
            if (event) {
                callback(*event);
            }
        }
    }

    Salix::RawEventCallbackHandle register_raw_event_callback(Salix::RawEventCallback callback) override {
        const auto handle = next_handle++;
        raw_callbacks.emplace(handle, std::move(callback));
        return handle;
    }

    void unregister_raw_event_callback(Salix::RawEventCallbackHandle handle) override {
        raw_callbacks.erase(handle);
    }

    void queue_event(std::unique_ptr<Salix::IEvent> event) {
        queued_events.push_back(std::move(event));
    }

    void clear_events() {
        queued_events.clear();
    }

    void emit_raw(void* native_event) {
        for (auto& [_, callback] : raw_callbacks) {
            callback(native_event);
        }
    }

    std::size_t raw_callback_count() const {
        return raw_callbacks.size();
    }

    int poll_calls = 0;

private:
    Salix::RawEventCallbackHandle next_handle = 1;
    std::vector<std::unique_ptr<Salix::IEvent>> queued_events;
    std::unordered_map<Salix::RawEventCallbackHandle, Salix::RawEventCallback> raw_callbacks;
};
