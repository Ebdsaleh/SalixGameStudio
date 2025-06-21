// Salix/assets/AssetManager.cpp

#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/ITexture.h>

namespace Salix {
    // Define the implementation struct here, inside the .cpp file.
    struct AssetManager::Pimpl {
        IRenderer* renderer;
        std::map<std::string, std::unique_ptr<ITexture>> texture_cache;
    };

    AssetManager::AssetManager() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->renderer = nullptr;
    }

    AssetManager::~AssetManager() = default;

    void AssetManager::initialize(IRenderer* renderer_ptr) {
        pimpl->renderer = renderer_ptr;
    }

    void AssetManager::shutdown() {
        pimpl->texture_cache.clear();
    }

    ITexture* AssetManager::get_texture(const std::string& file_path) {
        // Check if the texture is already in our cache.
        auto cache_iterator = pimpl->texture_cache.find(file_path);
        if (cache_iterator != pimpl->texture_cache.end()) {
           
            return cache_iterator->second.get();
        }

        // Not in the texture cache. Ask the renderer to load it from disk.
        // The renderer gives us a raw pointer with the texture data.
        ITexture* new_texture = pimpl->renderer->load_texture(file_path.c_str());

        if (new_texture) {
            pimpl->texture_cache[file_path] = std::unique_ptr<ITexture>(new_texture);
            
            // Return the raw pointer from the newly added texture.
            return new_texture;
        }

        return nullptr;
    }
} // namespace Salix