// Salix/core/InitContext.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitEnums.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/core/EngineMode.h>  // For EngineMode enum
#include <string>

// Forward declarations to avoid circular dependencies
namespace Salix {
    class Engine;
    class AssetManager;
    class IInputManager;
    class IRenderer;
    class IWindow;
    class ITimer;
    class IGui;
    class IEventPoller;
    class IEventManager;

    struct SALIX_API InitContext {
        RendererType  renderer_type = RendererType::None;
        GuiType gui_type = GuiType::None;
        IGui* gui;
        ITimer* timer;
        TimerType timer_type;
        EngineInterface* engine = nullptr;
        AssetManager* asset_manager = nullptr;
        IInputManager* input_manager = nullptr;
        IRenderer* renderer = nullptr;
        IWindow* window = nullptr;
        IEventPoller* event_poller;
        IEventManager* event_manager;
        EngineMode engine_mode = EngineMode::None;
        std::string project_file_path;
        std::string  project_dll_path;
    };
}
