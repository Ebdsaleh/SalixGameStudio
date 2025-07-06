// src/Sandbox/TestProject/main.cpp
// This is the main entry point for the standalone TestProject game executable.

#include <Salix/core/Engine.h>
#include <Salix/core/InitContext.h>
#include <Game/states/GameState.h> // Assuming GameState is defined in Game.dll and linked via Game.lib
#include <Salix/states/IAppState.h>
#include <Salix/window/IWindow.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/core/ITimer.h>
#include <memory>
#include <iostream>
#include <string>
#include <filesystem> // For path manipulation

int main(int /*argc*/, char* /*argv[]*/) {
    std::cout << "TestProject.exe: Game Application Starting..." << std::endl;

    // Define window configuration. This would later be loaded from a project configuration file.
    Salix::WindowConfig config;
    config.title = "TestProject";
    config.width = 1280;
    config.height = 720;

    Salix::Engine engine;

    // Initialize the core engine systems (window, renderer, timer).
    // This sets up the fundamental components needed before loading any game-specific content.
    // The engine's internal InitContext is populated during this step.
    if (!engine.initialize(config, Salix::RendererType::SDL, Salix::TimerType::SDL, 60)) {
        std::cerr << "TestProject.exe Error: Engine initialization failed!" << std::endl;
        return 1;
    }

    // Retrieve the engine's current InitContext.
    // This context will carry engine-wide information and will be passed to app states.
    Salix::InitContext context = engine.make_context();

    // Determine paths for the project file and project DLL.
    // In a deployed game, these paths are typically relative to the executable's location.
    // Here, we derive them assuming the executable is in the 'build/' directory,
    // the project folder ('TestProject/') is also in 'build/',
    // and 'TestProject.dll' is directly in 'build/'.
    std::filesystem::path exe_path = std::filesystem::current_path(); // Gets the directory where the EXE is run from.

    // Set the path to the .salixproj file within the copied project folder.
    context.project_file_path = (exe_path / "TestProject" / "TestProject.salixproj").string();

    // Set the path to the compiled project DLL.
    context.project_dll_path = (exe_path / "TestProject.dll").string();

    std::cout << "TestProject.exe: Project file path: " << context.project_file_path << std::endl;
    std::cout << "TestProject.exe: Project DLL path: " << context.project_dll_path << std::endl;

    // Change the engine's state to GameState.
    // It is assumed that the Engine's 'switch_state' method will internally
    // use the 'context' (which now contains project paths) when initializing
    // the new state (GameState).
    engine.switch_state(Salix::AppStateType::Game);

    // Run the main engine loop. This loop drives the game's update and render cycles.
    engine.run();

    // Shutdown the engine and release all resources.
    engine.shutdown();

    std::cout << "TestProject.exe: Game Application Shutting Down." << std::endl;
    return 0;
}