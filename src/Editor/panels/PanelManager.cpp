// =================================================================================
// Filename:    Editor/panels/PanelManager.cpp
// Author:      SalixGameStudio
// Description: Implements the PanelManager class.
// =================================================================================
#include <Editor/panels/PanelManager.h>
#include <Editor/panels/IPanel.h> // We need the full definition of IPanel here.
#include <memory>
#include <vector>
#include <map>  // For later when implementing plugins
namespace Salix {

    // The Pimpl struct holds the actual data.
    struct PanelManager::Pimpl {
        std::vector<std::unique_ptr<IPanel>> panels;
    };

    // Constructor and Destructor
    PanelManager::PanelManager() : pimpl(std::make_unique<Pimpl>()) {}
    PanelManager::~PanelManager() = default;

    void PanelManager::register_panel(std::unique_ptr<IPanel> panel) {
        if (panel) {
            pimpl->panels.push_back(std::move(panel));
        }
    }

    void PanelManager::render_all_panels() {
        // Simple loop to render every panel in the list.
        for (const auto& panel : pimpl->panels) {
            if (panel) {
                panel->on_gui_render();
            }
        }
    }

} // namespace Salix