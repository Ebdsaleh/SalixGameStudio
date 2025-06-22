// Salix/management/Project.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
#include <memory>

namespace Salix {

    // Forward declarations
    class SceneManager;
    class IRenderer;
    class AssetManager;
    class FileManager;

    class SALIX_API Project {
    public:
        Project();
        ~Project();

        void initialize(AssetManager* asset_manager);
        void shutdown();
        void update(float delta_time);
        void render(IRenderer* renderer);
        
        
        // These methods allow the ProjectManager to populate this object with
        // data that would normally be read from a project file.
        void set_starting_scene(const std::string& scene_name);
        void add_scene_path(const std::string& path_to_scene_file);
        bool remove_scene_path(const std::string& path_to_scene_file);

        // --- Public Getters ---
        SceneManager* get_scene_manager() const;
        const std::string& get_name() const;
        const std::string& get_path() const;
        const std::string& get_starting_scene() const;
        const std::vector<std::string>& get_scene_paths() const;


    private:
        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        
    };
} // namespace Salix