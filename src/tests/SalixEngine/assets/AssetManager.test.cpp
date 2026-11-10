// =================================================================================
// Filename:    src/Tests/SalixEngine/assets/AssetManager.test.cpp
// Description: Contains unit tests for the AssetManager functions.
// ================================================================================= 
#include <doctest.h>
#include <Salix/core/Core.h>
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

// Regression coverage for additional system invariants; original tests retained.
TEST_SUITE("Salix::assets::AssetManagerBatch647") {
    TEST_CASE("loading several textures caches each unique path once") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        for(const auto* p:{"assets/one.png","assets/two.png","assets/three.png"}) REQUIRE(manager.get_texture(p)!=nullptr);
        CHECK(renderer.calls==3);
        for(const auto* p:{"assets/three.png","assets/two.png","assets/one.png"}) REQUIRE(manager.get_texture(p)!=nullptr);
        CHECK(renderer.calls==3);
    }

    TEST_CASE("a dot segment and normalized filename share a texture cache entry") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* a=manager.get_texture("assets/sprites/./character.png");
        auto* b=manager.get_texture("assets/sprites/character.png");
        REQUIRE(a!=nullptr);
        CHECK(a==b);
        CHECK(renderer.calls==1);
    }

    TEST_CASE("repeated parent navigation normalizes to the same asset") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* a=manager.get_texture("assets/ui/button.png");
        auto* b=manager.get_texture("assets/images/../misc/../ui/button.png");
        REQUIRE(a!=nullptr);
        CHECK(a==b);
        CHECK(renderer.calls==1);
    }

    TEST_CASE("a failed request for one asset does not poison another cached asset") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        renderer.should_texture_load_fail=true;
        CHECK(manager.get_texture("assets/failed.png")==nullptr);
        renderer.should_texture_load_fail=false;
        auto* valid=manager.get_texture("assets/good.png");
        REQUIRE(valid!=nullptr);
        CHECK(manager.get_texture("assets/good.png")==valid);
        CHECK(renderer.calls==2);
    }

    TEST_CASE("recovery after load failure caches only the successful retry") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        renderer.should_texture_load_fail=true;
        CHECK(manager.get_texture("assets/recover2.png")==nullptr);
        renderer.should_texture_load_fail=false;
        auto* result=manager.get_texture("assets/recover2.png");
        REQUIRE(result!=nullptr);
        CHECK(manager.get_texture("assets/recover2.png")==result);
        CHECK(renderer.calls==2);
    }

    TEST_CASE("shutdown permits fresh loading of previously cached assets") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        REQUIRE(manager.get_texture("assets/reload.png")!=nullptr);
        CHECK(renderer.calls==1);
        manager.shutdown();
        REQUIRE(manager.get_texture("assets/reload.png")!=nullptr);
        CHECK(renderer.calls==2);
    }

    TEST_CASE("clearing one manager cannot evict another manager cache") {
        CountingAssetRenderer renderer;
        Salix::AssetManager first,second; first.initialize(&renderer); second.initialize(&renderer);
        REQUIRE(first.get_texture("assets/shared2.png")!=nullptr);
        auto* retained=second.get_texture("assets/shared2.png"); REQUIRE(retained!=nullptr);
        first.shutdown();
        CHECK(second.get_texture("assets/shared2.png")==retained);
        CHECK(renderer.calls==2);
    }

    TEST_CASE("cache identity does not change when unrelated textures are loaded") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* first=manager.get_texture("assets/first.png"); REQUIRE(first!=nullptr);
        REQUIRE(manager.get_texture("assets/second.png")!=nullptr);
        REQUIRE(manager.get_texture("assets/third.png")!=nullptr);
        CHECK(manager.get_texture("assets/first.png")==first);
        CHECK(renderer.calls==3);
    }

    TEST_CASE("a single cached path is still cached when renderer later begins failing") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* first=manager.get_texture("assets/persistent.png"); REQUIRE(first!=nullptr);
        renderer.should_texture_load_fail=true;
        CHECK(manager.get_texture("assets/persistent.png")==first);
        CHECK(renderer.calls==1);
    }

    TEST_CASE("renderer records normalized asset paths without dot-dot segments") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        REQUIRE(manager.get_texture("assets/a/../b/./icon.png")!=nullptr);
        REQUIRE(renderer.paths.size()==1);
        auto normalized=std::filesystem::path(renderer.paths.front()).lexically_normal();
        CHECK(normalized==std::filesystem::path(renderer.paths.front()));
        CHECK(normalized.filename().string()=="icon.png");
        CHECK(normalized.parent_path().filename().string()=="b");
    }
}

TEST_SUITE("Salix::assets::AssetManagerBatch2247") {
    TEST_CASE("batch2247 lexical normalization collapses multiple parent and dot segments into one cache key") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* first=manager.get_texture("assets/ui/icons/button.png");
        auto* second=manager.get_texture("assets/ui/./temp/../icons/button.png");
        REQUIRE(first!=nullptr); CHECK(first==second); CHECK(renderer.calls==1);
    }
    TEST_CASE("batch2247 successful cached texture remains available after renderer begins failing") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* first=manager.get_texture("assets/stable.png"); REQUIRE(first!=nullptr);
        renderer.should_texture_load_fail=true;
        CHECK(manager.get_texture("assets/stable.png")==first); CHECK(renderer.calls==1);
    }
    TEST_CASE("batch2247 failed texture request is retried until a renderer succeeds") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        renderer.should_texture_load_fail=true;
        CHECK(manager.get_texture("assets/retry-2247.png")==nullptr);
        CHECK(manager.get_texture("assets/retry-2247.png")==nullptr);
        CHECK(renderer.calls==2);
        renderer.should_texture_load_fail=false;
        REQUIRE(manager.get_texture("assets/retry-2247.png")!=nullptr);
        CHECK(renderer.calls==3);
    }
    TEST_CASE("batch2247 shutdown clears every cached entry rather than only the most recent") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        REQUIRE(manager.get_texture("assets/a2247.png")!=nullptr);
        REQUIRE(manager.get_texture("assets/b2247.png")!=nullptr);
        CHECK(renderer.calls==2);
        manager.shutdown();
        REQUIRE(manager.get_texture("assets/a2247.png")!=nullptr);
        REQUIRE(manager.get_texture("assets/b2247.png")!=nullptr);
        CHECK(renderer.calls==4);
    }
    TEST_CASE("batch2247 repeated shutdown on an empty cache is harmless") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        CHECK_NOTHROW(manager.shutdown()); CHECK_NOTHROW(manager.shutdown());
        REQUIRE(manager.get_texture("assets/after-empty-shutdown.png")!=nullptr);
        CHECK(renderer.calls==1);
    }
    TEST_CASE("batch2247 reinitializing renderer keeps an already cached texture without a new backend call") {
        CountingAssetRenderer first_renderer,second_renderer; Salix::AssetManager manager; manager.initialize(&first_renderer);
        auto* cached=manager.get_texture("assets/reinit-cached.png"); REQUIRE(cached!=nullptr);
        manager.initialize(&second_renderer);
        CHECK(manager.get_texture("assets/reinit-cached.png")==cached);
        CHECK(first_renderer.calls==1); CHECK(second_renderer.calls==0);
    }
    TEST_CASE("batch2247 reinitializing renderer directs future uncached loads to the new backend") {
        CountingAssetRenderer first_renderer,second_renderer; Salix::AssetManager manager; manager.initialize(&first_renderer);
        REQUIRE(manager.get_texture("assets/backend-one.png")!=nullptr);
        manager.initialize(&second_renderer);
        REQUIRE(manager.get_texture("assets/backend-two.png")!=nullptr);
        CHECK(first_renderer.calls==1); CHECK(second_renderer.calls==1);
    }
    TEST_CASE("batch2247 separate managers never share cache entries even with one renderer") {
        CountingAssetRenderer renderer; Salix::AssetManager first,second; first.initialize(&renderer); second.initialize(&renderer);
        auto* a=first.get_texture("assets/isolated-2247.png"); auto* b=second.get_texture("assets/isolated-2247.png");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); CHECK(a!=b); CHECK(renderer.calls==2);
    }
    TEST_CASE("batch2247 normalized renderer path contains no dot path component") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        REQUIRE(manager.get_texture("./assets/./icons/../icon2247.png")!=nullptr);
        REQUIRE(renderer.paths.size()==1);
        const auto p=std::filesystem::path(renderer.paths.front()).lexically_normal();
        CHECK(p==std::filesystem::path(renderer.paths.front()));
        CHECK(p.filename().string()=="icon2247.png");
    }
    TEST_CASE("batch2247 loading different normalized paths keeps independent cache identities") {
        CountingAssetRenderer renderer; Salix::AssetManager manager; manager.initialize(&renderer);
        auto* a=manager.get_texture("assets/folder/one.png"); auto* b=manager.get_texture("assets/folder/two.png");
        REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); CHECK(a!=b);
        CHECK(manager.get_texture("assets/folder/./one.png")==a);
        CHECK(manager.get_texture("assets/folder/sub/../two.png")==b);
        CHECK(renderer.calls==2);
    }
}

