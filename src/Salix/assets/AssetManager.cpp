// Salix/assets/AssetManager.cpp

#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/ITexture.h>

// No need for SDL.h here, the AssetManager shouldn't know about specific implementations.

namespace Salix {
    AssetManager::AssetManager() : renderer(nullptr) {}

    AssetManager::~AssetManager() {
        shutdown();
    }

    void AssetManager::initialize(IRenderer* renderer_ptr) {
        renderer = renderer_ptr;
    }

    void AssetManager::shutdown() {
        // --- FIX #1 ---
        // The magic of unique_ptr! When the map is cleared, each unique_ptr
        // is destroyed, and it automatically calls 'delete' on the texture it owns.
        // The for loop is no longer needed. This one line does everything.
        texture_cache.clear();
    }

    ITexture* AssetManager::get_texture(const std::string& file_path) {
        // Check if the texture is already in our cache.
        auto cache_iterator = texture_cache.find(file_path);
        if (cache_iterator != texture_cache.end()) {
            // Found it!
            // --- FIX #2 ---
            // 'cache_iterator->second' is the unique_ptr.
            // .get() returns the raw ITexture* pointer without giving up ownership.
            return cache_iterator->second.get();
        }

        // Not in the texture cache. Ask the renderer to load it from disk.
        // The renderer gives us a raw pointer with the texture data.
        ITexture* new_texture = renderer->load_texture(file_path.c_str());

        if (new_texture) {
            // --- FIX #3 ---
            // We can't assign a raw pointer to a unique_ptr directly.
            // We must explicitly create a unique_ptr that takes ownership of the
            // raw pointer, and then MOVE that unique_ptr into the map.
            texture_cache[file_path] = std::unique_ptr<ITexture>(new_texture);
            
            // Return the raw pointer from the newly added texture.
            return new_texture;
        }

        return nullptr;
    }
} // namespace Salix