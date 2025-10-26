// =================================================================================
// Filename:    Salix/management/ProjectManager.cpp
// Author:      SalixGameStudio
// Description: Implements the ProjectManager, which creates, loads, and manages
//              game projects.
// =================================================================================

#include <Salix/core/Core.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/management/Project.h>
#include <Salix/management/FileManager.h>
#include <Salix/management/ProjectConfig.h>
#include <Salix/management/RealmData.h>
#include <Salix/core/InitContext.h>
#include <filesystem>  
#include <fstream>     
#include <iostream>
#include <string>
#include <iomanip>     
// For std::setw if you want formatted JSON output
// Cereal headers required for ProjectConfig and RealmData manifest
#include <cereal/cereal.hpp>
// Cereal archives
#include <cereal/archives/json.hpp>
// For JSON archives (ProjectConfig & RealmData manifest)
// Cereal types (needed for std::string, std::vector inside ProjectConfig/RealmData)
#include <cereal/types/string.hpp>         
#include <cereal/types/vector.hpp>         
#include <cereal/types/array.hpp>

namespace Salix {
    SALIX_API std::filesystem::path g_project_root_path;
    // --- Pimpl struct definition ---
    struct ProjectManager::Pimpl {
        std::unique_ptr<Project> active_project;
        InitContext context;
        std::string project_to_load;
        std::string loaded_project_directory;
    };

    // --- Constructor and Destructor ---
    ProjectManager::ProjectManager() : pimpl(std::make_unique<Pimpl>()) {}
    ProjectManager::~ProjectManager() = default;


    // --- Lifecycle Methods ---
    void ProjectManager::initialize(const InitContext& new_context) {
        pimpl->context = new_context;
        std::cout << "ProjectManager Initialized." << std::endl;
       
    }

    const std::string ProjectManager::get_project_to_load() const {
        return pimpl->project_to_load;
    }

    void ProjectManager::set_project_to_load(const std::string full_project_file_path) {
        pimpl->project_to_load = full_project_file_path;
    }

    const std::string ProjectManager::get_loaded_project_directory() const {
        return pimpl->loaded_project_directory;
    }

    void ProjectManager::set_loaded_project_directory(const std::string loaded_project_directory) {
        pimpl->loaded_project_directory = loaded_project_directory;
    }

    void ProjectManager::shutdown() {
        if (pimpl->active_project) {
            pimpl->active_project->shutdown();
            pimpl->active_project.reset();
        }
        std::cout << "ProjectManager Shutdown." << std::endl;
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

    // --- CREATE NEW PROJECT (The simpler text-based one - Keeping for reference if needed) ---
    // This method is not using Cereal. It creates a simple text file.
    bool ProjectManager::create_new_project(const std::string& project_path, const std::string& project_name) {
        std::filesystem::path root_path(project_path);
        
        std::filesystem::path project_root = root_path / project_name;
        

        std::cout << "ProjectManager: Creating new project at '" << project_root.string() << "'" << std::endl;

        if (!FileManager::create_directories((project_root / "Assets" / "Realms").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Scripts").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Images").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Audio").string())) return false;
        if (!FileManager::create_directories((project_root / "Assets" / "Models").string())) return false;

        std::filesystem::path project_file_path = project_root / (project_name + ".salixproj");

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
            project_file << "StartingRealm: MainLevel\n";
            project_file.close();
        }

        std::cout << "ProjectManager: Successfully created new project '" << project_name << "'." << std::endl;
        return true;
    }

    // --- DEVELOPMENT-ONLY METHODS ---

    bool ProjectManager::create_new_external_project(const std::string& project_path, const std::string& project_name) {
        // --- 1. Construct all paths ---
        std::filesystem::path root_path(project_path);
        std::filesystem::path project_root = root_path / project_name;
        std::filesystem::path dest_assets_dir = project_root / "Assets";
        std::filesystem::path dest_scripts_dir = dest_assets_dir / "Scripts";
        std::filesystem::path dest_realms_dir = dest_assets_dir / "Realms";
        std::filesystem::path dest_images_dir = dest_assets_dir / "Images";
        std::filesystem::path dest_audio_dir = dest_assets_dir / "Audio";
        std::filesystem::path dest_models_dir = dest_assets_dir / "Models";
        g_project_root_path = project_root;

        if ( FileManager::path_exists( project_root.string() ) ) {
            std::cout << "ProjectManager: Project '" << project_name << "' already exists. No action taken." << std::endl;
            return true;
        }

        std::cout << "ProjectManager: Creating new project '" << project_name << "' at '" << project_path << "'" << std::endl;

        // --- 2. Create Destination Directory Structure ---
        if (!FileManager::create_directories(dest_realms_dir.string())) return false;
        if (!FileManager::create_directories((dest_scripts_dir).string())) return false;
        if (!FileManager::create_directories(dest_images_dir.string())) return false;
        if (!FileManager::create_directories(dest_audio_dir.string())) return false;
        if (!FileManager::create_directories(dest_models_dir.string())) return false;

        // --- 3. Generate the Project File (.salixproj) from Template using Cereal ---
        std::cout << "ProjectManager: Generating project file using Cereal..." << std::endl;

        std::filesystem::path project_config_template_src = "src/Salix/resources/templates/default/project.json";

        Salix::ProjectConfig project_config;

        // --- START DEBUG & PARSING CODE ---
        // This block now contains all logic for reading the template file and parsing it.

        // Print the absolute path the program is trying to open
        std::cout << "DEBUG: Attempting to open project template from absolute path: "
                  << std::filesystem::absolute(project_config_template_src).string() << std::endl;
        // Print the current working directory - this is key to find out where all our file operations are starting from.
        std::cout << "DEBUG: Current Working Directory is: '" << std::filesystem::current_path() 
                  << "'." << std::endl;

        // Check file size (debug)
        if (FileManager::is_regular_file_and_exists(project_config_template_src.string())) {
            std::cout << "DEBUG: File size: " << FileManager::get_file_size(project_config_template_src.string()) << " bytes" << std::endl;
        } else {
            std::cout << "DEBUG: File does not exist or is not a regular file." << std::endl;
        }

        // Read the entire file content into a string buffer using FileManager utility
        std::string file_content_buffer = FileManager::read_file_content(project_config_template_src.string());

        // DEBUG: Hex dump of the content read (useful for invisible characters)
        std::cout << FileManager::generate_hex_dump(project_config_template_src.string());

        
        // Create a stringstream from the buffer.
        std::stringstream ss(file_content_buffer);

        // --- Cereal Deserialization: Parse from the stringstream ---
        try {
            cereal::JSONInputArchive archive(ss); // Archive parses from stringstream 'ss'
            std::cout << "DEBUG: Buffer content:\n" << file_content_buffer << std::endl;
            archive (
                cereal::make_nvp("project_data", project_config.project_data),
                cereal::make_nvp("build_settings", project_config.build_settings)
            );

        } catch (const cereal::Exception& e) {
            std::cerr << "ProjectManager: Error: Failed to deserialize project configuration template: " <<
            e.what() << std::endl;
            return false; // Return false on deserialization error
        }
        // --- END DEBUG & PARSING CODE ---
        
        // --- Modify the ProjectConfig object in C++ memory (rest of your original code) ---
        std::string project_file_name = project_name + ".salixproj";
        project_config.project_data.project_path = std::filesystem::absolute(project_root).string();
        project_config.project_data.project_file_name = project_file_name;
        project_config.project_data.project_name = project_name;

        // This line is redundant and can be removed
        // project_config.build_settings.engine_version = project_config.build_settings.engine_version; 
        project_config.build_settings.game_dll_name = project_name + ".dll";

        bool default_realm_exists_in_config = false;
        for (const auto& realm_info: project_config.project_data.realms) {
            if (realm_info.name == "Default") {
                default_realm_exists_in_config = true;
                break;
            }
        }
        if (!default_realm_exists_in_config) {
            project_config.project_data.realms.emplace_back("Default", "Assets/Realms/Default.realm");
        }
        project_config.project_data.starting_realm = "Default";

        std::filesystem::path project_file_dest = project_root / project_file_name;
        std::ofstream new_project_file_os(project_file_dest);
        if ( !new_project_file_os.is_open() ) {
            std::cerr << "ProjectManager Error: Could not create new project file at '" << project_file_dest.string() << "'" << std::endl;
            return false;
        }
      
        try {
            cereal::JSONOutputArchive archive(new_project_file_os);
            archive(project_config);
        } catch (const cereal::Exception& e) {
            std::cerr << "ProjectManager: Error: Failed to serialize project config: " << e.what() << std::endl;
            new_project_file_os.close();
            return false;
        }
        new_project_file_os.close();

        // --- 4. Copying default asset templates (unchanged) ---
        std::cout << "ProjectManager: Copying default asset templates..." << std::endl;

        std::filesystem::path realm_template_src = "src/Salix/resources/templates/default/Assets/Realms/Default.realm";
        //std::filesystem::path realm_manifest_template_src = "src/Salix/resources/templates/default/Assets/Realms/Default.realm.manifest";
        std::filesystem::path sprite_template_src = "src/Salix/resources/templates/default/Assets/Images/Sprites/test.png";
        
        std::filesystem::path realm_template_dest = dest_realms_dir / "Default.realm";
        //std::filesystem::path realm_manifest_dest = dest_realms_dir / "Default.realm.manifest";
        std::filesystem::path sprite_template_dest = dest_images_dir / "test.png";

        if ( !FileManager::copy_file(realm_template_src.string(), realm_template_dest.string() ) ){
            std::cerr << "ProjectManager Error: Failed to copy default realm template." << std::endl;
            return false;
        }
        
        // The manifest file might play a part at a later implementation but it's not required for now.
        /* if ( !FileManager::copy_file(realm_manifest_template_src.string(), realm_manifest_dest.string()) ) {
            std::cerr << "ProjectManager Error: Failed to copy default realm manifest template." << std::endl;
            return false;
        }
        */

        if (!FileManager::copy_file(sprite_template_src.string(), sprite_template_dest.string())) {
            std::cerr << "ProjectManager Error: Failed to copy default sprite template." << std::endl;
            return false;
        }
        
        std::cout << "ProjectManager: Successfully created new project '" << project_name << "'." << std::endl;
        return true;
    }


    bool ProjectManager::create_new_internal_project(const std::string& project_name) {
        // This is the wrapper that calls the external version with our fixed sandbox path.
        const std::filesystem::path internal_path = "src/Sandbox";
        return create_new_external_project(internal_path.string(), project_name);
    }
    // --- END OF DEVELOPER-ONLY METHODS ---


    bool ProjectManager::load_project(const std::string& project_file_path) {
        if (!FileManager::path_exists(project_file_path)) {
            std::cerr << "ProjectManager Error: Failed to load '" << project_file_path << "'. Path does not exist." << std::endl;
            return false;
        }

        if (pimpl->active_project) {
            pimpl->active_project->shutdown(); // Shutdown current project if any
        }

        std::cout << "ProjectManager: Loading project from '" << project_file_path << "'..." << std::endl;

        Salix::ProjectConfig loaded_config; // Object to hold deserialized project data

        // --- Cereal Deserialization of ProjectConfig ---
        std::ifstream project_file_is(project_file_path);
        if (!project_file_is.is_open()) {
            std::cerr << "ProjectManager Error: Could not open project file '" << project_file_path << "' for reading." << std::endl;
            return false;
        }

        try {
            cereal::JSONInputArchive archive(project_file_is);
            archive(loaded_config); // Load the entire project config
        } catch (const cereal::Exception& e) {
            std::cerr << "ProjectManager Error: Failed to deserialize project config from '" << project_file_path << "': " << e.what() << std::endl;
            project_file_is.close();
            return false;
        }
        project_file_is.close(); // Close the file stream after deserialization

        // --- Use the loaded_config to create and initialize the Project ---
        // The Project constructor that takes name and root_path is ideal here.
        pimpl->active_project = std::make_unique<Project>(
            loaded_config.project_data.project_name,
            loaded_config.project_data.project_path
        );

        // Now, populate the Project with loaded realm paths and starting realm info
        // Your Project::add_realm_path method signature is `const std::string& path_to_realm_file`.
        // If you later change it to accept name, you'd modify this line accordingly.
        for (const auto& realm_info : loaded_config.project_data.realms) {
            pimpl->active_project->add_realm_path(realm_info.name, realm_info.path);
        }
        pimpl->active_project->set_starting_realm(loaded_config.project_data.starting_realm);


        // --- KICK OFF THE CHAIN OF COMMAND ---
        if (pimpl->active_project) {
            // The Project will now initialize itself, which includes creating the RealmManager
            // and telling it to load the starting realm.
            pimpl->active_project->initialize(pimpl->context); // Pass the asset manager
        }
        std::cout << "ProjectManager: Successfully loaded project '" << loaded_config.project_data.project_name << "'." << std::endl;
        return true;
    }

    Project* ProjectManager::load_project_from_file(const std::string& project_file_path) {
        if (!FileManager::path_exists(project_file_path)) {
            std::cerr << "ProjectManager Error: Failed to load '" << project_file_path << "'. Path does not exist." << std::endl;
            return nullptr;
        }

        if (pimpl->active_project) {
            pimpl->active_project->shutdown(); // Shutdown current project if any
        }

        std::cout << "ProjectManager: Loading project from '" << project_file_path << "'..." << std::endl;

        Salix::ProjectConfig loaded_config; // Object to hold deserialized project data

        // --- Cereal Deserialization of ProjectConfig ---
        std::ifstream project_file_is(project_file_path);
        if (!project_file_is.is_open()) {
            std::cerr << "ProjectManager Error: Could not open project file '" << project_file_path << "' for reading." << std::endl;
            return nullptr;
        }

        try {
            cereal::JSONInputArchive archive(project_file_is);
            archive(loaded_config); // Load the entire project config
        } catch (const cereal::Exception& e) {
            std::cerr << "ProjectManager Error: Failed to deserialize project config from '" << project_file_path << "': " << e.what() << std::endl;
            project_file_is.close();
            return nullptr;
        }
        project_file_is.close(); // Close the file stream after deserialization

        // --- Use the loaded_config to create and initialize the Project ---
        // The Project constructor that takes name and root_path is ideal here.
        pimpl->active_project = std::make_unique<Project>(
            loaded_config.project_data.project_name,
            loaded_config.project_data.project_path
        );

        // Now, populate the Project with loaded realm paths and starting realm info
        // Your Project::add_realm_path method signature is `const std::string& path_to_realm_file`.
        // If you later change it to accept name, you'd modify this line accordingly.
        for (const auto& realm_info : loaded_config.project_data.realms) {
            pimpl->active_project->add_realm_path(realm_info.name, realm_info.path);
        }
        pimpl->active_project->set_starting_realm(loaded_config.project_data.starting_realm);


        // --- KICK OFF THE CHAIN OF COMMAND ---
        if (pimpl->active_project) {
            // The Project will now initialize itself, which includes creating the RealmManager
            // and telling it to load the starting realm.
            std::cout << "ProjectManager::load_project_from_file - active_project set to '" <<
                pimpl->active_project->get_name() << "'..." << std::endl;
            pimpl->active_project->initialize(pimpl->context); // Pass the asset manager
        }
        std::cout << "ProjectManager: Successfully loaded project '" << loaded_config.project_data.project_name << "'." << std::endl;
        return get_active_project();
    }


    Project* ProjectManager::get_active_project(){
        if (pimpl->active_project) {
            return pimpl->active_project.get();
        }
        return nullptr;
    }

    void ProjectManager::set_active_project(const std::string& /*project_name*/) {
        // This method still needs to be fully implemented if you use it.
        // It would likely involve loading a project similarly to load_project.
        pimpl->active_project = std::make_unique<Project>(); // placeholder for now.
    }
} // namespace Salix