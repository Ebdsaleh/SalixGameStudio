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
#include <json.hpp>

// for convenience.
using json = nlohmann::json;

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
        // --- 1. Construct all paths using the robust std::filesystem::path object ---
        std::filesystem::path project_root = std::filesystem::path(project_path) / project_name;
        std::filesystem::path dest_assets_dir = project_root / "Assets";
        std::filesystem::path dest_scenes_dir = dest_assets_dir / "Scenes";
        std::filesystem::path dest_images_dir = dest_assets_dir / "Images";
        std::filesystem::path dest_audio_dir = dest_assets_dir / "Audio";
        std::filesystem::path dest_models_dir = dest_assets_dir / "Models";

        // Check if the project already exists.
        if (FileManager::path_exists(project_root.string())) {
            std::cout << "ProjectManager: Project '" << project_name << "' already exists. No action taken." << std::endl;
            return true;
        }
        
        std::cout << "ProjectManager: Creating new project '" << project_name << "' at '" << project_path << "'" << std::endl;

        // --- 2. Create Destination Directory Structure ---
        if (!FileManager::create_directories(dest_scenes_dir.string())) return false;
        if (!FileManager::create_directories((dest_assets_dir / "Scripts").string())) return false;
        if (!FileManager::create_directories(dest_images_dir.string())) return false;
        if (!FileManager::create_directories((dest_audio_dir / "Audio").string())) return false;
        if (!FileManager::create_directories((dest_models_dir / "Models").string())) return false;
        
        // --- 3. Generate the Project File from the Template ---
        std::cout << "ProjectManager: Generating project file..." << std::endl;
        
        // Use filesystem paths for source templates as well for consistency.
        std::filesystem::path project_template_src = "src/Salix/resources/templates/default/project.json";
        
        std::ifstream template_file(project_template_src);
        if (!template_file.is_open()) {
            std::cerr << "ProjectManager Error: Could not open project template at '" << project_template_src.string() << "'" << std::endl;
            return false;
        }

        json project_data;
        template_file >> project_data;
        template_file.close();

        project_data["project_name"] = project_name;
        project_data["build_settings"]["game_dll_name"] = project_name + ".dll";

        std::filesystem::path project_file_dest = project_root / (project_name + ".salixproj");
        std::ofstream new_project_file(project_file_dest);
        new_project_file << std::setw(4) << project_data << std::endl;
        new_project_file.close();

        // --- 4. Copy the other template files (scene and sprite) ---
        std::cout << "ProjectManager: Copying default assets..." << std::endl;
        
        // Define the CORRECT source paths, based on your directory structure.
        std::filesystem::path scene_template_src = "src/Salix/resources/templates/default/Assets/Scenes/JSON/Default.json";
        std::filesystem::path sprite_template_src = "src/Salix/resources/templates/default/Assets/Images/Sprites/test.png";

        // Define the destination paths using the path objects.
        std::filesystem::path scene_template_dest = dest_scenes_dir / "Default.scene";
        std::filesystem::path sprite_template_dest = dest_images_dir / "test.png";

        if (!FileManager::copy_file(scene_template_src.string(), scene_template_dest.string())) return false;
        if (!FileManager::copy_file(sprite_template_src.string(), sprite_template_dest.string())) return false;


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