// Salix/management/SceneManager.cpp
#include <Salix/management/SceneManager.h>
#include <Salix/ecs/Scene.h>
#include <Salix/assets/AssetManager.h>
#include <iostream>
#include <algorithm>


namespace Salix {

    struct SceneManager::Pimpl {
        std::vector<std::unique_ptr<Scene>> scene_list;
        Scene* active_scene = nullptr;
        AssetManager* asset_manager = nullptr;
    };
    SceneManager::SceneManager() : pimpl(std::make_unique<Pimpl>()) {}

    SceneManager::~SceneManager() = default;

    void SceneManager::initialize(AssetManager* asset_manager_ptr) {
        pimpl->asset_manager = asset_manager_ptr;
        std::cout<< "SceneManager Initialized." << std::endl;
    }

    void SceneManager::shutdown() {
        pimpl->scene_list.clear();
        pimpl->active_scene = nullptr;
    }

    void SceneManager::update(float delta_time) {
        if (pimpl->active_scene) {
            pimpl->active_scene->update(delta_time);
        }
    }

    void SceneManager::render(IRenderer* renderer) {
        if (pimpl->active_scene) {
            pimpl->active_scene->render(renderer);
        }
    }

    // --- API IMPLEMENTATIONS ---

    // This is useful as an Engine API call.
    Scene* SceneManager::create_scene(const std::string& scene_name) {
        if (get_scene(scene_name)) {
            std::cerr << "SceneManager: Scene '" << scene_name << "' already exists." << std::endl;
            return nullptr;
        }

        std::cout << "SceneManager: Creating new scene '" << scene_name << "'" << std::endl;
        auto new_scene_owner = std::make_unique<Scene>(scene_name);
        Scene* new_scene_ptr = new_scene_owner.get();
        new_scene_ptr->on_load(pimpl->asset_manager);
        pimpl->scene_list.push_back(std::move(new_scene_owner));
        return new_scene_ptr;
    }

    // This is useful as a 'user' API call.
    void SceneManager::add_scene(std::unique_ptr<Scene> scene_to_add) {
        if (!scene_to_add) return;
        if (get_scene(scene_to_add->get_name())) {
            std::cerr << "SceneManager: Scene '" << scene_to_add->get_name() << "' already exists." << std::endl;
            return;
        }
        std::cout << "SceneManager: Adding existing scene '" << scene_to_add->get_name() << "'" << std::endl;
        pimpl->scene_list.push_back(std::move(scene_to_add));
    }

    // This uses the find-by-name and remove pattern.
    void SceneManager::remove_scene(const std::string& scene_name) {
        auto it = std::remove_if(pimpl->scene_list.begin(), pimpl->scene_list.end(),
            [&](const std::unique_ptr<Scene>& scene) {
                bool should_remove = scene->get_name() == scene_name;
                if (should_remove && scene.get() == pimpl->active_scene) {
                    pimpl->active_scene = nullptr;
                }
                return should_remove;
            });
        pimpl->scene_list.erase(it, pimpl->scene_list.end());
    }

    // This uses a find-by-name and remove pattern but by passing in the Scene pointer.
    void SceneManager::remove_scene(Scene* scene_to_remove) {
        if (scene_to_remove) {
            remove_scene(scene_to_remove->get_name());
        }
    }

    // This uses a find-by-index and remove pattern.
    void SceneManager::remove_scene_at(int index) {
        if (index >= 0 && index < pimpl->scene_list.size()) {
            if (pimpl->scene_list[index].get() == pimpl->active_scene) {
                pimpl->active_scene = nullptr;
            }
            pimpl->scene_list.erase(pimpl->scene_list.begin() + index);
        } else {
            std::cerr << "SceneManager: Invalid index for remove_scene_at." << std::endl;
        }
    }

    void SceneManager::set_active_scene(const std::string& scene_name) {
        Scene* scene = get_scene(scene_name);
        if (scene) {
            pimpl->active_scene = scene;
            std::cout << "SceneManager: Set active scene to '" << scene_name << "'" << std::endl;
        } else {
            std::cerr << "SceneManager: Could not find scene '" << scene_name << "' to set as active." << std::endl;
        }
    }

    Scene* SceneManager::get_active_scene() const {
        return pimpl->active_scene;
    }

    // This uses as find-by-name and retrieve pattern.
    Scene* SceneManager::get_scene(const std::string& scene_name) const {
        auto it = std::find_if(pimpl->scene_list.begin(), pimpl->scene_list.end(), 
            [&](const auto& scene) { return scene->get_name() == scene_name; });
        return (it != pimpl->scene_list.end()) ? it->get() : nullptr;
    }

    // This uses a find-by-index and retrieve pattern.
    Scene* SceneManager::get_scene_at(int index) const {
        if (index >= 0 && index < pimpl->scene_list.size()) {
            return pimpl->scene_list[index].get();
        }
        return nullptr;
    }

    size_t SceneManager::get_scene_count() const {
        return pimpl->scene_list.size();
    }
} // namespace Salix