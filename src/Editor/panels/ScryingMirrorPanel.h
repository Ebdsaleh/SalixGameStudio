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

    class PropertyHandle;
    struct TypeInfo;
    class EDITOR_API ScryingMirrorPanel : public LockablePanel {
    
    public:

        ScryingMirrorPanel();
        ~ScryingMirrorPanel() override;
        void initialize(EditorContext* context) override;
        bool is_panel_derived() const override { return true; }

        void on_event(IEvent& event) override;
#ifdef SALIX_TESTS_ENABLED
        SimpleGuid get_selected_entity_id_for_testing() const;
        SimpleGuid get_selected_element_id_for_testing() const;
        void select_media_for_testing(PropertyHandle& handle, ElementArchetype* element, const TypeInfo* type);
        void resize_collider_for_testing(ElementArchetype* collider, const ElementArchetype* source);
        void activate_camera_for_testing(const ElementArchetype& archetype, const PropertyHandle& handle);
#endif
        
    protected:
        void on_panel_gui_update() override; // Main content
        ImGuiWindowFlags get_window_flags() const override;
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix