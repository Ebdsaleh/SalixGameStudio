// Editor/panels/RealmPortalPanel.cpp
#include <Editor/panels/RealmPortalPanel.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IconInfo.h>
#include <Editor/EditorContext.h>
#include <Salix/ecs/Camera.h>
#include <Salix/core/SDLTimer.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/DummyCamera.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <SDL.h>


namespace Salix {

    struct RealmPortalPanel::Pimpl { 
        EditorContext* context = nullptr;
        ImGuiIconManager* icon_manager = nullptr;
        bool is_visible = true;
        bool is_locked = false;
        std::string name = "Realm Portal";
        uint32_t framebuffer_id = 0;
        ImVec2 viewport_size = { 1280, 720 };
        bool is_panel_focused_this_frame = false;
        std::unique_ptr<DummyCamera> failsafe_camera;
        
        void draw_scene();
        
    };

    RealmPortalPanel::RealmPortalPanel() : pimpl(std::make_unique<Pimpl>()) {

    }


    RealmPortalPanel::~RealmPortalPanel() = default;


    void RealmPortalPanel::initialize(EditorContext* context) {

        pimpl->context = context;

        if (!pimpl->context || !pimpl->context->init_context || !pimpl->context->init_context->renderer) {
            std::cerr << "RealmPortalPanel Error: Renderer is not available, cannot create framebuffer!" << std::endl;
            return;
        }
        pimpl->failsafe_camera = std::make_unique<DummyCamera>();
        pimpl->icon_manager = dynamic_cast<ImGuiIconManager*>(pimpl->context->gui->get_icon_manager());
        if (!pimpl->icon_manager) { return; }
        

        // Get the renderer from the context
        IRenderer* renderer = pimpl->context->init_context->renderer;

        // Create the framebuffer with the panel's default size
        pimpl->framebuffer_id = renderer->create_framebuffer(
            static_cast<int>(pimpl->viewport_size.x),
            static_cast<int>(pimpl->viewport_size.y)
        );

        if (pimpl->framebuffer_id == 0) {
            std::cerr << "RealmPortalPanel Error: Failed to create framebuffer!" << std::endl;
        } else {
            std::cout << "RealmPortalPanel Initialized and created framebuffer with ID: " << 
                pimpl->framebuffer_id << std::endl;
        }

         
        
    }





    void RealmPortalPanel::on_gui_update() {
        if (!pimpl->is_visible) return;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Realm Portal", &pimpl->is_visible)) {
            IRenderer* renderer = pimpl->context->init_context->renderer;
            if (!renderer) {
                std::cerr << "[ERROR] Renderer is null in on_gui_update.\n";
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            
            // handle icon adjustment for OpenGL rendering
            ImVec2 icon_size = ImVec2(16, 16);
            ImVec2 top_left = ImVec2(0, 0);
            ImVec2 bottom_right = ImVec2(1, 1);
            

            ImGui::Separator();
            // --- PANEL LOCK/UNLOCK BUTTON ---
            ImGui::AlignTextToFramePadding();
            // ImGui::SameLine(

            const IconInfo& lock_icon = pimpl->is_locked ? 
                pimpl->icon_manager->get_icon_by_name("Panel Locked") :
                pimpl->icon_manager->get_icon_by_name("Panel Unlocked");

            ImVec4 tint_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white (no tint)
            if (pimpl->is_locked) {
                tint_color = ImVec4(0.50f, 0.0f, 0.0f, 1.0f); // Red tint when locked
            }

            // Only attempt to draw if the icon texture ID is valid
            if (lock_icon.texture_id != 0) {
                if (ImGui::ImageButton("##PanelLockBtn", lock_icon.texture_id, icon_size, top_left, bottom_right, ImVec4(0,0,0,0), tint_color)) {
                    // Button was clicked, toggle the lock state
                    pimpl->is_locked = !pimpl->is_locked; 
                    std::cout << "Realm Portal Panel lock toggled to: " << (pimpl->is_locked ? "LOCKED" : "UNLOCKED") << std::endl;
                }
            } 
            
            if (pimpl->is_locked) {
                ImGui::BeginDisabled(); // Disable all interactive widgets below this point
            }

            ImVec2 new_size = ImGui::GetContentRegionAvail();
            if (new_size.x > 0 && new_size.y > 0 &&
                ((int)new_size.x != (int)pimpl->viewport_size.x ||
                (int)new_size.y != (int)pimpl->viewport_size.y)) {

                if (pimpl->framebuffer_id != 0)
                    renderer->delete_framebuffer(pimpl->framebuffer_id);

                pimpl->framebuffer_id = renderer->create_framebuffer(
                    (int)new_size.x, (int)new_size.y);
                pimpl->viewport_size = new_size;

                if (pimpl->context && pimpl->context->main_camera) {
                    pimpl->context->main_camera->set_viewport_size(
                    (int)new_size.x, (int)new_size.y
                    );
                }
            }

            if (pimpl->framebuffer_id != 0) {
                ImTextureID tex_id = renderer->get_framebuffer_texture_id(pimpl->framebuffer_id);
                if (tex_id != 0) {
                    ImGui::Image(tex_id, pimpl->viewport_size, ImVec2(0, 1), ImVec2(1, 0));
                }
            }
        }
        if (pimpl->is_locked) {
                    ImGui::EndDisabled(); // Disable all interactive widgets below this point
                }
        ImGui::End();
        ImGui::PopStyleVar();
    }




