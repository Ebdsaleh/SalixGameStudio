// =================================================================================
// Filename:    src/main.cpp
// Author:      SalixGameStudio
// Description: The main entry point for the Salix Game Engine application.
// =================================================================================

// We only need to include the headers for the components we directly interact with here.

#include <Salix/core/Engine.h>
#include <Salix/window/IWindow.h>     // For WindowConfig
#include <Salix/rendering/IRenderer.h> // For RendererType
#include <Salix/core/ITimer.h>         // For TimerType
#include <cereal/cereal.hpp>
#include <iostream>
#include <memory>

// The main entry point for the application.
// We add (void) to argc and argv to tell the compiler we are intentionally not using them,
// which silences the C4100 warning.
int main(int /*argc*/, char* /*argv*/[]) {

    // 1. Create the configuration for our window.
    Salix::WindowConfig config;
    config.title = "Salix Game Studio v0.1";
    config.width = 1280;
    config.height = 720;

    // 2. Create the main engine object using a smart pointer for safety.
    auto engine = std::make_unique<Salix::Engine>();

    // 3. Initialize the engine with our configuration and desired subsystems.
    // This now calls the new, correct initialize method.
    if (engine->initialize(config, Salix::RendererType::SDL, Salix::TimerType::SDL, 60)) {
        // 4. If initialization was successful, run the main game loop.
        engine->run();
    } else {
        std::cerr << "Fatal Error: Engine failed to initialize." << std::endl;
    }

    // 5. Shutdown the engine, which cleans up all subsystems.
    // The unique_ptr will also call the destructor when it goes out of scope,
    // but explicit shutdown is good practice.
    engine->shutdown();

    // The program exits here.
    return 0;
}