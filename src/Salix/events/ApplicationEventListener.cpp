// Salix/core/ApplicationEventListener.cpp
#include <Salix/events/ApplicationEventListener.h> // Include its own header
#include <Salix/core/Engine.h>                 // To access the Engine pointer
#include <Salix/events/IEvent.h>               // To access IEvent types
#include <Salix/events/sdl/SDLEvent.h>              // To access WindowCloseEvent
#include <iostream>                             // For debug output

namespace Salix {

    struct ApplicationEventListener::Pimpl {
        Engine* engine_ptr = nullptr;
    };

    ApplicationEventListener::ApplicationEventListener()
        : pimpl(std::make_unique<Pimpl>()) {
        // You might add a debug print here if you have a logging system.
        std::cout << "ApplicationEventListener created, linked to Engine." << std::endl;
    }

    ApplicationEventListener::~ApplicationEventListener() {
        // You might add a debug print here.
        std::cout << "ApplicationEventListener destroyed." << std::endl;
    }

    bool ApplicationEventListener::initialize(Engine* engine_ptr) {
        if (!engine_ptr) {
            std::cerr << "ApplicationEventListener::initialize - Failed to initialize! " << 
                "Received a nullptr for the 'engine_ptr' referernce." << std::endl;
                return false;
        }
        pimpl->engine_ptr = engine_ptr;
        std::cout << "ApplicationEventListener::initialize - Initialization successful..." << 
            std::endl;
        return true;
    }

    void ApplicationEventListener::on_event(IEvent& event) {
        // Handle WindowCloseEvent directly by this listener.
        if (event.get_event_type() == EventType::WindowClose) {
            if (pimpl->engine_ptr) { // Ensure the engine pointer is valid
                // Access the Engine's Pimpl (assuming it's accessible or through a public method like request_shutdown())
                // For simplicity, directly accessing pimpl->is_running for now,
                // but a public Engine::request_shutdown() method would be cleaner.
                pimpl->engine_ptr->is_running(false); 
                std::cout << "DEBUG: ApplicationEventListener - WindowCloseEvent received, signaling Engine to shut down." << std::endl;
                
            }
        }
    }

} // namespace Salix