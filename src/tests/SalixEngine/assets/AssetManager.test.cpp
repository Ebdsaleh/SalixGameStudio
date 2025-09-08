// =================================================================================
// Filename:    src/Tests/SalixEngine/assets/AssetManager.test.cpp
// Description: Contains unit tests for the AssetManager functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/assets/AssetManager.h>
#include <doctest.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/window/WindowConfig.h>
// mocking interfaces.
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
// std library.
#include <string>
#include <memory>
#include <map>



TEST_SUITE("Salix::assets::AssetManager") {
    // Test case for the basic initialization and shutdown process.
    TEST_CASE("initialization and shutdown") {
        Salix::AssetManager asset_manager;
        MockIRenderer mock_renderer_instance;
        const Salix::WindowConfig config;
        mock_renderer_instance.initialize(config);
        MockIRenderer* mock_renderer = &mock_renderer_instance;

        // The AssetManager should not have a renderer pointer before initialization.
        // We can't directly check the private pimpl, but we can verify behavior.
        // We'll trust the load test to confirm the renderer is set.

        asset_manager.initialize(mock_renderer);
        // At this point, the renderer pointer should be set internally.
        
        // When we shut down, the cache should be cleared.
        asset_manager.shutdown();
    }

    // Test case for the primary get_texture method.
    TEST_CASE("get texture from a file path") {
        Salix::AssetManager asset_manager;
        MockIRenderer mock_renderer_instance;
        MockIRenderer* mock_renderer = &mock_renderer_instance;
        asset_manager.initialize(mock_renderer);

        SUBCASE("load a texture for the first time") {
            const std::string file_path = "assets/textures/test.png";

            // When we get the texture for the first time, it should return a valid pointer.
            Salix::ITexture* texture1 = asset_manager.get_texture(file_path);
            CHECK(texture1 != nullptr);
        }

        SUBCASE("load the same texture again from cache") {
            const std::string file_path = "assets/textures/test.png";

            // Load the texture once.
            Salix::ITexture* texture1 = asset_manager.get_texture(file_path);

            // Load the same texture again.
            Salix::ITexture* texture2 = asset_manager.get_texture(file_path);
            
            // The pointers should be identical, indicating it was loaded from the cache.
            CHECK(texture1 == texture2);
        }

        SUBCASE("load a different texture") {
            const std::string file_path1 = "assets/textures/test1.png";
            const std::string file_path2 = "assets/textures/test2.png";

            // Load two different textures.
            Salix::ITexture* texture1 = asset_manager.get_texture(file_path1);
            Salix::ITexture* texture2 = asset_manager.get_texture(file_path2);
            
            // The pointers should be different, as they are distinct assets.
            CHECK(texture1 != texture2);
        }
    }
}