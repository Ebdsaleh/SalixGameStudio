#pragma once

#include <Salix/core/EngineMode.h>  // For EngineMode enum

// Forward declarations to avoid circular dependencies
namespace Salix {
    class Engine;
    class AssetManager;
    class IInputManager;
    class IRenderer;

    struct InitContext {
        Engine * engine = nullptr;
        AssetManager* asset_manager = nullptr;
        IInputManager* input_manager = nullptr;
        IRenderer* renderer = nullptr;
        EngineMode engine_mode = EngineMode::None;
    };
}
