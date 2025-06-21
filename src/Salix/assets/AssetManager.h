// Salix/assets/AssetManager.h
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <memory>
#include <map>

namespace Salix {

    // Forward declarations
    class IRenderer;
    class ITexture;

    class SALIX_API AssetManager {
        public:
            AssetManager();
            ~AssetManager();

            // The asset manager needs a renderer to create textures with.
            void initialize(IRenderer* renderer_ptr);
            void shutdown();

            // The main function to load a texture, this will call the IRenderer load_texture method.
            ITexture* get_texture(const std::string& file_path);

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
            
    };
} // namespace Salix