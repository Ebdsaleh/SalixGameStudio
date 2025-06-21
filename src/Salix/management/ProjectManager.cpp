// Salix/management/ProjectManager.cpp
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <iostream>

namespace Salix {
    // Define the implementation struct here, inside the .cpp file.
    struct ProjectManager::Pimpl {
        std::unique_ptr<Project> active_project;
    };

    ProjectManager::ProjectManager() : pimpl(std::make_unique<Pimpl>()) {
        
    }
    ProjectManager::~ProjectManager() = default;  // The unique_ptr will handle cleanup.

    void ProjectManager::initialize(AssetManager* asset_mananger) {
        std::cout << "ProjectManager initializing..." << std::endl;
        // We will immediately 'load' our one and only test project, for now.
        load_project("DUMMY_PATH");
        if (pimpl->active_project) {
            pimpl->active_project->initialize(asset_mananger);
        }
    }

    void ProjectManager::shutdown() {
        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
            pimpl->active_project.reset();  // Safely deletes the object and sets the pointer to null.
        }
    }
    void ProjectManager::update(float delta_time) {
        if(pimpl->active_project){
            pimpl->active_project->update(delta_time);
        }
    }

    void ProjectManager::render(IRenderer* renderer) {
        if (pimpl->active_project) {
            pimpl->active_project->render(renderer);
        }
    }

    void ProjectManager::load_project(const std::string& project_path) {
        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
        }
        std::cout << "ProjectManager: Loading project from '" << project_path << "'" << std::endl;
        pimpl->active_project = std::make_unique<Project>();
    }

    Project* ProjectManager::get_active_project(){
        if (pimpl->active_project) {
            return pimpl->active_project.get();
        }
        return nullptr;
    }

    void ProjectManager::set_active_project(const std::string& /*project_name*/) {
        // Search file system or perhaps,
        // use a recent_projects map<const std::string& project_name, const std::string& file_path>() 
        // to find the project file then go the process of creating and loading the project from that data.
        // like its done in 'load_project'.
        pimpl->active_project = std::make_unique<Project>();  // placeholder for now. This method isn't used yet.
    }
} // namespace Salix