// ProjectManager.h
#pragma once

#include <string>
#include <memory>
// Forward declarations
class Project;
class IRenderer;
class AssetManager;

class ProjectManager {
    public:
        ProjectManager();
        ~ProjectManager();
        
        void initialize(AssetManager* asset_manager);
        void shutdown();
        void update(float delta_time);
        void render(IRenderer* renderer);

        void load_project(const std::string& project_path);
        Project* get_active_project();
        void set_active_project(const std::string& project_name);
        

    private:
    std::unique_ptr<Project> active_project;
};