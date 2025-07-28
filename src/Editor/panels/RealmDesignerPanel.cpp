// Editor/panels/RealmDesignerPanel.cpp
#include <Editor/panels/RealmDesignerPanel.h>
#include <Editor/EditorContext.h>
#include <Salix/core/SDLTimer.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/core/InitContext.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <SDL.h>


namespace Salix {

    struct RealmDesignerPanel::Pimpl { 
        EditorContext* context = nullptr;
        bool is_visible = true;
        bool is_locked = false;
        std::string name = "Realm Designer";
        uint32_t framebuffer_id = 0;
        ImVec2 viewport_size = { 1280, 720 };
        void draw_test_cube();
        bool is_panel_focused_this_frame = false;
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
        
        std::cout << "DEBUG: Address of EditorContext::InitContext* received by RealmDesignerPanel::initialize(): " 
              << context->init_context << std::endl;

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
        if (ImGui::Begin(pimpl->name.c_str(), &pimpl->is_visible)) {
            IRenderer* renderer = pimpl->context->init_context->renderer;
            if (!renderer) {
                std::cerr << "[ERROR] Renderer is null in on_gui_update.\n";
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            // Allow camera to move if window is in focus
            pimpl->is_panel_focused_this_frame = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
            pimpl->context->editor_camera->set_mouse_inside_scene(pimpl->is_panel_focused_this_frame);

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
        ImGui::End();
        ImGui::PopStyleVar();
    }




    void RealmDesignerPanel::on_gui_render() {
        
    }


    void RealmDesignerPanel::on_render() {
        if (pimpl->is_visible) {
            if (pimpl->context->editor_camera->get_projection_mode() == ProjectionMode::Perspective) { 
                pimpl->draw_test_cube();    
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