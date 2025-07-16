// =================================================================================
// Filename:    Editor/behaviours/EditorBehaviour.h
// Author:      SalixGameStudio
// Description: An abstract class for all Ui elements in the editor to inherit from.
// =================================================================================
#pragma once

#include <Editor/EditorAPI.h>

namespace Salix {

// Forward declaration for the event system we'll build later
class EditorEvent;

class EDITOR_API EditorBehaviour {
public:
    virtual ~EditorBehaviour() = default;

    // Called once when the behaviour is first initialized.
    virtual void on_start() {}

    // Called every frame, for logic that needs to run continuously.
    virtual void on_update(float delta_time) {}

    // Called when a specific editor event is dispatched.
    virtual void on_event(EditorEvent& event) {}
};

} // namespace Salix