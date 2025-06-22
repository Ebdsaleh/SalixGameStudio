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
        // Use the C++17 filesystem library to safely construct our paths.
        std::filesystem::path root_path(project_path);
        std::filesystem::path project_root = root_path / project_name;

        // 1. Check if a directory with this name already exists.
        if (FileManager::path_exists(project_root.string())) {
            std::cerr << "ProjectManager Error: A directory named '" << project_name << "' already exists at that location." << std::endl;
            return false;
        }

        std::cout << "ProjectManager: Creating new project at '" << project_root.string() << "'" << std::endl;

        // 2. Create the main project directory.
        if (!FileManager::create_directory(project_root.string())) {
            return false; // create_directory will print its own error.
        }

        // 3. Create the standard asset sub-folders, as per your vision.
        if (!FileManager::create_directory((project_root / "Assets").string())) return false;
        if (!FileManager::create_directory((project_root / "Assets" / "Scenes").string())) return false;
        if (!FileManager::create_directory((project_root / "Assets" / "Scripts").string())) return false;
        if (!FileManager::create_directory((project_root / "Assets" / "Images").string())) return false;
        if (!FileManager::create_directory((project_root / "Assets" / "Audio").string())) return false;
        if (!FileManager::create_directory((project_root / "Assets" / "Models").string())) return false;

        // 4. Create the main project file itself (e.g., "MyGame.salixproj").
        std::filesystem::path project_file_path = project_root / (project_name + ".salixproj");
        std::ofstream project_file(project_file_path);
        if (!project_file.is_open()) {
            std::cerr << "ProjectManager Error: Could not create project file at '" << project_file_path.string() << "'" << std::endl;
            return false;
        }

        // Write some placeholder data to the file for the future.
        project_file << "// Salix Game Studio Project File\n";
        project_file << "ProjectName: " << project_name << "\n";
        project_file << "StartingScene: Assets/Scenes/Main.scene\n";
        project_file.close();

        std::cout << "ProjectManager: Successfully created new project '" << project_name << "'." << std::endl;
        return true;
    }


    bool ProjectManager::load_project(const std::string& project_path) {
        // 1. Validate the project path using our FileManager.
        if (!FileManager::path_exists(project_path)) {
            std::cerr << "ProjectManager Error: Failed to load '" << project_path << "'. Path does not exist." << std::endl;
            return false;
        }

        // 2. Unload any currently active project. This is correct.
        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
        }

        // 3. Use the <filesystem> library to safely parse the path.
        std::filesystem::path path_obj(project_path);
        std::string project_name = path_obj.stem().string(); // "MyGame"
        std::string root_path = path_obj.parent_path().string(); // "../SandboxProject"

        // 4. Create the new Project object with the correct name and path.
        std::cout << "ProjectManager: Loading project '" << project_name << "' from '" << root_path << "'" << std::endl;
        pimpl->active_project = std::make_unique<Project>(project_name, root_path);

        // --- 5. SIMULATE PARSING THE .salixproj FILE ---
        // In the future, you would open the file and read this data.
        // For now, we hard-code it to test the pipeline.
        pimpl->active_project->add_scene_path("Assets/Scenes/MainLevel.scene");
        pimpl->active_project->set_starting_scene("MainLevel");
        // --- END SIMULATION ---

        // 6. KICK OFF THE CHAIN OF COMMAND
        // Tell the newly created project to initialize itself. This is the crucial step.
        // The project will then tell its SceneManager to load the scenes, etc.
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