// =================================================================================
// Filename:    Salix/core/Core.h
// Author:      SalixGameStudio
// Description: Defines core platform-specific macros for the Salix Engine API,
//              handling DLL import/export.
// =================================================================================
#pragma once

#ifdef _WIN32
    #ifdef SALIX_BUILD_ENGINE // This will be defined when we build our Engine.exe
        #define SALIX_API __declspec(dllexport)
    #else // This will be defined when our Game.dll uses the Engine
        #define SALIX_API __declspec(dllimport)
    #endif
#else
    #error Salix Engine only supports Windows for now!
#endif