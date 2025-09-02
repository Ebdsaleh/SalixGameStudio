// Editor/panels/RealmPortalPanel.cpp
#include <Editor/panels/RealmPortalPanel.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Editor/events/OnMainCameraChangedEvent.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/IconInfo.h>
#include <Editor/EditorContext.h>
#include <Salix/ecs/Camera.h>
#include <Salix/core/SDLTimer.h>
#include <Editor/camera/EditorCamera.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/events/EventManager.h>
#include <Salix/reflection/EditorDataMode.h>
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
        ICamera* game_camera = nullptr;  
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

        pimpl->context->event_manager->subscribe(EventCategory::Editor, this);
         
        
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

        Scene* scene_to_use = pimpl->context->active_scene;
        
        // If we don't have a camera pointer yet, try to find it using the scene's ID
        if (!pimpl->game_camera) {
            SimpleGuid cam_id = pimpl->context->active_scene->get_main_camera_entity_id();
            if (cam_id.is_valid()) {
                Entity* cam_entity = pimpl->context->active_scene->get_entity_by_id(cam_id);
                
                if (cam_entity) {
                    std::cout << "Camera Entity ID" << cam_id.get_value() << std::endl;
                    
                    pimpl->game_camera = cam_entity->get_element<Camera>();
                }
            }
        }

        ICamera* camera_to_use = pimpl->game_camera && pimpl->game_camera->get_is_active() ? pimpl->game_camera : pimpl->failsafe_camera.get();
    
        IRenderer* renderer = pimpl->context->renderer;
        if (!renderer) return;

        // --- Begin The Render Pass ---
        renderer->begin_render_pass(pimpl->framebuffer_id);
        renderer->set_viewport(0, 0, (int)pimpl->viewport_size.x, (int)pimpl->viewport_size.y);
        renderer->clear();

        //  Use the MainCamera for this render pass 
        renderer->set_active_camera(camera_to_use);
        
        if (camera_to_use->get_is_active()) {
           // --- Draw the Scene ---
            pimpl->draw_scene();
        }

        // --- End The Render Pass ---
        renderer->end_render_pass();
    }


    // A small struct to hold all the info needed for a single draw call
