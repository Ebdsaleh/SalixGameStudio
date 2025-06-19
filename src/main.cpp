#include <Salix/core/Engine.h>
using namespace Salix;

int main(int argc, char* argv[]) { 
    Engine engine;
    // Create the window configuration data
    WindowConfig config;

    // We could change these values later, based on a settings file.
    config.title = "Salix Game Studio";
    config.width = 1280;
    config.height = 720;
    config.renderer_type = RendererType::SDL;

    // give a framerate limit here. in the future, we can get it from a settings file.
    int fps_target = 60;
    // explicitly set the TimerType
    TimerType timer_type = TimerType::SDL;
    // Initialize the engine. If it fails, exit.
    if (!engine.initialize(config, timer_type, fps_target)) {
        return 1;
    }

    // Run the main loop.
    engine.run();

    // shutdown the engine and clean up.
    engine.shutdown();
    
    return 0;
}