TEST_SUITE("Salix::assets::AssetManagerBatch2847") {
    TEST_CASE("batch2847 three lexical aliases collapse to one cache entry") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        auto* a=assets.get_texture("assets/ui/icon2847.png");
        auto* b=assets.get_texture("assets/ui/./icon2847.png");
        auto* c=assets.get_texture("assets/temp/../ui/icon2847.png");
        REQUIRE(a!=nullptr); CHECK(a==b); CHECK(b==c); CHECK(renderer.calls==1);
    }
    TEST_CASE("batch2847 unrelated successful loads never disturb an earlier cached pointer") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        auto* first=assets.get_texture("assets/first2847.png"); REQUIRE(first!=nullptr);
        REQUIRE(assets.get_texture("assets/second2847.png")!=nullptr);
        CHECK(assets.get_texture("assets/first2847.png")==first);
        CHECK(renderer.calls==2);
    }
    TEST_CASE("batch2847 failed load of one path leaves another cached path usable") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        auto* good=assets.get_texture("assets/good2847.png"); REQUIRE(good!=nullptr);
        renderer.should_texture_load_fail=true;
        CHECK(assets.get_texture("assets/bad2847.png")==nullptr);
        CHECK(assets.get_texture("assets/good2847.png")==good);
    }
    TEST_CASE("batch2847 shutdown followed by retry uses renderer again for every previous cache key") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/a2847.png")!=nullptr);
        REQUIRE(assets.get_texture("assets/b2847.png")!=nullptr);
        CHECK(renderer.calls==2);
        assets.shutdown();
        REQUIRE(assets.get_texture("assets/a2847.png")!=nullptr);
        REQUIRE(assets.get_texture("assets/b2847.png")!=nullptr);
        CHECK(renderer.calls==4);
    }
    TEST_CASE("batch2847 switching renderers affects only uncached requests") {
        CountingAssetRenderer first,second; Salix::AssetManager assets; assets.initialize(&first);
        auto* cached=assets.get_texture("assets/cached2847.png"); REQUIRE(cached!=nullptr);
        assets.initialize(&second);
        CHECK(assets.get_texture("assets/cached2847.png")==cached);
        REQUIRE(assets.get_texture("assets/new2847.png")!=nullptr);
        CHECK(first.calls==1); CHECK(second.calls==1);
    }
    TEST_CASE("batch2847 failed request after renderer switch is not cached") {
        CountingAssetRenderer first,second; Salix::AssetManager assets; assets.initialize(&first);
        REQUIRE(assets.get_texture("assets/base2847.png")!=nullptr);
        second.should_texture_load_fail=true;
        assets.initialize(&second);
        CHECK(assets.get_texture("assets/retry-switch2847.png")==nullptr);
        CHECK(assets.get_texture("assets/retry-switch2847.png")==nullptr);
        CHECK(second.calls==2);
    }
    TEST_CASE("batch2847 recovery after renderer switch caches the first successful retry") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        renderer.should_texture_load_fail=true;
        CHECK(assets.get_texture("assets/recover2847.png")==nullptr);
        renderer.should_texture_load_fail=false;
        auto* recovered=assets.get_texture("assets/recover2847.png"); REQUIRE(recovered!=nullptr);
        CHECK(assets.get_texture("assets/recover2847.png")==recovered);
        CHECK(renderer.calls==2);
    }
    TEST_CASE("batch2847 each manager independently normalizes identical logical paths") {
        CountingAssetRenderer renderer; Salix::AssetManager a,b; a.initialize(&renderer); b.initialize(&renderer);
        auto* first=a.get_texture("assets/./shared2847.png");
        auto* second=b.get_texture("assets/temp/../shared2847.png");
        REQUIRE(first!=nullptr); REQUIRE(second!=nullptr); CHECK(first!=second); CHECK(renderer.calls==2);
    }
    TEST_CASE("batch2847 renderer sees lexically normalized parent traversal path") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/a/b/../../final2847.png")!=nullptr);
        REQUIRE(renderer.paths.size()==1);
        CHECK(std::filesystem::path(renderer.paths[0]).generic_string()=="assets/final2847.png");
    }
    TEST_CASE("batch2847 repeated shutdown remains safe before a fresh load") {
        CountingAssetRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        CHECK_NOTHROW(assets.shutdown()); CHECK_NOTHROW(assets.shutdown());
        REQUIRE(assets.get_texture("assets/post-shutdown2847.png")!=nullptr);
        CHECK(renderer.calls==1);
    }
}

