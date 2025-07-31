// =================================================================================
// Filename:    Editor/EditorAPI.h
// Author:      SalixGameStudio
// Description: Defines the public API that the Editor.dll exports for the Engine.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>        // For the SALIX_API macro
#include <Salix/states/IAppState.h> // For IAppState and AppStateType
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#ifdef SALIX_BUILD_EDITOR
    // Exports the editor state's factory function
    extern "C" {
        EDITOR_API Salix::IAppState* create_editor_state(Salix::AppStateType state_type);
        EDITOR_API void set_imgui_context(ImGuiContext* context);
        
    }
#else
    extern "C" {
        EDITOR_API Salix::IAppState* create_editor_state(Salix::AppStateType state_type);
        EDITOR_API void set_imgui_context(ImGuiContext* context);
        
    } 
#endif

