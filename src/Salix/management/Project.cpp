// Salix/management/Project.cpp
#include <Salix/core/InitContext.h>
#include <Salix/management/Project.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/scripting/ScriptFactory.h> // Include the script factory.
#include <Salix/management/SceneManager.h>
#include <Salix/management/FileManager.h>
#include <Salix/management/ProjectConfig.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Scene.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/core/SerializationRegistrations.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <windows.h> // Required for LoadLibraryA
#include <Salix/scripting/ScriptLoader.h>

namespace Salix {
    struct Project::Pimpl {
        std::unique_ptr<SceneManager> scene_manager; // runtime, does not get serialized.
        std::string name;
        std::string root_path;
        std::string project_file_name;
        std::string starting_scene;
        std::map<std::string, std::string> scene_paths;
        InitContext context;

        // --- Build Settings (to match BuildSettings in ProjectConfig) ---
        std::string engine_version;
        std::string game_dll_name;

        Pimpl() = default;
        // define the Pimpl constructor
        // Pimpl's serialization function (already here or in .h, make sure it's correct)
        template<class Archive>
        void serialize(Archive & archive) {
            // Serialize all persistent data members of Pimpl
            archive(cereal::make_nvp("name", name));
            archive(cereal::make_nvp("root_path", root_path));
            archive(cereal::make_nvp("project_file_name", project_file_name)); 
            archive(cereal::make_nvp("scene_paths", scene_paths));
            archive(cereal::make_nvp("starting_scene", starting_scene));
            archive(cereal::make_nvp("engine_version", engine_version));
            archive(cereal::make_nvp("game_dll_name", game_dll_name));
            // Don't serialize AssetManager* (raw pointer to external object)
            // Don't serialize scene_manager directly here in Pimpl,
            // as it's typically managed/created at runtime (as per the flow).
            // If you did want to serialize scene_manager itself, it would also need
            // its full type visible.
        }
            
    };

