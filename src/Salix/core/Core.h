// =================================================================================
// Filename:    Salix/core/Core.h
// Author:      SalixGameStudio
// Description: Defines core platform-specific macros for the Salix Engine API,
//              handling DLL import/export.
// =================================================================================
#pragma once
#include <Salix/core/EngineInfo.h>
#ifdef _WIN32
    // --- FOR THE ENGINE ---
    #ifdef SALIX_BUILD_ENGINE // Defined when building the Engine exe
        #define SALIX_API __declspec(dllexport)
    #else // Defined when the Game.dll uses the Engine
        #define SALIX_API __declspec(dllimport)
    #endif

    // --- FOR THE GAME ---
    #ifdef SALIX_BUILD_GAME // Defined when building the Game DLL
        #define GAME_API __declspec(dllexport)
    #else   // Defined when the Engine uses the DLL
        #define GAME_API __declspec(dllimport)
    #endif
#else
    #error Salix Engine only supports Windows for now!
#endif