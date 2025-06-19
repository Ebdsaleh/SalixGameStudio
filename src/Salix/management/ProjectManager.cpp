// Salix/management/ProjectManager.cpp
#include <ProjectManager.h>
#include <Project.h>
#include <iostream>

namespace Salix {

    ProjectManager::ProjectManager() = default;   // use default constructor.
    ProjectManager::~ProjectManager() = default;  // The unique_ptr will handle cleanup.

    void ProjectManager::initialize(AssetManager* asset_mananger) {
        std::cout << "ProjectManager initializing..." << std::endl;
        // We will immediately 'load' our one and only test project, for now.
        load_project("DUMMY_PATH");
        if (active_project) {
            active_project->initialize(asset_mananger);
        }
    }

    void ProjectManager::shutdown() {
        if (active_project) {
            active_project->shutdown();
            active_project.reset();  // Safely deletes the object and sets the pointer to null.
        }
    }
    void ProjectManager::update(float delta_time) {
        if(active_project){
            active_project->update(delta_time);
        }
    }

    void ProjectManager::render(IRenderer* renderer) {
        if (active_project) {
            active_project->render(renderer);
        }
    }

    void ProjectManager::load_project(const std::string& project_path) {
        if (active_project) {
            active_project->shutdown();
        }
        std::cout << "ProjectManager: Loading project from '" << project_path << "'" << std::endl;
        active_project = std::make_unique<Project>();
    }

    Project* ProjectManager::get_active_project(){
        if (active_project) {
            return active_project.get();
        }
        return nullptr;
    }

    void ProjectManager::set_active_project(const std::string& project_name) {
        // Search file system or perhaps,
        // use a recent_projects map<const std::string& project_name, const std::string& file_path>() 
        // to find the project file then go the process of creating and loading the project from that data.
        // like its done in 'load_project'.
        active_project = std::make_unique<Project>();  // placeholder for now. This method isn't used yet.
    }
} // namespace Salix