    // Constructor and Destructor
    Project::Project() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = "TestProject";
        pimpl->root_path = "Sandbox/TestProject";
        pimpl->project_file_name = pimpl->name + ".salixproj"; // <--- ADD THIS (derive from name)
        pimpl->engine_version = "0.1.0-Alpha"; // <--- ADD THIS (default, will be overwritten by loaded_config in PM)
        pimpl->game_dll_name = pimpl->name + ".dll"; // <--- ADD THIS (derive, will be overwritten)
    }
    
    Project::Project(const std::string& project_name, const std::string& project_root_path) 
        : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = project_name;
        pimpl->root_path = project_root_path;
        pimpl->project_file_name = project_name + ".salixproj"; // <--- ADD THIS (derive from name)
        pimpl->engine_version = "0.1.0-Alpha"; // <--- ADD THIS (default, will be overwritten by loaded_config in PM)
        pimpl->game_dll_name = project_name + ".dll"; // <--- ADD THIS (derive, will be overwritten)
    }

    Project::~Project() = default;

    void Project::initialize(const InitContext& new_context) {
        std::cout << "Project Initializing..." << std::endl;
        pimpl->context = new_context;
        
        if(pimpl->context.asset_manager == nullptr){
            std::cerr << "Project::initialize - AssetManager pointer is null_ptr, cancelling Project initialization." <<
                std::endl;
        }
        pimpl->scene_manager = std::make_unique<SceneManager>();
        pimpl->scene_manager->initialize(pimpl->context);
        pimpl->scene_manager->set_project_root_path(pimpl->root_path);

        // Populate SceneManager with lightweight scene shells
        for (const auto& pair : pimpl->scene_paths) {
            pimpl->scene_manager->create_scene(pair.first, pair.second);
        }
        
        // Now that the project is set up, load its specific game logic DLL.
        std::cout << "Project: Loading game script DLL '" << pimpl->game_dll_name << "'..." << std::endl;

        std::filesystem::path dll_path = std::filesystem::current_path() / "build" / pimpl->game_dll_name;

        if (!ScriptLoader::load_script_library(dll_path.string())) {
            std::cerr << "FATAL ERROR: Could not load game DLL at '" << dll_path.string() << "'!" << std::endl;
            return;
        } else {
            std::cout << "Project: Game script DLL loaded successfully." << std::endl;
        }
    }

    void Project::shutdown() {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->shutdown();
            pimpl->scene_manager.reset();
        }
    }

    void Project::update(float delta_time) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->update(delta_time);
        }
    }

    void Project::render(IRenderer* renderer) {
        if (pimpl->scene_manager) {
            pimpl->scene_manager->render(renderer);
        }
    }

    void Project::set_starting_scene(const std::string& scene_name) {
        pimpl->starting_scene = scene_name;
    }
    
    void Project::add_scene_path(const std::string& scene_name, const std::string& path_to_scene_file) {
        pimpl->scene_paths[scene_name] = path_to_scene_file; // Store in the map
    }

    bool Project::remove_scene_path(const std::string& path_to_scene_file) {
        if(!FileManager::path_exists(path_to_scene_file)) { // This check is for the file itself, not the map entry
            std::cerr << "Project::remove_scene_path - Failed to remove scene path: '" <<
            path_to_scene_file << "', file does not exist on disk." << std::endl;
            // Optionally, still attempt to remove from map even if file doesn't exist on disk,
            // if this method means "remove from project list".
        }

        // Iterate the map to find the entry by value (path) and remove by key (name)
        for (auto it = pimpl->scene_paths.begin(); it != pimpl->scene_paths.end(); ++it) {
            if (it->second == path_to_scene_file) {
                pimpl->scene_paths.erase(it); // Erase by iterator
                std::cout << "Project: Removed scene path '" << path_to_scene_file << "' from project." << std::endl;
                return true; // Scene path found and removed from map
            }
        }
        std::cerr << "Project::remove_scene_path - Scene path '" << path_to_scene_file << "' not found in project map." << std::endl;
        return false; // Scene path not found in the map
    }
    
    // --- Public Getter Methods ---
    SceneManager* Project::get_scene_manager() const {
        return pimpl->scene_manager.get();
    }

    const std::string& Project::get_name() const {

        return pimpl->name;
    }
    const std::string& Project::get_path() const {
        return pimpl->root_path;
    }
    const std::string& Project::get_starting_scene() const {
        return pimpl->starting_scene;
    }
    const std::map<std::string, std::string>& Project::get_scene_paths() const {
        return pimpl->scene_paths;
    }

    const std::string& Project::get_engine_version() const {
        return pimpl->engine_version;
    }
    
    const std::string& Project::get_game_dll_name() const {
        return pimpl->game_dll_name;
    }
    
    const std::string& Project::get_project_file_name() const {
        return pimpl->project_file_name;
    }

    bool Project::load_starting_scene() {
        if (pimpl->starting_scene.empty()) {
        std::cerr << "Project Error: No starting scene specified!" << std::endl;
        return false;
    }
        // This now attempts to load the scene from disk and returns the result
        return pimpl->scene_manager->set_active_scene(pimpl->starting_scene);
    }

    void Project::create_and_save_default_scene() {
        if (!pimpl->scene_manager) {
        return;
    }

    std::cout << "Project: Creating default content for scene 'Default'..." << std::endl;

    // 1. Ensure the "Default" scene is the active one.
    //    This is necessary so that save_active_scene() knows what to save.
    pimpl->scene_manager->set_active_scene("Default");
    Scene* default_scene = pimpl->scene_manager->get_active_scene();

    if (!default_scene) {
        std::cerr << "Project Error: Could not find 'Default' scene shell to populate." << std::endl;
        return;
    }

    // 2. Create the default "Player" entity and its components.
    Entity* player = default_scene->create_entity("Player");
    Transform* transform = player->get_transform();
    Sprite2D* sprite = player->add_element<Sprite2D>();

    // 3. Set the default properties.
    transform->set_position({ 640.0f, 360.0f, 0.0f });
    transform->set_rotation({ 0.0f, 0.0f, 0.0f });
    transform->set_scale({ 1.0f, 1.0f, 1.0f });

    sprite->texture_path = "Assets/Images/test.png";

    sprite->load_texture(pimpl->context.asset_manager, sprite->get_texture_path());
    // --- THIS IS THE CHANGE ---
    // Instead of player->add_element<PlayerMovement>(), we use the factory.
    std::unique_ptr<ScriptElement> script_element = Salix::ScriptFactory::get().create_script("PlayerMovement");

    if (script_element) {
        std::cout << "SCRIPTELEMENT EXISTS: - Type name: " << typeid(*script_element).name() << std::endl;
        // Use the new public method. The unique_ptr will be correctly moved and converted.
        player->add_element(std::move(script_element));
        
    }
    // -------------------------

    // 4. Tell the SceneManager to save the now-populated active scene to its file.
    pimpl->scene_manager->save_active_scene();

    }
    template<class Archive>
    void Project::serialize(Archive & archive) {
        archive(
        cereal::make_nvp("name", pimpl->name),
        cereal::make_nvp("root_path", pimpl->root_path),
        cereal::make_nvp("project_file_name", pimpl->project_file_name),
        cereal::make_nvp("scene_paths", pimpl->scene_paths),
        cereal::make_nvp("starting_scene", pimpl->starting_scene),
        cereal::make_nvp("engine_version", pimpl->engine_version),
        cereal::make_nvp("game_dll_name", pimpl->game_dll_name)
    );
    }

    template void Project::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& );
    template void Project::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Project::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Project::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);

} // namespace Salix