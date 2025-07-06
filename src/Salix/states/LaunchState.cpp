// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/IInputManager.h>
#include <Salix/gui/imgui/SDLImGui.h>
#include <Salix/rendering/SDLRenderer.h>
#include <Salix/window/SDLWindow.h>
#include <memory>
#include <iostream>

namespace Salix {

    struct LaunchState::Pimpl {
        InitContext context;
        bool should_switch_to_options = false;
        bool should_switch_to_editor = false;
        bool should_switch_to_game = false;
        bool should_quit_engine = false;
     };
    LaunchState::LaunchState() : pimpl(std::make_unique<Pimpl>()) {}
    LaunchState::~LaunchState() {}

    void LaunchState::on_enter(const InitContext& new_context) {
        std::cout << "Entering LaunchState..." << std::endl;
        pimpl->context = new_context;
        pimpl->context.engine->set_mode(EngineMode::Launch);
        
        if (!pimpl->context.gui) {
            std::cerr << "LaunchState Error: GUI system is null in InitContext!" << std::endl;
        } else {
            std::cout << "LaunchState: ✅ GUI system received." << std::endl;
        }
    }

    void LaunchState::on_exit() {
        std::cout << "Exiting LaunchState..." << std::endl;
    }

    void LaunchState::update(float /*delta_time*/) {
        // Use flags to defer state switching
        
        if (pimpl->context.gui) {
            pimpl->context.gui->new_frame(); // ImGui frame begins here

            
            
            // --- NEW: Build the ImGui UI ---
            ImGui::Begin("Salix Game Studio Launcher"); 
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
                pimpl->should_switch_to_options = true; // Set flag
            }

            if (ImGui::Button("Quit")) {
                std::cout << "Quit button clicked!" << std::endl;
                pimpl->should_quit_engine = true; // Set flag
            }
            ImGui::End(); // <-- IMPORTANT: Always call ImGui::End() after ImGui::Begin()

            // --- Handle keyboard input for state switching ---
            // Ensure this logic is outside the ImGui::Begin/End block if it needs to trigger state switches
            if (pimpl->context.input_manager->is_down(KeyCode::E)) {
                std::cout << "'E' key press detected. Switching to EditorState." << std::endl;
                pimpl->should_switch_to_editor = true; // Set flag
            }
            
            if (pimpl->context.input_manager->is_down(KeyCode::G)) {
                std::cout << "'G' key press detected.Switching to GameState." << std::endl;
                pimpl->should_switch_to_game = true; // Set flag
            }
            
            if (pimpl->context.input_manager->is_down(KeyCode::O)) {
                std::cout << "'O' key press detected.Switching to OptionsMenuState." << std::endl;
                pimpl->should_switch_to_options = true; // Set flag
            }
            // --- END UI building and Input Handling ---

            // --- Now, handle state switches AFTER ImGui::End() ---
            if (pimpl->should_switch_to_options) {
                pimpl->context.engine->switch_state(AppStateType::Options);
                return; // Return after switching state
            }
            if (pimpl->should_switch_to_editor) {
                pimpl->context.engine->switch_state(AppStateType::Editor);
                return;
            }
            if (pimpl->should_switch_to_game) {
                pimpl->context.engine->switch_state(AppStateType::Game);
                return;
            }
            if (pimpl->should_quit_engine) {
                pimpl->context.engine->is_running(false);
                return; // Return after requesting quit
            }
        }
    }
    
    

    void LaunchState::render(IRenderer*  renderer) {
        // This state's visual content is primarily handled by the GUI system (ImGui).
        // The renderer's begin_frame() and end_frame() are called by the Engine's main render loop.
        // So, this method can be empty if the state itself doesn't draw any game world elements.
        if (renderer) {
               
        }

    }
} // namespace Salix