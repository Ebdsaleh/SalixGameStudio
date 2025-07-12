// =================================================================================
// Filename:    src/Launcher/main.cpp
// Author:      SalixGameStudio
// Description: The main entry point for the Salix Game Engine application.
// =================================================================================

// We only need to include the headers for the components we directly interact with here.
#include <Salix/core/Core.h>
#include <Salix/core/InitEnums.h>
#include <Salix/core/Engine.h>
#include <Salix/window/IWindow.h>     // For WindowConfig
#include <Salix/rendering/IRenderer.h> // For RendererType
#include <Salix/core/ITimer.h>         // For TimerType
#include <Salix/core/InitContext.h>
#include <Salix/states/IAppState.h>
#include <Salix/core/ApplicationConfig.h>
#include <iostream>
#include <memory>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

// Command line argument helper for use later, not now.
std::string get_arg_value(const std::vector<std::string>& args, const std::string& key) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == key && (i + 1) < args.size()) {
            return args[i+1];
        }
    }
    return "";
}

// The main entry point for the application.
// We add (void) to argc and argv to tell the compiler we are intentionally not using them,
// which silences the C4100 warning.
int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);  // for parsing command line arguments
    std::cout << "SalixGameStudio.exe" << std::endl;
 
    // Enable memory leak checks at shutdown
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    
    
    // 1. Create the configuration for our application.
    Salix::ApplicationConfig config;
    config.window_config.title = "Salix Game Studio v0.1";
    config.window_config.width = 1280; 
    config.window_config.height = 720;
    config.gui_type = Salix::GuiType::ImGui;
    config.initial_state = Salix::AppStateType::Launch;
    config.renderer_type = Salix::RendererType::SDL;
    config.timer_type = Salix::TimerType::SDL;
    config.target_fps = 60;


    // 2. Create the main engine object using a smart pointer for safety.
    auto engine = std::make_unique<Salix::Engine>();

    // 3. Initialize the engine with our configuration and desired subsystems.
    // This now calls the new, correct initialize method.
    if (engine->initialize(config)) {
        // engine->switch_state(Salix::AppStateType::Launch);
        // 4. If initialization was successful, run the main game loop.
        engine->run();
    } else {
        std::cerr << "Fatal Error: Engine failed to initialize." << std::endl;
    }
    std::cout << "Engine is shutting down." << std::endl;
    // 5. Shutdown the engine, which cleans up all subsystems.
    // The unique_ptr will also call the destructor when it goes out of scope,
    // but explicit shutdown is good practice.
    engine->shutdown();

    // The program exits here.
    return 0;
}