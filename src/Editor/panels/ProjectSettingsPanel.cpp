// Editor/panels/ProjectSettingsPanel.cpp
#include <Editor/panels/ProjectSettingsPanel.h>
#include <Editor/EditorContext.h>
#include <Editor/GridSettings.h>
#include <imgui/imgui.h>


namespace Salix {
    struct ProjectSettingsPanel::Pimpl {
        EditorContext* context = nullptr;
        void show_project_settings();
    };

    ProjectSettingsPanel::ProjectSettingsPanel() : pimpl(std::make_unique<Pimpl>()) {
        set_name("Project Settings Panel");
        set_title("Project Settings");
        set_lock_icon_size(ImVec2(16, 16)); 
    }


    ProjectSettingsPanel::~ProjectSettingsPanel() = default;


    void ProjectSettingsPanel::initialize(EditorContext* context) {
        if (!context) {return; }

        LockablePanel::initialize(context); // CRUCIAL: Base init first
        pimpl->context = context;
        
        
        // Configure lock visuals
        set_locked_state_tint_color(ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        set_unlocked_state_tint_color(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        set_lock_icon("Panel Unlocked");
        set_visibility(false);
    }

    ImGuiWindowFlags ProjectSettingsPanel::get_window_flags() const {
        return ImGuiWindowFlags_None; // Add any custom flags here
    }

    void ProjectSettingsPanel::on_panel_gui_update() {

        pimpl->show_project_settings();
    } 

   
    
    void ProjectSettingsPanel::Pimpl::show_project_settings() {
        
        GridSettings& grid = context->grid_settings;
        bool settings_changed = false;
        
        // Grid Settings Section
        if (ImGui::CollapsingHeader("Grid Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            settings_changed |= ImGui::Checkbox("Enable Grid", &grid.snap_enabled);
            settings_changed |= ImGui::SliderFloat("Grid Size", &grid.size, 1.0f, 100.0f, "%.1f");
            settings_changed |= ImGui::SliderFloat("Major Divisions", &grid.major_division, 0.1f, 10.0f, "%.1f");
            
            // Only show minor division if major division is set
            if (grid.major_division > 0.1f) {
                float prev_minor = grid.minor_division;
                ImGui::SliderFloat("Minor Divisions", &grid.minor_division, 
                                0.01f, grid.major_division * 0.9f, "%.2f");
                settings_changed |= (grid.minor_division != prev_minor);
            }
            
            settings_changed |= ImGui::SliderFloat("Snap Increment", &grid.snap_size, 0.01f, grid.major_division, "%.2f");
            settings_changed |= ImGui::ColorEdit4("Grid Color", &grid.color.r);
        }
        
        // Add some spacing between sections
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Future sections can be added here
        if (ImGui::CollapsingHeader("Camera Settings")) {
            // Add camera settings controls here if needed
        }
        
        // Bottom buttons - only show if changes were made
        if (settings_changed) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Save Changes", ImVec2(120, 0))) {
                // TODO: Save settings to project file
                grid.validate();
            }
            
            // Show asterisk to indicate unsaved changes
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1,1,0,1), "* Unsaved changes");
        }
    }

        

}  // namespace Salix