// Editor/panels/ProjectSettingsPanel.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/panels/LockablePanel.h>
#include <Editor/EditorContext.h>
#include <memory>

namespace Salix {

    class EDITOR_API ProjectSettingsPanel : public LockablePanel { 
        public:
            ProjectSettingsPanel();
            ~ProjectSettingsPanel() override;

            void initialize(EditorContext* context) override;
            bool is_panel_derived() const override { return true; }

        protected:
            void on_panel_gui_update() override; // Main content
            ImGuiWindowFlags get_window_flags() const override;

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
    };

}