TEST_SUITE("Salix::assets::AssetManagerBatch3247") {
 TEST_CASE("batch3247 four lexical aliases share one cached texture") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); auto* p=a.get_texture("assets/ui/icon3247.png"); REQUIRE(p); CHECK(a.get_texture("assets/ui/./icon3247.png")==p); CHECK(a.get_texture("assets/tmp/../ui/icon3247.png")==p); CHECK(a.get_texture("./assets/ui/icon3247.png")==p); CHECK(r.calls==1); }
 TEST_CASE("batch3247 cached success survives later renderer failure") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); auto* p=a.get_texture("assets/cached3247.png"); REQUIRE(p); r.should_texture_load_fail=true; CHECK(a.get_texture("assets/cached3247.png")==p); CHECK(r.calls==1); }
 TEST_CASE("batch3247 shutdown clears this manager without disturbing another manager") { CountingAssetRenderer r; Salix::AssetManager a,b; a.initialize(&r); b.initialize(&r); auto* pa=a.get_texture("assets/shared3247.png"); auto* pb=b.get_texture("assets/shared3247.png"); REQUIRE(pa); REQUIRE(pb); CHECK(r.calls==2); a.shutdown(); CHECK(b.get_texture("assets/shared3247.png")==pb); REQUIRE(a.get_texture("assets/shared3247.png")!=nullptr); CHECK(r.calls==3); }
 TEST_CASE("batch3247 shutdown then renderer replacement reloads old key through replacement backend") { CountingAssetRenderer first,second; Salix::AssetManager a; a.initialize(&first); REQUIRE(a.get_texture("assets/reload3247.png")); CHECK(first.calls==1); a.shutdown(); a.initialize(&second); REQUIRE(a.get_texture("assets/reload3247.png")); CHECK(first.calls==1); CHECK(second.calls==1); }
 TEST_CASE("batch3247 path containing spaces remains a stable cache key") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); auto* p=a.get_texture("assets/UI Images/Icon 3247.png"); REQUIRE(p); CHECK(a.get_texture("assets/UI Images/./Icon 3247.png")==p); CHECK(r.calls==1); }
 TEST_CASE("batch3247 case-distinct logical paths remain distinct cache entries") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); auto* lower=a.get_texture("assets/case3247.png"); auto* upper=a.get_texture("assets/CASE3247.png"); REQUIRE(lower); REQUIRE(upper); CHECK(lower!=upper); CHECK(r.calls==2); }
 TEST_CASE("batch3247 failed load followed by success caches successful retry only") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); r.should_texture_load_fail=true; CHECK(a.get_texture("assets/recover3247.png")==nullptr); r.should_texture_load_fail=false; auto* p=a.get_texture("assets/recover3247.png"); REQUIRE(p); CHECK(a.get_texture("assets/recover3247.png")==p); CHECK(r.calls==2); }
 TEST_CASE("batch3247 renderer switch preserves old cache while directing new key to new backend") { CountingAssetRenderer first,second; Salix::AssetManager a; a.initialize(&first); auto* old=a.get_texture("assets/old3247.png"); REQUIRE(old); a.initialize(&second); CHECK(a.get_texture("assets/old3247.png")==old); REQUIRE(a.get_texture("assets/new3247.png")); CHECK(first.calls==1); CHECK(second.calls==1); }
 TEST_CASE("batch3247 repeated shutdown after populated cache remains safe before fresh load") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); REQUIRE(a.get_texture("assets/pre3247.png")); a.shutdown(); CHECK_NOTHROW(a.shutdown()); CHECK_NOTHROW(a.shutdown()); REQUIRE(a.get_texture("assets/pre3247.png")); CHECK(r.calls==2); }
 TEST_CASE("batch3247 normalized parent traversal reaches renderer without dot segments") { CountingAssetRenderer r; Salix::AssetManager a; a.initialize(&r); REQUIRE(a.get_texture("assets/a/b/../../final3247.png")); REQUIRE(r.paths.size()==1); const auto p=std::filesystem::path(r.paths[0]).lexically_normal(); CHECK(p==std::filesystem::path(r.paths[0])); CHECK(p.filename().string()=="final3247.png"); }
}
TEST_SUITE("Salix::assets::AssetManagerBatch3647") {
    TEST_CASE("batch3647 texture request before initialization is a safe null result") {
        Salix::AssetManager assets;
        CHECK_NOTHROW((void)assets.get_texture("assets/preinit3647.png"));
        CHECK(assets.get_texture("assets/preinit3647.png") == nullptr);
    }

    TEST_CASE("batch3647 explicit null renderer initialization keeps uncached requests safe") {
        Salix::AssetManager assets;
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/null-renderer3647.png") == nullptr);
    }

    TEST_CASE("batch3647 valid renderer can recover after earlier preinitialization misses") {
        Salix::AssetManager assets;
        CHECK(assets.get_texture("assets/recover3647.png") == nullptr);
        CountingAssetRenderer renderer;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("assets/recover3647.png");
        REQUIRE(texture != nullptr);
        CHECK(renderer.calls == 1);
        CHECK(assets.get_texture("assets/recover3647.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch3647 cached texture remains readable after renderer pointer is cleared") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("assets/cached3647.png");
        REQUIRE(texture != nullptr);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/cached3647.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch3647 uncached request while renderer is cleared does not call previous backend") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/known3647.png") != nullptr);
        CHECK(renderer.calls == 1);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/new-while-null3647.png") == nullptr);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch3647 replacement renderer handles new request after temporary null renderer") {
        CountingAssetRenderer first, second;
        Salix::AssetManager assets;
        assets.initialize(&first);
        REQUIRE(assets.get_texture("assets/first3647.png") != nullptr);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/second3647.png") == nullptr);
        assets.initialize(&second);
        REQUIRE(assets.get_texture("assets/second3647.png") != nullptr);
        CHECK(first.calls == 1);
        CHECK(second.calls == 1);
    }

    TEST_CASE("batch3647 shutdown clears cached texture but retains active renderer for reload") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("assets/reload3647.png");
        REQUIRE(first != nullptr);
        assets.shutdown();
        auto* second = assets.get_texture("assets/reload3647.png");
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch3647 failed load can recover through a replacement renderer") {
        CountingAssetRenderer failing, replacement;
        failing.should_texture_load_fail = true;
        Salix::AssetManager assets;
        assets.initialize(&failing);
        CHECK(assets.get_texture("assets/replacement3647.png") == nullptr);
        assets.initialize(&replacement);
        auto* recovered = assets.get_texture("assets/replacement3647.png");
        REQUIRE(recovered != nullptr);
        CHECK(failing.calls == 1);
        CHECK(replacement.calls == 1);
        CHECK(assets.get_texture("assets/replacement3647.png") == recovered);
    }

    TEST_CASE("batch3647 cached lexical alias remains readable without renderer") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("assets/ui/icon3647.png");
        REQUIRE(texture != nullptr);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/ui/./icon3647.png") == texture);
        CHECK(assets.get_texture("assets/tmp/../ui/icon3647.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch3647 initialized and uninitialized managers remain completely independent") {
        CountingAssetRenderer renderer;
        Salix::AssetManager initialized, uninitialized;
        initialized.initialize(&renderer);
        auto* texture = initialized.get_texture("assets/shared3647.png");
        REQUIRE(texture != nullptr);
        CHECK(uninitialized.get_texture("assets/shared3647.png") == nullptr);
        CHECK(initialized.get_texture("assets/shared3647.png") == texture);
        CHECK(renderer.calls == 1);
    }
}

TEST_SUITE("Salix::assets::AssetManagerBatch4047") {
    TEST_CASE("batch4047 repeated initialize with same renderer preserves existing cache") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("assets/batch4047/repeat.png");
        REQUIRE(texture != nullptr);
        assets.initialize(&renderer);
        CHECK(assets.get_texture("assets/batch4047/repeat.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4047 complex lexical aliases collapse to one cached entry") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("assets/batch4047/ui/icon.png");
        REQUIRE(texture != nullptr);
        CHECK(assets.get_texture("assets/batch4047/ui/./icon.png") == texture);
        CHECK(assets.get_texture("assets/batch4047/temp/../ui/icon.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4047 two cached entries both survive renderer replacement") {
        CountingAssetRenderer first, second;
        Salix::AssetManager assets;
        assets.initialize(&first);
        auto* a = assets.get_texture("assets/batch4047/a.png");
        auto* b = assets.get_texture("assets/batch4047/b.png");
        REQUIRE(a != nullptr); REQUIRE(b != nullptr);
        assets.initialize(&second);
        CHECK(assets.get_texture("assets/batch4047/a.png") == a);
        CHECK(assets.get_texture("assets/batch4047/b.png") == b);
        CHECK(first.calls == 2);
        CHECK(second.calls == 0);
    }

    TEST_CASE("batch4047 renderer replacement handles only newly requested key") {
        CountingAssetRenderer first, second;
        Salix::AssetManager assets;
        assets.initialize(&first);
        REQUIRE(assets.get_texture("assets/batch4047/old.png") != nullptr);
        assets.initialize(&second);
        REQUIRE(assets.get_texture("assets/batch4047/new.png") != nullptr);
        CHECK(first.calls == 1);
        CHECK(second.calls == 1);
    }

    TEST_CASE("batch4047 clearing renderer leaves multiple cached entries readable") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* a = assets.get_texture("assets/batch4047/cached-a.png");
        auto* b = assets.get_texture("assets/batch4047/cached-b.png");
        REQUIRE(a != nullptr); REQUIRE(b != nullptr);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/batch4047/cached-a.png") == a);
        CHECK(assets.get_texture("assets/batch4047/cached-b.png") == b);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4047 uncached request during null renderer does not poison later recovery") {
        Salix::AssetManager assets;
        assets.initialize(nullptr);
        CHECK(assets.get_texture("assets/batch4047/recover.png") == nullptr);
        CountingAssetRenderer renderer;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/batch4047/recover.png") != nullptr);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4047 failed key does not disturb a separately cached successful key") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* cached = assets.get_texture("assets/batch4047/good.png");
        REQUIRE(cached != nullptr);
        renderer.should_texture_load_fail = true;
        CHECK(assets.get_texture("assets/batch4047/bad.png") == nullptr);
        CHECK(assets.get_texture("assets/batch4047/good.png") == cached);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4047 shutdown forces every previous cache key through renderer again") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/batch4047/one.png") != nullptr);
        REQUIRE(assets.get_texture("assets/batch4047/two.png") != nullptr);
        CHECK(renderer.calls == 2);
        assets.shutdown();
        REQUIRE(assets.get_texture("assets/batch4047/one.png") != nullptr);
        REQUIRE(assets.get_texture("assets/batch4047/two.png") != nullptr);
        CHECK(renderer.calls == 4);
    }

    TEST_CASE("batch4047 shutdown while renderer is null leaves subsequent request safely null") {
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("assets/batch4047/null-after-shutdown.png") != nullptr);
        assets.initialize(nullptr);
        assets.shutdown();
        CHECK(assets.get_texture("assets/batch4047/null-after-shutdown.png") == nullptr);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4047 separate managers never share aliases of the same logical asset") {
        CountingAssetRenderer renderer;
        Salix::AssetManager first, second;
        first.initialize(&renderer);
        second.initialize(&renderer);
        auto* a = first.get_texture("assets/batch4047/shared.png");
        auto* b = second.get_texture("assets/batch4047/./shared.png");
        REQUIRE(a != nullptr); REQUIRE(b != nullptr);
        CHECK(a != b);
        CHECK(renderer.calls == 2);
    }
}

