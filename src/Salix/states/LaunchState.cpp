// Salix/states/LaunchState.cpp
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/IInputManager.h>
#include <Salix/gui/IGui.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/window/IWindow.h>
#include <Salix/gui/ITheme.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/IFont.h>
#include <Salix/gui/IFontManager.h>
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <ImGuiFileDialog.h>
#include <memory>
#include <iostream>

namespace Salix {

    struct LaunchState::Pimpl {
        InitContext context;
        bool should_switch_to_options = false;
        bool should_switch_to_editor = false;
        bool should_switch_to_game = false;
        bool should_quit_engine = false;
        bool should_close_dialog = false;
        
        void reset_transition_flags();
        void present_launcher();
        void handle_transitions();
        void open_new_project_dialog();
        void open_existing_project_dialog();
        void open_existing_project_to_run_dialog();
        void handle_file_dialogs();
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

        // --- NEW: Register Default Font ---
        if (pimpl->context.font_manager) {
            const std::string default_font_name = "Roboto-Regular"; 
            const std::string default_font_path = "Assets/Fonts/Roboto-Regular.ttf";
            const float default_font_size = 16.0f; // Or 20.0f as per your ImGuiThemeData

            // Only load and register if the font doesn't already exist
            if (!pimpl->context.font_manager->get_font(default_font_name)) {
                if (!pimpl->context.font_manager->load_font_from_file(
                        default_font_path,
                        default_font_name,
                        default_font_size,
                        false // Do NOT apply immediately, theme manager will apply
                    )) {
                    std::cerr << "LaunchState Error: Failed to load and register default font!" << std::endl;
                    // This might be a critical error if no font can be loaded.
                    // You might want to return false here if font is mandatory.
                } else {
                    std::cout << "LaunchState: Default font '" << default_font_name << "' registered." << std::endl;
                }
            }
        } else {
            std::cerr << "LaunchState Warning: Font Manager is null, cannot register default font." << std::endl;
        }
        // --- END NEW ---


        // --- Existing Theme Application Logic ---
        if (pimpl->context.theme_manager) {
            std::cout << "LaunchState: ✅ Theme Manager received in InitContext." << std::endl;

            const std::string default_theme_name = "Default ImGui Theme";
            if (!pimpl->context.theme_manager->get_theme(default_theme_name)) {
                pimpl->context.theme_manager->register_theme(std::make_unique<ImGuiTheme>(default_theme_name));
            }

            // Apply the default theme
            if (!pimpl->context.theme_manager->apply_theme(default_theme_name)) {
                std::cerr << "LaunchState Error: Failed to apply default ImGui theme!" << std::endl;
            } else {
                std::cout << "LaunchState: Default ImGui Theme applied." << std::endl;
            }
        } else {
            std::cerr << "LaunchState Warning: Theme Manager is null in InitContext!" << std::endl;
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

            pimpl->handle_file_dialogs();

        } 
        pimpl->handle_transitions();
    }
    
    

    void LaunchState::render(IRenderer*  renderer) {
        // This state's visual content is primarily handled by the GUI system (ImGui).
        // The renderer's begin_frame() and end_frame() are called by the Engine's main render loop.
        // So, this method can be empty if the state itself doesn't draw any game world elements.
        if (renderer) {
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
        ImGui::Separator();

        if (ImGui::Button("New Project")) {
            std::cout << "New Project button clicked!" << std::endl;
            // Future: Trigger new project workflow.
            context.gui->open_save_file_dialog("ChooseNewProjectLocation", "Create New Project", ".salixproj");
        }

        if (ImGui::Button("Open Project")) {
            std::cout << "Open Project button clicked!" << std::endl;
            // Future: Trigger open project dialog
            context.gui->open_load_file_dialog("ChooseExistingProject", "Open An Existing Project", ".salixproj");
        }

        if (ImGui::Button("Run Project")) {
            std::cout << "Run Project button clicked!" << std::endl;
            context.gui->open_load_file_dialog("ChooseExistingProjectToRun", "Run Existing Project", ".salixproj");
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

    void LaunchState::Pimpl::open_new_project_dialog() {
        
        context.gui->set_common_dialog_properties();
        // 1. Create an instance of FileDialogConfig
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.fileName = "";
        config.countSelectionMax = 1;
        config.userDatas = nullptr;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;

        // Open the save file dialog
        ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseNewProjectLocation", // Unique dialog key
                "Create New Project",       // Dialog title
                ".salixproj",               // Filter for .salixproj files
                config
            );
            context.gui->open_save_file_dialog("ChooseNewProjectLocation", "Create New Project", ".salixproj", config.path, config.fileName);
        }
    
    void LaunchState::Pimpl::open_existing_project_dialog() {
        
        context.gui->set_common_dialog_properties();
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.fileName = "";
        config.countSelectionMax = 1;
        config.userDatas = nullptr;
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog( 
            "ChooseExistingProject",
            "Open An Existing Project",
            ".salixproj",
            config
        );
        context.gui->open_load_file_dialog("ChooseExistingProject", "Open An Existing Project", ".salixproj");
    }

    void LaunchState::Pimpl::open_existing_project_to_run_dialog() {

        context.gui->set_common_dialog_properties();
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.fileName = "";
        config.countSelectionMax = 1;
        config.userDatas = nullptr;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseExistingProjectToRun", // Unique dialog key for run dialog
            "Run Existing Project",       // Dialog title
            ".salixproj",                 // Filter for .salixproj files
            config
        );
        context.gui->open_load_file_dialog("ChooseExistingProjectToRun", "Run Existing Project", ".salixproj");
    }

    void LaunchState::Pimpl::handle_file_dialogs() {
        // NEW: Call IGui to display all active dialogs (this updates ImGuiFileDialog internal state)
        context.gui->display_dialogs();

        // Check results of dialogs that were closed this frame
        if (context.gui->is_dialog_closed_this_frame("ChooseNewProjectLocation")) {
            FileDialogResult result = context.gui->get_dialog_result("ChooseNewProjectLocation");
            if (result.is_ok) {
                std::cout << "New Project selected path: " << result.file_path_name << std::endl;
                should_switch_to_editor = true;
            }
        }

        if (context.gui->is_dialog_closed_this_frame("ChooseExistingProject")) {
            FileDialogResult result = context.gui->get_dialog_result("ChooseExistingProject");
            if (result.is_ok) {
                std::cout << "Open Project selected path: " << result.file_path_name << std::endl;
                should_switch_to_editor = true;
            }
        }

        if (context.gui->is_dialog_closed_this_frame("ChooseExistingProjectToRun")) {
            FileDialogResult result = context.gui->get_dialog_result("ChooseExistingProjectToRun");
            if (result.is_ok) {
                std::cout << "Run Project selected path: " << result.file_path_name << std::endl;
                should_switch_to_game = true;
            }
        }
    }
} // namespace Salix