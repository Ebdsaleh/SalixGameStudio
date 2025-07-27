// =================================================================================
// Filename:    Editor/panels/PanelManager.h
// Author:      SalixGameStudio
// Description: Manages the lifecycle and rendering of all editor panels.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h> // For the EDITOR_API macro
#include <vector>
#include <memory>
#include <functional>

namespace Salix {

    // Forward declare the IPanel interface to avoid including the full header.
    class IPanel;

    class EDITOR_API PanelManager {
    public:
        PanelManager();
        ~PanelManager();

        // Adds a new panel to the manager. The manager takes ownership of the panel.
        void register_panel(std::unique_ptr<IPanel> panel, const std::string& name);
        
        // Loops through all registered panels and calls their on_gui_update() method.
        void update_all_panels();

        // Loops through all registered panels and calls their on_gui_render() method.
        void render_all_panels();

        IPanel* get_panel(const std::string& panel_name);

        void for_each_visible(std::function<void(IPanel&)> callback);
    private:
        // Using the Pimpl idiom to hide implementation details.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix