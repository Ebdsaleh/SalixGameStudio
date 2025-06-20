// Salix/assets/AssetManager.h
#pragma once

#include <string>
#include <memory>
#include <map>

namespace Salix {

    // Forward declarations
    class IRenderer;
    struct ITexture;

    class AssetManager {
        public:
            AssetManager();
            ~AssetManager();

            // The asset manager needs a renderer to create textures with.
            void initialize(IRenderer* renderer_ptr);
            void shutdown();

            // The main function to load a texture, this will call the IRenderer load_texture method.
            ITexture* get_texture(const std::string& file_path);

        private:
            // A pointer to the renderer, so we can create the textures.
            // This is a non-owning pointer.
            IRenderer* renderer;

            // The cache: maps a file path (string) to a loaded texture.
            std::map<std::string, std::unique_ptr<ITexture>> texture_cache;
    };
} // namespace Salix