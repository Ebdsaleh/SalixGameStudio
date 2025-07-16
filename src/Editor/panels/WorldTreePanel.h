// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.h
// Author:      SalixGameStudio
// Description: The panel responsible for displaying the scene entity hierarchy.
// =================================================================================
#pragma once
#include <Editor/panels/IPanel.h> // We must implement the IPanel interface
#include <Editor/EditorAPI.h>
#include <memory>

namespace Salix {
    class Entity;
    class Element;
    class Scene;
    class SceneManager;
    struct EditorContext;

    class EDITOR_API WorldTreePanel : public IPanel {
    public:
        WorldTreePanel();
        ~WorldTreePanel();

        void initialize(EditorContext* context) override;

        // This is the implementation of the virtual function from IPanel.
        void on_gui_render() override;
        void set_visibility(bool visibility) override;
        bool get_visibility() const override;
        void on_event(IEvent& event) override;
    private:
        // In the future, this panel will need a pointer to the active scene
        // to get the list of entities to display. We can add that later.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix