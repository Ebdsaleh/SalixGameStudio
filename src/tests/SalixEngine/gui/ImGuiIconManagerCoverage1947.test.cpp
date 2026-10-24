#include <doctest.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/IIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/Sprite2D.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <string>

namespace {
struct Batch1947IconFixture {
    MockIRenderer renderer;
    Salix::AssetManager assets;
    Salix::ImGuiIconManager icons;
    Batch1947IconFixture() { assets.initialize(&renderer); icons.initialize(&assets); }
    ~Batch1947IconFixture() { assets.shutdown(); }
};
}

TEST_SUITE("Salix::gui::ImGuiIconManagerCoverage1947") {
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
