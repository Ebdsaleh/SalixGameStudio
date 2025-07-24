// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/IInputManager.h>
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
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Salix {

    struct LaunchState::Pimpl {
        InitContext context;
        bool should_switch_to_options = false;
        bool should_switch_to_editor = false;
        bool should_switch_to_game = false;
        bool should_quit_engine = false;
        bool should_close_dialog = false;
        bool is_first_frame = true;


        // dialog box references
        std::string new_project_key = "ChooseNewProjectLocation";
        std::string new_project_title = "Create New Project";

        std::string open_project_key = "ChooseExistingProject";
        std::string open_project_title = "Open An Existing Project";

        std::string run_project_key = "ChooseExistingProjectToRun";
        std::string run_project_title ="Run An Existing Project";
        
        
        void setup_ui();
        void reset_transition_flags();
        void present_launcher();
        void handle_transitions();
        void create_dialog_boxes();
        
     };
    LaunchState::LaunchState() : pimpl(std::make_unique<Pimpl>()) {}
    LaunchState::~LaunchState() {}

    void LaunchState::on_enter(const InitContext& new_context) {
        std::cout << "Entering LaunchState..." << std::endl;
        pimpl->context = new_context;
        pimpl->context.engine->set_mode(EngineMode::Launch);
        // Reset flags on entry to ensure a clean state
        pimpl->reset_transition_flags();
        if (!pimpl->context.gui) {
            std::cerr << "LaunchState Error: GUI system is null in InitContext!" << std::endl;
        } else {
            std::cout << "LaunchState: ✅ GUI system received." << std::endl;
            
        }   
        
    }



    void LaunchState::on_exit() {
        std::cout << "Exiting LaunchState..." << std::endl;
        if (pimpl->context.renderer) {
            
        }

        
    }

    void LaunchState::update(float /*delta_time*/) {
        if (pimpl->is_first_frame) {
        pimpl->setup_ui();
        pimpl->is_first_frame = false;
        }

        pimpl->reset_transition_flags(); 

        if (pimpl->context.gui) {
            
            pimpl->present_launcher();

            pimpl->context.gui->display_dialogs();
            
        } 
        pimpl->handle_transitions();
    }
    
    

    void LaunchState::render(IRenderer*  renderer) {


        // This state's visual content is primarily handled by the GUI system (ImGui).
        // The renderer's begin_frame() and end_frame() are called by the Engine's main render loop.
        // So, this method can be empty if the state itself doesn't draw any game world elements.
        if (renderer) {
            OpenGLRenderer* opengl_renderer = dynamic_cast<OpenGLRenderer*>(renderer);
            if (opengl_renderer) {
                glm::mat4 dummy_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -100.0f));
                opengl_renderer->draw_cube(dummy_model, {0, 0, 0, 0}); // Invisible color
            }

            // NEW: Call the GUI system's render method here
            if (pimpl->context.gui) {
                pimpl->context.gui->render();
                // Also call update_and_render_platform_windows if you enable ImGui viewports/docking
                pimpl->context.gui->update_and_render_platform_windows();
            }
        }
    }

    void LaunchState::Pimpl::reset_transition_flags() {
        should_switch_to_options = false;
        should_switch_to_editor = false;
        should_switch_to_game = false;
        should_quit_engine = false;
    }

    void LaunchState::Pimpl::present_launcher() {
        int window_width, window_height;
        context.window->query_dimensions(window_width, window_height);

        // --- Create a full-screen host window, just like in EditorState ---
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("LauncherHostWindow", nullptr, host_flags);

        // --- Create a dockspace inside it ---
        ImGuiID dockspace_id = ImGui::GetID("LauncherDockspace");
        ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dock_flags);

        ImGui::End(); // End LauncherHostWindow

        // --- Now, create your actual launcher window as a separate window ---
        // It will appear centered and floating on top of the main view.
        ImGui::Begin("Welcome to Salix Game Studio!");

        if (ImGui::Button("New Project")) {
            context.gui->show_dialog_by_key(new_project_key);
        }
        if (ImGui::Button("Open Project")) {
            context.gui->show_dialog_by_key(open_project_key);
        }
        if (ImGui::Button("Run Project")) {
            context.gui->show_dialog_by_key(run_project_key);
        }
        if (ImGui::Button("Options")) {
            should_switch_to_options = true;
        }
        if (ImGui::Button("Quit")) {
            should_quit_engine = true;
        }

        ImGui::End(); // End "Welcome to Salix Game Studio!"
    }

    

    void LaunchState::Pimpl::handle_transitions() {
        if (should_switch_to_options) {
            context.engine->switch_state(AppStateType::Options);
            return; // Return after switching state
        }

        if (should_switch_to_editor) {
            context.engine->switch_state(AppStateType::Editor);
            return;
        }

        if (should_switch_to_game) {
            context.engine->switch_state(AppStateType::Game);
            return;
        }

        if (should_quit_engine) {
            context.engine->is_running(false);
            return; // Return after requesting quit
        }
    }

    void LaunchState::Pimpl::create_dialog_boxes() {

        // New Project
        
        DialogBox* new_project_dialog = context.gui->create_dialog(
            new_project_key,
            new_project_title,
            DialogType::File,
            true);
        
        new_project_dialog->SetFilters(std::string("Salix Project (*.salixproj){.salixproj},All files (*.*){.*}"));

        new_project_dialog->SetCallback([this](const FileDialogResult& result) {
            
            if (result.is_ok) {
                // will add new project creation method later
                this->should_switch_to_editor = true;
                std::cout << "Action: New Project selected (via callback)." << std::endl; // Add a debug message
            } else {
                std::cout << "Action: New Project dialog canceled (via callback)." << std::endl;
            }
        });


        // Open Project
        
        DialogBox* open_project_dialog = context.gui->create_dialog(
            open_project_key,
            open_project_title,
            DialogType::File,
            false);

        open_project_dialog->SetFilters(std::string("Salix Project (*.salixproj){.salixproj},All files (*.*){.*}"));

        open_project_dialog->SetCallback([this](const FileDialogResult& result) {
            
            if (result.is_ok) {
                // will add new project creation method later
                this->should_switch_to_editor = true;
                std::cout << "Action: Open Project selected (via callback)." << std::endl; // Add a debug message
            } else {
                std::cout << "Action: Open Project dialog canceled (via callback)." << std::endl;
            }
        });

        // Run Project
        
        DialogBox* run_project_dialog = context.gui->create_dialog(
           run_project_key,
           run_project_title,
           DialogType::File,
           false);

        run_project_dialog->SetFilters(std::string("Salix Project (*.salixproj){.salixproj},All files (*.*){.*}"));

        run_project_dialog->SetCallback([this](const FileDialogResult& result) {
            
            if (result.is_ok) {
                // will add new project creation method later
                this->should_switch_to_game = true;
                std::cout << "Action: Run Project selected (via callback)." << std::endl; // Add a debug message
            } else {
                std::cout << "Action: Run Project dialog canceled (via callback)." << std::endl;
            }
        });

    }
    
   
    void LaunchState::Pimpl::setup_ui() {
        
        create_dialog_boxes();
        is_first_frame = false;
    }
    
} // namespace Salix