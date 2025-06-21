// ProjectManager.h
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <memory>

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

            void load_project(const std::string& project_path);
            Project* get_active_project();
            void set_active_project(const std::string& project_name);
            

        private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix