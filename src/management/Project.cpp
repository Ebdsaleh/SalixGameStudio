// Project.cpp
#include "Project.h"
#include "SceneManager.h"
#include <iostream>

Project::Project() = default;
Project::~Project() = default;

void Project::initialize(AssetManager* asset_manager) {
    std::cout << "Project Initializing..." << std::endl;
    scene_manager = std::make_unique<SceneManager>();
    scene_manager->initialize(asset_manager);
}

void Project::shutdown() {
    if (scene_manager) {
        scene_manager->shutdown();
        scene_manager.reset();
    }
}

void Project::update(float delta_time) {
    if (scene_manager) {
        scene_manager->update(delta_time);
    }
}

void Project::render(IRenderer* renderer) {
    if (scene_manager) {
        scene_manager->render(renderer);
    }
}

SceneManager* Project::get_scene_manager() const {
    return scene_manager.get();
}