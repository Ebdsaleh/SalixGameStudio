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
        void on_event(IEvent& event) override;
    protected:
        void on_panel_gui_update() override; // Main content
        ImGuiWindowFlags get_window_flags() const override;
        void show_empty_space_context_menu();
        void show_entity_context_menu(Entity* entity);
        void create_new_entity(Entity* parent = nullptr);
        void add_element_to_entity(Entity* entity, const std::string& element_type);

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
        // Entity/element rendering helpers
        void render_entity_tree(Entity* entity);
        void render_element(Element* element);
        void setup_entity_drag_source(Entity* entity);
        void process_entity_drop(Entity* dragged_entity, Entity* target_entity);
        void handle_inter_entity_drop_target(Entity* anchor_entity);
        void handle_entity_drop_target(Entity* entity);
        void handle_keyboard_shortcuts(); 
    };
}  // namespace Salix