namespace {
struct Batch4347ProjectRootGuard {
    std::filesystem::path original = Salix::g_project_root_path;
    ~Batch4347ProjectRootGuard() { Salix::g_project_root_path = original; }
};
}

TEST_SUITE("Salix::assets::AssetManagerBatch4347") {
    TEST_CASE("batch4347 project root prefixes relative texture requests before renderer load") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4347Root";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("Assets/Textures/player.png") != nullptr);
        REQUIRE(renderer.paths.size() == 1);
        CHECK(std::filesystem::path(renderer.paths[0]) ==
              (Salix::g_project_root_path / "Assets/Textures/player.png").lexically_normal());
    }

    TEST_CASE("batch4347 absolute texture request is not prefixed by project root") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/IgnoredRoot4347";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        const std::filesystem::path absolute = "D:/External4347/icon.png";
        REQUIRE(assets.get_texture(absolute.string()) != nullptr);
        REQUIRE(renderer.paths.size() == 1);
        CHECK(std::filesystem::path(renderer.paths[0]) == absolute.lexically_normal());
    }

    TEST_CASE("batch4347 changing project root makes same relative path a distinct cache key") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::g_project_root_path = "C:/RootA4347";
        auto* first = assets.get_texture("Assets/shared.png");
        REQUIRE(first != nullptr);
        Salix::g_project_root_path = "C:/RootB4347";
        auto* second = assets.get_texture("Assets/shared.png");
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4347 returning to earlier project root reuses its cached texture") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::g_project_root_path = "C:/RootA4347";
        auto* first = assets.get_texture("Assets/revisit.png");
        REQUIRE(first != nullptr);
        Salix::g_project_root_path = "C:/RootB4347";
        REQUIRE(assets.get_texture("Assets/revisit.png") != nullptr);
        Salix::g_project_root_path = "C:/RootA4347";
        CHECK(assets.get_texture("Assets/revisit.png") == first);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4347 project root containing spaces is preserved in renderer path") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Projects/Batch 4347 Root";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("Assets/My Texture.png") != nullptr);
        REQUIRE(renderer.paths.size() == 1);
        CHECK(std::filesystem::path(renderer.paths[0]) ==
              (Salix::g_project_root_path / "Assets/My Texture.png").lexically_normal());
    }

    TEST_CASE("batch4347 failed load under one root does not poison same relative path under another root") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        renderer.should_texture_load_fail = true;
        Salix::g_project_root_path = "C:/FailRoot4347";
        CHECK(assets.get_texture("Assets/recover.png") == nullptr);
        renderer.should_texture_load_fail = false;
        Salix::g_project_root_path = "C:/GoodRoot4347";
        REQUIRE(assets.get_texture("Assets/recover.png") != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4347 lexical parent traversal cannot escape cache identity within same normalized root path") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Root4347";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("Assets/UI/icon.png");
        REQUIRE(first != nullptr);
        CHECK(assets.get_texture("Assets/Temp/../UI/icon.png") == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4347 shutdown clears cached entries created under multiple project roots") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::g_project_root_path = "C:/RootA4347";
        REQUIRE(assets.get_texture("Assets/a.png") != nullptr);
        Salix::g_project_root_path = "C:/RootB4347";
        REQUIRE(assets.get_texture("Assets/b.png") != nullptr);
        CHECK(renderer.calls == 2);
        assets.shutdown();
        Salix::g_project_root_path = "C:/RootA4347";
        REQUIRE(assets.get_texture("Assets/a.png") != nullptr);
        Salix::g_project_root_path = "C:/RootB4347";
        REQUIRE(assets.get_texture("Assets/b.png") != nullptr);
        CHECK(renderer.calls == 4);
    }

    TEST_CASE("batch4347 clearing renderer leaves root specific cached entry readable") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Root4347";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* texture = assets.get_texture("Assets/cached.png");
        REQUIRE(texture != nullptr);
        assets.initialize(nullptr);
        CHECK(assets.get_texture("Assets/./cached.png") == texture);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4347 renderer replacement receives only uncached root specific request") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Root4347";
        CountingAssetRenderer first, second;
        Salix::AssetManager assets;
        assets.initialize(&first);
        auto* cached = assets.get_texture("Assets/old.png");
        REQUIRE(cached != nullptr);
        assets.initialize(&second);
        CHECK(assets.get_texture("Assets/old.png") == cached);
        REQUIRE(assets.get_texture("Assets/new.png") != nullptr);
        CHECK(first.calls == 1);
        CHECK(second.calls == 1);
    }
}

