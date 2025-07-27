// Editor/panels/RealmDesignerPanel.h
#pragma once
#include<Editor/EditorAPI.h>
#include <Editor/panels/IPanel.h>
#include <memory>
#include <string>

namespace Salix {
    struct EditorContext;



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

            uint32_t get_framebuffer_id() const;
            ImVec2 get_viewport_size() const;

        private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}