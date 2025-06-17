// LaunchState.cpp
#include "LaunchState.h"
#include <iostream>

LaunchState::LaunchState() {}
LaunchState::~LaunchState() {}

void LaunchState::on_enter(Engine* engine) {
    std::cout << "Entering LaunchState..." << std::endl;
}

void LaunchState::on_exit() {
    std::cout << "Exiting LaunchState..." << std::endl;
}

void LaunchState::update(float delta_time) {
    // In the future we'll check for button clicks here.
    // For now, it does nothing.
}

void LaunchState::render(IRenderer* renderer) {
    // This state has no game world to render, but it might have UI.
    // For now, this does nothing.
}