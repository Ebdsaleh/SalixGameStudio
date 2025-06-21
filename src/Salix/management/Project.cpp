// Salix/management/Project.cpp
#include <Salix/management/Project.h>
#include <Salix/management/SceneManager.h>
#include <iostream>

namespace Salix {
    struct Project::Pimpl {
        std::unique_ptr<SceneManager> scene_manager;
    };
    Project::Project() : pimpl(std::make_unique<Pimpl>()) {}
    Project::~Project() = default;

    void Project::initialize(AssetManager* asset_manager) {
        std::cout << "Project Initializing..." << std::endl;
        pimpl->scene_manager = std::make_unique<SceneManager>();
        pimpl->scene_manager->initialize(asset_manager);
    }

    void Project::shutdown() {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->shutdown();
            pimpl->scene_manager.reset();
        }
    }

    void Project::update(float delta_time) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->update(delta_time);
        }
    }

    void Project::render(IRenderer* renderer) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->render(renderer);
        }
    }

    SceneManager* Project::get_scene_manager() const {
        return pimpl->scene_manager.get();
    }
} // namespace Salix