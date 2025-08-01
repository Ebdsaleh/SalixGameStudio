// =================================================================================
// Filename:    Editor/panels/ScryingMirrorPanel.h
// Author:      SalixGameStudio
// Description: The panel responsible for displaying information about selected objects in the Scene and Workspace.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/panels/LockablePanel.h>
#include <Editor/EditorContext.h>
#include <memory>

namespace Salix {

    class EDITOR_API ScryingMirrorPanel : public LockablePanel {
    
    public:

        ScryingMirrorPanel();
        ~ScryingMirrorPanel() override;
        void initialize(EditorContext* context) override;
        bool is_panel_derived() const override { return true; }

        void on_event(IEvent& event) override;
        
    protected:
        void on_panel_gui_update() override; // Main content
        ImGuiWindowFlags get_window_flags() const override;
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix