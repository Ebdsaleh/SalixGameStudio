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
