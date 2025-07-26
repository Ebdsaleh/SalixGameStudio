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

        pimpl->context = context;

        if (!pimpl->context || !pimpl->context->init_context || !pimpl->context->init_context->renderer) {
            std::cerr << "RealmDesignerPanel Error: Renderer is not available, cannot create framebuffer!" << std::endl;
            return;
        }
        
        // Get the renderer from the context
        IRenderer* renderer = pimpl->context->init_context->renderer;

        // Create the framebuffer with the panel's default size
        pimpl->framebuffer_id = renderer->create_framebuffer(
            static_cast<int>(pimpl->viewport_size.x),
            static_cast<int>(pimpl->viewport_size.y)
        );

        if (pimpl->framebuffer_id == 0) {
            std::cerr << "RealmDesignerPanel Error: Failed to create framebuffer!" << std::endl;
        } else {
            std::cout << "RealmDesignerPanel Initialized and created framebuffer with ID: " << 
                pimpl->framebuffer_id << std::endl;
        }
        
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

    uint32_t RealmDesignerPanel::get_framebuffer_id() const {
        
        return pimpl->framebuffer_id;
    }

    ImVec2 RealmDesignerPanel::get_viewport_size() const {
        return pimpl->viewport_size;
    }


}  // namespace Salix