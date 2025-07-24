// =================================================================================
// Filename:    Editor/panels/ScryingMirrorPanel.h
// Author:      SalixGameStudio
// Description: The panel responsible for displaying information about selected objects in the Scene and Workspace.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/panels/IPanel.h>
#include <Editor/EditorContext.h>
#include <memory>

namespace Salix {

    class EDITOR_API ScryingMirrorPanel : public IPanel {
    
    public:

        ScryingMirrorPanel();
        ~ScryingMirrorPanel() override;
        void initialize(EditorContext* context) override;
        void on_gui_render() override;
        void set_visibility(bool visibility) override;
        bool get_visibility() const override;
        void on_event(IEvent& event) override;
        void set_name(const std::string& new_name) override;
        const std::string& get_name() override;
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix