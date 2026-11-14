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

TEST_SUITE("Salix::gui::IIconManagerBatch6647") {
    TEST_CASE("batch6647 fresh registry is empty before default registration") {
        MockIIconManager manager;
        CHECK(manager.get_icon_registry().empty());
    }

    TEST_CASE("batch6647 missing icon without default returns empty descriptor") {
        MockIIconManager manager;
        const auto& icon = manager.get_icon_by_name("Missing6647");
        CHECK(icon.path.empty());
        CHECK(icon.texture_id == static_cast<ImTextureID>(0));
        CHECK(icon.size.x == doctest::Approx(16.0f));
    }

    TEST_CASE("batch6647 update with empty type name creates empty key entry") {
        MockIIconManager manager;
        manager.update_icon("", "empty-key6647.png");
        CHECK(manager.get_icon_registry().count("") == 1);
        CHECK(manager.get_icon_by_name("").path == "empty-key6647.png");
    }

    TEST_CASE("batch6647 update with empty path preserves named registry entry") {
        MockIIconManager manager;
        manager.update_icon("EmptyPath6647", "");
        CHECK(manager.get_icon_registry().count("EmptyPath6647") == 1);
        CHECK(manager.get_icon_by_name("EmptyPath6647").path.empty());
    }

    TEST_CASE("batch6647 entity lookup uses fallback default when entity icon absent") {
        MockIIconManager manager;
        manager.register_default_icons();
        const auto& icon = manager.get_icon_for_entity(nullptr);
        CHECK(manager.entity_lookup_calls == 1);
        CHECK(icon.path == "mock/default.png");
    }

    TEST_CASE("batch6647 element lookup uses registered element descriptor when present") {
        MockIIconManager manager;
        manager.register_default_icons();
        manager.update_icon("Element", "element6647.png");
        const auto& icon = manager.get_icon_for_element(nullptr);
        CHECK(manager.element_lookup_calls == 1);
        CHECK(icon.path == "element6647.png");
    }

    TEST_CASE("batch6647 default registration does not overwrite explicitly updated default") {
        MockIIconManager manager;
        manager.update_icon("Default", "custom-default6647.png");
        manager.register_default_icons();
        CHECK(manager.get_icon_by_name("Default").path == "custom-default6647.png");
    }

    TEST_CASE("batch6647 registry reference observes later icon updates") {
        MockIIconManager manager;
        const auto& registry = manager.get_icon_registry();
        REQUIRE(registry.empty());
        manager.update_icon("Later6647", "later6647.png");
        CHECK(registry.size() == 1);
        CHECK(registry.at("Later6647").path == "later6647.png");
    }

    TEST_CASE("batch6647 repeated initialization keeps latest asset manager pointer") {
        MockIIconManager manager;
        auto* first = reinterpret_cast<Salix::AssetManager*>(static_cast<uintptr_t>(0x6647));
        auto* second = reinterpret_cast<Salix::AssetManager*>(static_cast<uintptr_t>(0x7466));
        manager.initialize(first);
        manager.initialize(second);
        CHECK(manager.initialize_calls == 2);
        CHECK(manager.get_mock_asset_manager() == second);
    }

    TEST_CASE("batch6647 unrelated icon update leaves default descriptor unchanged") {
        MockIIconManager manager;
        manager.register_default_icons();
        const std::string before = manager.get_icon_by_name("Default").path;
        manager.update_icon("Camera6647", "camera6647.png");
        CHECK(manager.get_icon_by_name("Default").path == before);
        CHECK(manager.get_icon_by_name("Camera6647").path == "camera6647.png");
    }
}
