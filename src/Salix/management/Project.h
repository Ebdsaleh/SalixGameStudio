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
    class RealmManager;
    class IRenderer;
    class AssetManager;
    class FileManager;
    struct InitContext;

    class SALIX_API Project {
    public:
        Project();
        Project(const std::string& project_name, const std::string& project_root_path);
        ~Project();

        void initialize(const InitContext& new_context);
        void shutdown();
        void update(float delta_time);
        void render(IRenderer* renderer);
        
        
        // These methods allow the ProjectManager to populate this object with
        // data that would normally be read from a project file.
        void set_starting_realm(const std::string& realm_name);
        void add_realm_path(const std::string& realm_name,  const std::string& path_to_realm_file);
        bool remove_realm_path(const std::string& path_to_realm_file);

        // --- Public Getters ---
        RealmManager* get_realm_manager() const;
        const std::string& get_name() const;
        const std::string& get_path() const;
        const std::string& get_starting_realm() const;
        const std::map<std::string, std::string>& get_realm_paths() const;
        // NEW: Getters for Build Settings & Project File Name
        const std::string& get_engine_version() const;
        const std::string& get_game_dll_name() const;
        const std::string& get_project_file_name() const;
        // Returns true on success, false on failure
        bool load_starting_realm();
        // Creates the default realm content and saves it to a file.
        void create_and_save_default_realm();
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        friend class cereal::access;
        template<class Archive>
        void serialize(Archive& archive);
        
    };
} // namespace Salix