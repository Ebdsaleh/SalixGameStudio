// Salix/assets/AssetManager.cpp

#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/ITexture.h>
#include <filesystem>
#include <iostream>

namespace Salix {
    extern std::filesystem::path g_project_root_path;

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
        std::string relative_path = file_path;
        // 1. Combine the project root with the relative path to get the true absolute path.
        std::filesystem::path absolute_path = Salix::g_project_root_path / relative_path;
        std::string absolute_path_str = absolute_path.lexically_normal().string();

        // 2. Use the FULL, ABSOLUTE path as the key for your cache. This is more robust.
        auto cache_iterator = pimpl->texture_cache.find(absolute_path_str);
        if (cache_iterator != pimpl->texture_cache.end()) {
            return cache_iterator->second.get();
        }

        // 3. Ask the renderer to load from the ABSOLUTE path.
        ITexture* new_texture = pimpl->renderer->load_texture(absolute_path_str.c_str());

        if (new_texture) {
            // 4. Cache the new texture using its absolute path.
            pimpl->texture_cache[absolute_path_str] = std::unique_ptr<ITexture>(new_texture);
            return new_texture;
        }
        return nullptr;
    }
} // namespace Salix