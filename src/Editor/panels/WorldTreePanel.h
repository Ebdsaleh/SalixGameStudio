// =================================================================================
// Filename:    Editor/panels/WorldTreePanel.h
// Author:      SalixGameStudio
// Description: The panel responsible for displaying the scene entity hierarchy.
// =================================================================================

#pragma once
#include <Editor/panels/LockablePanel.h>
#include <Editor/EditorAPI.h>
#include <memory>
#include <string>
#include <Salix/core/SimpleGuid.h>

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
        void on_event(IEvent& event) override;
#ifdef SALIX_TESTS_ENABLED
        void process_entity_drop_for_testing(SimpleGuid dragged_id, SimpleGuid target_id);
        void queue_add_root_for_testing();
        void queue_add_element_for_testing(SimpleGuid owner_id, const std::string& type);
#endif
    protected:
        void on_panel_gui_update() override; // Main content
        ImGuiWindowFlags get_window_flags() const override;
        
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
        // Entity/element rendering helpers
        
    };
}  // namespace Salix