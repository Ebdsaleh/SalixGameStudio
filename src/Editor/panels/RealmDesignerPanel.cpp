// Editor/panels/RealmDesignerPanel.cpp
#include <Editor/panels/RealmDesignerPanel.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IconInfo.h>
#include <Editor/EditorContext.h>
#include <Salix/core/SDLTimer.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/core/InitContext.h>
#include <Salix/rendering/IRenderer.h>
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
#include <Salix/math/RayCasting.h>
#include <Salix/events/EventManager.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <iostream>
#include <memory>
#include <thread>
#include <SDL.h>


namespace Salix {

    struct RealmDesignerPanel::Pimpl { 
        EditorContext* context = nullptr;
        ImGuiIconManager* icon_manager = nullptr;
        bool is_visible = true;
        bool is_locked = false;
        std::string name = "Realm Designer";
        uint32_t framebuffer_id = 0;
        ImVec2 viewport_size = { 1280, 720 };
        
        bool is_panel_focused_this_frame = false;
        GLint render_pass_begin();
        void render_pass_end(GLint last_fbo);
        void draw_scene();
        void draw_test_cube();
        void draw_test_cube_only();
        void handle_mouse_picking();
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
            std::cerr << "RealmDesignerPanel Error: Failed to create framebuffer!" << std::endl;
        } else {
            std::cout << "RealmDesignerPanel Initialized and created framebuffer with ID: " << 
                pimpl->framebuffer_id << std::endl;
        }
        
    }





    void RealmDesignerPanel::on_gui_update() {
        if (!pimpl->is_visible) return;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Realm Designer", &pimpl->is_visible)) {
            IRenderer* renderer = pimpl->context->init_context->renderer;
            if (!renderer) {
                std::cerr << "[ERROR] Renderer is null in on_gui_update.\n";
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            if (!pimpl->is_locked) {
                // Allow camera to move if window is in focus
                pimpl->is_panel_focused_this_frame = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
                pimpl->context->editor_camera->set_mouse_inside_scene(pimpl->is_panel_focused_this_frame);
            } else {
                pimpl->context->editor_camera->set_mouse_inside_scene(false);
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
                    std::cout << "Realm Designer Panel lock toggled to: " << (pimpl->is_locked ? "LOCKED" : "UNLOCKED") << std::endl;
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

                if (pimpl->context && pimpl->context->editor_camera) {
                    pimpl->context->editor_camera->set_viewport_size(
                        (int)new_size.x, (int)new_size.y);
                }
            }

            if (pimpl->framebuffer_id != 0) {
                ImTextureID tex_id = renderer->get_framebuffer_texture_id(pimpl->framebuffer_id);
                if (tex_id != 0) {
                    ImGui::Image(tex_id, pimpl->viewport_size, ImVec2(0, 1), ImVec2(1, 0));
                }
            }
        }

        pimpl->handle_mouse_picking();

        if (pimpl->is_locked) {
                    ImGui::EndDisabled(); // Disable all interactive widgets below this point
                }
        ImGui::End();
        ImGui::PopStyleVar();
    }




    void RealmDesignerPanel::Pimpl::handle_mouse_picking() {
        if (is_locked || !ImGui::IsWindowHovered() || !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            return;
        }

        // --- Step 1: Get Mouse and Viewport Info ---
        // We need to do the ImVec2 math component-wise
        ImVec2 content_min = ImGui::GetWindowContentRegionMin();
        ImVec2 panel_pos   = ImGui::GetWindowPos();
        ImVec2 viewport_pos = { panel_pos.x + content_min.x, panel_pos.y + content_min.y };
        
        ImVec2 panel_size = ImGui::GetContentRegionAvail();
        ImVec2 mouse_pos  = ImGui::GetMousePos();

        // --- Step 2 & 3: Create the 3D Ray ---
        // Use your new static Raycast class
        Ray ray = Raycast::CreateRayFromScreen(
            context->editor_camera,
            mouse_pos,
            viewport_pos,
            panel_size
        );

        // --- Step 4: Perform Ray-Intersection Test ---
        Entity* selected_entity = nullptr;
        float closest_hit_distance = FLT_MAX;

        for (Entity* entity : context->active_scene->get_entities()) {
            Transform* transform = entity->get_transform();
            if (!transform) continue;

            glm::vec3 scale = transform->get_scale().to_glm();
            glm::vec3 box_min = transform->get_position().to_glm() - (glm::vec3(0.5f) * scale);
            glm::vec3 box_max = transform->get_position().to_glm() + (glm::vec3(0.5f) * scale);

            float distance;
            // Use your new static Raycast class
            if (Raycast::IntersectsAABB(ray, box_min, box_max, distance)) {
                if (distance < closest_hit_distance) {
                    closest_hit_distance = distance;
                    selected_entity = entity;
                }
            }
        }

        // --- Step 5: Fire the Selection Event ---
        if (context->event_manager) {
            // Create a named event object on the stack
            EntitySelectedEvent event(selected_entity);
            // Now, pass the named object to the dispatch function
            context->event_manager->dispatch(event);
        }
    }






    void RealmDesignerPanel::on_gui_render() {
        
    }


    void RealmDesignerPanel::on_render() {
        if (!pimpl->is_visible || !pimpl->context || !pimpl->context->active_scene) {
            return;
        }
       // OpenGLRenderer* renderer = pimpl->context->renderer->as_opengl_renderer();
        //if (!renderer) return;


        // 1. Prepare the render pass (state management)
        pimpl->context->renderer->begin_render_pass(pimpl->framebuffer_id);
        pimpl->context->renderer->set_viewport(0, 0, (int)pimpl->viewport_size.x, (int)pimpl->viewport_size.y);
        pimpl->context->renderer->set_active_camera(pimpl->context->editor_camera);       
        pimpl->context->renderer->clear();

         
        pimpl->draw_test_cube(); // draw 3D first.
        pimpl->draw_scene(); // draw 2D (in game gui/HUD elements).
        

        pimpl->context->renderer->end_render_pass();
    }

    GLint RealmDesignerPanel::Pimpl::render_pass_begin() {
        OpenGLRenderer* renderer = context->renderer->as_opengl_renderer();
        if (!renderer) return 0;

        // --- 1. Prepare the Render Pass (The Boilerplate) ---
        GLint last_bound_fbo = renderer->get_current_framebuffer_binding();
        renderer->bind_framebuffer(framebuffer_id);
        renderer->set_viewport(0, 0, (int)viewport_size.x, (int)viewport_size.y);
        renderer->set_active_camera(context->editor_camera);

        // --- 2. Clear the canvas ONCE ---
        renderer->clear();

        return last_bound_fbo;
    }

    void RealmDesignerPanel::Pimpl::render_pass_end(GLint last_fbo) {
        OpenGLRenderer* renderer = context->renderer->as_opengl_renderer();
        if (!renderer) return;

        // --- 4. Restore the Previous State (The Boilerplate) ---
        renderer->restore_framebuffer_binding(last_fbo);
    }


    void RealmDesignerPanel::Pimpl::draw_scene() {
        Scene* active_scene = context->active_scene;
        IRenderer* renderer = context->renderer; // Use the interface
        if (!renderer || !active_scene) return;

        // Loop through all entities in the scene and draw them
        for (Entity* entity : active_scene->get_entities()) {
            if (!entity) continue;
            
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

    void RealmDesignerPanel::Pimpl::draw_test_cube() {
        OpenGLRenderer* cube_renderer = context->renderer->as_opengl_renderer();
        if (!cube_renderer) return;

        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        Color cube_color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        cube_renderer->draw_cube(model_matrix, cube_color);
    }

    void RealmDesignerPanel::Pimpl::draw_test_cube_only() {
        // Designed to be used inside an 'on_render' method with no explict framebuffering setup.
        // This will not work, if framebuffering and render calls are inside the 'on_render' method.
        if (!context) {
            std::cerr << "[RealmDesignerPanel] context is nullptr!\n";
        return;
        }

        if (!context->renderer) {
            std::cerr << "[RealmDesignerPanel] context->renderer is nullptr!\n";
            return;
        }
     
        IRenderer* base_renderer = context->renderer;
        if (!base_renderer) { /* error */ return; }
        OpenGLRenderer* cube_renderer = base_renderer->as_opengl_renderer();
        if (!cube_renderer) { /* error */ return; }

        int fbo_width = static_cast<int>(viewport_size.x);
        int fbo_height = static_cast<int>(viewport_size.y);
        if (fbo_width <= 0 || fbo_height <= 0) {
            fbo_width = 1280;
            fbo_height = 720;
            std::cerr << "[RealmDesignerPanel] on_render: Using fallback FBO dimensions for test.\n";
        }
        if (framebuffer_id == 0) { /* error */ return; }

        // Save the currently bound framebuffer
        GLint last_bound_fbo = cube_renderer->get_current_framebuffer_binding();

        cube_renderer->bind_framebuffer(framebuffer_id);
        cube_renderer->set_viewport(0, 0, fbo_width, fbo_height);
        cube_renderer->set_clear_color({1.0f, 0.0f, 0.0f, 1.0f}); // Make FBO 1 RED!
        cube_renderer->clear();

        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        Color cube_color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        cube_renderer->draw_cube(model_matrix, cube_color);

        // Restore the previous framebuffer binding (using the new method!)
        cube_renderer->restore_framebuffer_binding(last_bound_fbo); 

        cube_renderer->set_clear_color({0.1f, 0.1f, 0.2f, 1.0f}); 
    }

    bool RealmDesignerPanel::is_locked() {
        return pimpl->is_locked;
    }


    void RealmDesignerPanel::unlock() {
        pimpl->is_locked = false;
    }


    void RealmDesignerPanel::lock() {
        pimpl->is_locked = true;
    }   

}  // namespace Salix