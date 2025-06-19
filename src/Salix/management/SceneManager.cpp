// SceneManager.cpp
#include "SceneManager.h"
#include "../ecs/Scene.h"
#include "../assets/AssetManager.h"
#include <iostream>
#include <algorithm>

// Test includes
#include "../ecs/Entity.h"
#include "../ecs/Sprite2D.h"

SceneManager::SceneManager() : active_scene(nullptr), asset_manager(nullptr) {}
SceneManager::~SceneManager() = default;

void SceneManager::initialize(AssetManager* manager) {
    asset_manager = manager;

    // --- TEST ---
    // Use our new API to create a scene, then populate it.
    Scene* test_scene = create_scene("TestScene");
    set_active_scene("TestScene");

    if (test_scene) {
        Entity* parent = test_scene->create_entity("Parent");
        parent->get_transform()->position = { 200.0f, 200.0f, 0.0f };
        Entity* child = test_scene->create_entity("Child");
        child->get_transform()->set_parent(parent->get_transform());
        child->get_transform()->scale = { 0.2f, 0.2f, 1.0f };
        Sprite2D* sprite = child->add_element<Sprite2D>();
        sprite->load_texture(asset_manager, "assets/test.png");
        sprite->pivot = { 0.5f, 0.5f };
    }
}

void SceneManager::shutdown() {
    scene_list.clear();
    active_scene = nullptr;
}

void SceneManager::update(float delta_time) {
    if (active_scene) {
        active_scene->update(delta_time);
    }
}

void SceneManager::render(IRenderer* renderer) {
    if (active_scene) {
        active_scene->render(renderer);
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
    new_scene_ptr->on_load(asset_manager);
    scene_list.push_back(std::move(new_scene_owner));
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
    scene_list.push_back(std::move(scene_to_add));
}

// This uses the find-by-name and remove pattern.
void SceneManager::remove_scene(const std::string& scene_name) {
    auto it = std::remove_if(scene_list.begin(), scene_list.end(),
        [&](const std::unique_ptr<Scene>& scene) {
            bool should_remove = scene->get_name() == scene_name;
            if (should_remove && scene.get() == active_scene) {
                active_scene = nullptr;
            }
            return should_remove;
        });
    scene_list.erase(it, scene_list.end());
}

// This uses a find-by-name and remove pattern but by passing in the Scene pointer.
void SceneManager::remove_scene(Scene* scene_to_remove) {
    if (scene_to_remove) {
        remove_scene(scene_to_remove->get_name());
    }
}

// This uses a find-by-index and remove pattern.
void SceneManager::remove_scene_at(int index) {
    if (index >= 0 && index < scene_list.size()) {
        if (scene_list[index].get() == active_scene) {
            active_scene = nullptr;
        }
        scene_list.erase(scene_list.begin() + index);
    } else {
        std::cerr << "SceneManager: Invalid index for remove_scene_at." << std::endl;
    }
}

void SceneManager::set_active_scene(const std::string& scene_name) {
    Scene* scene = get_scene(scene_name);
    if (scene) {
        active_scene = scene;
        std::cout << "SceneManager: Set active scene to '" << scene_name << "'" << std::endl;
    } else {
        std::cerr << "SceneManager: Could not find scene '" << scene_name << "' to set as active." << std::endl;
    }
}

Scene* SceneManager::get_active_scene() const {
    return active_scene;
}

// This uses as find-by-name and retrieve pattern.
Scene* SceneManager::get_scene(const std::string& scene_name) const {
    auto it = std::find_if(scene_list.begin(), scene_list.end(), 
        [&](const auto& scene) { return scene->get_name() == scene_name; });
    return (it != scene_list.end()) ? it->get() : nullptr;
}

// This uses a find-by-index and retrieve pattern.
Scene* SceneManager::get_scene_at(int index) const {
    if (index >= 0 && index < scene_list.size()) {
        return scene_list[index].get();
    }
    return nullptr;
}

int SceneManager::get_scene_count() const {
    return scene_list.size();
}