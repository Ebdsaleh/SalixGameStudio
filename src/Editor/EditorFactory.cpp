// Editor/EditorFactory.cpp
// =================================================================================
// Filename:    Editor/EditorFactory.cpp
// Author:      SalixGameStudio
// Description: Implements the factory function for creating game-specific states.
// =================================================================================
#include <Editor/EditorAPI.h>
#include <Editor/states/EditorState.h>

Salix::IAppState* create_editor_state(Salix::AppStateType state_type) {
    switch (state_type) {
        case Salix::AppStateType::Editor:
            // Now the DLL will create the GameState.
            return new Salix::EditorState();


        default:
            // If the engine asks for a state this DLL doesn't know about, return null.
            return nullptr;
    }
}