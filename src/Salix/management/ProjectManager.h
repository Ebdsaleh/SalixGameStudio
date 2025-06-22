// ProjectManager.h
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <memory>
#include <Salix/management/FileManager.h>

namespace Salix {

    // Forward declarations
    class Project;
    class IRenderer;
    class AssetManager;

    // Added SALIX_API to the class declaration.
    class SALIX_API ProjectManager {
        public:
            ProjectManager();
            ~ProjectManager();
            
            void initialize(AssetManager* asset_manager);
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
            Project* get_active_project();
            void set_active_project(const std::string& project_name);
            
            

        private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix