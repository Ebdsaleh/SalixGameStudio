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

TEST_SUITE("Salix::gui::ImGuiIconManagerBatch2447") {
    TEST_CASE("batch2447 registry reference remains stable across successful icon updates") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto* registry = &fixture.icons.get_icon_registry();
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch2447/camera.png");
        CHECK(&fixture.icons.get_icon_registry() == registry);
        CHECK(registry->at("Camera").path == "Assets/Icons/Batch2447/camera.png");
    }

    TEST_CASE("batch2447 transform element lookup returns registered transform icon") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        Salix::Transform* transform = entity.get_transform();
        REQUIRE(transform != nullptr);
        const auto& icon = fixture.icons.get_icon_for_element(transform);
        CHECK(icon.path == "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_092.png");
    }

    TEST_CASE("batch2447 camera element lookup returns registered camera icon") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Camera camera;
        const auto& icon = fixture.icons.get_icon_for_element(&camera);
        CHECK(icon.path == "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_039.png");
    }

    TEST_CASE("batch2447 sprite element lookup uses registered Sprite2D icon without requiring texture") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Sprite2D sprite;
        const auto& icon = fixture.icons.get_icon_for_element(&sprite);
        CHECK(icon.path == "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_031.png");
    }

    TEST_CASE("batch2447 updating panel locked mapping replaces only that registry path") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto camera_before = fixture.icons.get_icon_by_name("Camera").path;
        fixture.icons.update_icon("Panel Locked", "Assets/Icons/Batch2447/locked.png");
        CHECK(fixture.icons.get_icon_by_name("Panel Locked").path == "Assets/Icons/Batch2447/locked.png");
        CHECK(fixture.icons.get_icon_by_name("Camera").path == camera_before);
    }

    TEST_CASE("batch2447 unknown icon update leaves registry size and contents unchanged") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto before = fixture.icons.get_icon_registry();
        fixture.icons.update_icon("NeverRegistered2447", "Assets/Icons/Batch2447/unused.png");
        CHECK(fixture.icons.get_icon_registry().size() == before.size());
        CHECK(fixture.icons.get_icon_registry().count("NeverRegistered2447") == 0);
        CHECK(fixture.icons.get_icon_by_name("Entity").path == before.at("Entity").path);
    }

    TEST_CASE("batch2447 failed default icon update preserves fallback value") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto before = fixture.icons.get_icon_by_name("Default");
        fixture.renderer.should_texture_load_fail = true;
        fixture.icons.update_icon("Default", "Assets/Icons/Batch2447/missing-default.png");
        const auto& fallback = fixture.icons.get_icon_by_name("Missing2447");
        CHECK(fixture.icons.get_icon_by_name("Default").path == before.path);
        CHECK(fallback.path == before.path);
        CHECK(fallback.texture_id == before.texture_id);
    }

    TEST_CASE("batch2447 null entity after default registration returns copied default fallback values") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto& result = fixture.icons.get_icon_for_entity(nullptr);
        const auto& registered = fixture.icons.get_icon_by_name("Default");
        CHECK(result.path == registered.path);
        CHECK(result.texture_id == registered.texture_id);
        CHECK(result.size.x == doctest::Approx(registered.size.x));
        CHECK(result.size.y == doctest::Approx(registered.size.y));
    }

    TEST_CASE("batch2447 rejected null reinitialization does not discard prior usable asset manager") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.initialize(nullptr);
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch2447/still-usable.png");
        CHECK(fixture.icons.get_icon_by_name("Camera").path == "Assets/Icons/Batch2447/still-usable.png");
    }

    TEST_CASE("batch2447 separate icon managers keep independent registries and updates") {
        Batch1947IconFixture first;
        Batch1947IconFixture second;
        first.icons.register_default_icons();
        second.icons.register_default_icons();
        first.icons.update_icon("Camera", "Assets/Icons/Batch2447/first-camera.png");
        CHECK(first.icons.get_icon_by_name("Camera").path == "Assets/Icons/Batch2447/first-camera.png");
        CHECK(second.icons.get_icon_by_name("Camera").path != "Assets/Icons/Batch2447/first-camera.png");
    }
}
TEST_SUITE("Salix::gui::ImGuiIconManagerBatch3447") {
    TEST_CASE("batch3447 successful Default update immediately changes unknown-name fallback") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-new.png");
        const auto& registered = fixture.icons.get_icon_by_name("Default");
        const auto& fallback = fixture.icons.get_icon_by_name("Missing3447");
        CHECK(registered.path == "Assets/Icons/Batch3447/default-new.png");
        CHECK(fallback.path == registered.path);
        CHECK(fallback.texture_id == registered.texture_id);
    }

    TEST_CASE("batch3447 null entity follows the latest successful Default update") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-entity.png");
        const auto& icon = fixture.icons.get_icon_for_entity(nullptr);
        CHECK(icon.path == "Assets/Icons/Batch3447/default-entity.png");
        CHECK(icon.texture_id == fixture.icons.get_icon_by_name("Default").texture_id);
    }

    TEST_CASE("batch3447 null element follows the latest successful Default update") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-element.png");
        const auto& icon = fixture.icons.get_icon_for_element(nullptr);
        CHECK(icon.path == "Assets/Icons/Batch3447/default-element.png");
        CHECK(icon.texture_id == fixture.icons.get_icon_by_name("Default").texture_id);
    }

    TEST_CASE("batch3447 unregistered element type follows the latest Default update") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-script.png");
        Salix::CppScript script;
        const auto& icon = fixture.icons.get_icon_for_element(&script);
        CHECK(icon.path == "Assets/Icons/Batch3447/default-script.png");
        CHECK(icon.texture_id == fixture.icons.get_icon_by_name("Default").texture_id);
    }

    TEST_CASE("batch3447 successful Default update preserves fallback icon size metadata") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto before = fixture.icons.get_icon_by_name("Default");
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-size.png");
        const auto& after = fixture.icons.get_icon_by_name("MissingSize3447");
        CHECK(after.size.x == doctest::Approx(before.size.x));
        CHECK(after.size.y == doctest::Approx(before.size.y));
        CHECK(after.path == "Assets/Icons/Batch3447/default-size.png");
    }

    TEST_CASE("batch3447 updating nondefault icon never changes current fallback mapping") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto fallback_before = fixture.icons.get_icon_by_name("Default");
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch3447/camera-only.png");
        const auto& fallback_after = fixture.icons.get_icon_by_name("MissingCamera3447");
        CHECK(fallback_after.path == fallback_before.path);
        CHECK(fallback_after.texture_id == fallback_before.texture_id);
        CHECK(fixture.icons.get_icon_by_name("Camera").path == "Assets/Icons/Batch3447/camera-only.png");
    }

    TEST_CASE("batch3447 two successful Default updates expose only the latest fallback") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-first.png");
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-second.png");
        CHECK(fixture.icons.get_icon_by_name("Default").path == "Assets/Icons/Batch3447/default-second.png");
        CHECK(fixture.icons.get_icon_by_name("MissingLatest3447").path == "Assets/Icons/Batch3447/default-second.png");
    }

    TEST_CASE("batch3447 default registration after override restores canonical fallback path") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-override.png");
        fixture.icons.register_default_icons();
        const std::string canonical = "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_153.png";
        CHECK(fixture.icons.get_icon_by_name("Default").path == canonical);
        CHECK(fixture.icons.get_icon_by_name("MissingCanonical3447").path == canonical);
    }

    TEST_CASE("batch3447 successful updates never grow the canonical registry") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        REQUIRE(fixture.icons.get_icon_registry().size() == 7);
        fixture.icons.update_icon("Default", "Assets/Icons/Batch3447/default-registry.png");
        fixture.icons.update_icon("Camera", "Assets/Icons/Batch3447/camera-registry.png");
        fixture.icons.update_icon("Transform", "Assets/Icons/Batch3447/transform-registry.png");
        CHECK(fixture.icons.get_icon_registry().size() == 7);
    }

    TEST_CASE("batch3447 separate managers keep independently updated Default fallbacks") {
        Batch1947IconFixture first;
        Batch1947IconFixture second;
        first.icons.register_default_icons();
        second.icons.register_default_icons();
        first.icons.update_icon("Default", "Assets/Icons/Batch3447/first-default.png");
        CHECK(first.icons.get_icon_by_name("Missing3447").path == "Assets/Icons/Batch3447/first-default.png");
        CHECK(second.icons.get_icon_by_name("Missing3447").path != "Assets/Icons/Batch3447/first-default.png");
    }
}
TEST_SUITE("Salix::gui::ImGuiIconManagerBatch3847") {
    TEST_CASE("batch3847 generic entity resolves canonical Entity icon after default registration") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        CHECK(icon.path == fixture.icons.get_icon_by_name("Entity").path);
        CHECK(icon.texture_id == fixture.icons.get_icon_by_name("Entity").texture_id);
    }

    TEST_CASE("batch3847 camera entity resolves canonical Camera icon after default registration") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        REQUIRE(entity.add_element<Salix::Camera>() != nullptr);
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        CHECK(icon.path == fixture.icons.get_icon_by_name("Camera").path);
        CHECK(icon.texture_id == fixture.icons.get_icon_by_name("Camera").texture_id);
    }

    TEST_CASE("batch3847 textured sprite entity exposes its runtime texture identity and portable path") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        auto* sprite = entity.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&fixture.assets, "Assets/Icons/Batch3847/runtime-sprite.png");
        REQUIRE(sprite->get_texture() != nullptr);
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        CHECK(icon.texture_id == sprite->get_texture()->get_imgui_texture_id());
        CHECK(icon.path == sprite->get_texture_path());
    }

    TEST_CASE("batch3847 textured sprite takes priority over Camera icon on same entity") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        Salix::Entity entity;
        REQUIRE(entity.add_element<Salix::Camera>() != nullptr);
        auto* sprite = entity.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&fixture.assets, "Assets/Icons/Batch3847/priority.png");
        REQUIRE(sprite->get_texture() != nullptr);
        const auto& icon = fixture.icons.get_icon_for_entity(&entity);
        CHECK(icon.texture_id == sprite->get_texture()->get_imgui_texture_id());
        CHECK(icon.path == "Assets/Icons/Batch3847/priority.png");
        CHECK(icon.path != fixture.icons.get_icon_by_name("Camera").path);
    }

    TEST_CASE("batch3847 separate managers retain independent dynamic sprite icon scratch values") {
        Batch1947IconFixture first;
        Batch1947IconFixture second;
        Salix::Entity first_entity;
        Salix::Entity second_entity;
        auto* first_sprite = first_entity.add_element<Salix::Sprite2D>();
        auto* second_sprite = second_entity.add_element<Salix::Sprite2D>();
        REQUIRE(first_sprite != nullptr);
        REQUIRE(second_sprite != nullptr);
        first_sprite->load_texture(&first.assets, "Assets/Icons/Batch3847/first.png");
        second_sprite->load_texture(&second.assets, "Assets/Icons/Batch3847/second.png");
        const auto& first_icon = first.icons.get_icon_for_entity(&first_entity);
        const std::string first_path = first_icon.path;
        const auto first_texture = first_icon.texture_id;
        const auto& second_icon = second.icons.get_icon_for_entity(&second_entity);
        CHECK(first_icon.path == first_path);
        CHECK(first_icon.texture_id == first_texture);
        CHECK(second_icon.path == "Assets/Icons/Batch3847/second.png");
        CHECK(first_icon.path != second_icon.path);
    }

    TEST_CASE("batch3847 dynamic sprite lookup never grows canonical icon registry") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto size_before = fixture.icons.get_icon_registry().size();
        Salix::Entity entity;
        auto* sprite = entity.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&fixture.assets, "Assets/Icons/Batch3847/no-registry-growth.png");
        REQUIRE(sprite->get_texture() != nullptr);
        fixture.icons.get_icon_for_entity(&entity);
        CHECK(fixture.icons.get_icon_registry().size() == size_before);
    }

    TEST_CASE("batch3847 updating existing icon with new path preserves registry cardinality") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto size_before = fixture.icons.get_icon_registry().size();
        fixture.icons.update_icon("Entity", "Assets/Icons/Batch3847/entity-new.png");
        CHECK(fixture.icons.get_icon_registry().size() == size_before);
        CHECK(fixture.icons.get_icon_by_name("Entity").path == "Assets/Icons/Batch3847/entity-new.png");
    }

    TEST_CASE("batch3847 failed nondefault update preserves both target and fallback icons") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        const auto entity_before = fixture.icons.get_icon_by_name("Entity");
        const auto default_before = fixture.icons.get_icon_by_name("Default");
        fixture.renderer.should_texture_load_fail = true;
        fixture.icons.update_icon("Entity", "Assets/Icons/Batch3847/missing.png");
        CHECK(fixture.icons.get_icon_by_name("Entity").path == entity_before.path);
        CHECK(fixture.icons.get_icon_by_name("Entity").texture_id == entity_before.texture_id);
        CHECK(fixture.icons.get_icon_by_name("Default").path == default_before.path);
        CHECK(fixture.icons.get_icon_by_name("Default").texture_id == default_before.texture_id);
    }

    TEST_CASE("batch3847 repeated default registration keeps canonical registry size and fallback path") {
        Batch1947IconFixture fixture;
        fixture.icons.register_default_icons();
        REQUIRE(fixture.icons.get_icon_registry().size() == 7);
        fixture.icons.register_default_icons();
        CHECK(fixture.icons.get_icon_registry().size() == 7);
        CHECK(fixture.icons.get_icon_by_name("Missing3847").path ==
            "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_153.png");
    }

    TEST_CASE("batch3847 fresh null initialized manager rejects unknown update without dereference") {
        Salix::ImGuiIconManager icons;
        icons.initialize(nullptr);
        CHECK_NOTHROW(icons.update_icon("Unknown3847", "unused3847.png"));
        CHECK(icons.get_icon_registry().empty());
        const auto& fallback = icons.get_icon_by_name("Missing3847");
        CHECK(fallback.texture_id == ImTextureID(0));
        CHECK(fallback.path.empty());
    }
}

