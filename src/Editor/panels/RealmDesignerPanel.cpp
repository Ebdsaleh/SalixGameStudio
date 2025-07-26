// Editor/panels/RealmDesignerPanel.cpp
#include <Editor/panels/RealmDesignerPanel.h>
#include <Editor/EditorContext.h>
#include <Salix/core/InitContext.h>
#include <Salix/rendering/IRenderer.h>
#include <imgui.h>
#include <iostream>
#include <memory>


namespace Salix {

    struct RealmDesignerPanel::Pimpl { 
        EditorContext* context = nullptr;
        bool is_visible = true;
        std::string name = "Realm Designer";
        uint32_t framebuffer_id = 0;
        ImVec2 viewport_size = { 1280, 720 };
    };

    RealmDesignerPanel::RealmDesignerPanel() : pimpl(std::make_unique<Pimpl>()) {

    }


    RealmDesignerPanel::~RealmDesignerPanel() = default;


    void RealmDesignerPanel::initialize(EditorContext* context) {
        if (!context) {
            std::cerr << "RealmDesignerPanel::initialize - Failed to initialize, EditorContext is nullptr!" << std::endl;
        }
        pimpl->context = context;
        // Create frame buffer here...
    }

    void RealmDesignerPanel::on_gui_render() {
        if (!pimpl->is_visible) {
            return;
        }

        // Add some padding to the window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin(pimpl->name.c_str(), &pimpl->is_visible)) {
            // TODO: Handle viewport resizing and draw the ImGui::Image here
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void RealmDesignerPanel::on_event(IEvent& event) { (void)event; }

    void RealmDesignerPanel::set_visibility(bool visibility) {
         pimpl->is_visible = visibility; 
        }

    bool RealmDesignerPanel::get_visibility() const {
        return pimpl->is_visible; 
    }

    void RealmDesignerPanel::set_name(const std::string& new_name) { pimpl->name = new_name; }

    const std::string& RealmDesignerPanel::get_name() { return pimpl->name; }

}  // namespace Salix