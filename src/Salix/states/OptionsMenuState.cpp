// Salix/states/OptionsMenuState.cpp
#include <Salix/states/OptionsMenuState.h>
#include <iostream>

namespace Salix {

    OptionsMenuState::OptionsMenuState() {}
    OptionsMenuState::~OptionsMenuState() {}

    void OptionsMenuState::on_enter(Engine* /*engine*/) {
        std::cout << "Entering OptionsMenuState..." << std::endl;
    }

    void OptionsMenuState::on_exit() {
        std::cout << "Exiting OptionsMenuState..." << std::endl;
    }

    void OptionsMenuState::update(float /*delta_time*/) {
        // Handle input for changing options here in the future.
    }

    void OptionsMenuState::render(IRenderer* /*renderer*/) {
        // Render the options UI here in the future.
    }
} // namespace Salix