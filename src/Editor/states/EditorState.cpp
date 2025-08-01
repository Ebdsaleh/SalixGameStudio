// Editor/state/EditorState.cpp
#include <Editor/states/EditorState.h>
#include <Editor/camera/EditorCamera.h>

// Core engine systems needed
#include <Salix/core/EngineMode.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Salix/window/sdl/SDLWindow.h>
#include <Salix/core/SDLTimer.h>
// Scene related 
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>

// Editor-specific systems
#include <Editor/EditorContext.h>
#include <Editor/panels/PanelManager.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/panels/RealmDesignerPanel.h>
#include <Editor/panels/RealmPortalPanel.h>
#include <Editor/panels/ProjectSettingsPanel.h>
#include <Editor/panels/ThemeEditorPanel.h>

// For ImGui Docking
#include <Salix/gui/IGui.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/window/IWindow.h>
#include <Salix/gui/ITheme.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/IFont.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/DialogBox.h>
#include <ImGuiFileDialog.h>
#include <imgui/imgui.h>
#include <ImGuizmo.h>
#include <iostream>
#include <memory>
#include <fstream> // For file logging
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Salix {

    // The Pimpl struct holds all the state's data
    struct EditorState::Pimpl {
        bool is_first_frame = true;
        float total_time = 0.0f;

        
        std::unique_ptr<EditorCamera> camera;
        std::unique_ptr<EditorContext> editor_context;
        std::unique_ptr<PanelManager> panel_manager;
        std::unique_ptr<Project> mock_project;
        RealmDesignerPanel* realm_designer = nullptr;
        RealmPortalPanel* realm_portal = nullptr;
        bool show_theme_editor = false; 
        bool show_camera_debug_window = true;
        bool show_project_settings  = false;
        void handle_first_frame_setup();
        void begin_dockspace();
        void update_menu_bar_and_panels();
        void render_menu_bar_and_panels();
        void end_dockspace();
        // for testing
        void create_mock_scene();
        void draw_test_cube(); 
        void process_input();
        void draw_debug_window();
       
    };

    EditorState::EditorState() : pimpl(std::make_unique<Pimpl>()) {}
    EditorState::~EditorState() {}

    void EditorState::on_enter(const InitContext& engine_context) {
        // Open a log file to trace the initialization steps
        std::ofstream log_file("editor_init.log");
        if (!log_file.is_open()) {
            std::cerr << "FATAL DEBUG ERROR: Could not open editor_init.log for writing!" << std::endl;
            return;
        }

        log_file << "[DEBUG] Entering EditorState::on_enter..." << std::endl;
        
        // --- Let's check every pointer from the engine_context ---
        if (!engine_context.engine) { log_file << "[FATAL] engine_context.engine is NULL!" << std::endl; return; }
        if (!engine_context.gui) { log_file << "[FATAL] engine_context.gui is NULL!" << std::endl; return; }
        if (!engine_context.renderer) { log_file << "[FATAL] engine_context.renderer is NULL!" << std::endl; return; }
        if (!engine_context.asset_manager) { log_file << "[FATAL] engine_context.asset_manager is NULL!" << std::endl; return; }
        if (!engine_context.event_manager) { log_file << "[FATAL] engine_context.event_manager is NULL!" << std::endl; return; }
        if (!engine_context.theme_manager) { log_file << "[FATAL] engine_context.theme_manager is NULL!" << std::endl; return; }
        if (!engine_context.font_manager) { log_file << "[FATAL] engine_context.font_manager is NULL!" << std::endl; return; }
        
        log_file << "[DEBUG] All pointers in engine_context are valid." << std::endl;

        // 1. Create the editor's systems
        log_file << "[DEBUG] Creating EditorContext..." << std::endl;
        pimpl->editor_context = std::make_unique<EditorContext>();
        log_file << "[DEBUG] Creating PanelManager..." << std::endl;
        pimpl->panel_manager = std::make_unique<PanelManager>();
        log_file << "[DEBUG] Editor systems created." << std::endl;

        // 2. Populate the EditorContext with services from the engine
        log_file << "[DEBUG] Populating EditorContext..." << std::endl;
        pimpl->editor_context->init_context = &engine_context;
        pimpl->editor_context->gui = engine_context.gui;
        pimpl->editor_context->renderer = engine_context.renderer;
        pimpl->editor_context->asset_manager = engine_context.asset_manager;
        pimpl->editor_context->event_manager = engine_context.event_manager;
        pimpl->editor_context->panel_manager = pimpl->panel_manager.get();
        pimpl->editor_context->theme_manager = engine_context.theme_manager;
        pimpl->editor_context->font_manager = engine_context.font_manager;
        log_file << "[DEBUG] EditorContext populated." << std::endl;

        // 4. Set the engine mode
        log_file << "[DEBUG] Setting engine mode to Editor..." << std::endl;
        engine_context.engine->set_mode(EngineMode::Editor);
        log_file << "[DEBUG] Engine mode set." << std::endl;

        // 5. Create and initialize all the editor panels
        log_file << "[DEBUG] Creating WorldTreePanel..." << std::endl;
        auto world_tree_panel = std::make_unique<WorldTreePanel>();
        std::string world_tree_name = "World Tree Panel";
        log_file << "[DEBUG] Initializing WorldTreePanel..." << std::endl;
        world_tree_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering WorldTreePanel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(world_tree_panel), world_tree_name);
        log_file << "[DEBUG] WorldTreePanel registered." << std::endl;

        log_file << "[DEBUG] Creating ScryingMirrorPanel..." << std::endl;
        auto scrying_mirror_panel = std::make_unique<ScryingMirrorPanel>();
        std::string scrying_mirror_name = "Scrying Mirror Panel";
        log_file << "[DEBUG] Initializing ScryingMirrorPanel..." << std::endl;
        scrying_mirror_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering ScryingMirrorPanel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(scrying_mirror_panel), scrying_mirror_name);
        log_file << "[DEBUG] ScryingMirrorPanel registered." << std::endl;

        log_file << "[DEBUG] Creating RealmDesignerPanel..." << std::endl;
        auto realm_designer_panel = std::make_unique<RealmDesignerPanel>();
        std::string realm_designer_name = "Realm Designer Panel";
        log_file << "[DEBUG] Initializing RealmDesignerPanel..." << std::endl;
        realm_designer_panel->initialize(pimpl->editor_context.get());
        pimpl->panel_manager->register_panel(std::move(realm_designer_panel), realm_designer_name);
        pimpl->realm_designer = dynamic_cast<RealmDesignerPanel*>(pimpl->panel_manager->get_panel("Realm Designer Panel"));
        log_file << "[DEBUG] RealmDesignerPanel registered..." << std::endl;

        log_file << "[DEBUG] Creating RealmPortalPanel..." << std::endl;
        auto realm_portal_panel = std::make_unique<RealmPortalPanel>();
        std::string realm_portal_name = "Realm Portal Panel";
        log_file << "[DEBUG] Initializing RealmPortalPanel..." << std::endl;
        realm_portal_panel->initialize(pimpl->editor_context.get());
        pimpl->panel_manager->register_panel(std::move(realm_portal_panel), realm_portal_name);
        pimpl->realm_portal = dynamic_cast<RealmPortalPanel*>(pimpl->panel_manager->get_panel("Realm Portal Panel"));
        log_file << "[DEBUG] RealmPotalPanel registered..." << std::endl;


        log_file << "[DEBUG] Creating ThemeEditorPanel..." << std::endl;
        auto theme_editor_panel = std::make_unique<ThemeEditorPanel>();
        std::string theme_editor_name = "Theme Editor Panel";
        log_file << "[DEBUG] Initializing ThemeEditorPanel..." << std::endl;
        theme_editor_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering ThemeEditorPanel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(theme_editor_panel), theme_editor_name);
        log_file << "[DEBUG] ThemeEditorPanel registered." << std::endl;

        // NEW ProjectSettingsPanel
        log_file << "[DEBUG] Creating ProjectSettingsPanel..." << std::endl;
        auto project_settings_panel = std::make_unique<ProjectSettingsPanel>();
        std::string project_settings_name = "Project Settings Panel";
        log_file << "[DEBUG] Initializing Project Settings Panel..." << std::endl;
        project_settings_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering Project Settings Panel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(project_settings_panel), project_settings_name);
        log_file << "[DEBUG] ProjectSettingsPanel registered." << std::endl;

        log_file << "[DEBUG] EditorState::on_enter FINISHED successfully." << std::endl;

        pimpl->camera = std::make_unique<EditorCamera>();
        pimpl->camera->initialize(pimpl->editor_context.get());
        pimpl->editor_context->editor_camera = pimpl->camera.get();
        OpenGLRenderer* renderer = dynamic_cast<OpenGLRenderer*>(engine_context.renderer);
        renderer->set_active_camera(pimpl->camera.get());
        
        engine_context.window->set_size(
            engine_context.app_config->window_config.width,
            engine_context.app_config->window_config.height
        );

            engine_context.renderer->on_window_resize(
            engine_context.app_config->window_config.width,
            engine_context.app_config->window_config.height
        );
        pimpl->create_mock_scene();

    }




    void EditorState::on_exit() {
        std::cout << "Exiting EditorState..." << std::endl;
        pimpl->realm_designer = nullptr;
        pimpl->realm_portal = nullptr;
    }





    void EditorState::update(float delta_time ) {
        // Update animation timer
        pimpl->total_time += delta_time;
        pimpl->handle_first_frame_setup();
        pimpl->process_input();
        if (pimpl->camera) {
            pimpl->camera->on_update(delta_time);
        }
        IFont* active_font = pimpl->editor_context->font_manager->get_active_font();
        if (active_font && active_font->get_imgui_font_ptr()) {
            ImGui::PushFont(active_font->get_imgui_font_ptr());
        }

        if (pimpl->editor_context->init_context->engine->is_running()) {
            ImGuizmo::BeginFrame();
        }  
        pimpl->begin_dockspace();
        pimpl->draw_debug_window();
        // moved pimpl->render_menu_bar_and_panels() to EditorState::render()
        pimpl->update_menu_bar_and_panels(); 
        if (pimpl->editor_context && pimpl->editor_context->gui) {
            pimpl->editor_context->gui->display_dialogs();
        }
        pimpl->end_dockspace();   
        
        // Pop the font immediately after to balance the stack.
        if (active_font && active_font->get_imgui_font_ptr()) {
            ImGui::PopFont();
        }
        
    }




    void EditorState::render(IRenderer* renderer_param) {
        if (renderer_param) {
            // 1. CRITICAL: Clear BOTH the color and depth buffers for the new frame.
            renderer_param->clear(); 

            if (pimpl->realm_designer) {
                pimpl->realm_designer->on_render();
            }

            if (pimpl->realm_portal) {
                pimpl->realm_portal->on_render();
            }

            
            // 3. Clear ONLY the depth buffer again before drawing the UI.
            // This is a common technique to make sure the UI always draws on top of the 3D scene.
            
            renderer_param->clear_depth_buffer(); 
            if (pimpl->editor_context->gui) {
                pimpl->editor_context->gui->render();
                pimpl->render_menu_bar_and_panels();
                pimpl->editor_context->gui->update_and_render_platform_windows();
                
            }
        }
    }


    void EditorState::Pimpl::handle_first_frame_setup() {
        if (!is_first_frame) {
            return; // Only run this once
        }
        
        std::cout << "EditorState - Performing first-frame UI setup." << std::endl;
    
        
        is_first_frame = false;
    }



    void EditorState::Pimpl::begin_dockspace() {
        const ImGuiIO& io = ImGui::GetIO();
        ImVec2 display_size = io.DisplaySize;

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(display_size);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        // Push the transparent color right before beginning the window.
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        flags |= ImGuiWindowFlags_NoBackground;

        

        ImGui::Begin("Salix Editor Dockspace", nullptr, flags);
        
    }


    void EditorState::Pimpl::update_menu_bar_and_panels() {
        ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
        // This flag tells ImGui that if the central node is empty,
        // it should be transparent, allowing our 3D scene to show through.
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        ImGui::PopStyleColor();
        // ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // --- Main Menu Bar ---
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project...")) { /* TODO */ }
                if (ImGui::MenuItem("Open Project...")) { /* TODO */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { editor_context->init_context->engine->is_running(false);}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
            if (ImGui::MenuItem("Preferences")) { /* TODO: Open preferences window */ }
            if (ImGui::MenuItem("Theme")) {
                show_theme_editor = true;
                if( IPanel* theme_panel = panel_manager->get_panel(std::string("Theme Editor Panel"))) {
                    theme_panel->set_visibility(true);
                }
                }
            if (ImGui::MenuItem("Project Settings")) {
                    show_project_settings = true;
                    if( IPanel* project_settings_panel = panel_manager->get_panel(std::string("Project Settings Panel"))) {
                        project_settings_panel->set_visibility(true);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::BeginMenu("Editor Camera")) {
                                
                    ImGui::SeparatorText("Projection Mode");
                    if (ImGui::MenuItem("Perspective")) {
                        editor_context->editor_camera->set_projection_mode(ProjectionMode::Perspective);
                    }
                    if (ImGui::MenuItem("Orthographic")) {
                        editor_context->editor_camera->set_projection_mode(ProjectionMode::Orthographic);
                    }
                    ImGui::SeparatorText("Debug");
                    if (ImGui::MenuItem("Debug Window")) {
                        show_camera_debug_window = true;
                        draw_debug_window();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Panels")) {
                    ImGui::SeparatorText("Workflow");
                    if (ImGui::MenuItem("World Tree")) {
                        panel_manager->get_panel("World Tree Panel")->set_visibility(true);
                    }
                    if (ImGui::MenuItem("Scrying Mirror")) {
                        panel_manager->get_panel("Scrying Mirror Panel")->set_visibility(true);
                    }
                    if (ImGui::MenuItem("Realm Designer")) {
                        realm_designer->set_visibility(true);
                    }
                    if (ImGui::MenuItem("Realm Portal")) {
                        realm_portal->set_visibility(true);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // --- All Editor Panels ---
        if (panel_manager) {
            panel_manager->update_all_panels();
         }
    }

    void EditorState::Pimpl::render_menu_bar_and_panels() {
        // --- All Editor Panels ---
        if (panel_manager) {
            panel_manager->render_all_panels();
        }
    }

    void EditorState::Pimpl::end_dockspace() {
        ImGui::End();
        // --- Now, pop all style modifications in reverse order ---
        ImGui::PopStyleColor(); // Pop the transparent background color
        ImGui::PopStyleVar(3);  // Pop the 3 style vars (padding, border, rounding)
    }


    void EditorState::Pimpl::create_mock_scene() {
        if(!editor_context) {
            std::cout << "EditorState::create_mock_scene - EditorContext is NULL, aborting..." << std::endl;
        }
        
        mock_project = std::make_unique<Project>();
        mock_project->initialize(*editor_context->init_context);
        editor_context->scene_manager = mock_project->get_scene_manager();
        editor_context->active_scene = editor_context->scene_manager->create_scene("New Scene");
        Entity* camera_entity = editor_context->active_scene->create_entity("Main Camera");
        
        Camera* main_camera= camera_entity->add_element<Camera>();
        main_camera->initialize();
        main_camera->set_viewport_size(1280, 720); // Example size
        main_camera->set_field_of_view(60.0f);
        main_camera->set_projection_mode(ProjectionMode::Perspective);
        // Set the active camera on the renderer immediately after creating it.
        // if (auto* opengl_renderer = dynamic_cast<OpenGLRenderer*>(editor_context->renderer)) {
        //    opengl_renderer->set_active_camera(main_camera);
        // }
        editor_context->main_camera = main_camera;
        editor_context->active_scene->set_active_camera(main_camera);
        Entity* player = editor_context->active_scene->create_entity("Player");
        Sprite2D* player_sprite = player->add_element<Sprite2D>();
        const std::string sprite_file_path = "src/Sandbox/TestProject/Assets/Images/test.png";
        player_sprite->load_texture(editor_context->asset_manager, sprite_file_path);
        BoxCollider* player_collider = player->get_element<BoxCollider>();
        float ppu = editor_context->renderer->get_pixels_per_unit();
        player_collider->set_size(
            Vector3(
                (float)player_sprite->get_texture()->get_width() / ppu,
                (float)player_sprite->get_texture()->get_height() / ppu,
                0.1f
            )
        );

        editor_context->active_project = mock_project.get();     
    }

    void EditorState::Pimpl::draw_test_cube() {
        // Safety check to ensure the renderer and camera exist
        if (!editor_context || !editor_context->renderer || !camera) {
            std::cerr << "EditorState::Pimpl::draw_test_cube - failed initial dependency checks." << std::endl;
            return;
        }

        // 1. Define the cube's position in the world (hard-coded)
        glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));

        
        // 2. Define the cube's color
        Color cube_color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        OpenGLRenderer* cube_renderer = dynamic_cast<OpenGLRenderer*>(editor_context->renderer);
        // 3. Get the renderer from the context and call draw_cube
        if (cube_renderer) {
            cube_renderer->draw_cube(model_matrix, cube_color);
        } else {
            std::cerr << "EditorState::Pimpl::draw_test_cube - Failed to cast 'editor_context->renderer' to type OpenGLRenderer!"
            << std::endl;
        }

    }


    
    void EditorState::Pimpl::process_input() {
        
    }

    void EditorState::Pimpl::draw_debug_window() {
        if (!camera) return;

        if (!show_camera_debug_window) return;
        ImGui::Begin("Camera Debug Info", &show_camera_debug_window);

        const glm::mat4& view = camera->get_view_matrix();
        const glm::mat4& projection = camera->get_projection_matrix();
        Vector3 position = camera->get_transform()->get_position();
        Vector3 rotation = camera->get_transform()->get_rotation();
        Vector3 scale = camera->get_transform()->get_scale();


        ImGui::Text("View Matrix:");
        // Use ImGui::Text with formatting for each row
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", view[0][0], view[0][1], view[0][2], view[0][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", view[1][0], view[1][1], view[1][2], view[1][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", view[2][0], view[2][1], view[2][2], view[2][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", view[3][0], view[3][1], view[3][2], view[3][3]);

        ImGui::Text("Projection Matrix:");
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", projection[0][0], projection[0][1], projection[0][2], projection[0][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", projection[1][0], projection[1][1], projection[1][2], projection[1][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", projection[2][0], projection[2][1], projection[2][2], projection[2][3]);
        ImGui::Text("%.3f, %.3f, %.3f, %.3f", projection[3][0], projection[3][1], projection[3][2], projection[3][3]);

        ImGui::Text("Transform:");
        ImGui::Text("Position: %.3f, %.3f, %.3f", position.x, position.y, position.z);
        ImGui::Text("Rotation: %.3f, %.3f, %.3f", rotation.x, rotation.y, rotation.z);
        ImGui::Text("Scale: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
        ImGui::End();
    }
        


} // namespace Salix