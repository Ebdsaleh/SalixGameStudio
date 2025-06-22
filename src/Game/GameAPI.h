// =================================================================================
// Filename:    Game/GameAPI.h
// Author:      SalixGameStudio
// Description: Defines the public Application Programming Interface (API) that
//              the Game.dll exports for the Salix Engine to use.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>        // For the SALIX_API export macro
#include <Salix/states/IAppState.h> // For IAppState and AppStateType

// This is the function signature for the game-state factory.
// It must be exported from the Game.dll so the Engine can find it.
// We use extern "C" to prevent C++ name mangling, which makes the function
// much easier and more reliable to load at runtime.
extern "C" SALIX_API Salix::IAppState* create_state(Salix::AppStateType state_type);