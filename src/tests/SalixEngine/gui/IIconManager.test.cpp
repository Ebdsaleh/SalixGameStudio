#include <doctest.h>
#include <Salix/gui/IIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Tests/SalixEngine/mocking/gui/MockIIconManager.h>
#include <memory>
#include <cstdint>

TEST_SUITE("Salix::gui::IIconManagerBatch1447") {
    TEST_CASE("icon manager initialization stores exact asset manager pointer") {
        MockIIconManager manager;
        Salix::AssetManager* fake = reinterpret_cast<Salix::AssetManager*>(static_cast<uintptr_t>(0x1234));
        Salix::IIconManager* interface_ptr = &manager;
        interface_ptr->initialize(fake);
        CHECK(manager.get_mock_asset_manager() == fake);
        CHECK(manager.initialize_calls == 1);
    }

    TEST_CASE("icon manager initialization accepts null asset manager for isolated testing") {
        MockIIconManager manager;
        manager.initialize(nullptr);
        CHECK(manager.get_mock_asset_manager() == nullptr);
        CHECK(manager.initialize_calls == 1);
    }

    TEST_CASE("icon manager default registration creates fallback descriptor") {
        MockIIconManager manager;
        manager.register_default_icons();
        CHECK(manager.register_default_calls == 1);
        CHECK(manager.get_icon_registry().count("Default") == 1);
        CHECK(manager.get_icon_by_name("Default").path == "mock/default.png");
    }

    TEST_CASE("icon manager repeated default registration remains idempotent in registry size") {
        MockIIconManager manager;
        manager.register_default_icons();
        manager.register_default_icons();
        CHECK(manager.register_default_calls == 2);
        CHECK(manager.get_icon_registry().size() == 1);
    }

    TEST_CASE("icon manager update creates named icon path in registry") {
        MockIIconManager manager;
        manager.update_icon("Camera", "icons/camera.png");
        CHECK(manager.update_calls == 1);
        CHECK(manager.last_update_name == "Camera");
        CHECK(manager.last_update_path == "icons/camera.png");
        CHECK(manager.get_icon_by_name("Camera").path == "icons/camera.png");
    }

    TEST_CASE("icon manager update replaces path without adding duplicate registry key") {
        MockIIconManager manager;
        manager.update_icon("Camera", "first.png");
        manager.update_icon("Camera", "second.png");
        CHECK(manager.get_icon_registry().size() == 1);
        CHECK(manager.get_icon_by_name("Camera").path == "second.png");
    }

    TEST_CASE("icon manager missing name falls back to registered default") {
        MockIIconManager manager;
        manager.register_default_icons();
        const auto& missing = manager.get_icon_by_name("Missing");
        const auto& fallback = manager.get_icon_by_name("Default");
        CHECK(missing.path == fallback.path);
        CHECK(missing.texture_id == fallback.texture_id);
    }

    TEST_CASE("icon manager entity lookup uses entity entry when registered") {
        MockIIconManager manager;
        manager.register_default_icons();
        manager.update_icon("Entity", "icons/entity.png");
        const auto& icon = manager.get_icon_for_entity(nullptr);
        CHECK(manager.entity_lookup_calls == 1);
        CHECK(icon.path == "icons/entity.png");
    }

    TEST_CASE("icon manager element lookup falls back safely when no element icon exists") {
        MockIIconManager manager;
        manager.register_default_icons();
        const auto& icon = manager.get_icon_for_element(nullptr);
        CHECK(manager.element_lookup_calls == 1);
        CHECK(icon.path == "mock/default.png");
    }

    TEST_CASE("icon manager registry reference exposes current managed descriptors") {
        MockIIconManager manager;
        manager.register_default_icons();
        manager.update_icon("Sprite2D", "icons/sprite.png");
        const auto& registry = manager.get_icon_registry();
        REQUIRE(registry.size() == 2);
        CHECK(registry.at("Sprite2D").path == "icons/sprite.png");
        CHECK(registry.at("Default").path == "mock/default.png");
    }
}
