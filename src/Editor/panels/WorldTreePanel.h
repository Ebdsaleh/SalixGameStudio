// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.h
// Author:      SalixGameStudio
// Description: The panel responsible for displaying the scene entity hierarchy.
// =================================================================================

#pragma once
#include <Editor/panels/LockablePanel.h>
#include <Editor/EditorAPI.h>
#include <memory>

namespace Salix {
    class Entity;
    class Element;
    class Scene;
    struct EditorContext;

    class EDITOR_API WorldTreePanel : public LockablePanel {
    public:
        WorldTreePanel();
        ~WorldTreePanel() override;

        void initialize(EditorContext* context) override;
        bool is_panel_derived() const override { return true; }
    protected:
        void on_panel_gui_update() override; // Main content
        ImGuiWindowFlags get_window_flags() const override;
        
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
        // Entity/element rendering helpers
        void render_entity_tree(Entity* entity);
        void render_element(Element* element);
    };
}  // namespace Salix