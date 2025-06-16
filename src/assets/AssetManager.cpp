// AssetManager.cpp

#include "AssetManager.h"
#include "../rendering/IRenderer.h"
#include "../rendering/ITexture.h"
#include <SDL.h>  // For SDL_Texture and SDL_DestroyTexture

AssetManager::AssetManager() : renderer(nullptr) {}
AssetManager::~AssetManager() {
    // Shutdown should be called, but this is a safeguard.
    shutdown();
}

void AssetManager::initialize(IRenderer* renderer_ptr) {
    renderer = renderer_ptr;
}

void AssetManager::shutdown() {
    // Clear the cache and free all the loaded textures.
    for (auto const& [key, val] : texture_cache) {
        delete val;
    }
    texture_cache.clear();
}

ITexture* AssetManager::get_texture(const std::string& file_path) {
    // Check if the texture is already in our cache.
    auto it = texture_cache.find(file_path);
    if (it != texture_cache.end()) {
        // Found it! Return the existing texture.
        return it->second;
    }

    // Not in the texture cache. Ask the renderer to load if from disk.
    ITexture*  texture = renderer->load_texture(file_path.c_str());

    if (texture) {
        // If loading was successful, store it in our cache for the next time.
        texture_cache[file_path] = texture;
    }
    return texture;
}