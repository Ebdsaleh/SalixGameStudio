#include <doctest.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <imgui/imgui.h>
#include <string>
#include <Salix/gui/IIconManager.h>
#include <Salix/ecs/CppScript.h>

namespace {
struct Batch1147IconFixture {
    MockIRenderer renderer;
    Salix::AssetManager assets;
    Salix::ImGuiIconManager icons;

    Batch1147IconFixture() {
        assets.initialize(&renderer);
        icons.initialize(&assets);
    }

    ~Batch1147IconFixture() {
        assets.shutdown();
    }

    void register_defaults() {
        icons.register_default_icons();
    }
};
}

TEST_SUITE("Salix::gui::ImGuiIconManagerBatch1147") {
    TEST_CASE("fresh icon manager starts with an empty registry") {
        Salix::ImGuiIconManager icons;
        CHECK(icons.get_icon_registry().empty());
    }

    TEST_CASE("null AssetManager initialization leaves default registration safely disabled") {
        Salix::ImGuiIconManager icons;
        icons.initialize(nullptr);
        CHECK_NOTHROW(icons.register_default_icons());
        CHECK(icons.get_icon_registry().empty());
    }

    TEST_CASE("default registration installs the complete canonical icon set") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        const auto& registry = fixture.icons.get_icon_registry();
        CHECK(registry.size() == 7);
        CHECK(registry.count("Entity") == 1);
        CHECK(registry.count("Camera") == 1);
        CHECK(registry.count("Transform") == 1);
        CHECK(registry.count("Sprite2D") == 1);
        CHECK(registry.count("Panel Locked") == 1);
        CHECK(registry.count("Panel Unlocked") == 1);
        CHECK(registry.count("Default") == 1);
    }

    TEST_CASE("named icon lookup returns registered path and mock texture identity") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        const Salix::IconInfo& camera = fixture.icons.get_icon_by_name("Camera");
        CHECK(camera.path == "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_039.png");
        CHECK(camera.texture_id == ImTextureID(18));
        CHECK(camera.size.x == doctest::Approx(16.0f));
        CHECK(camera.size.y == doctest::Approx(16.0f));
    }

    TEST_CASE("unknown icon name falls back to the registered Default icon") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        const Salix::IconInfo& fallback = fixture.icons.get_icon_by_name("Batch1147 Missing Icon");
        const Salix::IconInfo& expected = fixture.icons.get_icon_by_name("Default");
        CHECK(fallback.path == expected.path);
        CHECK(fallback.texture_id == expected.texture_id);
        CHECK(fallback.size.x == doctest::Approx(expected.size.x));
        CHECK(fallback.size.y == doctest::Approx(expected.size.y));
    }

    TEST_CASE("null entity lookup returns absolute default icon") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        const Salix::IconInfo& result = fixture.icons.get_icon_for_entity(nullptr);
        const Salix::IconInfo& expected = fixture.icons.get_icon_by_name("Default");
        CHECK(result.path == expected.path);
        CHECK(result.texture_id == expected.texture_id);
        CHECK(result.size.x == doctest::Approx(expected.size.x));
        CHECK(result.size.y == doctest::Approx(expected.size.y));
    }

    TEST_CASE("generic entity without special elements resolves to Entity icon") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        Salix::Entity entity;
        const Salix::IconInfo& result = fixture.icons.get_icon_for_entity(&entity);
        const Salix::IconInfo& expected = fixture.icons.get_icon_by_name("Entity");
        CHECK(&result == &expected);
        CHECK(result.path == expected.path);
    }

    TEST_CASE("entity with Camera element resolves to Camera icon") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        Salix::Entity entity;
        Salix::Camera* camera = entity.add_element<Salix::Camera>();
        REQUIRE(camera != nullptr);
        const Salix::IconInfo& result = fixture.icons.get_icon_for_entity(&entity);
        const Salix::IconInfo& expected = fixture.icons.get_icon_by_name("Camera");
        CHECK(&result == &expected);
        CHECK(result.path == expected.path);
    }

    TEST_CASE("entity with loaded Sprite2D uses sprite texture as highest-priority dynamic icon") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        Salix::Entity entity;
        Salix::Sprite2D* sprite = entity.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&fixture.assets, "Assets/Images/Batch1147Sprite.png");
        REQUIRE(sprite->get_texture() != nullptr);
        const Salix::IconInfo& result = fixture.icons.get_icon_for_entity(&entity);
        CHECK(result.texture_id == sprite->get_texture()->get_imgui_texture_id());
        CHECK(result.path.empty());
    }

    TEST_CASE("element lookup and icon update follow registry type names and preserve texture-backed mapping") {
        Batch1147IconFixture fixture;
        fixture.register_defaults();
        Salix::Entity entity;
        Salix::Transform* transform = entity.get_transform();
        REQUIRE(transform != nullptr);

        const Salix::IconInfo& before = fixture.icons.get_icon_for_element(transform);
        CHECK(before.path == "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_092.png");

        fixture.icons.update_icon("Transform", "Assets/Icons/Batch1147/transform-new.png");
        const Salix::IconInfo& after = fixture.icons.get_icon_for_element(transform);
        CHECK(after.path == "Assets/Icons/Batch1147/transform-new.png");
        CHECK(after.texture_id == ImTextureID(18));

        const auto size_before_missing_update = fixture.icons.get_icon_registry().size();
        fixture.icons.update_icon("Batch1147UnknownType", "Assets/Icons/never-used.png");
        CHECK(fixture.icons.get_icon_registry().size() == size_before_missing_update);
    }
}

namespace {
struct Batch1947IconFixture {
    MockIRenderer renderer;
    Salix::AssetManager assets;
    Salix::ImGuiIconManager icons;
    Batch1947IconFixture() { assets.initialize(&renderer); icons.initialize(&assets); }
    ~Batch1947IconFixture() { assets.shutdown(); }
};
}

TEST_SUITE("Salix::gui::ImGuiIconManagerBatch1947") {
    TEST_CASE("missing name on fresh manager returns neutral default icon") {
        Salix::ImGuiIconManager icons;
        const auto& icon = icons.get_icon_by_name("Missing");
        CHECK(icon.texture_id == ImTextureID(0));
        CHECK(icon.path.empty());
        CHECK(icon.size.x == doctest::Approx(16.0f));
        CHECK(icon.size.y == doctest::Approx(16.0f));
    }

    TEST_CASE("null element on fresh manager returns neutral default icon") {
        Salix::ImGuiIconManager icons;
        const auto& icon = icons.get_icon_for_element(nullptr);
        CHECK(icon.texture_id == ImTextureID(0));
        CHECK(icon.path.empty());
    }

    TEST_CASE("unregistered element type falls back to registered Default icon") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::CppScript script;
        const auto& icon = fixture.icons.get_icon_for_element(&script);
        const auto& fallback = fixture.icons.get_icon_by_name("Default");
        CHECK(icon.path == fallback.path);
        CHECK(icon.texture_id == fallback.texture_id);
    }

    TEST_CASE("camera entity before default registration falls through to neutral default") {
        Batch1947IconFixture fixture;
        Salix::Entity entity;
        REQUIRE(entity.add_element<Salix::Camera>() != nullptr);
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        CHECK(icon.texture_id == ImTextureID(0));
        CHECK(icon.path.empty());
    }

    TEST_CASE("registering default icons repeatedly replaces mappings without growing registry") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        REQUIRE(fixture.icons.get_icon_registry().size() == 7);
        fixture.icons.register_default_icons();
        CHECK(fixture.icons.get_icon_registry().size() == 7);
        CHECK(fixture.icons.get_icon_registry().count("Default") == 1);
    }

    TEST_CASE("failed renderer loads leave default registration empty") {
        Batch1947IconFixture fixture;
        fixture.renderer.should_texture_load_fail = true;
        fixture.icons.register_default_icons();
        CHECK(fixture.icons.get_icon_registry().empty());
        const auto& fallback = fixture.icons.get_icon_by_name("Default");
        CHECK(fallback.texture_id == ImTextureID(0));
        CHECK(fallback.path.empty());
    }

    TEST_CASE("failed update of existing icon preserves old mapping") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const std::string old_path = fixture.icons.get_icon_by_name("Camera").path;
        fixture.renderer.should_texture_load_fail = true;
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch1947/missing-camera.png");
        CHECK(fixture.icons.get_icon_by_name("Camera").path == old_path);
    }

    TEST_CASE("successful update of existing icon replaces path and texture identity") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch1947/camera-new.png");
        const auto& icon = fixture.icons.get_icon_by_name("Camera");
        CHECK(icon.path == "Assets/Icons/Batch1947/camera-new.png");
        CHECK(icon.texture_id == ImTextureID(18));
    }

    TEST_CASE("sprite without texture falls through to camera icon when entity also has camera") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        REQUIRE(entity.add_element<Salix::Sprite2D>() != nullptr);
        REQUIRE(entity.add_element<Salix::Camera>() != nullptr);
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        const auto& camera = fixture.icons.get_icon_by_name("Camera");
        CHECK(&icon == &camera);
    }

    TEST_CASE("icon manager virtual interface dispatches registry and fallback lookup") {
        Batch1947IconFixture fixture;
        Salix::IIconManager* icons = &fixture.icons;
        icons->register_default_icons();
        CHECK(icons->get_icon_registry().size() == 7);
        const auto& missing = icons->get_icon_by_name("NeverRegistered");
        const auto& fallback = icons->get_icon_by_name("Default");
        CHECK(missing.texture_id == fallback.texture_id);
        CHECK(missing.path == fallback.path);
    }
}

// ---- Relocated from src/tests/SalixEngine/gui/IIconManager.test.cpp; canonical tests mirror production .cpp files ----
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


// ---- Relocated from src/tests/SalixEngine/gui/IconInfo.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/gui/IconInfo.h>
#include <string>

TEST_SUITE("Salix::gui::IconInfoBatch1247") {
    TEST_CASE("icon info defaults to no texture and empty path") {
        Salix::IconInfo icon;
        CHECK(icon.texture_id == static_cast<ImTextureID>(0));
        CHECK(icon.path.empty());
    }

    TEST_CASE("icon info defaults to sixteen pixel render size") {
        Salix::IconInfo icon;
        CHECK(icon.size.x == doctest::Approx(16.0f));
        CHECK(icon.size.y == doctest::Approx(16.0f));
    }

    TEST_CASE("icon info defaults to full texture UV range") {
        Salix::IconInfo icon;
        CHECK(icon.uv0.x == doctest::Approx(0.0f));
        CHECK(icon.uv0.y == doctest::Approx(0.0f));
        CHECK(icon.uv1.x == doctest::Approx(1.0f));
        CHECK(icon.uv1.y == doctest::Approx(1.0f));
    }

    TEST_CASE("icon info texture assignment preserves all presentation fields") {
        Salix::IconInfo icon;
        icon.path = "icons/player.png";
        icon.texture_id = static_cast<ImTextureID>(42);
        CHECK(icon.texture_id == static_cast<ImTextureID>(42));
        CHECK(icon.path == "icons/player.png");
        CHECK(icon.size.x == doctest::Approx(16.0f));
        CHECK(icon.uv1.x == doctest::Approx(1.0f));
    }

    TEST_CASE("icon info accepts independent non-square render dimensions") {
        Salix::IconInfo icon;
        icon.size = ImVec2(32.0f, 18.0f);
        CHECK(icon.size.x == doctest::Approx(32.0f));
        CHECK(icon.size.y == doctest::Approx(18.0f));
        CHECK(icon.uv0.x == doctest::Approx(0.0f));
    }

    TEST_CASE("icon info sprite sheet UV coordinates preserve arbitrary subregion") {
        Salix::IconInfo icon;
        icon.uv0 = ImVec2(0.25f, 0.5f);
        icon.uv1 = ImVec2(0.75f, 0.875f);
        CHECK(icon.uv0.x == doctest::Approx(0.25f));
        CHECK(icon.uv0.y == doctest::Approx(0.5f));
        CHECK(icon.uv1.x == doctest::Approx(0.75f));
        CHECK(icon.uv1.y == doctest::Approx(0.875f));
    }

    TEST_CASE("icon info does not clamp reversed UV coordinates") {
        Salix::IconInfo icon;
        icon.uv0 = ImVec2(1.0f, 1.0f);
        icon.uv1 = ImVec2(0.0f, 0.0f);
        CHECK(icon.uv0.x == doctest::Approx(1.0f));
        CHECK(icon.uv1.x == doctest::Approx(0.0f));
    }

    TEST_CASE("icon info copy construction preserves the complete descriptor") {
        Salix::IconInfo source;
        source.texture_id = static_cast<ImTextureID>(99);
        source.path = "icons/copied.png";
        source.size = ImVec2(24.0f, 30.0f);
        source.uv0 = ImVec2(0.1f, 0.2f);
        source.uv1 = ImVec2(0.8f, 0.9f);
        Salix::IconInfo copy(source);
        CHECK(copy.texture_id == source.texture_id);
        CHECK(copy.path == source.path);
        CHECK(copy.size.x == doctest::Approx(source.size.x));
        CHECK(copy.uv0.y == doctest::Approx(source.uv0.y));
        CHECK(copy.uv1.x == doctest::Approx(source.uv1.x));
    }

    TEST_CASE("icon info copied string and geometry state remain independent") {
        Salix::IconInfo source;
        source.path = "icons/original.png";
        source.size = ImVec2(16.0f, 16.0f);
        Salix::IconInfo copy = source;
        copy.path = "icons/copy.png";
        copy.size.x = 64.0f;
        CHECK(source.path == "icons/original.png");
        CHECK(source.size.x == doctest::Approx(16.0f));
        CHECK(copy.path == "icons/copy.png");
        CHECK(copy.size.x == doctest::Approx(64.0f));
    }

    TEST_CASE("icon info zero render size remains a representable descriptor state") {
        Salix::IconInfo icon;
        icon.size = ImVec2(0.0f, 0.0f);
        icon.texture_id = static_cast<ImTextureID>(7);
        CHECK(icon.size.x == doctest::Approx(0.0f));
        CHECK(icon.size.y == doctest::Approx(0.0f));
        CHECK(icon.texture_id == static_cast<ImTextureID>(7));
    }
}


TEST_SUITE("Salix::gui::IconInfoBatch2047") {
    TEST_CASE("copy assignment transfers every descriptor field") {
        Salix::IconInfo source; source.texture_id = static_cast<ImTextureID>(123); source.path = "icons/source.png";
        source.size = ImVec2(21.0f, 34.0f); source.uv0 = ImVec2(0.2f, 0.3f); source.uv1 = ImVec2(0.7f, 0.9f);
        Salix::IconInfo destination; destination = source;
        CHECK(destination.texture_id == source.texture_id); CHECK(destination.path == source.path);
        CHECK(destination.size.x == doctest::Approx(21.0f)); CHECK(destination.size.y == doctest::Approx(34.0f));
        CHECK(destination.uv0.x == doctest::Approx(0.2f)); CHECK(destination.uv1.y == doctest::Approx(0.9f));
    }
    TEST_CASE("self assignment preserves complete icon descriptor") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(17); icon.path = "icons/self.png"; icon.size = ImVec2(3.0f, 4.0f);
        icon = icon; CHECK(icon.texture_id == static_cast<ImTextureID>(17)); CHECK(icon.path == "icons/self.png");
        CHECK(icon.size.x == doctest::Approx(3.0f)); CHECK(icon.size.y == doctest::Approx(4.0f));
    }
    TEST_CASE("icon path preserves embedded null bytes as owned string data") {
        Salix::IconInfo icon; icon.path = std::string("icon\0sheet.png", 14); CHECK(icon.path.size() == 14); CHECK(icon.path[4] == '\0');
    }
    TEST_CASE("icon path preserves UTF8 bytes without changing texture identity") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(88); icon.path = u8"icons/世界/icon.png";
        CHECK(icon.path == u8"icons/世界/icon.png"); CHECK(icon.texture_id == static_cast<ImTextureID>(88));
    }
    TEST_CASE("negative render dimensions remain representable without clamping") {
        Salix::IconInfo icon; icon.size = ImVec2(-16.0f, -32.0f);
        CHECK(icon.size.x == doctest::Approx(-16.0f)); CHECK(icon.size.y == doctest::Approx(-32.0f));
    }
    TEST_CASE("UV coordinates outside normalized range remain representable") {
        Salix::IconInfo icon; icon.uv0 = ImVec2(-0.5f, 1.5f); icon.uv1 = ImVec2(2.0f, -1.0f);
        CHECK(icon.uv0.x == doctest::Approx(-0.5f)); CHECK(icon.uv0.y == doctest::Approx(1.5f));
        CHECK(icon.uv1.x == doctest::Approx(2.0f)); CHECK(icon.uv1.y == doctest::Approx(-1.0f));
    }
    TEST_CASE("mutating lower UV bound does not implicitly alter upper bound") {
        Salix::IconInfo icon; icon.uv0 = ImVec2(0.4f, 0.6f);
        CHECK(icon.uv1.x == doctest::Approx(1.0f)); CHECK(icon.uv1.y == doctest::Approx(1.0f));
    }
    TEST_CASE("texture identity survives independent path size and UV edits") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(999); icon.path = "changed.png";
        icon.size = ImVec2(100.0f, 50.0f); icon.uv0 = ImVec2(0.1f, 0.2f); icon.uv1 = ImVec2(0.3f, 0.4f);
        CHECK(icon.texture_id == static_cast<ImTextureID>(999));
    }
    TEST_CASE("later source mutations do not affect an already copy assigned descriptor") {
        Salix::IconInfo source; source.path = "before.png"; source.size = ImVec2(10.0f, 20.0f);
        Salix::IconInfo destination; destination = source; source.path = "after.png"; source.size.x = 99.0f;
        CHECK(destination.path == "before.png"); CHECK(destination.size.x == doctest::Approx(10.0f)); CHECK(destination.size.y == doctest::Approx(20.0f));
    }
    TEST_CASE("zero texture handle may coexist with nondefault presentation metadata") {
        Salix::IconInfo icon; icon.texture_id = static_cast<ImTextureID>(0); icon.path = "metadata-only.png";
        icon.size = ImVec2(48.0f, 24.0f); icon.uv0 = ImVec2(0.25f, 0.25f);
        CHECK(icon.texture_id == static_cast<ImTextureID>(0)); CHECK(icon.path == "metadata-only.png");
        CHECK(icon.size.x == doctest::Approx(48.0f)); CHECK(icon.uv0.x == doctest::Approx(0.25f));
    }
}
