// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/IInputManager.h>
#include <Salix/gui/IGui.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/window/IWindow.h>
#include <memory>
#include <iostream>

namespace Salix {

    struct LaunchState::Pimpl {
        InitContext context;
        bool should_switch_to_options = false;
        bool should_switch_to_editor = false;
        bool should_switch_to_game = false;
        bool should_quit_engine = false;
        void reset_transition_flags();
        void present_launcher();
        void process_input();
        void handle_transitions();
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
        if (pimpl->context.renderer) { pimpl->context.renderer->clear(); }
    }

    void LaunchState::update(float /*delta_time*/) {

        pimpl->reset_transition_flags(); 

        if (pimpl->context.gui) {
            pimpl->context.gui->new_frame(); // ImGui frame begins here
            
            pimpl->present_launcher();

            pimpl->process_input();
        } 
        pimpl->handle_transitions();
    }
    
    

    void LaunchState::render(IRenderer*  renderer) {
        // This state's visual content is primarily handled by the GUI system (ImGui).
        // The renderer's begin_frame() and end_frame() are called by the Engine's main render loop.
        // So, this method can be empty if the state itself doesn't draw any game world elements.
        if (renderer) {
               
        }
    }

    void LaunchState::Pimpl::reset_transition_flags() {
        should_switch_to_options = false;
        should_switch_to_editor = false;
        should_switch_to_game = false;
        should_quit_engine = false;
    }

    void LaunchState::Pimpl::present_launcher() {
        int window_width = 0;
        int window_height = 0;
        if (context.window) { 
            context.window->query_dimensions(window_width, window_height);
        }

        // Set the next ImGui window's position and size to fill the SDL window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_width), static_cast<float>(window_height)));

        // Set ImGui window flags to remove decorations and make it a "viewport"
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | // No title bar, borders, resize grip
                                        ImGuiWindowFlags_NoMove |       // Cannot be moved
                                        ImGuiWindowFlags_NoResize |     // Cannot be resized
                                        ImGuiWindowFlags_NoBringToFrontOnFocus | // Stays behind other windows
                                        ImGuiWindowFlags_NoNavFocus |   // Not selectable by navigation
                                        ImGuiWindowFlags_NoBackground;  // No background color (optional, depending on desired look)
                                        // ImGuiWindowFlags_MenuBar; // If you plan to add a menu bar to this window

        // Begin the ImGui window with these flags and an appropriate name
        // Use a unique name like "LauncherMainViewport"
        ImGui::Begin("LauncherMainViewport", nullptr, window_flags); // nullptr for p_open as it's always open
        ImGui::Text("Welcome to Salix Game Studio!");
        ImGui::Text("Press 'E' for Editor, 'G' for Game, 'O' for Options.");
        ImGui::Separator();

        if (ImGui::Button("New Project")) {
            std::cout << "New Project button clicked!" << std::endl;
            // Future: Trigger new project workflow.
        }

        if (ImGui::Button("Open Project")) {
            std::cout << "Open Project button clicked!" << std::endl;
            // Future: Trigger open project dialog
        }

        if (ImGui::Button("Options")) {
            std::cout << "Options button clicked!" << std::endl;
            should_switch_to_options = true; // Set flag
        }

        if (ImGui::Button("Quit")) {
            std::cout << "Quit button clicked!" << std::endl;
            should_quit_engine = true; // Set flag
        }
        ImGui::End(); // <-- IMPORTANT: Always call ImGui::End() after ImGui::Begin()

        
    }

    void LaunchState::Pimpl::process_input() {
        // --- Handle keyboard input for state switching ---
        // Ensure this logic is outside the ImGui::Begin/End block if it needs to trigger state switches
        if (context.input_manager->is_down(KeyCode::E)) {
            std::cout << "'E' key press detected. Switching to EditorState." << std::endl;
            should_switch_to_editor = true; // Set flag
        }
        
        if (context.input_manager->is_down(KeyCode::G)) {
            std::cout << "'G' key press detected.Switching to GameState." << std::endl;
            should_switch_to_game = true; // Set flag
        }
        
        if (context.input_manager->is_down(KeyCode::O)) {
            std::cout << "'O' key press detected.Switching to OptionsMenuState." << std::endl;
            should_switch_to_options = true; // Set flag
        }
        // --- END UI building and Input Handling ---

        // --- Now, handle state switches AFTER ImGui::End() ---
        
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
} // namespace Salix