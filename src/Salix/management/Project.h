// Project.h
#pragma once

#include <string>
#include <memory>

// Forward declarations
class SceneManager;
class IRenderer;
class AssetManager;

class Project {
public:
    Project();
    ~Project();

    void initialize(AssetManager* asset_manager);
    void shutdown();
    void update(float delta_time);
    void render(IRenderer* renderer);
    
    SceneManager* get_scene_manager() const;

private:
    std::unique_ptr<SceneManager> scene_manager;
};