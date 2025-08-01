// Editor/panels/LockablePanel.cpp
#include <Editor/panels/LockablePanel.h>
#include <Editor/EditorContext.h>
#include <Salix/core/InitContext.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>

#include <memory>
#include <iostream>
#include <string>


namespace Salix {
    struct LockablePanel::Pimpl {
        std::string name = "New Lockable Panel";
        std::string title;
        bool is_visible = true;
        bool is_locked = false;
        std::string locked_state_icon_name = "Panel Locked";
        std::string unlocked_state_icon_name = "Panel Unlocked";
        ImVec4 unlocked_tint_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 locked_tint_color = ImVec4(0.50f, 0.0f, 0.0f, 1.0f);
        ImVec2 lock_icon_size = ImVec2(16, 16);
        ImVec2 top_left = ImVec2(0, 0);
        ImVec2 bottom_right = ImVec2(1, 1);
        IconInfo lock_icon;
        EditorContext* context = nullptr;
        IIconManager* icon_manager = nullptr;

        // 1. Handle lock button
         
        
    };


    LockablePanel::LockablePanel() : pimpl(std::make_unique<Pimpl>()) { 
        pimpl->lock_icon = {};
    }


    LockablePanel::~LockablePanel() = default;


    void LockablePanel::initialize(EditorContext* context) {
        if (!context) return;
        pimpl->context = context;
        pimpl->icon_manager = context->init_context->icon_manager;

        if (!pimpl->icon_manager) {
            std::cerr << "Icon manager not available!" << std::endl;
            return;
        }

        pimpl->lock_icon = pimpl->icon_manager->get_icon_by_name(pimpl->unlocked_state_icon_name);
    }


    void LockablePanel::draw_lock_ui() {

        if (!pimpl->icon_manager) {
            ImGui::TextColored(ImVec4(1,0,0,1), "No icon manager!");
            return;
        }

        ImGui::PushID(this);
        if (pimpl->lock_icon.texture_id) {
            const bool clicked = ImGui::ImageButton(
                "##PanelLock", 
                pimpl->lock_icon.texture_id,
                pimpl->lock_icon_size,
                pimpl->top_left, 
                pimpl->bottom_right,
                ImVec4(0,0,0,0), 
                pimpl->is_locked ? pimpl->locked_tint_color : pimpl->unlocked_tint_color
            );

            if (clicked && pimpl->icon_manager) {
                pimpl->is_locked = !pimpl->is_locked;
                // Auto-swap icon (optional)
                pimpl->lock_icon = pimpl->icon_manager->get_icon_by_name(
                    pimpl->is_locked ? pimpl->locked_state_icon_name : pimpl->unlocked_state_icon_name
                );
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", pimpl->is_locked ? "Unlock panel" : "Lock panel");
            }
        }
        ImGui::PopID();
    }


    bool LockablePanel::begin() {
        return ImGui::Begin(
            pimpl->title.c_str(), 
            &pimpl->is_visible, 
            get_window_flags()
        );
    }

    void LockablePanel::end() {
        ImGui::End();
    }


    void LockablePanel::draw_panel_contents() {
        if (!pimpl->is_locked) {
                on_panel_gui_update();  // For derived classes
        }
    }



    void LockablePanel::on_gui_update() {
        if (!pimpl->is_visible) return;
        
        // Case 1: Standalone mode (direct instantiation)
        if (!is_panel_derived()) {  // Implement this flag (see below)
            if (ImGui::Begin(pimpl->title.c_str(), &pimpl->is_visible)) {
                draw_panel_contents();
                ImGui::End();
            }
        } 
        else {
            // 2. Let derived class implement content
            if (ImGui::Begin(pimpl->title.c_str(), &pimpl->is_visible)) {
                draw_lock_ui();
                ImGui::End();
            }
            if (pimpl->is_locked) { 
                ImGui::BeginDisabled();
            }
                on_panel_gui_update();  // For derived classes
            if (pimpl->is_locked) {
                ImGui::EndDisabled();
            }
            
            
        }
    } 




    void LockablePanel::on_gui_render() {

    }



    void LockablePanel::on_render() {

    }


     void LockablePanel::on_event(IEvent& event) {
        (void)event;
    }

    void LockablePanel::set_visibility(bool visibility) {
         pimpl->is_visible = visibility;
    }



    bool LockablePanel::get_visibility() const {
        return pimpl->is_visible;
    }



    void LockablePanel::set_name(const std::string& new_name) {
        pimpl->name = new_name;
    }


    void LockablePanel::set_title(const std::string& new_title) {
        pimpl->title = new_title;
    }

    const std::string& LockablePanel::get_title() const {
        return pimpl->title;
    }

    bool LockablePanel::is_locked() {
        return pimpl->is_locked;
    }



    void LockablePanel::unlock() {
        pimpl->is_locked = false;
        pimpl->lock_icon = pimpl->icon_manager->get_icon_by_name(pimpl->unlocked_state_icon_name);
    }



    void LockablePanel::lock() {

        pimpl->is_locked = true;
        pimpl->lock_icon = pimpl->icon_manager->get_icon_by_name(pimpl->locked_state_icon_name);
    }




    const std::string& LockablePanel::get_name() {
        return pimpl->name;
    }


    void LockablePanel::set_locked_state_tint_color(const ImVec4& color) {
        pimpl->locked_tint_color = color;
    }

    void LockablePanel::set_unlocked_state_tint_color(const ImVec4& color) {
        pimpl->unlocked_tint_color = color;
    }

    void LockablePanel::set_lock_icon_size(const ImVec2& new_lock_icon_size) {
        pimpl->lock_icon_size = new_lock_icon_size;
    }

    const ImVec2& LockablePanel::get_lock_icon_size() const {
        return pimpl->lock_icon_size;
    }

    const ImVec2& LockablePanel::get_top_left() const {
        return pimpl->top_left;
    }
    const ImVec2& LockablePanel::get_bottom_right() const {
        return pimpl->bottom_right;
    }

    void LockablePanel::set_top_left(const ImVec2& new_top_left) {
        pimpl->top_left = new_top_left;
    }

    void LockablePanel::set_bottom_right(const ImVec2& new_bottom_right) {
        pimpl->bottom_right = new_bottom_right;
    }

    void LockablePanel::set_lock_icon (const std::string& icon_name) {
        if (icon_name.empty()) {
            std::cerr << "LockablePanel::set_lock_icon - Failed to set lock icon, 'icon_name' is_empty!" <<
            std::endl;
            return;
        }

        pimpl->lock_icon = pimpl->icon_manager->get_icon_by_name(icon_name);
        if (!pimpl->lock_icon.texture_id) {
            std::cerr << "LockablePanel::set_lock_icon - Failed to set lock icon, icon name: '" 
            << icon_name << "' does NOT exist in the icon registry!" << std::endl;
            return;
        }
        std::cout << "LockablePanel::set_lock_icon - lock icon '" << icon_name << "' set successfully." 
        << std::endl;
    }

    // Only returning the name of currently used icon as IconInfo doesn't have the registered name, only
    // the path.
    const std::string& LockablePanel::get_lock_icon() const {
        if (pimpl->is_locked) return pimpl->locked_state_icon_name;
        else return pimpl->unlocked_state_icon_name;
    }

    const std::string& LockablePanel::get_current_lock_icon_path() const {
        return pimpl->lock_icon.path;
    }

}  // namespace Salix