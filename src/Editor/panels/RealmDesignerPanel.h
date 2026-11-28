// Editor/panels/RealmDesignerPanel.h
#pragma once
#include<Editor/EditorAPI.h>
#include <Editor/panels/IPanel.h>
#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace Salix {
    struct EditorContext;
    class IRenderer;

    

    class EDITOR_API RealmDesignerPanel : public IPanel {
        public:
            RealmDesignerPanel();
            ~RealmDesignerPanel() override;

            // --- IPanel Interface ---
            void initialize(EditorContext* context) override;
            void on_gui_update() override;
            void on_gui_render() override;
            void on_render() override;
            void on_event(IEvent& event) override;
            void set_visibility(bool visibility) override;
            bool get_visibility() const override;
            void set_name(const std::string& new_name) override;
            const std::string& get_name() override;
            bool is_locked()override;
            void unlock() override;
            void lock() override;

            uint32_t get_framebuffer_id() const;
            ImVec2 get_viewport_size() const;
#ifdef SALIX_TESTS_ENABLED
            void apply_gizmo_matrix_for_testing(const glm::mat4& matrix);
            int get_gizmo_operation_for_testing() const;
            // Exercise and observe the same ImGuizmo instance used by this DLL.
            void begin_gizmo_frame_for_testing();
            void reset_gizmo_for_testing();
            bool is_gizmo_over_for_testing() const;
            bool is_gizmo_using_for_testing() const;
            void pick_at_for_testing(const ImVec2& mouse, const ImVec2& min, const ImVec2& max, bool double_click=false);
#endif
            
        private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
}