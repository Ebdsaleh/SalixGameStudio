// Salix/states/OptionsMenuState.cpp
#include <Salix/states/OptionsMenuState.h>
#include <Salix/core/InitContext.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/SDLRenderer.h>
#include <Salix/gui/imgui/SDLImGui.h>
#include <imgui.h>
#include <memory>
#include <iostream>

namespace Salix {

    struct OptionsMenuState::Pimpl {
        InitContext context;
        bool should_switch_to_launcher = false;
    };

    OptionsMenuState::OptionsMenuState() : pimpl(std::make_unique<Pimpl>()) {}
    OptionsMenuState::~OptionsMenuState() {}

    void OptionsMenuState::on_enter(const InitContext& new_context) {
        std::cout << "Entering OptionsMenuState..." << std::endl;
        pimpl->context = new_context;
        pimpl->context.engine->set_mode(EngineMode::Options);
    }

    void OptionsMenuState::on_exit() {
        std::cout << "Exiting OptionsMenuState..." << std::endl;
    }

    void OptionsMenuState::update(float delta_time) {
        (void)delta_time;
        // Handle input for changing options here in the future.
        // --- NEW: Build ImGui UI for OptionsMenuState ---
        if (pimpl->context.gui) { // Ensure GUI system is available
            pimpl->context.gui->new_frame(); // Start the ImGui frame for this state

            ImGui::Begin("Options Menu"); // Begin your Options Menu window
            ImGui::Text("--- Options Menu Content ---");
            ImGui::Separator();
            ImGui::Text("Volume: [Slider Placeholder]");
            ImGui::Text("Resolution: [Dropdown Placeholder]");
            
            ImGui::Spacing(); // Add some vertical space

            if (ImGui::Button("Back to Launcher")) {
                std::cout << "Options: Back to Launcher button clicked!" << std::endl;
                // Defer state switch until after ImGui::End() if needed,
                // or ensure there's no more ImGui drawing for this frame.
                pimpl->should_switch_to_launcher = true;
            }
            ImGui::End(); // End your Options Menu window

            if (pimpl->should_switch_to_launcher) {
                pimpl->context.engine->switch_state(AppStateType::Launch);
                return; // Return immediately after state switch
            }
        }
    }

    void OptionsMenuState::render(IRenderer* renderer) {
        // Render the options UI here in the future.
        if (renderer) {
            renderer->clear();
        }
       
    }
} // namespace Salix