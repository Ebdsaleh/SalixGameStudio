#include "core/Engine.h"


int main(int argc, char* argv[]) { 
    Engine engine;

    // Initialize the engine. If it fails, exit.
    if (!engine.initialize(RendererType::SDL)) {
        return 1;
    }

    // Run the main loop.
    engine.run();

    // shutdown the engine and clean up.
    engine.shutdown();
    
    return 0;
}