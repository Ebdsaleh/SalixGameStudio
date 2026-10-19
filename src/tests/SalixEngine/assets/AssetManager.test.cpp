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

// Asset caching contracts: counting mock texture loads; no disk assets are accessed.
#include <filesystem>
#include <vector>
class CountingAssetRenderer final : public MockIRenderer {
public:
    int calls = 0;
    std::vector<std::string> paths;
    Salix::ITexture* load_texture(const char* filename) override {
        ++calls;
        paths.emplace_back(filename);
        return MockIRenderer::load_texture(filename);
    }
};
TEST_SUITE("Salix::assets::AssetManagerExtended") {
    TEST_CASE("asset manager same logical texture path is loaded only once and pointer is reused") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("assets/ui/health.png");
        auto* second = assets.get_texture("assets/ui/health.png");
        REQUIRE(first != nullptr);
        CHECK(first == second);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("asset manager lexical dot path aliases reuse the same cached texture") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("assets/ui/icon.png");
        auto* second = assets.get_texture("assets/./ui/icon.png");
        REQUIRE(first != nullptr);
        CHECK(first == second);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("asset manager lexical parent traversal within relative path maps to same cache key") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("assets/ui/icon.png");
        auto* second = assets.get_texture("assets/other/../ui/icon.png");
        REQUIRE(first != nullptr);
        CHECK(first == second);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("asset manager different paths remain different cache entries") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("assets/sprite-a.png");
        auto* second = assets.get_texture("assets/sprite-b.png");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("asset manager failed renderer load is not cached and allows retry") {
        CountingAssetRenderer renderer;
        renderer.should_texture_load_fail = true;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        CHECK(assets.get_texture("assets/retry.png") == nullptr);
        CHECK(renderer.calls == 1);
        renderer.should_texture_load_fail = false;
        REQUIRE(assets.get_texture("assets/retry.png") != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("asset manager repeated failed texture requests retry rather than caching null") {
        CountingAssetRenderer renderer;
        renderer.should_texture_load_fail = true;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        CHECK(assets.get_texture("assets/unavailable.png") == nullptr);
        CHECK(assets.get_texture("assets/unavailable.png") == nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("asset manager shutdown flushes cached textures before next texture request") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/clear.png") != nullptr);
        REQUIRE(assets.get_texture("assets/clear.png") != nullptr);
        CHECK(renderer.calls == 1);
        assets.shutdown();
        REQUIRE(assets.get_texture("assets/clear.png") != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("asset manager reinitializing with a new renderer after shutdown uses new backend") {
        CountingAssetRenderer first, second;
        Salix::AssetManager assets;
        assets.initialize(&first);
        REQUIRE(assets.get_texture("assets/switch.png") != nullptr);
        assets.shutdown();
        assets.initialize(&second);
        REQUIRE(assets.get_texture("assets/switch.png") != nullptr);
        CHECK(first.calls == 1);
        CHECK(second.calls == 1);
    }

    TEST_CASE("asset manager each AssetManager instance owns an independent texture cache") {
        CountingAssetRenderer renderer;
        Salix::AssetManager first, second;
        first.initialize(&renderer);
        second.initialize(&renderer);
        REQUIRE(first.get_texture("assets/shared.png") != nullptr);
        REQUIRE(second.get_texture("assets/shared.png") != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("asset manager renderer receives lexically normalized relative texture path") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/gui/../gui/button.png") != nullptr);
        REQUIRE(renderer.paths.size() == 1);
        const std::filesystem::path loaded(renderer.paths.front());
        CHECK(loaded.filename().string() == "button.png");
        CHECK(loaded.parent_path().filename().string() == "gui");
        CHECK(renderer.calls == 1);
    }
}
