// ProjectManager.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/management/FileManager.h>
#include <string>
#include <memory>

namespace Salix {

    // Forward declarations
    class Project;
    class IRenderer;
    class AssetManager;
    struct InitContext;

    // Added SALIX_API to the class declaration.
    class SALIX_API ProjectManager {
        public:
            ProjectManager();
            ~ProjectManager();
            
            void initialize(const InitContext& new_context);  // needed for AssetManager
            void shutdown();
            void update(float delta_time);
            void render(IRenderer* renderer);
            // Creates the physical directory structure for a new project on disk.
            // Returns true on success.
            bool create_new_project(const std::string& project_path, const std::string& project_name);

            // --- DEVELOPMENT-ONLY METHODS ---
            // Create a new project on the filesystem at a static location inside our 'src/Sandbox/' directory.
            bool create_new_internal_project(const std::string& project_name);

            /* Create a new project on the filesystem in a dynamic location (will be used as a'sandbox'for testing.
                Linking systems together OUTSIDE of our development ecosystem. 'External end-user-developer experience'). */
            bool create_new_external_project(const std::string& project_path, const std::string& project_name);
            // --- END OF DEVELOPMENT-ONLY METHODS. ---

            // Loads a project from a given path. This will become the primary
            // way for the engine to open a game.
            bool load_project(const std::string& project_path);
            Project* load_project_from_file(const std::string& project_path);
            Project* get_active_project();
            void set_active_project(const std::string& project_name);
            const std::string get_project_to_load() const;
            void set_project_to_load(const std::string full_project_file_path);
            const std::string get_loaded_project_directory() const;
            void set_loaded_project_directory(const std::string loaded_project_directory); // Pass in just the directory path, not the filename.
            

        private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix