// Editor/state/EditorState.cpp
#include <Editor/states/EditorState.h>
#include <Editor/camera/EditorCamera.h>

// Core engine systems needed
#include <Salix/core/EngineMode.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/input/ImGuiInputManager.h>

// Scene related 
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>

// Editor-specific systems
#include <Editor/EditorContext.h>
#include <Editor/panels/PanelManager.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/panels/ScryingMirrorPanel.h>

// For ImGui Docking
#include <Salix/gui/IGui.h>
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
#include <iostream>
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
        void setup_theme_manager();
        void setup_font_manager();
        void handle_first_frame_setup();
        void begin_dockspace();
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
        log_file << "[DEBUG] Initializing WorldTreePanel..." << std::endl;
        world_tree_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering WorldTreePanel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(world_tree_panel));
        log_file << "[DEBUG] WorldTreePanel registered." << std::endl;

        log_file << "[DEBUG] Creating ScryingMirrorPanel..." << std::endl;
        auto scrying_mirror_panel = std::make_unique<ScryingMirrorPanel>();
        log_file << "[DEBUG] Initializing ScryingMirrorPanel..." << std::endl;
        scrying_mirror_panel->initialize(pimpl->editor_context.get());
        log_file << "[DEBUG] Registering ScryingMirrorPanel..." << std::endl;
        pimpl->panel_manager->register_panel(std::move(scrying_mirror_panel));
        log_file << "[DEBUG] ScryingMirrorPanel registered." << std::endl;

        log_file << "[DEBUG] EditorState::on_enter FINISHED successfully." << std::endl;

        pimpl->camera = std::make_unique<EditorCamera>();
        pimpl->camera->initialize(pimpl->editor_context.get());
        pimpl->editor_context->editor_camera = pimpl->camera.get();
        OpenGLRenderer* renderer = dynamic_cast<OpenGLRenderer*>(engine_context.renderer);
        renderer->set_active_camera(pimpl->camera.get());
        pimpl->create_mock_scene();

    }




    void EditorState::on_exit() {
        std::cout << "Exiting EditorState..." << std::endl;
    }





    void EditorState::update(float delta_time ) {
        // Update animation timer
        pimpl->total_time += delta_time;
        pimpl->handle_first_frame_setup();
        pimpl->process_input();
        if (pimpl->camera) {
            pimpl->camera->on_update(delta_time);
        }
        pimpl->begin_dockspace();
        pimpl->draw_debug_window();
        pimpl->render_menu_bar_and_panels();
        pimpl->end_dockspace();        
        
    }




    void EditorState::render(IRenderer* renderer_param) {
        if (renderer_param) {
            // 1. CRITICAL: Clear BOTH the color and depth buffers for the new frame.
            renderer_param->clear(); 

            // 2. Now, draw your 3D test cube into the empty scene.
            pimpl->draw_test_cube();

            // 3. Clear ONLY the depth buffer again before drawing the UI.
            // This is a common technique to make sure the UI always draws on top of the 3D scene.
            renderer_param->clear_depth_buffer(); 
            if (pimpl->editor_context->gui) {
                pimpl->editor_context->gui->render();
                pimpl->editor_context->gui->update_and_render_platform_windows();
            }
        }
    }




    void EditorState::Pimpl::setup_theme_manager() {

        // --- Existing Theme Application Logic ---
        if (editor_context->theme_manager) {
            std::cout << "EditorState: ✅ Theme Manager received in EditorContext." << std::endl;

            const std::string default_theme_name = "Default ImGui Theme";
            if (!editor_context->theme_manager->get_theme(default_theme_name)) {
                editor_context->theme_manager->register_theme(std::make_unique<ImGuiTheme>(default_theme_name));
            }

            // Apply the default theme
            if (!editor_context->theme_manager->apply_theme(default_theme_name)) {
                std::cerr << "EditorState Error: Failed to apply default ImGui theme!" << std::endl;
            } else {
                std::cout << "EditorState: Default ImGui Theme applied." << std::endl;
            }
        } else {
            std::cerr << "EditorState Warning: Theme Manager is null in EditorContext!" << std::endl;
        }
    }




    void EditorState::Pimpl::setup_font_manager() {

        // --- Register Default Font ---
        if (editor_context->font_manager) {
            const std::string default_font_name = "Roboto-Regular"; 
            const std::string default_font_path = "Assets/Fonts/Roboto-Regular.ttf";
            const float default_font_size = 16.0f; // Or 20.0f as per your ImGuiThemeData

            // Only load and register if the font doesn't already exist
            if (!editor_context->font_manager->get_font(default_font_name)) {
                if (!editor_context->font_manager->load_font_from_file(
                        default_font_path,
                        default_font_name,
                        default_font_size,
                        false // Do NOT apply immediately, theme manager will apply
                    )) {
                    std::cerr << "EditorState Error: Failed to load and register default font!" << std::endl;
                    // This might be a critical error if no font can be loaded.
                    // You might want to return false here if font is mandatory.
                } else {
                    std::cout << "EditorState: Default font '" << default_font_name << "' registered." << std::endl;
                }
            }
        } else {
            std::cerr << "EditorState Warning: Font Manager is null, cannot register default font." << std::endl;
        }


    }
    void EditorState::Pimpl::handle_first_frame_setup() {
        if (!is_first_frame) {
            return; // Only run this once
        }
        
        std::cout << "EditorState - Performing first-frame UI setup." << std::endl;
        setup_font_manager();
        setup_theme_manager();
        
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


    void EditorState::Pimpl::render_menu_bar_and_panels() {
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
                if (ImGui::MenuItem("Exit")) { /* TODO: Signal engine to quit */ }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

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
        main_camera->set_viewport_size(800, 600); // Example size
        main_camera->set_field_of_view(60.0f);
        main_camera->set_projection_mode(Salix::Camera::ProjectionMode::Perspective);
        // Set the active camera on the renderer immediately after creating it.
        // if (auto* opengl_renderer = dynamic_cast<OpenGLRenderer*>(editor_context->renderer)) {
        //    opengl_renderer->set_active_camera(main_camera);
        // }

        Entity* player = editor_context->active_scene->create_entity("Player");
        Sprite2D* player_sprite = player->add_element<Sprite2D>();
        const std::string sprite_file_path = "src/Sandbox/TestProject/Assets/Images/test.png";
        player_sprite->load_texture(editor_context->asset_manager, sprite_file_path);


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

        ImGui::Begin("Camera Debug Info");

        const glm::mat4& view = camera->get_view_matrix();
        const glm::mat4& projection = camera->get_projection_matrix();

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

        ImGui::End();
    }
        
}