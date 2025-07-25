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

    void PanelManager::register_panel(std::unique_ptr<IPanel> panel, const std::string& name) {
        if (panel) {
            panel->set_name(name);
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

    IPanel* PanelManager::get_panel(const std::string& panel_name) {
        for (const auto& panel_ptr : pimpl->panels) {
            if (panel_ptr->get_name() == panel_name) {
                return panel_ptr.get();
            }
            
        }
        // if no panel exists with that name, return nullptr.
        return nullptr;
    }


} // namespace Salix