// Editor/panels/LockablePanel.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/panels/IPanel.h>
#include <Editor/EditorContext.h>
#include <memory>
#include <string>
#include <imgui/imgui.h>

namespace Salix {

    class SALIX_API LockablePanel : public IPanel {

        public:

            LockablePanel();
            
            virtual ~LockablePanel() override;

            void initialize(EditorContext* context);

            void on_gui_update() override;

            void on_gui_render() override;

            void on_render() override;

            void set_visibility(bool visibility) override;

            bool get_visibility() const override;

            void set_name(const std::string& new_name) override;

            void set_title(const std::string& new_title);

            const std::string& get_title() const;

            bool is_locked() override;

            void unlock() override;

            void lock() override;
            
            void set_locked_state_tint_color(const ImVec4& color);

            void set_unlocked_state_tint_color(const ImVec4& color);

            void set_lock_icon_size(const ImVec2& new_lock_icon_size);

            const ImVec2& get_lock_icon_size() const;

            const ImVec2& get_top_left() const;

            void set_top_left(const ImVec2& top_left);

            const ImVec2& get_bottom_right() const;

            void set_bottom_right(const ImVec2& bottom_right);

            void set_lock_icon (const std::string& icon_name);

            const std::string& get_lock_icon() const;

            const std::string& get_current_lock_icon_path() const;

            const std::string& get_name() override;

        protected:
        virtual void on_panel_gui_update() {}  // For derived classes
        virtual void on_panel_render() {}
        bool begin();
        void end();
    
        virtual void draw_panel_contents();
        // For base class to detect usage
        virtual bool is_panel_derived() const { return false; }

        // Optional: Allow panels to override window flags
        virtual ImGuiWindowFlags get_window_flags() const { 
            return ImGuiWindowFlags_None; 
        }

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix