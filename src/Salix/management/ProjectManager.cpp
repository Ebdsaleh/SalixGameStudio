// =================================================================================
// Filename:    Salix/management/ProjectManager.cpp
// Author:      SalixGameStudio
// Description: Implements the ProjectManager, which creates, loads, and manages
//              game projects.
// =================================================================================
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/FileManager.h> // We need our file manager
#include <filesystem>                     // The modern C++ way to handle paths
#include <fstream>                        // For writing the project file
#include <iostream>

namespace Salix {

    // --- Pimpl struct definition ---
    struct ProjectManager::Pimpl {
        std::unique_ptr<Project> active_project;
        AssetManager* asset_manager = nullptr; // The PM needs to know about the AssetManager
    };


    // --- Constructor and Destructor ---
    ProjectManager::ProjectManager() : pimpl(std::make_unique<Pimpl>()) {}
    ProjectManager::~ProjectManager() = default;
    

    // --- Lifecycle Methods ---
    void ProjectManager::initialize(AssetManager* asset_manager_ptr) {
        pimpl->asset_manager = asset_manager_ptr;
        std::cout << "ProjectManager Initialized." << std::endl;
        // For our test, we'll immediately load our sandbox project.
        // The path is relative to the executable in the 'build' folder.
        load_project("../SandboxProject/MyGame.salixproj");
    }

    void ProjectManager::shutdown() {
        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
            pimpl->active_project.reset();
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

    // --- CREATE NEW PROJECT ---
    bool ProjectManager::create_new_project(const std::string& project_path, const std::string& project_name) {
        std::filesystem::path root_path(project_path);
        std::filesystem::path project_root = root_path / project_name;

        // We don't need to check if it exists first, because create_directories will handle it.
        std::cout << "ProjectManager: Creating new project at '" << project_root.string() << "'" << std::endl;

        // --- THE FIX ---
        // We now use our new, more robust create_directories function for each path.
        // This function is designed to handle complex relative paths safely.
        if (!FileManager::create_directories((project_root / "Assets" / "Scenes").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Scripts").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Images").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Audio").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Models").string())) return false;

        // Create the main project file itself (e.g., "TestProject.salixproj").
        std::filesystem::path project_file_path = project_root / (project_name + ".salixproj");
        
        // Check if file already exists before creating it
        if (FileManager::path_exists(project_file_path.string())){
             std::cout << "ProjectManager: Project file already exists." << std::endl;
        }
        else {
            std::ofstream project_file(project_file_path);
            if (!project_file.is_open()) {
                std::cerr << "ProjectManager Error: Could not create project file at '" << project_file_path.string() << "'" << std::endl;
                return false;
            }
            project_file << "// Salix Game Studio Project File\n";
            project_file << "ProjectName: " << project_name << "\n";
            project_file << "StartingScene: MainLevel\n";
            project_file.close();
        }

        std::cout << "ProjectManager: Successfully created new project '" << project_name << "'." << std::endl;
        return true;
    }

    // --- DEVELOPMENT-ONLY METHODS ---
    
   bool ProjectManager::create_new_external_project(const std::string& project_path, const std::string& project_name) {
        // This is the robust directory creation logic we built before.
        std::filesystem::path root_path(project_path);
        std::filesystem::path project_root = root_path / project_name;

        if (FileManager::path_exists(project_root.string())) {
            std::cout << "ProjectManager: Project '" << project_name << "' already exists. No action taken." << std::endl;
            return true;
        }

        std::cout << "ProjectManager: Creating new project '" << project_name << "' at '" << project_path << "'" << std::endl;

        // Use our robust create_directories function.
        if (!FileManager::create_directories((project_root / "Assets" / "Scenes").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Scripts").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Images").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Audio").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Models").string())) return false;

        // Create the Default.scene file
        // --- NEW: Create a default, empty scene file ---
        std::filesystem::path default_scene_path = project_root / "Assets" / "Scenes" / "Default.scene";
        std::ofstream scene_file(default_scene_path);
        if (!scene_file.is_open()) {
            std::cerr << "ProjectManager Error: Could not create default scene file." << std::endl;
            return false;
        }
        scene_file << "// Salix Engine Scene File\n";
        scene_file << "Entities: []\n"; // Placeholder for future YAML/JSON data
        scene_file.close();

        std::filesystem::path project_file_path = project_root / (project_name + ".salixproj");
        std::ofstream project_file(project_file_path);
        if (!project_file.is_open()) {
            std::cerr << "ProjectManager Error: Could not create project file." << std::endl;
            return false;
        }

        // 3. Write the project data, now pointing to our new default scene.
        project_file << "// Salix Game Studio Project File\n";
        project_file << "ProjectName: " << project_name << "\n";
        project_file << "StartingScene: Assets/Scenes/Default.scene\n"; // <-- Points to our new file
        project_file.close();

        std::cout << "ProjectManager: Successfully created new project '" << project_name << "'." << std::endl;
        return true;
    }
    

    bool ProjectManager::create_new_internal_project(const std::string& project_name) {
        // This is the wrapper that calls the external version with our fixed sandbox path.
        const std::string internal_path = "src/Sandbox";
        return create_new_external_project(internal_path, project_name);
    }
    // --- END OF DEVELOPER-ONLY METHODS ---


    bool ProjectManager::load_project(const std::string& project_file_path) {
        if (!FileManager::path_exists(project_file_path)) {
            std::cerr << "ProjectManager Error: Failed to load '" << project_file_path << "'. Path does not exist." << std::endl;
            return false;
        }

        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
        }

        std::filesystem::path path_obj(project_file_path);
        std::string project_name = path_obj.stem().string();
        std::string root_path = path_obj.parent_path().string();

        pimpl->active_project = std::make_unique<Project>(project_name, root_path);

        // --- SIMULATE PARSING THE FILE ---
        pimpl->active_project->add_scene_path("Assets/Scenes/MainLevel.scene");
        pimpl->active_project->set_starting_scene("MainLevel");

        // --- KICK OFF THE CHAIN OF COMMAND ---
        if (pimpl->active_project) {
            pimpl->active_project->initialize(pimpl->asset_manager);
        }
        return true;
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