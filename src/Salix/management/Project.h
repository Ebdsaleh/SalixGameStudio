// Salix/management/Project.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cereal/access.hpp>

namespace Salix {

    // Forward declarations
    class SceneManager;
    class IRenderer;
    class AssetManager;
    class FileManager;

    class SALIX_API Project {
    public:
        Project();
        Project(const std::string& project_name, const std::string& project_root_path);
        ~Project();

        void initialize(AssetManager* asset_manager);
        void shutdown();
        void update(float delta_time);
        void render(IRenderer* renderer);
        
        
        // These methods allow the ProjectManager to populate this object with
        // data that would normally be read from a project file.
        void set_starting_scene(const std::string& scene_name);
        void add_scene_path(const std::string& scene_name,  const std::string& path_to_scene_file);
        bool remove_scene_path(const std::string& path_to_scene_file);

        // --- Public Getters ---
        SceneManager* get_scene_manager() const;
        const std::string& get_name() const;
        const std::string& get_path() const;
        const std::string& get_starting_scene() const;
        const std::map<std::string, std::string>& get_scene_paths() const;
        // NEW: Getters for Build Settings & Project File Name
        const std::string& get_engine_version() const;
        const std::string& get_game_dll_name() const;
        const std::string& get_project_file_name() const;


    private:
        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        friend class cereal::access;
        template<class Archive>
        void serialize(Archive& archive);
        
    };
} // namespace Salix