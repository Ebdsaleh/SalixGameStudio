// Salix/core/InitContext.h
#pragma once

#include <Salix/core/Engine.h>     // For EngineMode
#include <Salix/assets/AssetManager.h> // For AssetManager*

namespace Salix {
    struct InitContext {
        AssetManager* asset_manager = nullptr;
        EngineMode engine_mode = EngineMode::None;
    };
}