TEST_SUITE("Salix::assets::AssetManagerBatch4847") {
    TEST_CASE("batch4847 dot project root and empty project root normalize same relative asset key") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::g_project_root_path = ".";
        auto* first = assets.get_texture("Assets/batch4847/dot.png");
        REQUIRE(first != nullptr);
        Salix::g_project_root_path.clear();
        auto* second = assets.get_texture("Assets/batch4847/dot.png");
        CHECK(second == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 lexically equivalent project roots reuse one cached texture") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::g_project_root_path = "C:/Batch4847Root/Temp/..";
        auto* first = assets.get_texture("Assets/shared.png");
        REQUIRE(first != nullptr);
        Salix::g_project_root_path = "C:/Batch4847Root";
        CHECK(assets.get_texture("Assets/shared.png") == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 absolute dot segment alias shares existing cache entry") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Ignored4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("D:/Batch4847/UI/icon.png");
        REQUIRE(first != nullptr);
        auto* second = assets.get_texture("D:/Batch4847/UI/./icon.png");
        CHECK(second == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 relative leading dot alias shares existing cache entry") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("Assets/UI/icon.png");
        REQUIRE(first != nullptr);
        CHECK(assets.get_texture("./Assets/UI/icon.png") == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 shutdown followed by null renderer makes former cached request safely unavailable") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("Assets/cleared.png") != nullptr);
        REQUIRE(renderer.calls == 1);
        assets.shutdown();
        assets.initialize(nullptr);
        CHECK(assets.get_texture("Assets/cleared.png") == nullptr);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 same renderer after shutdown reloads previously cached logical key") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("Assets/reload.png");
        REQUIRE(first != nullptr);
        assets.shutdown();
        assets.initialize(&renderer);
        auto* second = assets.get_texture("Assets/reload.png");
        REQUIRE(second != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4847 renderer failure under aliased path is retried by canonical path") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        renderer.should_texture_load_fail = true;
        CHECK(assets.get_texture("Assets/Temp/../retry.png") == nullptr);
        renderer.should_texture_load_fail = false;
        REQUIRE(assets.get_texture("Assets/retry.png") != nullptr);
        CHECK(renderer.calls == 2);
    }

    TEST_CASE("batch4847 cached texture remains available across multiple renderer pointer replacements") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer first, second, third;
        Salix::AssetManager assets;
        assets.initialize(&first);
        auto* cached = assets.get_texture("Assets/stable.png");
        REQUIRE(cached != nullptr);
        assets.initialize(&second);
        assets.initialize(&third);
        CHECK(assets.get_texture("Assets/stable.png") == cached);
        CHECK(first.calls == 1);
        CHECK(second.calls == 0);
        CHECK(third.calls == 0);
    }

    TEST_CASE("batch4847 two absolute paths differing only by parent alias normalize to one cache key") {
        Batch4347ProjectRootGuard guard;
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        auto* first = assets.get_texture("D:/Batch4847/Images/icon.png");
        REQUIRE(first != nullptr);
        auto* second = assets.get_texture("D:/Batch4847/Temp/../Images/icon.png");
        CHECK(second == first);
        CHECK(renderer.calls == 1);
    }

    TEST_CASE("batch4847 shutdown clears alias cache so next equivalent alias reaches renderer again") {
        Batch4347ProjectRootGuard guard;
        Salix::g_project_root_path = "C:/Batch4847";
        CountingAssetRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        REQUIRE(assets.get_texture("Assets/A/../icon.png") != nullptr);
        CHECK(renderer.calls == 1);
        assets.shutdown();
        REQUIRE(assets.get_texture("Assets/icon.png") != nullptr);
        CHECK(renderer.calls == 2);
    }
}