    void RealmPortalPanel::on_gui_render() {
        
    }


    void RealmPortalPanel::on_render() {
        if (!pimpl->is_visible || !pimpl->context || !pimpl->context->active_scene) {
            return;
        }

        // Find the MainCamera from the context (which is set when the scene is created) 
        ICamera* camera_to_use = pimpl->context->main_camera;
        if (!camera_to_use) {
            // If there's no main camera, we can't render the game view.

            camera_to_use = pimpl->failsafe_camera.get();
        }

        IRenderer* renderer = pimpl->context->renderer;
        if (!renderer) return;

        // --- Begin The Render Pass ---
        renderer->begin_render_pass(pimpl->framebuffer_id);
        renderer->set_viewport(0, 0, (int)pimpl->viewport_size.x, (int)pimpl->viewport_size.y);
        renderer->clear();

        //  Use the MainCamera for this render pass 
        renderer->set_active_camera(camera_to_use);

        // --- Draw the Scene ---
        pimpl->draw_scene();

        // --- End The Render Pass ---
        renderer->end_render_pass();
    }


    void RealmPortalPanel::Pimpl::draw_scene() {
        Scene* active_scene = context->active_scene;
        IRenderer* renderer = context->renderer; // Use the interface
        if (!renderer || !active_scene) return;

        // Loop through all entities in the scene and draw them
        for (Entity* entity : active_scene->get_entities()) {
            if (!entity || entity->is_purged()) continue;
            
            Transform* transform = entity->get_element<Transform>();
            Sprite2D* sprite = entity->get_element<Sprite2D>();

            if (transform && sprite && sprite->get_texture()) {
                // 1. Determine the correct flip state from the component's data
                SpriteFlip flip_state = SpriteFlip::None;
                if (sprite->flip_h && sprite->flip_v) {
                    flip_state = SpriteFlip::Both;
                } else if (sprite->flip_h) {
                    flip_state = SpriteFlip::Horizontal;
                } else if (sprite->flip_v) {
                    flip_state = SpriteFlip::Vertical;
                }

                // 2. Make the clean draw call
                //    The old, redundant Rect creation has been removed.
                renderer->draw_sprite(
                    sprite->get_texture(),
                    transform,
                    sprite->get_color(),
                    flip_state // Use the calculated flip state
                );
            }
        }
    }

    void RealmPortalPanel::on_event(IEvent& event) { (void)event; }


    void RealmPortalPanel::set_visibility(bool visibility) {
         pimpl->is_visible = visibility; 
    }


    bool RealmPortalPanel::get_visibility() const {
        return pimpl->is_visible; 
    }


    void RealmPortalPanel::set_name(const std::string& new_name) { pimpl->name = new_name; }


    const std::string& RealmPortalPanel::get_name() { return pimpl->name; }


    uint32_t RealmPortalPanel::get_framebuffer_id() const {
        
        return pimpl->framebuffer_id;
    }


    ImVec2 RealmPortalPanel::get_viewport_size() const {
        return pimpl->viewport_size;
    }

    
    bool RealmPortalPanel::is_locked() {
        return pimpl->is_locked;
    }


    void RealmPortalPanel::unlock() {
        pimpl->is_locked = false;
    }


    void RealmPortalPanel::lock() {
        pimpl->is_locked = true;
    }   

}  // namespace Salix