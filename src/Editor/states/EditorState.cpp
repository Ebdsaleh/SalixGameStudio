#include <Editor/states/EditorState.h>

// Core engine systems needed
#include <Salix/core/EngineMode.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineInterface.h>
#include <Salix/input/ImGuiInputManager.h>
// Editor-specific systems
#include <Editor/EditorContext.h>
#include <Editor/panels/PanelManager.h>
#include <Editor/panels/WorldTreePanel.h>

// For ImGui Docking
#include <Salix/gui/IGui.h>
#include <Salix/rendering/IRenderer.h>
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

namespace Salix {

    // The Pimpl struct holds all the state's data
    struct EditorState::Pimpl {
        bool is_first_frame = true;
        ImGuiContext* call_imgui = nullptr;
        std::unique_ptr<EditorContext> editor_context;
        std::unique_ptr<PanelManager> panel_manager;

        void setup_theme_manager();
        void setup_font_manager();
        void handle_first_frame_setup();
        void begin_dockspace();
        void render_menu_bar_and_panels();
        void end_dockspace();
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

        log_file << "[DEBUG] EditorState::on_enter FINISHED successfully." << std::endl;


        

       
        pimpl->call_imgui = pimpl->editor_context->gui->get_context();
    }




    void EditorState::on_exit() {
        std::cout << "Exiting EditorState..." << std::endl;
    }





    void EditorState::update(float delta_time ) {
        pimpl->handle_first_frame_setup();
        pimpl->begin_dockspace();
        pimpl->render_menu_bar_and_panels();
        pimpl->end_dockspace();
    }




    void EditorState::render(IRenderer* renderer_param) {
        // The editor UI is handled entirely in the update() method via ImGui.
        if (renderer_param) {
            if (pimpl->editor_context->gui) {
                pimpl->editor_context->gui->render();
                // Also call update_and_render_platform_windows if you enable ImGui viewports/docking
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

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Salix Editor Dockspace", nullptr, flags);
        ImGui::PopStyleVar(3);
    }


    void EditorState::Pimpl::render_menu_bar_and_panels() {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

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
        // if (pimpl->panel_manager) {
        //     pimpl->panel_manager->render_all_panels();
        // }
    }


    void EditorState::Pimpl::end_dockspace() {
        ImGui::End();
}
}