struct RenderJob {
    const Sprite2D* sprite;
    const Transform* transform;
    int sorting_layer;
};

    void RealmPortalPanel::Pimpl::draw_scene() {
        Scene* active_scene = context->active_scene;
        IRenderer* renderer = context->renderer;
        if (!renderer || !active_scene) return;

        // --- STEP 1: COLLECT all visible sprites into a render queue ---
        std::vector<RenderJob> render_queue;
        for (Entity* entity : active_scene->get_entities()) {
            if (!entity || entity->is_purged() || !entity->is_visible()) continue;
            Transform* transform = entity->get_transform();
            if (!transform) continue;

            std::vector<Element*> sprites = entity->get_elements_by_type_name("Sprite2D");
            for (auto* element : sprites) {
                Sprite2D* sprite = dynamic_cast<Sprite2D*>(element);
                if (sprite && sprite->is_visible() && sprite->get_texture()) {
                    render_queue.push_back({sprite, transform, sprite->get_sorting_layer()});
                }
            }
        }

        // --- STEP 2: SORT the render queue by sorting_layer ---
        std::sort(render_queue.begin(), render_queue.end(), [](const RenderJob& a, const RenderJob& b) {
            return a.sorting_layer < b.sorting_layer;
        });

        // --- STEP 3: RENDER the sorted queue ---
        for (const auto& job : render_queue) {
            const Sprite2D* sprite = job.sprite;
            const Transform* transform = job.transform;

            // Get the entity's base world matrix
            glm::mat4 entity_model_matrix;
            if (sprite->use_entity_rotation) {
                // Use the entity's full world matrix, including its rotation and scale.
                entity_model_matrix = transform->get_model_matrix();
            } else {
                // Create a matrix from ONLY the entity's world position for billboarding.
                entity_model_matrix = glm::translate(glm::mat4(1.0f), transform->get_world_position().to_glm());
            }

            // Get sprite's local properties
            Vector2 offset = sprite->offset;
            Vector2 pivot = sprite->pivot;
            float local_rotation_deg = sprite->get_local_rotation();
            const float PIXELS_PER_UNIT = renderer->get_pixels_per_unit();
            float world_width = (float)sprite->get_texture_width() / PIXELS_PER_UNIT;
            float world_height = (float)sprite->get_texture_height() / PIXELS_PER_UNIT;

            // Apply flip logic to the scale
            float scale_x = world_width;
            float scale_y = world_height;
            if (sprite->flip_h) scale_x *= -1.0f;
            if (sprite->flip_v) scale_y *= -1.0f;

            // Build the sprite's complete local transformation matrix
            glm::mat4 local_sprite_matrix = glm::mat4(1.0f);
            local_sprite_matrix = glm::translate(local_sprite_matrix, glm::vec3(offset.x, offset.y, 0.0f));
            local_sprite_matrix = glm::rotate(local_sprite_matrix, glm::radians(local_rotation_deg), glm::vec3(0.0f, 0.0f, 1.0f));
            local_sprite_matrix = glm::scale(local_sprite_matrix, glm::vec3(scale_x, scale_y, 1.0f));
            // Pivot correction shifts the quad so the pivot point is at the origin before scaling
            local_sprite_matrix = local_sprite_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f - pivot.x, 0.5f - pivot.y, 0.0f));

            // Combine with the entity's world matrix to get the final matrix for this sprite
            glm::mat4 final_model_matrix = entity_model_matrix * local_sprite_matrix;

            // Make the clean draw call with the FINAL calculated matrix
            renderer->draw_sprite(
                sprite->get_texture(),
                final_model_matrix,
                sprite->color
            );
        }
    }

    void RealmPortalPanel::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::EditorOnMainCameraChanged) {
            auto& e = static_cast<OnMainCameraChangedEvent&>(event);
            
            Scene* scene_to_use = pimpl->context->active_scene;
            // Safety check to ensure the scene exists.
            if (!scene_to_use) {
                return;
            }
            
            // 1. Update the Scene's data to record which entity is the main camera.
            scene_to_use->set_main_camera_entity(e.entity_id);

            // 2. Find the new live camera entity within the scene.
            Entity* camera_entity = scene_to_use->get_entity_by_id(e.entity_id);
            
            // 3. Update the panel's internal pointer to the live Camera component.
            if (camera_entity) {
                pimpl->game_camera = camera_entity->get_element<Camera>();
            } else {
                pimpl->game_camera = nullptr;
            }
        }
        else if (event.get_event_type() == EventType::BeforeEntityPurged) {
            auto& e = static_cast<BeforeEntityPurgedEvent&>(event);

           // Only proceed if we have a valid pointer to a game camera.
            if (pimpl->game_camera) {
                // Safely cast the ICamera* to an Element* to access the get_owner() method.
                Element* camera_as_element = dynamic_cast<Element*>(pimpl->game_camera);

                // Now we can safely check if the owner is the entity being purged.
                if (camera_as_element && e.entity == camera_as_element->get_owner()) {
                    // If it is, clear the pointer to prevent it from dangling.
                    pimpl->game_camera = nullptr;
                }
            }
        }
        else if (event.get_event_type() == EventType::BeforeElementPurged) {
            auto& e = static_cast<BeforeElementPurgedEvent&>(event);

            std::cout << "[RealmPortalPanel] BeforeElementPurgedEvent: Received..." << std::endl;
            if (pimpl->game_camera) {
                // Safely cast to Element* to get the Camera's ID
                if (Element* camera_as_element = dynamic_cast<Element*>(pimpl->game_camera)) {
                    // If the purged element's ID matches our camera's ID, clear the pointer.
                    if (camera_as_element->get_id() == e.element_id) {
                        std::cout << "[RealmPortalPanel] BeforeElementPurgedEvent: Processing..." << std::endl;
                        pimpl->game_camera = nullptr;
                        std::cout << "[RealmPortalPanel] BeforeElementPurgedEvent: Completed..." << std::endl;
                    }
                }
            }
        }
    }


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