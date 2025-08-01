// Editor/EditorFactory.cpp
// =================================================================================
// Filename:    Editor/EditorFactory.cpp
// Author:      SalixGameStudio
// Description: Implements the factory function for creating game-specific states.
// =================================================================================
#include <Editor/EditorAPI.h>
#include <Editor/states/EditorState.h>
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

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

// This will allow the ImGui context to traverse the Dll scope barrier.
void set_imgui_context(ImGuiContext* context) {
    ImGui::SetCurrentContext(context);
    ImGuizmo::SetImGuiContext(context);
}

