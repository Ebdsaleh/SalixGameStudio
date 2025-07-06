// Salix/core/ApplicationEventListener.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/events/IEventListener.h> // Inherit from IEventListener
#include <Salix/events/IEvent.h>        // For IEvent
#include <Salix/events/SDLEvent.h>       // For WindowCloseEvent type (or any other application events)
#include <memory>

namespace Salix {

    // Forward declare the Engine class as we'll need a pointer to it.
    // We only need the declaration here to avoid circular includes.
    class Engine; 

    class SALIX_API ApplicationEventListener : public IEventListener {
    public:
        // Constructor takes a pointer to the Engine, allowing it to signal shutdown.
        ApplicationEventListener();
        ~ApplicationEventListener() override;

        // Implement the IEventListener interface method.
        // This is where application-level events will be handled.
        void on_event(IEvent& event) override;

        bool initialize(Engine* engine_ptr);
    private:
        // A non-owning pointer to the Engine instance.
        // This listener will use this pointer to request engine shutdown.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix